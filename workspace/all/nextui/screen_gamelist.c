#include "screen.h"
#include "nextui.h"
#include "common/config.h"
#include <string.h>

// External variables from nextui.c
extern int can_resume;
extern int simple_mode;
extern int BTN_SLEEP;
extern int BTN_POWER;
extern int GetHDMI(void);

// Game List Screen Data
typedef struct {
    int selected;
    int total;
    int animation_direction;
} game_list_screen_data;

// Cleanup function
static void game_list_screen_cleanup(screen* scr) {
    if (scr && scr->data) {
        free(scr->data);
        scr->data = NULL;
    }
}

// On enter - register button hints
void game_list_screen_on_enter(screen* scr) {
    if (!scr) return;
    
    screen_clear_hints(scr);
    
    // Primary hints (top position)
    // Note: Default hints (brightness, volume) are registered in screen_manager
    // This screen only adds screen-specific hints
    if (can_resume) {
        screen_register_hint(scr, HINT_POSITION_PRIMARY, "X", TR("common.resume"), HINT_MODE_APPEND);
    }
    
    // Secondary hints (bottom position)
    // Note: Default hints will be merged with screen hints
    // Specific hints will be registered dynamically based on context
}

// On exit - clear button hints
void game_list_screen_on_exit(screen* scr) {
    if (!scr) return;
    screen_clear_hints(scr);
}

// Forward declarations (these will be implemented in screen_gamelist_impl.c)
extern void game_list_screen_update(screen* scr, unsigned long now);
extern void game_list_screen_render(screen* scr, SDL_Surface* surface);
extern void game_list_screen_handle_event(screen* scr, unsigned long now);

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