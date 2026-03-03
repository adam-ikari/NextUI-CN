#include "quick_menu_screen.h"
#include "../common/api.h"
#include "../common/i18n.h"
#include <stdlib.h>
#include <string.h>

QuickMenuScreen* quick_menu_screen_new(UIState* state, SDL_Surface* screen) {
    QuickMenuScreen* screen_data = (QuickMenuScreen*)malloc(sizeof(QuickMenuScreen));
    if (!screen_data) return NULL;

    screen_data->state = state;
    screen_data->screen = screen;
    screen_data->selected = 0;
    screen_data->menu_item_count = 4;

    // Initialize menu items
    screen_data->menu_items = (char**)malloc(sizeof(char*) * screen_data->menu_item_count);
    if (!screen_data->menu_items) {
        free(screen_data);
        return NULL;
    }

    screen_data->menu_items[0] = strdup(TR("common.resume"));
    screen_data->menu_items[1] = strdup(TR("common.save"));
    screen_data->menu_items[2] = strdup(TR("common.load"));
    screen_data->menu_items[3] = strdup(TR("common.quit"));

    // Create list component
    screen_data->list_component = list_component_new();
    if (!screen_data->list_component) {
        for (int i = 0; i < screen_data->menu_item_count; i++) {
            free(screen_data->menu_items[i]);
        }
        free(screen_data->menu_items);
        free(screen_data);
        return NULL;
    }

    return screen_data;
}

void quick_menu_screen_render(void* screen_instance, SDL_Surface* surface) {
    if (!screen_instance || !surface) return;

    QuickMenuScreen* screen_data = (QuickMenuScreen*)screen_instance;

    // Draw semi-transparent overlay
    SDL_Surface* overlay = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0);
    if (overlay) {
        SDL_FillRect(overlay, NULL, SDL_MapRGBA(overlay->format, 0, 0, 0, 200));
        SDL_BlitSurface(overlay, NULL, surface, NULL);
        SDL_FreeSurface(overlay);
    }

    // Draw menu background
    SDL_Rect menu_rect = {
        surface->w / 2 - 150,
        surface->h / 2 - 100,
        300,
        200
    };
    GFX_blitRoundedRect(surface, &menu_rect, THEME_BG_COLOR, SCALE1(10));

    // Draw menu items
    ListProps list_props = {
        .rect = {menu_rect.x + 10, menu_rect.y + 10, menu_rect.w - 20, menu_rect.h - 20},
        .items = NULL,
        .count = screen_data->menu_item_count,
        .selected = screen_data->selected,
        .columns = 1,
        .item_size = 45,
        .style = LIST_STYLE_VERTICAL,
        .show_thumbnails = false,
        .selected_color = THEME_COLOR1,
        .normal_color = RGB_WHITE,
        .text_color = COLOR_LIGHT_TEXT,
        .selected_text_color = COLOR_LIGHT_TEXT
    };

    // Create temporary item array for rendering
    ListItem* items = (ListItem*)malloc(sizeof(ListItem) * screen_data->menu_item_count);
    if (items) {
        for (int i = 0; i < screen_data->menu_item_count; i++) {
            items[i].label = screen_data->menu_items[i];
            items[i].icon_path = NULL;
            items[i].thumbnail = NULL;
            items[i].user_data = NULL;
        }
        list_props.items = items;
        screen_data->list_component->render(screen_data->list_component, surface, &list_props);
        free(items);
    }

    // Draw title
    SDL_Surface* title = TTF_RenderUTF8_Blended(font.large, "Quick Menu", COLOR_LIGHT_TEXT);
    if (title) {
        SDL_Rect title_rect = {
            menu_rect.x + (menu_rect.w - title->w) / 2,
            menu_rect.y - title->h - 10,
            title->w,
            title->h
        };
        SDL_BlitSurface(title, NULL, surface, &title_rect);
        SDL_FreeSurface(title);
    }
}

void quick_menu_screen_handle_input(void* screen_instance, int input) {
    if (!screen_instance) return;

    QuickMenuScreen* screen_data = (QuickMenuScreen*)screen_instance;

    switch (input) {
        case BTN_UP:
            if (screen_data->selected > 0) {
                screen_data->selected--;
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_DOWN:
            if (screen_data->selected < screen_data->menu_item_count - 1) {
                screen_data->selected++;
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_A:
            // Handle menu item selection
            switch (screen_data->selected) {
                case 0: // Resume
                    ui_state_set_screen(screen_data->state, screen_data->state->last_screen);
                    break;
                case 3: // Quit
                    ui_state_set_screen(screen_data->state, SCREEN_OFF);
                    break;
            }
            break;
        case BTN_B:
            // Close menu
            ui_state_set_screen(screen_data->state, screen_data->state->last_screen);
            break;
        case BTN_MENU:
            // Close menu
            ui_state_set_screen(screen_data->state, screen_data->state->last_screen);
            break;
    }
}

void quick_menu_screen_update(void* screen_instance) {
    // Update logic if needed
}

void quick_menu_screen_destroy(void* screen_instance) {
    if (!screen_instance) return;

    QuickMenuScreen* screen_data = (QuickMenuScreen*)screen_instance;

    if (screen_data->menu_items) {
        for (int i = 0; i < screen_data->menu_item_count; i++) {
            free(screen_data->menu_items[i]);
        }
        free(screen_data->menu_items);
    }

    if (screen_data->list_component) {
        list_component_free(screen_data->list_component);
    }

    free(screen_data);
}

ScreenModule* quick_menu_screen_module_new(UIState* state, SDL_Surface* screen) {
    QuickMenuScreen* screen_data = quick_menu_screen_new(state, screen);
    if (!screen_data) return NULL;

    return screen_module_new(
        quick_menu_screen_render,
        quick_menu_screen_handle_input,
        quick_menu_screen_destroy,
        quick_menu_screen_update,
        screen_data
    );
}