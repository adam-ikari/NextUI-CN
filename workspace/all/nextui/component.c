#include "component.h"
#include "state.h"
#include "nextui.h"
#include <stdlib.h>
#include <string.h>

// Component Factory Implementation

component_factory* component_factory_new(void) {
    component_factory* factory = (component_factory*)malloc(sizeof(component_factory));
    if (!factory) {
        LOG_error("Failed to allocate component_factory\n");
        return NULL;
    }

    memset(factory, 0, sizeof(component_factory));
    
    // Initialize default styles
    // Primary style
    factory->styles[0].bg_color = uintToColour(THEME_COLOR1);
    factory->styles[0].text_color = uintToColour(THEME_COLOR5);
    factory->styles[0].border_color = uintToColour(THEME_COLOR2);
    factory->styles[0].border_width = 2;
    factory->styles[0].corner_radius = 8;
    factory->styles[0].padding = SCALE1(BUTTON_PADDING);
    
    // Secondary style
    factory->styles[1].bg_color = uintToColour(THEME_COLOR3);
    factory->styles[1].text_color = uintToColour(THEME_COLOR4);
    factory->styles[1].border_color = uintToColour(THEME_COLOR2);
    factory->styles[1].border_width = 1;
    factory->styles[1].corner_radius = 4;
    factory->styles[1].padding = SCALE1(BUTTON_PADDING);
    
    // Tertiary style
    factory->styles[2].bg_color = uintToColour(THEME_COLOR3);
    factory->styles[2].text_color = uintToColour(THEME_COLOR6);
    factory->styles[2].border_color = uintToColour(THEME_COLOR4);
    factory->styles[2].border_width = 0;
    factory->styles[2].corner_radius = 2;
    factory->styles[2].padding = SCALE1(BUTTON_PADDING);
    
    return factory;
}

void component_factory_free(component_factory* factory) {
    if (!factory) return;
    
    if (factory->default_icon) {
        SDL_FreeSurface(factory->default_icon);
    }
    if (factory->default_background) {
        SDL_FreeSurface(factory->default_background);
    }
    
    free(factory);
}

void component_factory_set_style(component_factory* factory, int style_index, component_style* style) {
    if (!factory || !style) return;
    if (style_index < 0 || style_index >= 3) {
        LOG_error("component_factory_set_style: invalid style index %d\n", style_index);
        return;
    }
    
    memcpy(&factory->styles[style_index], style, sizeof(component_style));
}

component_style* component_factory_get_style(component_factory* factory, int style_index) {
    if (!factory) return NULL;
    if (style_index < 0 || style_index >= 3) {
        LOG_error("component_factory_get_style: invalid style index %d\n", style_index);
        return NULL;
    }
    
    return &factory->styles[style_index];
}

// Component Base Implementation

component* component_new(ComponentType type, component_vtable* vtable, void* data) {
    if (!vtable) {
        LOG_error("component_new: vtable is NULL\n");
        return NULL;
    }

    component* comp = (component*)malloc(sizeof(component));
    if (!comp) {
        LOG_error("Failed to allocate component\n");
        return NULL;
    }

    comp->type = type;
    comp->vtable = vtable;
    comp->data = data;
    comp->user_data = NULL;
    
    // Initialize state
    memset(&comp->state, 0, sizeof(component_state));
    comp->state.rect.x = 0;
    comp->state.rect.y = 0;
    comp->state.rect.w = 0;
    comp->state.rect.h = 0;
    comp->state.visible = 1;
    comp->state.enabled = 1;
    
    return comp;
}

void component_free(component* comp) {
    if (!comp) return;

    // Call cleanup function
    if (comp->vtable && comp->vtable->cleanup) {
        comp->vtable->cleanup(comp);
    }

    // Free surface if exists
    if (comp->state.surface) {
        SDL_FreeSurface(comp->state.surface);
    }

    free(comp);
}

void component_set_position(component* comp, int x, int y, int width, int height) {
    if (!comp) return;
    
    comp->state.rect.x = x;
    comp->state.rect.y = y;
    if (width > 0) comp->state.rect.w = width;
    if (height > 0) comp->state.rect.h = height;
}

