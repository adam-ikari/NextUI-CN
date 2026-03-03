#include "game_switcher_screen.h"
#include "../common/api.h"
#include <stdlib.h>

GameSwitcherScreen* game_switcher_screen_new(UIState* state, SDL_Surface* screen, Array* recent_games) {
    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)malloc(sizeof(GameSwitcherScreen));
    if (!screen_data) return NULL;

    screen_data->state = state;
    screen_data->screen = screen;
    screen_data->selected = 0;
    screen_data->recent_games = recent_games;
    screen_data->recent_game_count = recent_games ? recent_games->count : 0;

    return screen_data;
}

void game_switcher_screen_render(void* screen_instance, SDL_Surface* surface) {
    if (!screen_instance || !surface) return;

    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)screen_instance;

    // Draw semi-transparent overlay
    SDL_Surface* overlay = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0);
    if (overlay) {
        SDL_FillRect(overlay, NULL, SDL_MapRGBA(overlay->format, 0, 0, 0, 180));
        SDL_BlitSurface(overlay, NULL, surface, NULL);
        SDL_FreeSurface(overlay);
    }

    // Draw title
    SDL_Surface* title = TTF_RenderUTF8_Blended(font.large, "Recent Games", COLOR_LIGHT_TEXT);
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

    // Draw recent games (placeholder - needs actual implementation)
    if (screen_data->recent_games && screen_data->recent_games->count > 0) {
        int item_width = 150;
        int item_height = 150;
        int items_per_row = (surface->w - 40) / (item_width + 10);
        if (items_per_row < 1) items_per_row = 1;

        for (int i = 0; i < screen_data->recent_games->count; i++) {
            int row = i / items_per_row;
            int col = i % items_per_row;

            SDL_Rect item_rect = {
                20 + col * (item_width + 10),
                80 + row * (item_height + 10),
                item_width,
                item_height
            };

            bool is_selected = (i == screen_data->selected);
            uint32_t bg_color = is_selected ? THEME_COLOR1 : RGB_WHITE;

            GFX_blitRoundedRect(surface, &item_rect, bg_color, SCALE1(10));

            // Draw game name (placeholder)
            char* game_name = (char*)screen_data->recent_games->items[i];
            if (game_name) {
                SDL_Surface* text = TTF_RenderUTF8_Blended(font.medium, game_name, COLOR_LIGHT_TEXT);
                if (text) {
                    SDL_Rect text_rect = {
                        item_rect.x + (item_rect.w - text->w) / 2,
                        item_rect.y + item_rect.h - text->h - 10,
                        text->w,
                        text->h
                    };
                    SDL_BlitSurface(text, NULL, surface, &text_rect);
                    SDL_FreeSurface(text);
                }
            }
        }
    } else {
        // No recent games message
        SDL_Surface* msg = TTF_RenderUTF8_Blended(font.medium, "No recent games", COLOR_LIGHT_TEXT);
        if (msg) {
            SDL_Rect msg_rect = {
                surface->w / 2 - msg->w / 2,
                surface->h / 2 - msg->h / 2,
                msg->w,
                msg->h
            };
            SDL_BlitSurface(msg, NULL, surface, &msg_rect);
            SDL_FreeSurface(msg);
        }
    }
}

void game_switcher_screen_handle_input(void* screen_instance, int input) {
    if (!screen_instance) return;

    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)screen_instance;

    switch (input) {
        case BTN_UP:
        case BTN_LEFT:
            if (screen_data->selected > 0) {
                screen_data->selected--;
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_DOWN:
        case BTN_RIGHT:
            if (screen_data->selected < screen_data->recent_game_count - 1) {
                screen_data->selected++;
                ui_state_set_selected(screen_data->state, screen_data->selected);
            }
            break;
        case BTN_A:
            // Open selected game
            ui_state_set_screen(screen_data->state, SCREEN_GAME);
            break;
        case BTN_B:
        case BTN_MENU:
            // Close game switcher
            ui_state_set_game_switcher(screen_data->state, false);
            ui_state_set_screen(screen_data->state, screen_data->state->last_screen);
            break;
    }
}

void game_switcher_screen_update(void* screen_instance) {
    // Update logic if needed
}

void game_switcher_screen_destroy(void* screen_instance) {
    if (!screen_instance) return;

    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)screen_instance;
    free(screen_data);
}

ScreenModule* game_switcher_screen_module_new(UIState* state, SDL_Surface* screen, Array* recent_games) {
    GameSwitcherScreen* screen_data = game_switcher_screen_new(state, screen, recent_games);
    if (!screen_data) return NULL;

    return screen_module_new(
        game_switcher_screen_render,
        game_switcher_screen_handle_input,
        game_switcher_screen_destroy,
        game_switcher_screen_update,
        screen_data
    );
}