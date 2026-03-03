#ifndef __GAME_SWITCHER_SCREEN_H__
#define __GAME_SWITCHER_SCREEN_H__

#include "screen.h"
#include "../state/ui_state.h"
#include "sdl.h"

typedef struct {
    UIState* state;
    SDL_Surface* screen;
    int selected;
    int recent_game_count;
    Array* recent_games;
} GameSwitcherScreen;

GameSwitcherScreen* game_switcher_screen_new(UIState* state, SDL_Surface* screen, Array* recent_games);
void game_switcher_screen_render(void* screen_instance, SDL_Surface* surface);
void game_switcher_screen_handle_input(void* screen_instance, int input);
void game_switcher_screen_update(void* screen_instance);
void game_switcher_screen_destroy(void* screen_instance);

ScreenModule* game_switcher_screen_module_new(UIState* state, SDL_Surface* screen, Array* recent_games);

#endif // __GAME_SWITCHER_SCREEN_H__