void component_set_text(component* comp, const char* text) {
    if (!comp || !text) return;
    
    switch (comp->type) {
        case COMPONENT_TYPE_BUTTON: {
            button_component_data* data = (button_component_data*)comp->data;
            if (data) {
                strncpy(data->text, text, sizeof(data->text) - 1);
                data->text[sizeof(data->text) - 1] = '\0';
            }
            break;
        }
        case COMPONENT_TYPE_PILL: {
            pill_component_data* data = (pill_component_data*)comp->data;
            if (data) {
                strncpy(data->text, text, sizeof(data->text) - 1);
                data->text[sizeof(data->text) - 1] = '\0';
            }
            break;
        }
        default:
            break;
    }
}

void component_set_visible(component* comp, int visible) {
    if (comp) {
        comp->state.visible = visible;
    }
}

void component_set_enabled(component* comp, int enabled) {
    if (comp) {
        comp->state.enabled = enabled;
        // Call set_enabled vtable if available
        if (comp->vtable && comp->vtable->set_enabled) {
            comp->vtable->set_enabled(comp, enabled);
        }
    }
}

void component_set_style(component* comp, component_style* style) {
    if (!comp || !style) return;
    memcpy(&comp->style, style, sizeof(component_style));
}

int component_is_hovered(component* comp) {
    return comp ? comp->state.hovered : 0;
}

int component_is_pressed(component* comp) {
    return comp ? comp->state.pressed : 0;
}

// Component Renderers

void component_render_button(component* comp, SDL_Surface* surface) {
    if (!comp || !surface || !comp->state.visible) return;
    
    button_component_data* data = (button_component_data*)comp->data;
    if (!data) return;
    
    SDL_Rect rect = comp->state.rect;
    
    // Draw button background
    if (comp->state.pressed) {
        GFX_blitRectColor(ASSET_STATE_BG, surface, &rect, comp->style.bg_color);
    } else if (comp->state.hovered) {
        // Lighter color for hover
        uint32_t hover_color = THEME_COLOR2;
        GFX_blitRectColor(ASSET_STATE_BG, surface, &rect, hover_color);
    } else {
        GFX_blitRectColor(ASSET_WHITE_PILL, surface, &rect, comp->style.bg_color);
    }
    
    // Draw icon if present
    if (data->icon_path[0]) {
        SDL_Surface* icon = IMG_Load(data->icon_path);
        if (icon) {
            SDL_Surface* converted = SDL_ConvertSurfaceFormat(icon, SDL_PIXELFORMAT_RGBA8888, 0);
            if (converted) {
                SDL_FreeSurface(icon);
                icon = converted;
            }
            
            int x = rect.x + (rect.w - icon->w) / 2;
            int y = rect.y + (rect.h - icon->h) / 2;
            SDL_Rect destRect = {x, y, 0, 0};
            
            GFX_blitSurfaceColor(icon, NULL, surface, &destRect, comp->style.text_color);
            SDL_FreeSurface(icon);
        }
    }
    
    // Draw text
    if (data->text[0]) {
        int w, h;
        const char* label = data->text;
        GFX_sizeText(font.large, label, SCALE1(FONT_LARGE), &w, &h);
        
        SDL_Surface* text = TTF_RenderUTF8_Blended(font.large, label, uintToColour(comp->state.enabled ? comp->style.text_color : THEME_COLOR4));
        if (text) {
            int x = rect.x + (rect.w - text->w) / 2;
            int y = rect.y + (rect.h - text->h) / 2;
            SDL_BlitSurface(text, NULL, surface, &(SDL_Rect){x, y, 0, 0});
            SDL_FreeSurface(text);
        }
    }
}

