#include "game_list_screen.h"
#include "../common/api.h"
#include "../components/button_component.h"
#include <stdlib.h>
#include <string.h>

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
        int visible_items = (surface->h - 100) / SCALE1(PILL_SIZE);
        int start_index = screen_data->top_index;
        int end_index = start_index + visible_items;
        if (end_index > screen_data->entries->count) {
            end_index = screen_data->entries->count;
        }

        ListProps list_props = {
            .rect = {20, 80, surface->w - 40, surface->h - 100},
            .items = NULL,
            .count = end_index - start_index,
            .selected = screen_data->selected - start_index,
            .columns = 1,
            .item_size = SCALE1(PILL_SIZE),
            .style = LIST_STYLE_VERTICAL,
            .show_thumbnails = false,
            .selected_color = THEME_COLOR1,
            .normal_color = RGB_WHITE,
            .text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT),
            .selected_text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT)
        };

        // Create temporary item array for rendering
        ListItem* items = (ListItem*)malloc(sizeof(ListItem) * list_props.count);
        if (items) {
            for (int i = 0; i < list_props.count; i++) {
                int entry_index = start_index + i;
                if (entry_index < screen_data->entries->count) {
                    // Assuming entry is a struct with label field
                    // This needs to be adapted based on actual Entry structure
                    items[i].label = "Game"; // Placeholder
                    items[i].icon_path = NULL;
                    items[i].thumbnail = NULL;
                    items[i].user_data = screen_data->entries->items[entry_index];
                }
            }
            list_props.items = items;
            screen_data->list_component->render(screen_data->list_component, surface, &list_props);
            free(items);
        }
    }

    // Draw thumbnail if available
    if (screen_data->thumbnail) {
        SDL_Rect thumb_rect = {
            surface->w - screen_data->thumbnail->w - 20,
            80,
            screen_data->thumbnail->w,
            screen_data->thumbnail->h
        };
        SDL_BlitSurface(screen_data->thumbnail, NULL, surface, &thumb_rect);
    }

    // Draw buttons
    if (screen_data->entries && screen_data->entries->count > 0) {
        ButtonProps button_props = {
            .label = "Open",
            .hint = NULL,
            .rect = {surface->w - 100, surface->h - 50, 80, 40},
            .style = BUTTON_STYLE_PRIMARY,
            .highlighted = true,
            .color = THEME_COLOR1
        };
        // Note: This is a placeholder - actual button rendering needs component integration
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