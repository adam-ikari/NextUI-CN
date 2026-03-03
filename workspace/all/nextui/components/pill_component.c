#include "pill_component.h"
#include "../common/api.h"
#include <stdlib.h>
#include <string.h>

UIComponent* pill_component_new(void) {
    UIComponent* component = ui_component_new(pill_component_render, pill_component_destroy);
    if (!component) return NULL;

    PillProps* props = (PillProps*)malloc(sizeof(PillProps));
    if (!props) {
        ui_component_free(component);
        return NULL;
    }

    props->text = NULL;
    props->color = RGB_WHITE;
    props->fill_color = RGB_WHITE;
    props->style = PILL_STYLE_LIGHT;

    component->props = props;
    return component;
}

void pill_component_render(UIComponent* component, SDL_Surface* screen, void* props) {
    if (!component || !screen || !props) return;

    PillProps* pill_props = (PillProps*)props;

    switch (pill_props->style) {
        case PILL_STYLE_LIGHT:
            GFX_blitPillLight(ASSET_WHITE_PILL, screen, &pill_props->rect);
            break;
        case PILL_STYLE_DARK:
            GFX_blitPillDark(ASSET_WHITE_PILL, screen, &pill_props->rect);
            break;
        case PILL_STYLE_COLOR:
            GFX_blitPillColor(ASSET_WHITE_PILL, screen, &pill_props->rect, pill_props->color, pill_props->fill_color);
            break;
    }

    if (pill_props->text) {
        SDL_Surface* text_surf = TTF_RenderUTF8_Blended(font.large, pill_props->text, COLOR_LIGHT_TEXT);
        if (text_surf) {
            SDL_Rect text_rect = {
                pill_props->rect.x + (pill_props->rect.w - text_surf->w) / 2,
                pill_props->rect.y + (pill_props->rect.h - text_surf->h) / 2,
                text_surf->w,
                text_surf->h
            };
            SDL_BlitSurface(text_surf, NULL, screen, &text_rect);
            SDL_FreeSurface(text_surf);
        }
    }
}

void pill_component_destroy(UIComponent* component) {
    if (!component || !component->props) return;

    PillProps* props = (PillProps*)component->props;
    if (props->text) {
        free(props->text);
    }
    free(props);
    component->props = NULL;
}