void component_render_pill(component* comp, SDL_Surface* surface) {
    if (!comp || !surface || !comp->state.visible) return;
    
    pill_component_data* data = (pill_component_data*)comp->data;
    if (!data) return;
    
    SDL_Rect rect = comp->state.rect;
    
    // Draw pill background
    if (data->width > 0 && data->height > 0) {
        GFX_blitPillLight(ASSET_WHITE_PILL, surface, &rect);
    } else {
        GFX_blitPillLight(ASSET_WHITE_PILL, surface, &(SDL_Rect){rect.x, rect.y, rect.w, rect.h});
    }
    
    // Draw icon if present
    if (data->icon) {
        int x = rect.x + (rect.w - data->icon->w) / 2;
        int y = rect.y + (rect.h - data->icon->h) / 2;
        SDL_BlitSurface(data->icon, NULL, surface, &(SDL_Rect){x, y, 0, 0});
    }
    
    // Draw text
    if (data->text[0]) {
        int w, h;
        const char* label = data->text;
        GFX_sizeText(font.large, label, SCALE1(FONT_LARGE), &w, &h);
        
        SDL_Surface* text = TTF_RenderUTF8_Blended(font.large, label, comp->style.text_color);
        if (text) {
            int text_offset_y = (rect.h - text->h + 1) >> 1 + SCALE1(TEXT_Y_OFFSET);
            int x = rect.x + (rect.w - w) / 2;
            int y = rect.y + text_offset_y;
            SDL_BlitSurface(text, NULL, surface, &(SDL_Rect){x, y, 0, 0});
            SDL_FreeSurface(text);
        }
    }
}

void component_render_thumbnail(component* comp, SDL_Surface* surface) {
    if (!comp || !surface || !comp->state.visible) return;
    
    thumbnail_component_data* data = (thumbnail_component_data*)comp->data;
    if (!data) return;
    
    SDL_Rect rect = comp->state.rect;
    
    // Draw background
    if (data->selected) {
        GFX_blitRectColor(ASSET_STATE_BG, surface, &rect, THEME_COLOR1);
    } else {
        // Semi-transparent background
        SDL_Surface* bg = SDL_CreateRGBSurfaceWithFormat(0, rect.w, rect.h, 32, SDL_PIXELFORMAT_RGBA8888);
        if (bg) {
            SDL_FillRect(bg, NULL, SDL_MapRGBA(surface->format, THEME_COLOR3 >> 24, 
                             (THEME_COLOR3 >> 16) & 0xFF, (THEME_COLOR3 >> 8) & 0xFF, THEME_COLOR3 & 0xFF, 128));
            SDL_BlitSurface(bg, NULL, surface, &rect);
            SDL_FreeSurface(bg);
        }
    }
    
    // Draw image
    if (data->image_path[0]) {
        SDL_Surface* img = IMG_Load(data->image_path);
        if (img) {
            // Scale and center image
            float scale_x = (float)rect.w / img->w;
            float scale_y = (float)rect.h / img->h;
            float scale = (scale_x < scale_y) ? scale_x : scale_y;
            
            int scaled_w = (int)(img->w * scale);
            int scaled_h = (int)(img->h * scale);
            
            int x = rect.x + (rect.w - scaled_w) / 2;
            int y = rect.y + (rect.h - scaled_h) / 2;
            
            SDL_Surface* scaled = NULL;
            if (SDL_SCALE_SMOOTH(img, scaled_w, scaled_h, &scaled) == 0) {
                SDL_BlitSurface(scaled, NULL, surface, &(SDL_Rect){x, y, 0, 0});
                SDL_FreeSurface(scaled);
            }
            
            SDL_FreeSurface(img);
        }
    }
    
    // Draw label
    if (data->label[0]) {
        int w, h;
        GFX_sizeText(font.tiny, data->label, SCALE1(FONT_TINY), &w, &h);
        
        SDL_Surface* text = TTF_RenderUTF8_Blended(font.tiny, data->label, uintToColour(THEME_COLOR5));
        if (text) {
            SDL_Rect text_rect = {rect.x + (rect.w - w) / 2, rect.y + rect.h - h - SCALE1(BUTTON_MARGIN), w, h};
            SDL_BlitSurface(text, NULL, surface, &text_rect);
            SDL_FreeSurface(text);
        }
    }
}

