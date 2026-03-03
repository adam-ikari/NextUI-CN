#include "ui_state.h"
#include <stdlib.h>

UIState* ui_state_new(void) {
    UIState* state = (UIState*)malloc(sizeof(UIState));
    if (!state) return NULL;

    state->current_screen = CFG_getDefaultView();
    state->last_screen = SCREEN_OFF;
    state->selected_item = 0;
    state->show_setting = 0;
    state->menu_key_held = false;
    state->show_game_switcher = false;
    state->dirty = true;
    state->listener = NULL;
    state->listener_user_data = NULL;

    return state;
}

void ui_state_free(UIState* state) {
    if (state) {
        free(state);
    }
}

void ui_state_set_screen(UIState* state, ScreenType screen) {
    if (!state || state->current_screen == screen) return;

    state->current_screen = screen;
    ui_state_mark_dirty(state);

    if (state->listener) {
        state->listener(state->listener_user_data);
    }
}

void ui_state_set_last_screen(UIState* state, ScreenType screen) {
    if (!state) return;
    state->last_screen = screen;
}

void ui_state_set_selected(UIState* state, int selected) {
    if (!state || state->selected_item == selected) return;

    state->selected_item = selected;
    ui_state_mark_dirty(state);

    if (state->listener) {
        state->listener(state->listener_user_data);
    }
}

void ui_state_set_setting(UIState* state, int setting) {
    if (!state) return;
    state->show_setting = setting;
    ui_state_mark_dirty(state);

    if (state->listener) {
        state->listener(state->listener_user_data);
    }
}

void ui_state_toggle_menu_held(UIState* state) {
    if (!state) return;
    state->menu_key_held = !state->menu_key_held;
}

void ui_state_set_game_switcher(UIState* state, bool show) {
    if (!state || state->show_game_switcher == show) return;

    state->show_game_switcher = show;
    ui_state_mark_dirty(state);

    if (state->listener) {
        state->listener(state->listener_user_data);
    }
}

void ui_state_mark_dirty(UIState* state) {
    if (!state) return;
    state->dirty = true;
}

void ui_state_clear_dirty(UIState* state) {
    if (!state) return;
    state->dirty = false;
}

void ui_state_set_listener(UIState* state, StateChangeListener listener, void* user_data) {
    if (!state) return;
    state->listener = listener;
    state->listener_user_data = user_data;
}