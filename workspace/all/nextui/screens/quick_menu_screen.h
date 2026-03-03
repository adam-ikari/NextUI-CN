#ifndef __QUICK_MENU_SCREEN_H__
#define __QUICK_MENU_SCREEN_H__

#include "screen.h"
#include "../state/ui_state.h"
#include "../components/list_component.h"
#include "sdl.h"

typedef struct {
    UIState* state;
    SDL_Surface* screen;
    int selected;
    int menu_item_count;
    char** menu_items;
    UIComponent* list_component;
} QuickMenuScreen;

QuickMenuScreen* quick_menu_screen_new(UIState* state, SDL_Surface* screen);
void quick_menu_screen_render(void* screen_instance, SDL_Surface* surface);
void quick_menu_screen_handle_input(void* screen_instance, int input);
void quick_menu_screen_update(void* screen_instance);
void quick_menu_screen_destroy(void* screen_instance);

ScreenModule* quick_menu_screen_module_new(UIState* state, SDL_Surface* screen);

#endif // __QUICK_MENU_SCREEN_H__