void component_render_background(component* comp, SDL_Surface* surface) {
    if (!comp || !surface || !comp->state.visible) return;
    
    SDL_Rect rect = comp->state.rect;
    
    // Draw background
    if (comp->state.surface) {
        SDL_BlitSurface(comp->state.surface, NULL, surface, &rect);
    } else {
        SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, 
                     (comp->style.bg_color >> 24) & 0xFF,
                     (comp->style.bg_color >> 16) & 0xFF,
                     (comp->style.bg_color >> 8) & 0xFF,
                     comp->style.bg_color & 0xFF));
    }
}

void component_render_status_pill(component* factory, SDL_Surface* surface, int show_setting) {
    if (!factory || !surface) return;
    
    // This function doesn't use component system, calls existing API
    if (show_setting) {
        if (!GetHDMI()) {
            GFX_blitHardwareHints(surface, show_setting);
        }
    } else {
        GFX_blitHardwareGroup(surface, 0);
    }
}

// Button Component

static void button_component_cleanup(component* comp) {
    if (comp && comp->data) {
        free(comp->data);
        comp->data = NULL;
    }
}

static void button_component_render(component* comp, SDL_Surface* surface) {
    component_render_button(comp, surface);
}

static component_vtable button_vtable = {
    .cleanup = button_component_cleanup,
    .update = NULL,
    .render = button_component_render,
    .handle_event = NULL,
    .set_enabled = NULL
};

component* button_component_new(const char* text, const char* icon_path, int primary) {
    button_component_data* data = (button_component_data*)malloc(sizeof(button_component_data));
    if (!data) {
        LOG_error("Failed to allocate button_component_data\n");
        return NULL;
    }
    
    memset(data, 0, sizeof(button_component_data));
    if (text) strncpy(data->text, text, sizeof(data->text) - 1);
    if (icon_path) strncpy(data->icon_path, icon_path, sizeof(data->icon_path) - 1);
    data->primary = primary;
    
    component* comp = component_new(COMPONENT_TYPE_BUTTON, &button_vtable, data);
    if (!comp) {
        free(data);
        return NULL;
    }
    
    return comp;
}

// Pill Component

static void pill_component_cleanup(component* comp) {
    if (comp && comp->data) {
        pill_component_data* data = (pill_component_data*)comp->data;
        if (data->icon) {
            SDL_FreeSurface(data->icon);
        }
        free(comp->data);
        comp->data = NULL;
    }
}

static void pill_component_render(component* comp, SDL_Surface* surface) {
    component_render_pill(comp, surface);
}

static component_vtable pill_vtable = {
    .cleanup = pill_component_cleanup,
    .update = NULL,
    .render = pill_component_render,
    .handle_event = NULL,
    .set_enabled = NULL
};

component* pill_component_new(const char* text, int width, int height, SDL_Surface* icon) {
    pill_component_data* data = (pill_component_data*)malloc(sizeof(pill_component_data));
    if (!data) {
        LOG_error("Failed to allocate pill_component_data\n");
        return NULL;
    }
    
    memset(data, 0, sizeof(pill_component_data));
    if (text) strncpy(data->text, text, sizeof(data->text) - 1);
    data->width = width;
    data->height = height;
    data->icon = icon;
    
    component* comp = component_new(COMPONENT_TYPE_PILL, &pill_vtable, data);
    if (!comp) {
        free(data);
        if (icon) SDL_FreeSurface(icon);
        return NULL;
    }
    
    return comp;
}

// Thumbnail Component

static void thumbnail_component_cleanup(component* comp) {
    if (comp && comp->data) {
        free(comp->data);
        comp->data = NULL;
    }
}

static void thumbnail_component_render(component* comp, SDL_Surface* surface) {
    component_render_thumbnail(comp, surface);
}

static component_vtable thumbnail_vtable = {
    .cleanup = thumbnail_component_cleanup,
    .update = NULL,
    .render = thumbnail_component_render,
    .handle_event = NULL,
    .set_enabled = NULL
};

