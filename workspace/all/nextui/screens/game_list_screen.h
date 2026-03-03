#ifndef __GAME_LIST_SCREEN_H__
#define __GAME_LIST_SCREEN_H__

#include "screen.h"
#include "../state/ui_state.h"
#include "../components/list_component.h"
#include "../components/status_component.h"
#include "sdl.h"

typedef struct {
    UIState* state;
    SDL_Surface* screen;
    Array* entries;
    int selected;
    int top_index;
    ListComponent* list_component;
    StatusComponent* status_component;
    SDL_Surface* thumbnail;
    SDL_Surface* background;
} GameListScreen;

GameListScreen* game_list_screen_new(UIState* state, SDL_Surface* screen, Array* entries);
void game_list_screen_render(void* screen_instance, SDL_Surface* surface);
void game_list_screen_handle_input(void* screen_instance, int input);
void game_list_screen_update(void* screen_instance);
void game_list_screen_destroy(void* screen_instance);

ScreenModule* game_list_screen_module_new(UIState* state, SDL_Surface* screen, Array* entries);

#endif // __GAME_LIST_SCREEN_H__