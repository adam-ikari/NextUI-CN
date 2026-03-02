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
extern char preview_path[];
extern int has_preview;

// Game Switcher Screen Data
typedef struct {
    component* carousel_component;   // Carousel of recent games
    component* title_pill;           // Title pill showing current game name
    component_factory* factory;      // Component factory for creating components
    
    int selected;
    int total;
    int anim_direction;
} game_switcher_screen_data;

// Cleanup function
static void game_switcher_screen_cleanup(screen* scr) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    
    // Free components
    if (data->carousel_component) {
        component_free(data->carousel_component);
    }
    if (data->title_pill) {
        component_free(data->title_pill);
    }
    
    // Free factory
    if (data->factory) {
        component_factory_free(data->factory);
    }
    
    free(data);
    scr->data = NULL;
}

// Update function
static void game_switcher_screen_update(screen* scr, unsigned long now) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    
    // Update components if needed
    // (Currently no update logic needed for static components)
}

// Render function
static void game_switcher_screen_render(screen* scr, SDL_Surface* surface) {
    if (!scr || !surface) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    
    // Render game preview (if available)
    if (has_preview && preview_path[0]) {
        // Render preview image
        // (This will be handled by nextui.c for now, using existing preview rendering logic)
    } else {
        // Show "no preview" message
        SDL_Rect preview_rect = {0, 0, surface->w, surface->h};
        GFX_blitMessage(font.large, (char*)TR("common.no_preview"), surface, &preview_rect);
    }
    
    // Render title pill
    if (data->title_pill) {
        if (data->title_pill->vtable && data->title_pill->vtable->render) {
            data->title_pill->vtable->render(data->title_pill, surface);
        }
    }
    
    // Render carousel (if implemented)
    if (data->carousel_component) {
        if (data->carousel_component->vtable && data->carousel_component->vtable->render) {
            data->carousel_component->vtable->render(data->carousel_component, surface);
        }
    }
}

// Handle event function
static void game_switcher_screen_handle_event(screen* scr, unsigned long now) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    
    // Handle events for components
    // (Currently delegated to nextui.c main loop)
}

// On enter - initialize components and register hints
static void game_switcher_screen_on_enter(screen* scr) {
    if (!scr) return;
    
    screen_clear_hints(scr);
    
    // Note: Combined hints (brightness, volume) are registered separately
    // They only show when menu key is held
    // Default hints will be shown normally
    
    // Screen-specific hints
    if (can_resume) {
        screen_register_hint(scr, HINT_POSITION_PRIMARY, "A", TR("common.resume"), HINT_MODE_APPEND);
        screen_register_hint(scr, HINT_POSITION_SECONDARY, "Y", TR("common.remove"), HINT_MODE_APPEND);
    }
    screen_register_hint(scr, HINT_POSITION_TERTIARY, "B", TR("common.back"), HINT_MODE_APPEND);
    
    // Initialize game switcher screen data if not already initialized
    if (!scr->data) {
        game_switcher_screen_data* data = (game_switcher_screen_data*)malloc(sizeof(game_switcher_screen_data));
        if (!data) {
            LOG_error("Failed to allocate game_switcher_screen_data\n");
            return;
        }
        
        memset(data, 0, sizeof(game_switcher_screen_data));
        
        // Create component factory
        data->factory = component_factory_new();
        
        scr->data = data;
    }
}

// On exit - clear hints
static void game_switcher_screen_on_exit(screen* scr) {
    if (!scr) return;
    screen_clear_hints(scr);
}

// Screen VTable
static screen_vtable gameswitcher_vtable = {
    .cleanup = game_switcher_screen_cleanup,
    .update = game_switcher_screen_update,
    .render = game_switcher_screen_render,
    .handle_event = game_switcher_screen_handle_event,
    .on_enter = game_switcher_screen_on_enter,
    .on_exit = game_switcher_screen_on_exit
};

screen* game_switcher_screen_new(void) {
    game_switcher_screen_data* data = (game_switcher_screen_data*)malloc(sizeof(game_switcher_screen_data));
    if (!data) {
        LOG_error("Failed to allocate game_switcher_screen_data\n");
        return NULL;
    }

    memset(data, 0, sizeof(game_switcher_screen_data));

    screen* scr = screen_new(SCREEN_TYPE_GAMESWITCHER, &gameswitcher_vtable, data);
    if (!scr) {
        free(data);
        return NULL;
    }

    return scr;
}

// API for nextui.c to interact with the game switcher screen

// Set title pill
void game_switcher_screen_set_title(screen* scr, const char* title, int screen_width, int status_pill_width) {
    if (!scr || !scr->data || !title) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    
    // Free old title pill if exists
    if (data->title_pill) {
        component_free(data->title_pill);
    }
    
    // Calculate max width
    int max_width = screen_width - SCALE1(PADDING * 2) - status_pill_width;
    
    // Truncate text if needed
    char display_name[256];
    int text_width = GFX_truncateText(font.large, title, display_name, max_width, SCALE1(BUTTON_PADDING * 2));
    max_width = MIN(max_width, text_width);
    
    // Create title pill component
    data->title_pill = pill_component_new(display_name, max_width, SCALE1(PILL_SIZE), NULL);
    
    // Set position
    if (data->title_pill) {
        component_set_position(data->title_pill, SCALE1(PADDING), SCALE1(PADDING), max_width, SCALE1(PILL_SIZE));
    }
}

// Set carousel component
void game_switcher_screen_set_carousel(screen* scr, component* carousel) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    
    // Free old carousel if exists
    if (data->carousel_component) {
        component_free(data->carousel_component);
    }
    
    data->carousel_component = carousel;
}

// Get component factory
component_factory* game_switcher_screen_get_factory(screen* scr) {
    if (!scr || !scr->data) return NULL;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    return data->factory;
}

// Set selected index
void game_switcher_screen_set_selected(screen* scr, int selected) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    data->selected = selected;
}

// Set total count
void game_switcher_screen_set_total(screen* scr, int total) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    data->total = total;
}

// Set animation direction
void game_switcher_screen_set_anim_direction(screen* scr, int direction) {
    if (!scr || !scr->data) return;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    data->anim_direction = direction;
}

// Get selected index
int game_switcher_screen_get_selected(screen* scr) {
    if (!scr || !scr->data) return 0;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    return data->selected;
}

// Get total count
int game_switcher_screen_get_total(screen* scr) {
    if (!scr || !scr->data) return 0;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    return data->total;
}

// Get animation direction
int game_switcher_screen_get_anim_direction(screen* scr) {
    if (!scr || !scr->data) return 0;
    
    game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
    return data->anim_direction;
}