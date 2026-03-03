#ifndef __STATUS_COMPONENT_H__
#define __STATUS_COMPONENT_H__

#include "ui_component.h"
#include "sdl.h"

typedef struct {
    int battery_level;
    bool is_charging;
    bool has_wifi;
    char* time_string;
    SDL_Rect rect;
    bool show_battery;
    bool show_wifi;
    bool show_time;
    uint32_t text_color;
} StatusProps;

UIComponent* status_component_new(void);
void status_component_render(UIComponent* component, SDL_Surface* screen, void* props);
void status_component_destroy(UIComponent* component);

#endif // __STATUS_COMPONENT_H__