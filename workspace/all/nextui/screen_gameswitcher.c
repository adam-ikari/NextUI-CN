#include "screen.h"
#include "nextui.h"
#include "common/config.h"
#include <string.h>

// External variables from nextui.c
extern int simple_mode;
extern int BTN_SLEEP;
extern int BTN_POWER;

// Game Switcher Screen Data
typedef struct {
    int selected;
    int animation_direction;
    SDL_Surface* last_surface;
} game_switcher_screen_data;

// Cleanup function
static void game_switcher_screen_cleanup(screen* scr) {
    if (scr && scr->data) {
        game_switcher_screen_data* data = (game_switcher_screen_data*)scr->data;
        if (data->last_surface) {
            SDL_FreeSurface(data->last_surface);
            data->last_surface = NULL;
        }
        free(scr->data);
        scr->data = NULL;
    }
}

// On enter - register button hints
void game_switcher_screen_on_enter(screen* scr) {
    if (!scr) return;
    
    screen_clear_hints(scr);
    
    // Primary hints (top position)
    // Sleep or Info (will be merged with default brightness/volume hints)
    screen_register_hint(scr, HINT_POSITION_PRIMARY, 
        (BTN_SLEEP==BTN_POWER)?TR("common.power"):TR("common.menu"),
        (BTN_SLEEP==BTN_POWER||simple_mode)?TR("common.sleep"):TR("common.info"),
        HINT_MODE_APPEND);
    
    // Secondary hints (bottom position)
    screen_register_hint(scr, HINT_POSITION_SECONDARY, "B", TR("common.back"), HINT_MODE_APPEND);
    screen_register_hint(scr, HINT_POSITION_SECONDARY, "Y", TR("common.remove"), HINT_MODE_APPEND);
}

// On exit - clear button hints
void game_switcher_screen_on_exit(screen* scr) {
    if (!scr) return;
    screen_clear_hints(scr);
}

// Forward declarations (these will be implemented in screen_gameswitcher_impl.c)
extern void game_switcher_screen_update(screen* scr, unsigned long now);
extern void game_switcher_screen_render(screen* scr, SDL_Surface* surface);
extern void game_switcher_screen_handle_event(screen* scr, unsigned long now);

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