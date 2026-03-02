#include "screen.h"
#include "nextui.h"
#include <string.h>

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

// Forward declarations (these will be implemented in screen_gameswitcher_impl.c)
extern void game_switcher_screen_update(screen* scr, unsigned long now);
extern void game_switcher_screen_render(screen* scr, SDL_Surface* surface);
extern void game_switcher_screen_handle_event(screen* scr, unsigned long now);
extern void game_switcher_screen_on_enter(screen* scr);
extern void game_switcher_screen_on_exit(screen* scr);

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