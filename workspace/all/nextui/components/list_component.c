#include "list_component.h"
#include "../common/api.h"
#include <stdlib.h>
#include <string.h>

UIComponent* list_component_new(void) {
    UIComponent* component = ui_component_new(list_component_render, list_component_destroy);
    if (!component) return NULL;

    ListProps* props = (ListProps*)malloc(sizeof(ListProps));
    if (!props) {
        ui_component_free(component);
        return NULL;
    }

    props->items = NULL;
    props->count = 0;
    props->selected = 0;
    props->columns = 1;
    props->item_size = SCALE1(PILL_SIZE);
    props->style = LIST_STYLE_VERTICAL;
    props->show_thumbnails = false;
    props->selected_color = THEME_COLOR1;
    props->normal_color = RGB_WHITE;
    props->text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT);
    props->selected_text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT);

    component->props = props;
    return component;
}

void list_component_render(UIComponent* component, SDL_Surface* screen, void* props) {
    if (!component || !screen || !props) return;

    ListProps* list_props = (ListProps*)props;

    if (list_props->count == 0) return;

    if (list_props->style == LIST_STYLE_VERTICAL) {
        // Render vertical list
        for (int i = 0; i < list_props->count; i++) {
            int row = i / list_props->columns;
            int col = i % list_props->columns;

            SDL_Rect item_rect = {
                list_props->rect.x + col * list_props->item_size,
                list_props->rect.y + row * list_props->item_size,
                list_props->item_size,
                list_props->item_size
            };

            // Skip items that are outside the visible area
            if (item_rect.y + item_rect.h <= 0 || item_rect.y >= screen->h) {
                continue;
            }

            bool is_selected = (i == list_props->selected);
            ListItem* item = &list_props->items[i];

            // Render item background (pill)
            if (is_selected) {
                GFX_blitPillColor(ASSET_WHITE_PILL, screen, &item_rect, 
                                  list_props->selected_color, list_props->selected_color);
            } else {
                GFX_blitPillLight(ASSET_WHITE_PILL, screen, &item_rect);
            }

            // Render thumbnail if available
            if (list_props->show_thumbnails && item->thumbnail) {
                SDL_Rect thumb_rect = {
                    item_rect.x + (item_rect.w - item->thumbnail->w) / 2,
                    item_rect.y + (item_rect.h - item->thumbnail->h) / 2,
                    item->thumbnail->w,
                    item->thumbnail->h
                };
                SDL_BlitSurface(item->thumbnail, NULL, screen, &thumb_rect);
            }

            // Render label
            if (item->label) {
                uint32_t text_color = is_selected ? list_props->selected_text_color : list_props->text_color;
                SDL_Surface* text_surf = TTF_RenderUTF8_Blended(font.large, item->label, text_color);
                if (text_surf) {
                    SDL_Rect text_rect = {
                        item_rect.x + (item_rect.w - text_surf->w) / 2,
                        item_rect.y + (item_rect.h - text_surf->h) / 2,
                        text_surf->w,
                        text_surf->h
                    };
                    SDL_BlitSurface(text_surf, NULL, screen, &text_rect);
                    SDL_FreeSurface(text_surf);
                }
            }
        }
    } else if (list_props->style == LIST_STYLE_GRID) {
        // Render grid list
        int items_per_row = (list_props->rect.w + SCALE1(BUTTON_MARGIN)) / (list_props->item_size + SCALE1(BUTTON_MARGIN));
        if (items_per_row < 1) items_per_row = 1;

        for (int i = 0; i < list_props->count; i++) {
            int row = i / items_per_row;
            int col = i % items_per_row;

            SDL_Rect item_rect = {
                list_props->rect.x + col * (list_props->item_size + SCALE1(BUTTON_MARGIN)),
                list_props->rect.y + row * (list_props->item_size + SCALE1(BUTTON_MARGIN)),
                list_props->item_size,
                list_props->item_size
            };

            // Skip items that are outside the visible area
            if (item_rect.y + item_rect.h <= 0 || item_rect.y >= screen->h) {
                continue;
            }

            bool is_selected = (i == list_props->selected);
            ListItem* item = &list_props->items[i];

            // Render item background (rounded rectangle)
            if (is_selected) {
                GFX_blitRoundedRect(screen, &item_rect, list_props->selected_color, SCALE1(10));
            } else {
                GFX_blitRoundedRect(screen, &item_rect, list_props->normal_color, SCALE1(10));
            }

            // Render thumbnail if available
            if (list_props->show_thumbnails && item->thumbnail) {
                SDL_Rect thumb_rect = {
                    item_rect.x + (item_rect.w - item->thumbnail->w) / 2,
                    item_rect.y + (item_rect.h - item->thumbnail->h) / 2,
                    item->thumbnail->w,
                    item->thumbnail->h
                };
                SDL_BlitSurface(item->thumbnail, NULL, screen, &thumb_rect);
            }

            // Render label
            if (item->label) {
                uint32_t text_color = is_selected ? list_props->selected_text_color : list_props->text_color;
                SDL_Surface* text_surf = TTF_RenderUTF8_Blended(font.small, item->label, text_color);
                if (text_surf) {
                    SDL_Rect text_rect = {
                        item_rect.x + (item_rect.w - text_surf->w) / 2,
                        item_rect.y + item_rect.h - text_surf->h - SCALE1(5),
                        text_surf->w,
                        text_surf->h
                    };
                    SDL_BlitSurface(text_surf, NULL, screen, &text_rect);
                    SDL_FreeSurface(text_surf);
                }
            }
        }
    }
}

void list_component_destroy(UIComponent* component) {
    if (!component || !component->props) return;

    ListProps* props = (ListProps*)component->props;
    if (props->items) {
        for (int i = 0; i < props->count; i++) {
            if (props->items[i].label) {
                free(props->items[i].label);
            }
            if (props->items[i].icon_path) {
                free(props->items[i].icon_path);
            }
            if (props->items[i].thumbnail) {
                SDL_FreeSurface(props->items[i].thumbnail);
            }
        }
        free(props->items);
    }
    free(props);
    component->props = NULL;
}

void list_component_free(UIComponent* component) {
    if (!component) return;
    list_component_destroy(component);
    ui_component_free(component);
}