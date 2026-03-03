#ifndef __PILL_COMPONENT_H__
#define __PILL_COMPONENT_H__

#include "ui_component.h"
#include "sdl.h"

typedef enum {
    PILL_STYLE_LIGHT,
    PILL_STYLE_DARK,
    PILL_STYLE_COLOR
} PillStyle;

typedef struct {
    SDL_Rect rect;
    uint32_t color;
    uint32_t fill_color;
    char* text;
    PillStyle style;
} PillProps;

UIComponent* pill_component_new(void);
void pill_component_render(UIComponent* component, SDL_Surface* screen, void* props);
void pill_component_destroy(UIComponent* component);

#endif // __PILL_COMPONENT_H__