component* thumbnail_component_new(const char* image_path, const char* label, const char* label_secondary) {
    thumbnail_component_data* data = (thumbnail_component_data*)malloc(sizeof(thumbnail_component_data));
    if (!data) {
        LOG_error("Failed to allocate thumbnail_component_data\n");
        return NULL;
    }
    
    memset(data, 0, sizeof(thumbnail_component_data));
    if (image_path) strncpy(data->image_path, image_path, sizeof(data->image_path) - 1);
    if (label) strncpy(data->label, label, sizeof(data->label) - 1);
    if (label_secondary) strncpy(data->label_secondary, label_secondary, sizeof(data->label_secondary) - 1);
    
    component* comp = component_new(COMPONENT_TYPE_THUMBNAIL, &thumbnail_vtable, data);
    if (!comp) {
        free(data);
        return NULL;
    }
    
    return comp;
}

void thumbnail_component_set_selected(component* thumb, int selected) {
    if (thumb && thumb->data) {
        thumbnail_component_data* data = (thumbnail_component_data*)thumb->data;
        data->selected = selected;
    }
}

void thumbnail_component_set_label(component* thumb, const char* label) {
    if (thumb && thumb->data && label) {
        thumbnail_component_data* data = (thumbnail_component_data*)thumb->data;
        strncpy(data->label, label, sizeof(data->label) - 1);
        data->label[sizeof(data->label) - 1] = '\0';
    }
}

// List Component

static void list_component_cleanup(component* comp) {
    if (comp && comp->data) {
        list_component_data* data = (list_component_data*)comp->data;
        if (data->items) {
            for (int i = 0; i < data->count; i++) {
                if (data->items[i]) {
                    component_free(data->items[i]);
                }
            }
            free(data->items);
        }
        free(comp->data);
        comp->data = NULL;
    }
}

static void list_component_render(component* comp, SDL_Surface* surface) {
    if (!comp || !surface || !comp->state.visible) return;
    
    list_component_data* data = (list_component_data*)comp->data;
    if (!data) return;
    
    // Render visible items
    for (int i = data->start; i < data->start + data->visible_count && i < data->count; i++) {
        component* item = data->items[i];
        if (item) {
            component_set_position(item, data->rect.x, data->rect.y + (i - data->start) * data->item_height, 
                               data->rect.w, data->item_height);
            
            // Update selected state for thumbnails
            if (item->type == COMPONENT_TYPE_THUMBNAIL) {
                thumbnail_component_set_selected(item, (i == data->selected));
            }
            
            if (item->vtable && item->vtable->render) {
                item->vtable->render(item, surface);
            }
        }
    }
}

static component_vtable list_vtable = {
    .cleanup = list_component_cleanup,
    .update = NULL,
    .render = list_component_render,
    .handle_event = NULL,
    .set_enabled = NULL
};

component* list_component_new(int item_height) {
    list_component_data* data = (list_component_data*)malloc(sizeof(list_component_data));
    if (!data) {
        LOG_error("Failed to allocate list_component_data\n");
        return NULL;
    }
    
    memset(data, 0, sizeof(list_component_data));
    data->item_height = item_height;
    data->visible_count = MAIN_ROW_COUNT;
    
    component* comp = component_new(COMPONENT_TYPE_PILL, &list_vtable, data);
    if (!comp) {
        free(data);
        return NULL;
    }
    
    return comp;
}

void list_component_add_item(component* list, component* item) {
    if (!list || !item || !list->data) return;
    
    list_component_data* data = (list_component_data*)list->data;
    
    // Reallocate array
    component** new_items = (component**)realloc(data->items, (data->count + 1) * sizeof(component*));
    if (!new_items) {
        LOG_error("Failed to reallocate list items array\n");
        return;
    }
    
    data->items = new_items;
    data->items[data->count++] = item;
}

void list_component_remove_item(component* list, int index) {
    if (!list || !list->data || index < 0 || index >= list->data->count) return;
    
    list_component_data* data = (list_component_data*)list->data;
    
    component_free(data->items[index]);
    
    // Shift remaining items
    for (int i = index; i < data->count - 1; i++) {
        data->items[i] = data->items[i + 1];
    }
    
    data->count--;
}

void list_component_set_selected(component* list, int index) {
    if (!list || !list->data) return;
    
    list_component_data* data = (list_component_data*)list->data;
    
    if (index >= 0 && index < data->count) {
        data->selected = index;
        
        // Update scroll position
        if (index < data->start) {
            data->start = index;
        } else if (index >= data->start + data->visible_count) {
            data->start = index - data->visible_count + 1;
        }
        
        // Clamp start
        if (data->start < 0) data->start = 0;
        if (data->start > data->count - data->visible_count) {
            data->start = MAX(0, data->count - data->visible_count);
        }
    }
}

