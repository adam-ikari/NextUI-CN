#include "screen.h"
#include <stdlib.h>

ScreenModule* screen_module_new(
    ScreenRenderFunc render,
    ScreenHandleInputFunc handle_input,
    ScreenDestroyFunc destroy,
    ScreenUpdateFunc update,
    void* instance
) {
    ScreenModule* module = (ScreenModule*)malloc(sizeof(ScreenModule));
    if (!module) return NULL;

    module->render = render;
    module->handle_input = handle_input;
    module->destroy = destroy;
    module->update = update;
    module->instance = instance;

    return module;
}

void screen_module_free(ScreenModule* module) {
    if (!module) return;

    if (module->destroy && module->instance) {
        module->destroy(module->instance);
    }

    free(module);
}