#include "renderer.h"
#include <stdlib.h>

Renderer* renderer_new(SDL_Surface* screen, UIState* state) {
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    if (!renderer) return NULL;

    renderer->screen = screen;
    renderer->state = state;
    renderer->current_screen = NULL;
    renderer->components = Array_new();

    return renderer;
}

void renderer_free(Renderer* renderer) {
    if (!renderer) return;

    if (renderer->current_screen) {
        if (renderer->current_screen->destroy) {
            renderer->current_screen->destroy(renderer->current_screen->instance);
        }
        free(renderer->current_screen);
    }

    if (renderer->components) {
        for (int i = 0; i < renderer->components->count; i++) {
            UIComponent* component = (UIComponent*)renderer->components->items[i];
            ui_component_free(component);
        }
        Array_free(renderer->components);
    }

    free(renderer);
}

void renderer_add_component(Renderer* renderer, UIComponent* component) {
    if (!renderer || !component) return;
    Array_push(renderer->components, component);
}

void renderer_set_screen(Renderer* renderer, ScreenModule* screen_module) {
    if (!renderer) return;

    if (renderer->current_screen) {
        if (renderer->current_screen->destroy) {
            renderer->current_screen->destroy(renderer->current_screen->instance);
        }
        free(renderer->current_screen);
    }

    renderer->current_screen = screen_module;
}

void renderer_render(Renderer* renderer) {
    if (!renderer || !renderer->current_screen) return;

    if (!renderer->state->dirty) return;

    if (renderer->current_screen->render) {
        renderer->current_screen->render(renderer->current_screen->instance, renderer->screen);
    }

    ui_state_clear_dirty(renderer->state);
}