#ifndef __QUICK_MENU_SCREEN_H__
#define __QUICK_MENU_SCREEN_H__

#include "screen.h"
#include "../state/ui_state.h"
#include "../components/list_component.h"
#include "sdl.h"
#include "../common/api.h"
#include "../array.h"

typedef struct {
    UIState* state;
    SDL_Surface* screen;
    
    // Quick menu state (matching original nextui.c)
    int qm_row;        // 0 = quick entries, 1 = quick actions
    int qm_col;        // current column index
    int qm_slot;       // current slot in view
    int qm_shift;      // horizontal shift for scrolling
    int qm_slots;      // number of visible slots
    
    // Menu data
    Array* quick;      // quick entries (recents, games, tools, etc.)
    Array* quickActions; // quick actions (wifi, bluetooth, sleep, etc.)
    char* folderBgPath; // current background path
    
    // UI elements
    SDL_Surface* background;
} QuickMenuScreen;

QuickMenuScreen* quick_menu_screen_new(UIState* state, SDL_Surface* screen);
void quick_menu_screen_render(void* screen_instance, SDL_Surface* surface);
void quick_menu_screen_handle_input(void* screen_instance, int input);
void quick_menu_screen_update(void* screen_instance);
void quick_menu_screen_destroy(void* screen_instance);

ScreenModule* quick_menu_screen_module_new(UIState* state, SDL_Surface* screen);

#endif // __QUICK_MENU_SCREEN_H__