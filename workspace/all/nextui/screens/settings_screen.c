#include "settings_screen.h"
#include "../common/api.h"
#include "../../i18n/i18n.h"
#include <stdlib.h>
#include <string.h>

SettingsScreen* settings_screen_new(UIState* state, SDL_Surface* screen) {
    SettingsScreen* screen_data = (SettingsScreen*)malloc(sizeof(SettingsScreen));
    if (!screen_data) return NULL;

    screen_data->state = state;
    screen_data->screen = screen;
    screen_data->selected = 0;
    screen_data->setting_count = 5;

    // Initialize setting names
    screen_data->setting_names = (char**)malloc(sizeof(char*) * screen_data->setting_count);
    if (!screen_data->setting_names) {
        free(screen_data);
        return NULL;
    }

    screen_data->setting_names[0] = strdup(TR("common.brightness"));
    screen_data->setting_names[1] = strdup(TR("common.volume"));
    screen_data->setting_names[2] = strdup(TR("common.language"));
    screen_data->setting_names[3] = strdup(TR("common.cheats"));
    screen_data->setting_names[4] = strdup(TR("common.reset"));

    // Initialize setting values (placeholder)
    screen_data->setting_values = (void**)malloc(sizeof(void*) * screen_data->setting_count);
    if (!screen_data->setting_values) {
        for (int i = 0; i < screen_data->setting_count; i++) {
            free(screen_data->setting_names[i]);
        }
        free(screen_data->setting_names);
        free(screen_data);
        return NULL;
    }

    // These would be actual setting values in production
    screen_data->setting_values[0] = (void*)(long)80; // brightness
    screen_data->setting_values[1] = (void*)(long)50; // volume
    screen_data->setting_values[2] = (void*)"English"; // language
    screen_data->setting_values[3] = (void*)0; // cheats enabled
    screen_data->setting_values[4] = NULL; // reset

    return screen_data;
}

void settings_screen_render(void* screen_instance, SDL_Surface* surface) {
    if (!screen_instance || !surface) return;

    SettingsScreen* screen_data = (SettingsScreen*)screen_instance;

    // Clear screen
    SDL_FillRect(surface, NULL, RGB_BLACK);

    // Draw title
    SDL_Surface* title = TTF_RenderUTF8_Blended(font.large, TR("common.settings"), COLOR_LIGHT_TEXT);
    if (title) {
        SDL_Rect title_rect = {
            surface->w / 2 - title->w / 2,
            20,
            title->w,
            title->h
        };
        SDL_BlitSurface(title, NULL, surface, &title_rect);
        SDL_FreeSurface(title);
    }

    // Draw settings items
    int item_height = SCALE1(PILL_SIZE);
    int start_y = 80;

    for (int i = 0; i < screen_data->setting_count; i++) {
        SDL_Rect item_rect = {
            20,
            start_y + i * (item_height + 10),
            surface->w - 40,
            item_height
        };

        bool is_selected = (i == screen_data->selected);
        uint32_t bg_color = is_selected ? THEME_COLOR1 : RGB_WHITE;

        GFX_blitPillColor(ASSET_WHITE_PILL, surface, &item_rect, bg_color, bg_color);

        // Draw setting name
        SDL_Surface* name = TTF_RenderUTF8_Blended(font.large, screen_data->setting_names[i], COLOR_LIGHT_TEXT);
        if (name) {
            SDL_Rect name_rect = {
                item_rect.x + 20,
                item_rect.y + (item_rect.h - name->h) / 2,
                name->w,
                name->h
            };
            SDL_BlitSurface(name, NULL, surface, &name_rect);
            SDL_FreeSurface(name);
        }

        // Draw setting value
        if (screen_data->setting_values[i]) {
            char value_str[64];
            if (i == 0) { // brightness
                snprintf(value_str, sizeof(value_str), "%d%%", (int)(long)screen_data->setting_values[i]);
            } else if (i == 1) { // volume
                snprintf(value_str, sizeof(value_str), "%d%%", (int)(long)screen_data->setting_values[i]);
            } else if (i == 2) { // language
                snprintf(value_str, sizeof(value_str), "%s", (char*)screen_data->setting_values[i]);
            } else if (i == 3) { // cheats
                snprintf(value_str, sizeof(value_str), "%s", (long)screen_data->setting_values[i] ? "On" : "Off");
            } else {
                value_str[0] = '\0';
            }

            if (value_str[0]) {
                SDL_Surface* value = TTF_RenderUTF8_Blended(font.large, value_str, COLOR_LIGHT_TEXT);
                if (value) {
                    SDL_Rect value_rect = {
                        item_rect.x + item_rect.w - value->w - 20,
                        item_rect.y + (item_rect.h - value->h) / 2,
                        value->w,
                        value->h
                    };
                    SDL_BlitSurface(value, NULL, surface, &value_rect);
                    SDL_FreeSurface(value);
                }
            }
        }
    }

    // Draw navigation hints
    SDL_Surface* hint = TTF_RenderUTF8_Blended(font.medium, "D-Pad: Select | A: Change | B: Back", COLOR_LIGHT_TEXT);
    if (hint) {
        SDL_Rect hint_rect = {
            surface->w / 2 - hint->w / 2,
            surface->h - hint->h - 20,
            hint->w,
            hint->h
        };
        SDL_BlitSurface(hint, NULL, surface, &hint_rect);
        SDL_FreeSurface(hint);
    }
}

