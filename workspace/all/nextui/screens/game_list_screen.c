#include "game_list_screen.h"
#include "../common/api.h"
#include "../components/button_component.h"
#include <stdlib.h>
#include <string.h>

// Forward declaration for GetHDMI function
extern int GetHDMI(void);

GameListScreen* game_list_screen_new(UIState* state, SDL_Surface* screen, Array* entries) {
    GameListScreen* screen_data = (GameListScreen*)malloc(sizeof(GameListScreen));
    if (!screen_data) return NULL;

    screen_data->state = state;
    screen_data->screen = screen;
    screen_data->entries = entries;
    screen_data->selected = 0;
    screen_data->top_index = 0;
    screen_data->thumbnail = NULL;
    screen_data->background = NULL;

    // Create list component
    screen_data->list_component = list_component_new();
    if (!screen_data->list_component) {
        free(screen_data);
        return NULL;
    }

    // Create status component
    screen_data->status_component = status_component_new();
    if (!screen_data->status_component) {
        list_component_free(screen_data->list_component);
        free(screen_data);
        return NULL;
    }

    return screen_data;
}

void game_list_screen_render(void* screen_instance, SDL_Surface* surface) {
    if (!screen_instance || !surface) return;

    GameListScreen* screen_data = (GameListScreen*)screen_instance;

    // Clear screen
    SDL_FillRect(surface, NULL, RGB_BLACK);

    // Draw background if available
    if (screen_data->background) {
        SDL_BlitSurface(screen_data->background, NULL, surface, NULL);
    }

    // Draw status bar
    StatusProps status_props = {
        .battery_level = 80,
        .is_charging = false,
        .has_wifi = true,
        .time_string = "12:00",
        .rect = {10, 10, surface->w - 20, 30},
        .show_battery = true,
        .show_wifi = true,
        .show_time = true,
        .text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT)
    };
    screen_data->status_component->render(screen_data->status_component, surface, &status_props);

    // Draw game list
    if (screen_data->entries && screen_data->entries->count > 0) {
        int visible_items = (surface->h - SCALE1(PADDING * 2)) / SCALE1(PILL_SIZE);
        int start_index = screen_data->top_index;
        int end_index = start_index + visible_items;
        if (end_index > screen_data->entries->count) {
            end_index = screen_data->entries->count;
        }

        // Calculate available width for list items (considering thumbnail area)
        int ox = surface->w;
        int had_thumb = (screen_data->thumbnail != NULL);
        int max_w = (int)(surface->w - (surface->w * CFG_getGameArtWidth()));
        
        if (had_thumb) {
            ox = (int)(max_w) - SCALE1(BUTTON_MARGIN*5);
        }

        int available_width = MAX(0, (had_thumb ? ox + SCALE1(BUTTON_MARGIN) : surface->w - SCALE1(BUTTON_MARGIN)) - SCALE1(PADDING * 2));
        
        // Render each list item using original rendering logic
        for (int i = start_index; i < end_index; i++) {
            int j = i - start_index;
            int is_selected = (i == screen_data->selected);
            
            // Get entry data (placeholder - needs actual Entry structure access)
            void* entry_data = screen_data->entries->items[i];
            char* entry_name = "Game Name"; // Placeholder - should use Entry_label(entry_data)
            char* entry_unique = "Game Unique"; // Placeholder - should use entry->unique
            
            // Calculate text width and truncate if needed
            char display_name[256];
            int text_width = GFX_getTextWidth(font.large, entry_unique ? entry_unique : entry_name, 
                                              display_name, available_width, SCALE1(BUTTON_PADDING * 2));
            int max_width = MIN(available_width, text_width);
            
            // Calculate pill rectangle
            SDL_Rect item_rect = {
                SCALE1(BUTTON_MARGIN),
                SCALE1(PADDING + (j * PILL_SIZE)),
                max_width,
                SCALE1(PILL_SIZE)
            };

            // Render pill background
            if (is_selected) {
                // Selected item uses dark pill
                GFX_blitPillDark(ASSET_WHITE_PILL, surface, &item_rect);
            } else {
                // Non-selected item uses light pill
                GFX_blitPillLight(ASSET_WHITE_PILL, surface, &item_rect);
            }

            // Render text
            SDL_Color text_color = uintToColour(THEME_COLOR4_255);
            if (is_selected) {
                text_color = uintToColour(THEME_COLOR5_255);
            }

            SDL_Surface* text = TTF_RenderUTF8_Blended(font.large, entry_name, text_color);
            if (text) {
                const int text_offset_y = (SCALE1(PILL_SIZE) - text->h + 1) >> 1 + SCALE1(TEXT_Y_OFFSET);
                SDL_Rect text_rect = { 0, 0, max_width - SCALE1(BUTTON_PADDING*2), text->h };
                SDL_Rect dest_rect = { 
                    item_rect.x + SCALE1(BUTTON_PADDING), 
                    item_rect.y + text_offset_y 
                };
                
                SDL_BlitSurface(text, &text_rect, surface, &dest_rect);
                SDL_FreeSurface(text);
            }
        }
    }

    // Draw thumbnail if available
    if (screen_data->thumbnail) {
        int max_w = (int)(surface->w * CFG_getGameArtWidth());
        int max_h = (int)(surface->h * 0.6);
        int img_w = screen_data->thumbnail->w;
        int img_h = screen_data->thumbnail->h;
        double aspect_ratio = (double)img_h / img_w;
        int new_w = max_w;
        int new_h = (int)(new_w * aspect_ratio);
        
        if (new_h > max_h) {
            new_h = max_h;
            new_w = (int)(new_h / aspect_ratio);
        }

        int target_x = surface->w - (new_w + SCALE1(BUTTON_MARGIN*3));
        int target_y = (int)(surface->h * 0.50);
        int center_y = target_y - (new_h / 2);
        
        // Apply rounded corners if configured
        if (CFG_getThumbnailRadius() > 0) {
            GFX_ApplyRoundedCorners_RGBA8888(
                screen_data->thumbnail,
                &(SDL_Rect){0, 0, screen_data->thumbnail->w, screen_data->thumbnail->h},
                SCALE1((float)CFG_getThumbnailRadius() * ((float)img_w / (float)new_w))
            );
        }
        
        // Scale and blit thumbnail
        SDL_Rect thumb_rect = {target_x, center_y, new_w, new_h};
        GFX_blitScaleToFill(screen_data->thumbnail, surface);
    }

    // Draw button groups (bottom controls)
    if (screen_data->entries && screen_data->entries->count > 0) {
        // Check if we should show hardware hints or resume button
        // Placeholder logic - needs actual implementation
        bool can_resume = false; // Should check if game has save state
        int show_setting = 0;    // Should check current setting state
        
        if (show_setting && !GetHDMI()) {
            GFX_blitHardwareHints(surface, show_setting);
        } else if (can_resume) {
            GFX_blitButtonGroup((char*[]){"X", "Resume", NULL}, 0, surface, 0);
        } else {
            GFX_blitButtonGroup((char*[]){
                (char*)(BTN_SLEEP==BTN_POWER?"Power":"Menu"),
                (char*)(BTN_SLEEP==BTN_POWER?"Sleep":"Info"),
                NULL
            }, 0, surface, 0);
        }
        
        // Show back/open buttons
        if (screen_data->entries->count > 0) {
            GFX_blitButtonGroup((char*[]){"B", "Back", "A", "Open", NULL}, 1, surface, 1);
        }
    } else {
        // Show empty folder message
        GFX_blitMessage(font.large, (char*)"Empty Folder", surface, &(SDL_Rect){0,0,surface->w,surface->h});
    }
}

