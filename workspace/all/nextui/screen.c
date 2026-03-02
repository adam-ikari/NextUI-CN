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

    // Render button hints registered by the screen
    screen_render_hints(mgr->current_screen, surface);

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

void screen_register_hint(screen* scr, int position, const char* button, const char* text) {
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
    
    strncpy(scr->hints[position][count].button, button, sizeof(scr->hints[position][count].button) - 1);
    strncpy(scr->hints[position][count].text, text, sizeof(scr->hints[position][count].text) - 1);
    scr->hints[position][count].button[sizeof(scr->hints[position][count].button) - 1] = '\0';
    scr->hints[position][count].text[sizeof(scr->hints[position][count].text) - 1] = '\0';
    
    scr->hint_counts[position]++;
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

// Screen Manager additional API

void screen_manager_set_show_setting(screen_manager* mgr, int show_setting) {
    if (mgr) {
        mgr->show_setting = show_setting;
    }
}

int screen_manager_get_show_setting(screen_manager* mgr) {
    return mgr ? mgr->show_setting : 0;
}