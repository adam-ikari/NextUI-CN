#include "button_component.h"
#include "../common/api.h"
#include <stdlib.h>
#include <string.h>

UIComponent* button_component_new(void) {
    UIComponent* component = ui_component_new(button_component_render, button_component_destroy);
    if (!component) return NULL;

    ButtonProps* props = (ButtonProps*)malloc(sizeof(ButtonProps));
    if (!props) {
        ui_component_free(component);
        return NULL;
    }

    props->label = NULL;
    props->hint = NULL;
    props->style = BUTTON_STYLE_PRIMARY;
    props->highlighted = false;
    props->color = THEME_COLOR1;

    component->props = props;
    return component;
}

void button_component_render(UIComponent* component, SDL_Surface* screen, void* props) {
    if (!component || !screen || !props) return;

    ButtonProps* button_props = (ButtonProps*)props;

    if (button_props->label && button_props->hint) {
        // Render button with hint (like B+Back, A+Open)
        GFX_blitButton(button_props->hint, button_props->label, screen, &button_props->rect);
    } else if (button_props->label) {
        // Render button with label only
        SDL_Surface* text = TTF_RenderUTF8_Blended(font.medium, button_props->label, 
            button_props->highlighted ? COLOR_LIGHT_TEXT : ALT_BUTTON_TEXT_COLOR);
        
        if (text) {
            // Draw button background
            uint32_t bg_color = button_props->highlighted ? button_props->color : THEME_COLOR1;
            GFX_blitAssetColor(ASSET_BUTTON, NULL, screen, &button_props->rect, bg_color);
            
            // Draw label
            SDL_Rect text_rect = {
                button_props->rect.x + (button_props->rect.w - text->w) / 2,
                button_props->rect.y + (button_props->rect.h - text->h) / 2,
                text->w,
                text->h
            };
            SDL_BlitSurface(text, NULL, screen, &text_rect);
            SDL_FreeSurface(text);
        }
    }
}

void button_component_destroy(UIComponent* component) {
    if (!component || !component->props) return;

    ButtonProps* props = (ButtonProps*)component->props;
    if (props->label) {
        free(props->label);
    }
    if (props->hint) {
        free(props->hint);
    }
    free(props);
    component->props = NULL;
}