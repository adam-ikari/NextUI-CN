#ifndef NEXTUI_SCREEN_H__
#define NEXTUI_SCREEN_H__

#include <SDL2/SDL.h>
#include "common/config.h"
#include "common/api.h"

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
#define MAX_HINTS_PER_POSITION 4

// Forward declarations
typedef struct screen_manager screen_manager;
typedef struct screen screen;

// Button hint structure
typedef struct {
    char button[16];   // e.g., "A", "B", "X", "Y"
    char text[128];    // Localized text
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

// Screen base API
screen* screen_new(ScreenType type, screen_vtable* vtable, void* data);
void screen_free(screen* screen);

// Button hint API (called by screens)
void screen_clear_hints(screen* scr);
void screen_register_hint(screen* scr, int position, const char* button, const char* text);
void screen_render_hints(screen* scr, SDL_Surface* surface);

// Specific screen constructors
screen* game_list_screen_new(void);
screen* game_switcher_screen_new(void);

#endif // NEXTUI_SCREEN_H__