void settings_screen_handle_input(void* screen_instance, int input) {
    if (!screen_instance) return;

    SettingsScreen* screen_data = (SettingsScreen*)screen_instance;

    switch (input) {
        case BTN_UP:
            if (screen_data->selected > 0) {
                screen_data->selected--;
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_DOWN:
            if (screen_data->selected < screen_data->setting_count - 1) {
                screen_data->selected++;
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_LEFT:
            // Decrease setting value
            if (screen_data->selected == 0) { // brightness
                int brightness = (int)(long)screen_data->setting_values[0];
                if (brightness > 0) {
                    screen_data->setting_values[0] = (void*)(long)(brightness - 10);
                }
            } else if (screen_data->selected == 1) { // volume
                int volume = (int)(long)screen_data->setting_values[1];
                if (volume > 0) {
                    screen_data->setting_values[1] = (void*)(long)(volume - 10);
                }
            }
            break;
        case BTN_RIGHT:
            // Increase setting value
            if (screen_data->selected == 0) { // brightness
                int brightness = (int)(long)screen_data->setting_values[0];
                if (brightness < 100) {
                    screen_data->setting_values[0] = (void*)(long)(brightness + 10);
                }
            } else if (screen_data->selected == 1) { // volume
                int volume = (int)(long)screen_data->setting_values[1];
                if (volume < 100) {
                    screen_data->setting_values[1] = (void*)(long)(volume + 10);
                }
            }
            break;
        case BTN_A:
            // Toggle or change setting
            if (screen_data->selected == 3) { // cheats
                bool cheats_enabled = (bool)(long)screen_data->setting_values[3];
                screen_data->setting_values[3] = (void*)(long)(!cheats_enabled);
            } else if (screen_data->selected == 4) { // reset
                // Reset all settings to default
                screen_data->setting_values[0] = (void*)(long)80;
                screen_data->setting_values[1] = (void*)(long)50;
                screen_data->setting_values[2] = (void*)"English";
                screen_data->setting_values[3] = (void*)0;
            }
            break;
        case BTN_B:
            // Back to game list
            ui_state_set_screen(screen_data->state, SCREEN_GAMELIST);
            break;
        case BTN_MENU:
            // Back to game list
            ui_state_set_screen(screen_data->state, SCREEN_GAMELIST);
            break;
    }
}

void settings_screen_update(void* screen_instance) {
    // Update logic if needed
}

void settings_screen_destroy(void* screen_instance) {
    if (!screen_instance) return;

    SettingsScreen* screen_data = (SettingsScreen*)screen_instance;

    if (screen_data->setting_names) {
        for (int i = 0; i < screen_data->setting_count; i++) {
            free(screen_data->setting_names[i]);
        }
        free(screen_data->setting_names);
    }

    if (screen_data->setting_values) {
        free(screen_data->setting_values);
    }

    free(screen_data);
}

ScreenModule* settings_screen_module_new(UIState* state, SDL_Surface* screen) {
    SettingsScreen* screen_data = settings_screen_new(state, screen);
    if (!screen_data) return NULL;

    return screen_module_new(
        settings_screen_render,
        settings_screen_handle_input,
        settings_screen_destroy,
        settings_screen_update,
        screen_data
    );
}