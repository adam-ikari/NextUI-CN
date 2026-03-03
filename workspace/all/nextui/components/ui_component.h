#ifndef __UI_COMPONENT_H__
#define __UI_COMPONENT_H__

#include <stdbool.h>
#include "sdl.h"

typedef struct UIComponent UIComponent;

typedef void (*ComponentRenderFunc)(UIComponent* self, SDL_Surface* screen, void* props);
typedef void (*ComponentDestroyFunc)(UIComponent* self);

typedef struct UIComponent {
    ComponentRenderFunc render;
    ComponentDestroyFunc destroy;
    void* props;
    void* internal_data;
} UIComponent;

UIComponent* ui_component_new(ComponentRenderFunc render, ComponentDestroyFunc destroy);
void ui_component_free(UIComponent* component);

#endif // __UI_COMPONENT_H__