void list_component_scroll(component* list, int delta) {
    if (!list || !list->data) return;
    
    list_component_data* data = (list_component_data*)list->data;
    int new_selected = data->selected + delta;
    
    // Wrap around
    if (new_selected < 0) {
        new_selected = data->count - 1;
    } else if (new_selected >= data->count) {
        new_selected = 0;
    }
    
    list_component_set_selected(list, new_selected);
}

component* list_component_get_item(component* list, int index) {
    if (!list || !list->data) return NULL;
    
    list_component_data* data = (list_component_data*)list->data;
    if (index >= 0 && index < data->count) {
        return data->items[index];
    }
    return NULL;
}

// Data-driven update API implementation

void component_set_props(component* comp, state* props) {
    if (!comp) return;
    
    // Free old props if exists
    if (comp->props) {
        state_free(comp->props);
    }
    
    comp->props = props;
    
    // Mark component for update
    comp->should_update = 1;
}

void component_set_state(component* comp, state* state) {
    if (!comp) return;
    
    // Free old state if exists
    if (comp->comp_state) {
        state_free(comp->comp_state);
    }
    
    comp->comp_state = state;
    
    // Mark component for update
    comp->should_update = 1;
}

void component_force_update(component* comp) {
    if (comp) {
        comp->should_update = 1;
    }
}

int component_needs_update(component* comp) {
    return comp ? comp->should_update : 0;
}

void component_set_key(component* comp, int key) {
    if (comp) {
        comp->key = key;
    }
}

int component_get_key(component* comp) {
    return comp ? comp->key : 0;
}

// Virtual DOM rendering

component* component_render_virtual(component* parent, component* old_child, component* new_child) {
    if (!new_child) {
        // Remove old child
        if (old_child) {
            component_free(old_child);
        }
        return NULL;
    }
    
    if (!old_child) {
        // Create new child
        return new_child;
    }
    
    // Check if keys match
    if (old_child->key != new_child->key) {
        // Keys don't match, replace old child with new one
        component_free(old_child);
        return new_child;
    }
    
    // Keys match, check if types match
    if (old_child->type != new_child->type) {
        // Types don't match, replace old child with new one
        component_free(old_child);
        return new_child;
    }
    
    // Types and keys match, reuse old component and update it
    component_diff_and_update(old_child, new_child);
    
    // Free new_child as we're using old_child
    if (new_child->vtable && new_child->vtable->cleanup) {
        new_child->vtable->cleanup(new_child);
    }
    free(new_child);
    
    return old_child;
}

void component_diff_and_update(component* old_comp, component* new_comp) {
    if (!old_comp || !new_comp) return;
    
    // Check if component should update
    int should_update = 1;
    
    if (old_comp->vtable && old_comp->vtable->should_component_update) {
        should_update = old_comp->vtable->should_component_update(old_comp, old_comp->comp_state, new_comp->comp_state);
    }
    
    if (!should_update) {
        return;
    }
    
    // Update props
    if (new_comp->props) {
        if (old_comp->props) {
            state_free(old_comp->props);
        }
        old_comp->props = new_comp->props;
        new_comp->props = NULL;  // Transfer ownership
    }
    
    // Update state
    if (new_comp->comp_state) {
        if (old_comp->comp_state) {
            state_free(old_comp->comp_state);
        }
        old_comp->comp_state = new_comp->comp_state;
        new_comp->comp_state = NULL;  // Transfer ownership
    }
    
    // Update style
    memcpy(&old_comp->style, &new_comp->style, sizeof(component_style));
    
    // Update position
    old_comp->state.rect = new_comp->state.rect;
    
    // Update visibility and enabled state
    old_comp->state.visible = new_comp->state.visible;
    old_comp->state.enabled = new_comp->state.enabled;
    
    // Mark for update
    old_comp->should_update = 1;
    
    // Update component-specific data
    // (This is component-specific, each component type should handle its own data update)
}