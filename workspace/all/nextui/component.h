#ifndef NEXTUI_COMPONENT_H__
#define NEXTUI_COMPONENT_H__

#include <SDL2/SDL.h>
#include "common/config.h"
#include "common/api.h"

// Component types
typedef enum {
    COMPONENT_TYPE_BUTTON,
    COMPONENT_TYPE_PILL,
    COMPONENT_TYPE_ICON,
    COMPONENT_TYPE_THUMBNAIL,
    COMPONENT_TYPE_BACKGROUND,
    COMPONENT_TYPE_TEXT,
    COMPONENT_TYPE_STATUS_PILL,
    COMPONENT_TYPE_COUNT
} ComponentType;

// Forward declarations
typedef struct component component;
typedef struct component_factory component_factory;

// Component state
typedef struct {
    SDL_Surface* surface;
    SDL_Rect rect;
    SDL_Color color;
    int visible;
    int enabled;
    int hovered;
    int pressed;
} component_state;

// Component style
typedef struct {
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Color border_color;
    int border_width;
    int corner_radius;
    int shadow_enabled;
    SDL_Color shadow_color;
    int padding;
} component_style;

// Component lifecycle
typedef struct component_vtable {
    void (*cleanup)(component* comp);
    void (*update)(component* comp, unsigned long now);
    void (*render)(component* comp, SDL_Surface* surface);
    void (*handle_event)(component* comp, int button, int pressed);
    void (*set_enabled)(component* comp, int enabled);
} component_vtable;

// Button component
typedef struct {
    char text[128];
    char icon_path[MAX_PATH];
    int primary;  // Is primary button
} button_component_data;

// Pill component (rounded rectangle)
typedef struct {
    int width;
    int height;
    char text[128];
    SDL_Surface* icon;
} pill_component_data;

// Thumbnail component
typedef struct {
    char image_path[MAX_PATH];
    char label[128];
    char label_secondary[128];
    int selected;
} thumbnail_component_data;

// Base component structure
struct component {
    ComponentType type;
    component_vtable* vtable;
    component_style style;
    component_state state;
    void* data;  // Component-specific data
    void* user_data;  // User-defined data
};

// Component Factory
struct component_factory {
    // Pre-loaded common assets
    SDL_Surface* default_icon;
    SDL_Surface* default_background;
    
    // Style presets
    component_style styles[3];  // PRIMARY, SECONDARY, TERTIARY
};

// Component Factory API
component_factory* component_factory_new(void);
void component_factory_free(component_factory* factory);
void component_factory_set_style(component_factory* factory, int style_index, component_style* style);
component_style* component_factory_get_style(component_factory* factory, int style_index);

// Component API
component* component_new(ComponentType type, component_vtable* vtable, void* data);
void component_free(component* comp);
void component_set_position(component* comp, int x, int y, int width, int height);
void component_set_text(component* comp, const char* text);
void component_set_visible(component* comp, int visible);
void component_set_enabled(component* comp, int enabled);
void component_set_style(component* comp, component_style* style);
int component_is_hovered(component* comp);
int component_is_pressed(component* comp);

// Component renderers
void component_render_button(component* comp, SDL_Surface* surface);
void component_render_pill(component* comp, SDL_Surface* surface);
void component_render_thumbnail(component* comp, SDL_Surface* surface);
void component_render_background(component* comp, SDL_Surface* surface);
void component_render_status_pill(component* factory, SDL_Surface* surface, int show_setting);

// Button component API
component* button_component_new(const char* text, const char* icon_path, int primary);
void button_component_set_text(component* btn, const char* text);
void button_component_set_icon(component* btn, const char* icon_path);

// Pill component API
component* pill_component_new(const char* text, int width, int height, SDL_Surface* icon);
void pill_component_set_text(component* pill, const char* text);

// Thumbnail component API
component* thumbnail_component_new(const char* image_path, const char* label, const char* label_secondary);
void thumbnail_component_set_selected(component* thumb, int selected);
void thumbnail_component_set_label(component* thumb, const char* label);

// List component API
typedef struct {
    component** items;
    int count;
    int selected;
    int start;
    int visible_count;
    int item_height;
    SDL_Rect rect;
} list_component_data;

component* list_component_new(int item_height);
void list_component_add_item(component* list, component* item);
void list_component_remove_item(component* list, int index);
void list_component_set_selected(component* list, int index);
void list_component_scroll(component* list, int delta);
component* list_component_get_item(component* list, int index);

// Grid component API
typedef struct {
    component** items;
    int count;
    int selected;
    int columns;
    int item_width;
    int item_height;
    int spacing_x;
    int spacing_y;
    SDL_Rect rect;
} grid_component_data;

component* grid_component_new(int columns, int item_width, int item_height, int spacing_x, int spacing_y);
void grid_component_add_item(component* grid, component* item);
void grid_component_set_selected(component* grid, int index);
void grid_component_navigate(component* grid, int direction);

#endif // NEXTUI_COMPONENT_H__