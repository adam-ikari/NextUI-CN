#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "../state/ui_state.h"
#include "sdl.h"

typedef void (*ScreenRenderFunc)(void* screen_instance, SDL_Surface* surface);
typedef void (*ScreenHandleInputFunc)(void* screen_instance, int input);
typedef void (*ScreenDestroyFunc)(void* screen_instance);
typedef void (*ScreenUpdateFunc)(void* screen_instance);

typedef struct ScreenModule {
    ScreenRenderFunc render;
    ScreenHandleInputFunc handle_input;
    ScreenDestroyFunc destroy;
    ScreenUpdateFunc update;
    void* instance;
} ScreenModule;

ScreenModule* screen_module_new(
    ScreenRenderFunc render,
    ScreenHandleInputFunc handle_input,
    ScreenDestroyFunc destroy,
    ScreenUpdateFunc update,
    void* instance
);

void screen_module_free(ScreenModule* module);

#endif // __SCREEN_H__