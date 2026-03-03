#include "app.h"
#include "../common/defines.h"
#include "screens/game_list_screen.h"
#include "screens/quick_menu_screen.h"
#include "screens/game_switcher_screen.h"
#include "screens/settings_screen.h"
#include <stdlib.h>

NextUIApp* nextui_app_new(SDL_Surface* screen) {
    NextUIApp* app = (NextUIApp*)malloc(sizeof(NextUIApp));
    if (!app) return NULL;

    app->screen = screen;
    app->state = ui_state_new();
    app->renderer = renderer_new(screen, app->state);
    app->entries = NULL;
    app->recent_games = NULL;
    app->running = true;

    if (!app->state || !app->renderer) {
        nextui_app_free(app);
        return NULL;
    }

    return app;
}

void nextui_app_free(NextUIApp* app) {
    if (!app) return;

    if (app->state) {
        ui_state_free(app->state);
    }

    if (app->renderer) {
        renderer_free(app->renderer);
    }

    if (app->entries) {
        // Free entries if needed
        Array_free(app->entries);
    }

    if (app->recent_games) {
        // Free recent games if needed
        Array_free(app->recent_games);
    }

    free(app);
}

void nextui_app_init(NextUIApp* app) {
    if (!app) return;

    // Set up state change listener to trigger re-rendering
    ui_state_set_listener(app->state, (StateChangeListener)renderer_render, app->renderer);

    // Initialize entries and recent games (placeholder)
    app->entries = Array_new();
    app->recent_games = Array_new();

    // Set initial screen
    nextui_app_set_screen(app, app->state->current_screen);
}

void nextui_app_run(NextUIApp* app) {
    if (!app) return;

    while (app->running && app->state->current_screen != SCREEN_OFF) {
        // Handle input
        int input = getInputBlocking();
        if (input != -1) {
            nextui_app_handle_input(app, input);
        }

        // Update
        nextui_app_update(app);

        // Render
        nextui_app_render(app);
    }
}

void nextui_app_handle_input(NextUIApp* app, int input) {
    if (!app || !app->renderer->current_screen) return;

    // Pass input to current screen
    if (app->renderer->current_screen->handle_input) {
        app->renderer->current_screen->handle_input(app->renderer->current_screen->instance, input);
    }

    // Check for global hotkeys
    if (input == BTN_MENU) {
        ui_state_toggle_menu_held(app->state);
    }
}

void nextui_app_update(NextUIApp* app) {
    if (!app || !app->renderer->current_screen) return;

    // Update current screen
    if (app->renderer->current_screen->update) {
        app->renderer->current_screen->update(app->renderer->current_screen->instance);
    }

    // Update state
    ui_state_mark_dirty(app->state);
}

void nextui_app_render(NextUIApp* app) {
    if (!app) return;

    // Render using renderer
    renderer_render(app->renderer);
}

void nextui_app_set_screen(NextUIApp* app, ScreenType screen_type) {
    if (!app) return;

    // Store current screen as last screen before switching
    if (app->state->current_screen != SCREEN_OFF) {
        ui_state_set_last_screen(app->state, app->state->current_screen);
    }

    // Create new screen module based on screen type
    ScreenModule* screen_module = NULL;

    switch (screen_type) {
        case SCREEN_GAMELIST:
            screen_module = game_list_screen_module_new(app->state, app->screen, app->entries);
            break;
        case SCREEN_QUICKMENU:
            screen_module = quick_menu_screen_module_new(app->state, app->screen);
            break;
        case SCREEN_GAMESWITCHER:
            screen_module = game_switcher_screen_module_new(app->state, app->screen, app->recent_games);
            break;
        case SCREEN_GAME:
            // Game running - no UI to render
            ui_state_set_screen(app->state, SCREEN_OFF);
            return;
        case SCREEN_OFF:
            // Exit application
            app->running = false;
            return;
        default:
            return;
    }

    if (screen_module) {
        renderer_set_screen(app->renderer, screen_module);
        ui_state_set_screen(app->state, screen_type);
    }
}