void game_list_screen_handle_input(void* screen_instance, int input) {
    if (!screen_instance) return;

    GameListScreen* screen_data = (GameListScreen*)screen_instance;

    switch (input) {
        case BTN_UP:
            if (screen_data->selected > 0) {
                screen_data->selected--;
                if (screen_data->selected < screen_data->top_index) {
                    screen_data->top_index = screen_data->selected;
                }
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_DOWN:
            if (screen_data->entries && screen_data->selected < screen_data->entries->count - 1) {
                screen_data->selected++;
                int visible_items = (screen_data->screen->h - 100) / SCALE1(PILL_SIZE);
                if (screen_data->selected >= screen_data->top_index + visible_items) {
                    screen_data->top_index = screen_data->selected - visible_items + 1;
                }
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_A:
            // Open selected game
            break;
        case BTN_B:
            // Go back
            ui_state_set_screen(screen_data->state, SCREEN_OFF);
            break;
        case BTN_MENU:
            // Open quick menu
            ui_state_set_screen(screen_data->state, SCREEN_QUICKMENU);
            break;
    }
}

void game_list_screen_update(void* screen_instance) {
    // Update logic if needed (e.g., loading thumbnails, updating time)
}

void game_list_screen_destroy(void* screen_instance) {
    if (!screen_instance) return;

    GameListScreen* screen_data = (GameListScreen*)screen_instance;

    if (screen_data->list_component) {
        list_component_free(screen_data->list_component);
    }

    if (screen_data->status_component) {
        status_component_free(screen_data->status_component);
    }

    if (screen_data->thumbnail) {
        SDL_FreeSurface(screen_data->thumbnail);
    }

    if (screen_data->background) {
        SDL_FreeSurface(screen_data->background);
    }

    free(screen_data);
}

ScreenModule* game_list_screen_module_new(UIState* state, SDL_Surface* screen, Array* entries) {
    GameListScreen* screen_data = game_list_screen_new(state, screen, entries);
    if (!screen_data) return NULL;

    return screen_module_new(
        game_list_screen_render,
        game_list_screen_handle_input,
        game_list_screen_destroy,
        game_list_screen_update,
        screen_data
    );
}