#ifndef __LIST_COMPONENT_H__
#define __LIST_COMPONENT_H__

#include "ui_component.h"
#include "sdl.h"

typedef enum {
    LIST_STYLE_GRID,
    LIST_STYLE_VERTICAL
} ListStyle;

typedef struct {
    char* label;
    char* icon_path;
    SDL_Surface* thumbnail;
    void* user_data;
} ListItem;

typedef struct {
    SDL_Rect rect;
    ListItem* items;
    int count;
    int selected;
    int columns;
    int item_size;
    ListStyle style;
    bool show_thumbnails;
    uint32_t selected_color;
    uint32_t normal_color;
    uint32_t text_color;
    uint32_t selected_text_color;
} ListProps;

typedef struct {
    UIComponent base;
    ListProps props;
} ListComponent;

UIComponent* list_component_new(void);
void list_component_render(UIComponent* component, SDL_Surface* screen, void* props);
void list_component_destroy(UIComponent* component);
void list_component_free(UIComponent* component);

#endif // __LIST_COMPONENT_H__