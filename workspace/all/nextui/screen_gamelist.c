#include "screen.h"
#include "component.h"
#include "nextui.h"
#include "common/config.h"
#include <string.h>

// External variables from nextui.c
extern int can_resume;
extern int simple_mode;
extern int BTN_SLEEP;
extern int BTN_POWER;
extern int GetHDMI(void);
extern int show_setting;
extern int menu_key_held;
extern int stack_count; // Directory stack count

// Game List Screen Data
typedef struct {
    component* list_component;       // List of games
    component* thumbnail_component;  // Game preview thumbnail
    component* status_pill;          // Status indicator (battery, wifi, etc.)
    component_factory* factory;      // Component factory for creating components
    
    int selected;
    int total;
    int has_thumbnail;
} game_list_screen_data;

// Cleanup function
static void game_list_screen_cleanup(screen* scr) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    
    // Free components
    if (data->list_component) {
        component_free(data->list_component);
    }
    if (data->thumbnail_component) {
        component_free(data->thumbnail_component);
    }
    if (data->status_pill) {
        component_free(data->status_pill);
    }
    
    // Free factory
    if (data->factory) {
        component_factory_free(data->factory);
    }
    
    free(data);
    scr->data = NULL;
}

// Update function
static void game_list_screen_update(screen* scr, unsigned long now) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    
    // Update components if needed
    // (Currently no update logic needed for static components)
}

// Render function
static void game_list_screen_render(screen* scr, SDL_Surface* surface) {
    if (!scr || !surface) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    
    // Render status pill
    if (data->status_pill) {
        if (data->status_pill->vtable && data->status_pill->vtable->render) {
            data->status_pill->vtable->render(data->status_pill, surface);
        }
    }
    
    // Render thumbnail if available
    if (data->thumbnail_component && data->has_thumbnail) {
        if (data->thumbnail_component->vtable && data->thumbnail_component->vtable->render) {
            data->thumbnail_component->vtable->render(data->thumbnail_component, surface);
        }
    }
    
    // Render game list
    if (data->list_component) {
        if (data->list_component->vtable && data->list_component->vtable->render) {
            data->list_component->vtable->render(data->list_component, surface);
        }
    }
}

// Handle event function
static void game_list_screen_handle_event(screen* scr, unsigned long now) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    
    // Handle events for components
    // (Currently delegated to nextui.c main loop)
}

// On enter - initialize components and register hints
static void game_list_screen_on_enter(screen* scr) {
    if (!scr) return;
    
    screen_clear_hints(scr);
    
    // Note: Combined hints (brightness, volume) are registered separately
    // They only show when menu key is held
    // Default hints will be shown normally
    
    // Screen-specific hints
    if (can_resume) {
        screen_register_hint(scr, HINT_POSITION_PRIMARY, "X", TR("common.resume"), HINT_MODE_APPEND);
    }
    
    // Initialize game list screen data if not already initialized
    if (!scr->data) {
        game_list_screen_data* data = (game_list_screen_data*)malloc(sizeof(game_list_screen_data));
        if (!data) {
            LOG_error("Failed to allocate game_list_screen_data\n");
            return;
        }
        
        memset(data, 0, sizeof(game_list_screen_data));
        
        // Create component factory
        data->factory = component_factory_new();
        
        scr->data = data;
    }
}

// On exit - clear hints
static void game_list_screen_on_exit(screen* scr) {
    if (!scr) return;
    screen_clear_hints(scr);
}

// Screen VTable
static screen_vtable gamelist_vtable = {
    .cleanup = game_list_screen_cleanup,
    .update = game_list_screen_update,
    .render = game_list_screen_render,
    .handle_event = game_list_screen_handle_event,
    .on_enter = game_list_screen_on_enter,
    .on_exit = game_list_screen_on_exit
};

screen* game_list_screen_new(void) {
    game_list_screen_data* data = (game_list_screen_data*)malloc(sizeof(game_list_screen_data));
    if (!data) {
        LOG_error("Failed to allocate game_list_screen_data\n");
        return NULL;
    }

    memset(data, 0, sizeof(game_list_screen_data));

    screen* scr = screen_new(SCREEN_TYPE_GAMELIST, &gamelist_vtable, data);
    if (!scr) {
        free(data);
        return NULL;
    }

    return scr;
}

// API for nextui.c to interact with the game list screen

// Set game list component
void game_list_screen_set_list_component(screen* scr, component* list_comp) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    
    // Free old list component if exists
    if (data->list_component) {
        component_free(data->list_component);
    }
    
    data->list_component = list_comp;
}

// Set thumbnail component
void game_list_screen_set_thumbnail(screen* scr, component* thumb_comp, int has_thumb) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    
    // Free old thumbnail if exists
    if (data->thumbnail_component) {
        component_free(data->thumbnail_component);
    }
    
    data->thumbnail_component = thumb_comp;
    data->has_thumbnail = has_thumb;
}

// Get component factory
component_factory* game_list_screen_get_factory(screen* scr) {
    if (!scr || !scr->data) return NULL;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    return data->factory;
}

// Set selected index
void game_list_screen_set_selected(screen* scr, int selected) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    data->selected = selected;
    
    // Update list component selection
    if (data->list_component) {
        list_component_set_selected(data->list_component, selected);
    }
}

// Set total count
void game_list_screen_set_total(screen* scr, int total) {
    if (!scr || !scr->data) return;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    data->total = total;
}

// Get selected index
int game_list_screen_get_selected(screen* scr) {
    if (!scr || !scr->data) return 0;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    return data->selected;
}

// Get total count
int game_list_screen_get_total(screen* scr) {
    if (!scr || !scr->data) return 0;
    
    game_list_screen_data* data = (game_list_screen_data*)scr->data;
    return data->total;
}