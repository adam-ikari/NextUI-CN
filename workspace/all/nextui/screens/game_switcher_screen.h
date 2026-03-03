#ifndef __GAME_SWITCHER_SCREEN_H__
#define __GAME_SWITCHER_SCREEN_H__

#include "screen.h"
#include "../array.h"
#include "../state/ui_state.h"
#include "sdl.h"
#include "../common/api.h"

// Forward declarations
typedef struct Entry Entry;

typedef struct {
    UIState* state;
    SDL_Surface* screen;
    
    // Game switcher state (matching original nextui.c)
    int switcher_selected;  // current selected recent game index
    int can_resume;         // whether current game has save state
    int has_preview;        // whether current game has preview image
    char preview_path[256]; // path to preview image
    
    // Recent games data
    Array* recents;         // array of Recent* objects
    
    // Animation state
    int gsanimdir;          // animation direction (SLIDE_LEFT/SLIDE_RIGHT/NONE)
    ScreenType last_screen; // previous screen for animation
    
    // Cached surfaces
    SDL_Surface* blackBG;   // black background surface
    SDL_Surface* switcherSur; // cached switcher surface
    SDL_Surface* tmpOldScreen; // captured screen for animation
} GameSwitcherScreen;

GameSwitcherScreen* game_switcher_screen_new(UIState* state, SDL_Surface* screen);
void game_switcher_screen_render(void* screen_instance, SDL_Surface* surface);
void game_switcher_screen_handle_input(void* screen_instance, int input);
void game_switcher_screen_update(void* screen_instance);
void game_switcher_screen_destroy(void* screen_instance);

ScreenModule* game_switcher_screen_module_new(UIState* state, SDL_Surface* screen);

#endif // __GAME_SWITCHER_SCREEN_H__