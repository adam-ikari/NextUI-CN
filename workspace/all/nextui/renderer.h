#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "sdl.h"
#include "state/ui_state.h"
#include "components/ui_component.h"

typedef void (*ScreenRenderFunc)(void* screen_instance, SDL_Surface* surface);
typedef void (*ScreenHandleInputFunc)(void* screen_instance, int input);
typedef void (*ScreenDestroyFunc)(void* screen_instance);

typedef struct ScreenModule {
    ScreenRenderFunc render;
    ScreenHandleInputFunc handle_input;
    ScreenDestroyFunc destroy;
    void* instance;
} ScreenModule;

typedef struct Renderer {
    SDL_Surface* screen;
    UIState* state;
    ScreenModule* current_screen;
    Array* components;
} Renderer;

Renderer* renderer_new(SDL_Surface* screen, UIState* state);
void renderer_free(Renderer* renderer);

void renderer_add_component(Renderer* renderer, UIComponent* component);
void renderer_set_screen(Renderer* renderer, ScreenModule* screen_module);
void renderer_render(Renderer* renderer);

#endif // __RENDERER_H__