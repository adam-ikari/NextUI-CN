#ifndef NEXTUI_SCREEN_H__
#define NEXTUI_SCREEN_H__

#include <SDL2/SDL.h>
#include "common/config.h"
#include "common/api.h"

// Forward declarations
typedef struct state state;

// Screen types
typedef enum {
    SCREEN_TYPE_GAMELIST,
    SCREEN_TYPE_GAMESWITCHER,
    SCREEN_TYPE_COUNT
} ScreenType;

// Button hint positions
#define HINT_POSITION_PRIMARY 0
#define HINT_POSITION_SECONDARY 1
#define HINT_POSITION_COUNT 2

// Maximum hints per position
#define MAX_HINTS_PER_POSITION 8

// Hint registration modes
#define HINT_MODE_APPEND 0  // Append to default hints
#define HINT_MODE_REPLACE 1 // Replace default hints
#define HINT_MODE_APPEND_TOP 2 // Add to top of list

// Forward declarations
typedef struct screen_manager screen_manager;
typedef struct screen screen;

// Button hint structure
typedef struct {
    char button[32];   // e.g., "A", "B", "X", "Y", "START+VOL+"
    char text[128];    // Localized text
    int is_default;    // Is this a default hint (can be overridden)
} button_hint;

// Screen interface
typedef struct screen_vtable {
    void (*cleanup)(screen* screen);
    void (*update)(screen* screen, unsigned long now);
    void (*render)(screen* screen, SDL_Surface* surface);
    void (*handle_event)(screen* screen, unsigned long now);
    void (*on_enter)(screen* screen);
    void (*on_exit)(screen* screen);
} screen_vtable;

// Screen base structure
struct screen {
    ScreenType type;
    screen_vtable* vtable;
    void* data;  // Screen-specific data
    
    // Button hints (managed by screen framework)
    button_hint hints[HINT_POSITION_COUNT][MAX_HINTS_PER_POSITION];
    int hint_counts[HINT_POSITION_COUNT];
    
    // State management (React-like data-driven updates)
    state* state;  // Screen state
    effect_hooks* effects;  // Side effects
    memo_caches* memo;  // Memoized computations
    int needs_update;  // Flag indicating if screen needs re-render
};

// Screen Manager
struct screen_manager {
    screen* current_screen;
    screen* last_screen;
    SDL_Surface* last_screen_surface;
    int dirty;
    int animation_direction;
    
    // Status pill rendering (battery, wifi, clock)
    int show_setting;
    
    // Default hints (shared across all screens)
    button_hint default_hints[HINT_POSITION_COUNT][MAX_HINTS_PER_POSITION];
    int default_hint_counts[HINT_POSITION_COUNT];
    
    // Combined key hints (brightness, volume) - shown only when menu is held
    button_hint combined_hints[HINT_POSITION_COUNT][MAX_HINTS_PER_POSITION];
    int combined_hint_counts[HINT_POSITION_COUNT];
    int show_combined_hints;  // Flag to control visibility
};

// Screen Manager API
screen_manager* screen_manager_new(void);
void screen_manager_free(screen_manager* mgr);
void screen_manager_push(screen_manager* mgr, screen* screen);
void screen_manager_pop(screen_manager* mgr);
void screen_manager_replace(screen_manager* mgr, screen* screen);
void screen_manager_update(screen_manager* mgr, unsigned long now);
void screen_manager_render(screen_manager* mgr, SDL_Surface* surface);
int screen_manager_is_dirty(screen_manager* mgr);
void screen_manager_set_dirty(screen_manager* mgr, int dirty);
void screen_manager_set_animation_direction(screen_manager* mgr, int direction);
screen* screen_manager_get_current(screen_manager* mgr);
screen* screen_manager_get_last(screen_manager* mgr);
void screen_manager_set_show_setting(screen_manager* mgr, int show_setting);
int screen_manager_get_show_setting(screen_manager* mgr);

// Default hint API (set once at startup)
void screen_manager_register_default_hint(screen_manager* mgr, int position, const char* button, const char* text);
void screen_manager_clear_default_hints(screen_manager* mgr);

// Combined key hints API (brightness, volume - shown when menu is held)
void screen_manager_register_combined_hint(screen_manager* mgr, int position, const char* button, const char* text);
void screen_manager_clear_combined_hints(screen_manager* mgr);
void screen_manager_set_show_combined_hints(screen_manager* mgr, int show);
int screen_manager_get_show_combined_hints(screen_manager* mgr);

// Screen base API
screen* screen_new(ScreenType type, screen_vtable* vtable, void* data);
void screen_free(screen* screen);

// Button hint API (called by screens)
void screen_clear_hints(screen* scr);
void screen_register_hint(screen* scr, int position, const char* button, const char* text, int mode);
void screen_render_hints(screen* scr, SDL_Surface* surface);

// Specific screen constructors
screen* game_list_screen_new(void);
screen* game_switcher_screen_new(void);

// Game List Screen API
void game_list_screen_set_list_component(screen* scr, component* list_comp);
void game_list_screen_set_thumbnail(screen* scr, component* thumb_comp, int has_thumb);
component_factory* game_list_screen_get_factory(screen* scr);
void game_list_screen_set_selected(screen* scr, int selected);
void game_list_screen_set_total(screen* scr, int total);
int game_list_screen_get_selected(screen* scr);
int game_list_screen_get_total(screen* scr);

// Game Switcher Screen API
void game_switcher_screen_set_title(screen* scr, const char* title, int screen_width, int status_pill_width);
void game_switcher_screen_set_carousel(screen* scr, component* carousel);
component_factory* game_switcher_screen_get_factory(screen* scr);
void game_switcher_screen_set_selected(screen* scr, int selected);
void game_switcher_screen_set_total(screen* scr, int total);
void game_switcher_screen_set_anim_direction(screen* scr, int direction);
int game_switcher_screen_get_selected(screen* scr);
int game_switcher_screen_get_total(screen* scr);
int game_switcher_screen_get_anim_direction(screen* scr);

// State management API (React-like data-driven updates)

// Initialize state for a screen
void screen_init_state(screen* scr);

// Set screen state (triggers re-render)
void screen_set_state(screen* scr, state* state);

// Get screen state
state* screen_get_state(screen* scr);

// Set state value (convenience function)
void screen_set_state_value(screen* scr, const char* key, state_value value);

// Get state value (convenience function)
state_value screen_get_state_value(screen* scr, const char* key);

// Check if screen needs update
int screen_needs_update(screen* scr);

// Force screen update
void screen_force_update(screen* scr);

// Clear update flag
void screen_clear_update_flag(screen* scr);

// Effect management
void screen_use_effect(screen* scr, effect_function func, effect_cleanup cleanup, void* user_data, char** deps, int dep_count);

// Memo management
void* screen_use_memo(screen* scr, const char* key, void* (*compute_func)(void*), void* user_data, char** deps, int dep_count, void (*free_func)(void*));

// Clear all effects and memo
void screen_clear_effects(screen* scr);
void screen_clear_memo(screen* scr);

#endif // NEXTUI_SCREEN_H__