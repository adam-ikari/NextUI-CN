#ifndef __BUTTON_COMPONENT_H__
#define __BUTTON_COMPONENT_H__

#include "ui_component.h"
#include "sdl.h"

typedef enum {
    BUTTON_STYLE_PRIMARY,
    BUTTON_STYLE_SECONDARY
} ButtonStyle;

typedef struct {
    char* label;
    char* hint;
    SDL_Rect rect;
    ButtonStyle style;
    bool highlighted;
    uint32_t color;
} ButtonProps;

UIComponent* button_component_new(void);
void button_component_render(UIComponent* component, SDL_Surface* screen, void* props);
void button_component_destroy(UIComponent* component);

#endif // __BUTTON_COMPONENT_H__