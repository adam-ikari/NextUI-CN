#ifndef __SETTINGS_SCREEN_H__
#define __SETTINGS_SCREEN_H__

#include "screen.h"
#include "../state/ui_state.h"
#include "sdl.h"

typedef struct {
    UIState* state;
    SDL_Surface* screen;
    int selected;
    int setting_count;
    char** setting_names;
    void** setting_values;
} SettingsScreen;

SettingsScreen* settings_screen_new(UIState* state, SDL_Surface* screen);
void settings_screen_render(void* screen_instance, SDL_Surface* surface);
void settings_screen_handle_input(void* screen_instance, int input);
void settings_screen_update(void* screen_instance);
void settings_screen_destroy(void* screen_instance);

ScreenModule* settings_screen_module_new(UIState* state, SDL_Surface* screen);

#endif // __SETTINGS_SCREEN_H__