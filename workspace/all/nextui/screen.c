#include "screen.h"
#include <stdlib.h>
#include <string.h>

// Screen Manager implementation

screen_manager* screen_manager_new(void) {
    screen_manager* mgr = (screen_manager*)malloc(sizeof(screen_manager));
    if (!mgr) {
        LOG_error("Failed to allocate screen_manager\n");
        return NULL;
    }

    memset(mgr, 0, sizeof(screen_manager));
    mgr->dirty = 1;
    return mgr;
}

void screen_manager_free(screen_manager* mgr) {
    if (!mgr) return;

    if (mgr->current_screen) {
        screen_free(mgr->current_screen);
        mgr->current_screen = NULL;
    }

    if (mgr->last_screen) {
        screen_free(mgr->last_screen);
        mgr->last_screen = NULL;
    }

    if (mgr->last_screen_surface) {
        SDL_FreeSurface(mgr->last_screen_surface);
        mgr->last_screen_surface = NULL;
    }

    free(mgr);
}

void screen_manager_push(screen_manager* mgr, screen* screen) {
    if (!mgr || !screen) return;

    // Call on_exit on current screen
    if (mgr->current_screen && mgr->current_screen->vtable && mgr->current_screen->vtable->on_exit) {
        mgr->current_screen->vtable->on_exit(mgr->current_screen);
    }

    // Store current as last, but avoid double-free if they're the same
    if (mgr->last_screen && mgr->last_screen != mgr->current_screen) {
        screen_free(mgr->last_screen);
    }
    mgr->last_screen = mgr->current_screen;

    // Set new current screen
    mgr->current_screen = screen;

    // Call on_enter on new screen
    if (screen->vtable && screen->vtable->on_enter) {
        screen->vtable->on_enter(screen);
    }

    mgr->dirty = 1;
}

void screen_manager_pop(screen_manager* mgr) {
    if (!mgr || !mgr->current_screen) return;

    // Call on_exit on current screen
    if (mgr->current_screen->vtable && mgr->current_screen->vtable->on_exit) {
        mgr->current_screen->vtable->on_exit(mgr->current_screen);
    }

    // Swap screens
    screen* old_current = mgr->current_screen;
    mgr->current_screen = mgr->last_screen;
    mgr->last_screen = old_current;

    // Free the old current screen (now in last_screen)
    if (mgr->last_screen) {
        screen_free(mgr->last_screen);
        mgr->last_screen = NULL;
    }

    // Call on_enter on restored screen
    if (mgr->current_screen && mgr->current_screen->vtable && mgr->current_screen->vtable->on_enter) {
        mgr->current_screen->vtable->on_enter(mgr->current_screen);
    }

    mgr->dirty = 1;
}

void screen_manager_replace(screen_manager* mgr, screen* screen) {
    if (!mgr || !screen) return;

    // Call on_exit on current screen
    if (mgr->current_screen && mgr->current_screen->vtable && mgr->current_screen->vtable->on_exit) {
        mgr->current_screen->vtable->on_exit(mgr->current_screen);
    }

    // Free current screen
    if (mgr->current_screen) {
        screen_free(mgr->current_screen);
    }

    // Set new current screen
    mgr->current_screen = screen;

    // Call on_enter on new screen
    if (screen->vtable && screen->vtable->on_enter) {
        screen->vtable->on_enter(screen);
    }

    mgr->dirty = 1;
}

void screen_manager_update(screen_manager* mgr, unsigned long now) {
    if (!mgr || !mgr->current_screen || !mgr->current_screen->vtable) return;

    if (mgr->current_screen->vtable->update) {
        mgr->current_screen->vtable->update(mgr->current_screen, now);
    }
}

void screen_manager_render(screen_manager* mgr, SDL_Surface* surface) {
    if (!mgr || !mgr->current_screen || !mgr->current_screen->vtable) return;

    // Render screen content
    if (mgr->current_screen->vtable->render) {
        mgr->current_screen->vtable->render(mgr->current_screen, surface);
    }

    // Render status pill (battery, wifi, bluetooth, clock)
    if (mgr->show_setting) {
        if (!GetHDMI()) {
            GFX_blitHardwareHints(surface, mgr->show_setting);
        }
    } else {
        GFX_blitHardwareGroup(surface, 0);
    }

    // Merge default hints with screen hints for rendering
    screen* scr = mgr->current_screen;
    for (int pos = 0; pos < HINT_POSITION_COUNT; pos++) {
        // Check if screen has hints for this position
        if (scr->hint_counts[pos] == 0) {
            // No screen hints, use default hints
            for (int i = 0; i < mgr->default_hint_counts[pos]; i++) {
                screen_register_hint(scr, pos, 
                    mgr->default_hints[pos][i].button,
                    mgr->default_hints[pos][i].text,
                    HINT_MODE_APPEND);
            }
        }
    }

    // Render button hints (merged default + screen)
    screen_render_hints(scr, surface);
    
    // Clear screen hints to avoid accumulation on next frame
    screen_clear_hints(scr);

    mgr->dirty = 0;
}

int screen_manager_is_dirty(screen_manager* mgr) {
    return mgr ? mgr->dirty : 0;
}

void screen_manager_set_dirty(screen_manager* mgr, int dirty) {
    if (mgr) {
        mgr->dirty = dirty;
    }
}

void screen_manager_set_animation_direction(screen_manager* mgr, int direction) {
    if (mgr) {
        mgr->animation_direction = direction;
    }
}

