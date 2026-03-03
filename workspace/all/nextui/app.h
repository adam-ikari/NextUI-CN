#ifndef __APP_H__
#define __APP_H__

#include "sdl.h"
#include "state/ui_state.h"
#include "renderer.h"
#include "screens/screen.h"

typedef struct {
    UIState* state;
    Renderer* renderer;
    SDL_Surface* screen;
    Array* entries;
    Array* recent_games;
    bool running;
} NextUIApp;

NextUIApp* nextui_app_new(SDL_Surface* screen);
void nextui_app_free(NextUIApp* app);

void nextui_app_init(NextUIApp* app);
void nextui_app_run(NextUIApp* app);
void nextui_app_handle_input(NextUIApp* app, int input);
void nextui_app_update(NextUIApp* app);
void nextui_app_render(NextUIApp* app);

void nextui_app_set_screen(NextUIApp* app, ScreenType screen_type);

#endif // __APP_H__