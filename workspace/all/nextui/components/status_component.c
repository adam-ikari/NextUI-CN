#include "status_component.h"
#include "../common/api.h"
#include <stdlib.h>
#include <string.h>

UIComponent* status_component_new(void) {
    UIComponent* component = ui_component_new(status_component_render, status_component_destroy);
    if (!component) return NULL;

    StatusProps* props = (StatusProps*)malloc(sizeof(StatusProps));
    if (!props) {
        ui_component_free(component);
        return NULL;
    }

    props->battery_level = 100;
    props->is_charging = false;
    props->has_wifi = false;
    props->time_string = NULL;
    props->show_battery = true;
    props->show_wifi = true;
    props->show_time = true;
    props->text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT);

    component->props = props;
    return component;
}

void status_component_render(UIComponent* component, SDL_Surface* screen, void* props) {
    if (!component || !screen || !props) return;

    StatusProps* status_props = (StatusProps*)props;
    int x = status_props->rect.x;
    int y = status_props->rect.y;
    int gap = SCALE1(10);

    // Render battery indicator
    if (status_props->show_battery) {
        // Note: GFX_blitBattery only takes 2 parameters, using a simplified approach
        // In a full implementation, we would need to render battery based on level and charging state
        SDL_Rect battery_rect = {x, 10, SCALE1(40), SCALE1(20)};
        GFX_blitBattery(screen, &battery_rect);
        x += SCALE1(50) + gap;
    }

    // Render WiFi indicator
    if (status_props->show_wifi) {
        if (status_props->has_wifi) {
            GFX_blitWiFi(screen, true);
        }
        x += SCALE1(30) + gap;
    }

    // Render time
    if (status_props->show_time && status_props->time_string) {
        SDL_Surface* text_surf = TTF_RenderUTF8_Blended(font.large, status_props->time_string, status_props->text_color);
        if (text_surf) {
            SDL_Rect text_rect = {
                x,
                y + (status_props->rect.h - text_surf->h) / 2,
                text_surf->w,
                text_surf->h
            };
            SDL_BlitSurface(text_surf, NULL, screen, &text_rect);
            SDL_FreeSurface(text_surf);
        }
    }
}

void status_component_destroy(UIComponent* component) {
    if (!component || !component->props) return;

    StatusProps* props = (StatusProps*)component->props;
    if (props->time_string) {
        free(props->time_string);
    }
    free(props);
    component->props = NULL;
}

void status_component_free(UIComponent* component) {
    if (!component) return;
    status_component_destroy(component);
    ui_component_free(component);
}