screen* screen_manager_get_current(screen_manager* mgr) {
    return mgr ? mgr->current_screen : NULL;
}

screen* screen_manager_get_last(screen_manager* mgr) {
    return mgr ? mgr->last_screen : NULL;
}

// Screen base implementation

screen* screen_new(ScreenType type, screen_vtable* vtable, void* data) {
    if (!vtable) {
        LOG_error("screen_new: vtable is NULL\n");
        return NULL;
    }

    screen* scr = (screen*)malloc(sizeof(screen));
    if (!scr) {
        LOG_error("Failed to allocate screen\n");
        return NULL;
    }

    scr->type = type;
    scr->vtable = vtable;
    scr->data = data;
    
    // Initialize hints
    memset(scr->hints, 0, sizeof(scr->hints));
    memset(scr->hint_counts, 0, sizeof(scr->hint_counts));
    
    return scr;
}

void screen_free(screen* scr) {
    if (!scr) return;

    // Call cleanup function to free screen data
    if (scr->vtable && scr->vtable->cleanup) {
        scr->vtable->cleanup(scr);
    }

    free(scr);
}

// Button hint API

void screen_clear_hints(screen* scr) {
    if (!scr) return;
    memset(scr->hint_counts, 0, sizeof(scr->hint_counts));
}

void screen_register_hint(screen* scr, int position, const char* button, const char* text, int mode) {
    if (!scr || !button || !text) return;
    
    if (position < 0 || position >= HINT_POSITION_COUNT) {
        LOG_error("screen_register_hint: invalid position %d\n", position);
        return;
    }
    
    int count = scr->hint_counts[position];
    if (count >= MAX_HINTS_PER_POSITION) {
        LOG_error("screen_register_hint: too many hints at position %d\n", position);
        return;
    }
    
    int insert_pos = count;
    
    // Handle different modes
    if (mode == HINT_MODE_APPEND_TOP) {
        // Shift existing hints down
        if (count >= MAX_HINTS_PER_POSITION) {
            count = MAX_HINTS_PER_POSITION - 1;
        }
        for (int i = count; i > 0; i--) {
            memcpy(&scr->hints[position][i], &scr->hints[position][i-1], sizeof(button_hint));
        }
        insert_pos = 0;
        scr->hint_counts[position]++;
    } else {
        scr->hint_counts[position]++;
    }
    
    strncpy(scr->hints[position][insert_pos].button, button, sizeof(scr->hints[position][insert_pos].button) - 1);
    strncpy(scr->hints[position][insert_pos].text, text, sizeof(scr->hints[position][insert_pos].text) - 1);
    scr->hints[position][insert_pos].button[sizeof(scr->hints[position][insert_pos].button) - 1] = '\0';
    scr->hints[position][insert_pos].text[sizeof(scr->hints[position][insert_pos].text) - 1] = '\0';
    scr->hints[position][insert_pos].is_default = 0;
}

void screen_render_hints(screen* scr, SDL_Surface* surface) {
    if (!scr || !surface) return;
    
    for (int pos = 0; pos < HINT_POSITION_COUNT; pos++) {
        int count = scr->hint_counts[pos];
        if (count == 0) continue;
        
        // Build hint arrays for GFX_blitButtonGroup
        char* hint_pairs[(MAX_HINTS_PER_POSITION * 2) + 1];
        int hint_idx = 0;
        
        for (int i = 0; i < count; i++) {
            hint_pairs[hint_idx++] = scr->hints[pos][i].button;
            hint_pairs[hint_idx++] = scr->hints[pos][i].text;
        }
        hint_pairs[hint_idx] = NULL;
        
        // Render hints (position 0 = top/primary, position 1 = bottom/secondary)
        int align_right = (pos == HINT_POSITION_PRIMARY) ? 0 : 1;
        GFX_blitButtonGroup(hint_pairs, align_right, surface, pos);
    }
}

// Default hint API

void screen_manager_register_default_hint(screen_manager* mgr, int position, const char* button, const char* text) {
    if (!mgr || !button || !text) return;
    
    if (position < 0 || position >= HINT_POSITION_COUNT) {
        LOG_error("screen_manager_register_default_hint: invalid position %d\n", position);
        return;
    }
    
    int count = mgr->default_hint_counts[position];
    if (count >= MAX_HINTS_PER_POSITION) {
        LOG_error("screen_manager_register_default_hint: too many default hints at position %d\n", position);
        return;
    }
    
    strncpy(mgr->default_hints[position][count].button, button, sizeof(mgr->default_hints[position][count].button) - 1);
    strncpy(mgr->default_hints[position][count].text, text, sizeof(mgr->default_hints[position][count].text) - 1);
    mgr->default_hints[position][count].button[sizeof(mgr->default_hints[position][count].button) - 1] = '\0';
    mgr->default_hints[position][count].text[sizeof(mgr->default_hints[position][count].text) - 1] = '\0';
    mgr->default_hints[position][count].is_default = 1;
    
    mgr->default_hint_counts[position]++;
}

void screen_manager_clear_default_hints(screen_manager* mgr) {
    if (!mgr) return;
    memset(mgr->default_hint_counts, 0, sizeof(mgr->default_hint_counts));
}

// Screen Manager additional API

void screen_manager_set_show_setting(screen_manager* mgr, int show_setting) {
    if (mgr) {
        mgr->show_setting = show_setting;
    }
}

int screen_manager_get_show_setting(screen_manager* mgr) {
    return mgr ? mgr->show_setting : 0;
}