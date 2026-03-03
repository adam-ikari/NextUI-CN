#include "ui_component.h"
#include <stdlib.h>

UIComponent* ui_component_new(ComponentRenderFunc render, ComponentDestroyFunc destroy) {
    UIComponent* component = (UIComponent*)malloc(sizeof(UIComponent));
    if (!component) return NULL;

    component->render = render;
    component->destroy = destroy;
    component->props = NULL;
    component->internal_data = NULL;

    return component;
}

void ui_component_free(UIComponent* component) {
    if (!component) return;

    if (component->destroy) {
        component->destroy(component);
    }

    free(component);
}