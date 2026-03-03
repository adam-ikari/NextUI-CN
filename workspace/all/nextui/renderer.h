#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "sdl.h"
#include "array.h"
#include "state/ui_state.h"
#include "components/ui_component.h"
#include "screens/screen.h"

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