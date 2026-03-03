#ifndef __UI_STATE_H__
#define __UI_STATE_H__

#include "../common/config.h"
#include <stdbool.h>

typedef void (*StateChangeListener)(void* user_data);

typedef struct UIState {
    ScreenType current_screen;
    ScreenType last_screen;
    int selected_item;
    int show_setting;
    bool menu_key_held;
    bool show_game_switcher;
    bool dirty;
    StateChangeListener listener;
    void* listener_user_data;
} UIState;

UIState* ui_state_new(void);
void ui_state_free(UIState* state);

void ui_state_set_screen(UIState* state, ScreenType screen);
void ui_state_set_last_screen(UIState* state, ScreenType screen);
void ui_state_set_selected(UIState* state, int selected);
void ui_state_set_setting(UIState* state, int setting);
void ui_state_toggle_menu_held(UIState* state);
void ui_state_set_game_switcher(UIState* state, bool show);
void ui_state_mark_dirty(UIState* state);
void ui_state_clear_dirty(UIState* state);

void ui_state_set_listener(UIState* state, StateChangeListener listener, void* user_data);

#endif // __UI_STATE_H__