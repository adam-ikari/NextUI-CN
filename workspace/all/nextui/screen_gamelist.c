#include "screen.h"
#include "nextui.h"
#include <string.h>

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

// Forward declarations (these will be implemented in screen_gamelist_impl.c)
extern void game_list_screen_update(screen* scr, unsigned long now);
extern void game_list_screen_render(screen* scr, SDL_Surface* surface);
extern void game_list_screen_handle_event(screen* scr, unsigned long now);
extern void game_list_screen_on_enter(screen* scr);
extern void game_list_screen_on_exit(screen* scr);

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