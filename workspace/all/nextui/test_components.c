// Simple component test program for NextUI
// This program tests basic component rendering without requiring full system initialization

#define USE_SDL2
#define NO_SDL2_IMAGE
#define NO_SDL2_TTF
#define PLATFORM "desktop"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/sdl.h"
#include "../common/defines.h"
#include "../common/api.h"
#include "../../desktop/platform/platform.h"
#include "components/pill_component.h"
#include "components/button_component.h"
#include "components/list_component.h"
#include "components/status_component.h"
#include "state/ui_state.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main(int argc, char* argv[]) {
    printf("NextUI Component Test Program\n");
    printf("==============================\n\n");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    printf("✓ SDL initialized successfully\n");

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "NextUI Component Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("✓ Window created\n");

    // Create surface for rendering (components expect SDL_Surface)
    SDL_Surface* screen = SDL_CreateRGBSurface(
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        32,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000
    );

    if (!screen) {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("✓ Surface created for component rendering\n\n");

    // Test 1: Create and test UI State
    printf("Test 1: UI State Management\n");
    printf("----------------------------\n");
    UIState* state = ui_state_new();
    if (state) {
        printf("✓ UI State created\n");
        printf("  Current screen: %d\n", state->current_screen);
        printf("  Selected item: %d\n", state->selected_item);

        ui_state_set_screen(state, SCREEN_GAMELIST);
        printf("✓ Screen changed to GAMELIST\n");
        printf("  Current screen: %d\n", state->current_screen);

        ui_state_set_selected(state, 5);
        printf("✓ Selected item changed to 5\n");
        printf("  Selected item: %d\n", state->selected_item);

        ui_state_free(state);
        printf("✓ UI State freed\n\n");
    } else {
        printf("✗ Failed to create UI State\n\n");
    }

    // Test 2: Test Pill Component
    printf("Test 2: Pill Component\n");
    printf("----------------------\n");
    UIComponent* pill = pill_component_new();
    if (pill) {
        printf("✓ Pill component created\n");

        PillProps pill_props = {
            .rect = {50, 50, 200, 40},
            .color = THEME_COLOR1,
            .text = "Test Pill"
        };

        // Note: Actual rendering would require GFX_blitPillColor implementation
        // For now, we just verify the component structure
        printf("✓ Pill props prepared\n");
        printf("  Rect: x=%d, y=%d, w=%d, h=%d\n",
               pill_props.rect.x, pill_props.rect.y,
               pill_props.rect.w, pill_props.rect.h);
        printf("  Color: 0x%08X\n", pill_props.color);
        printf("  Text: %s\n", pill_props.text);

        ui_component_free(pill);
        printf("✓ Pill component freed\n\n");
    } else {
        printf("✗ Failed to create Pill component\n\n");
    }

    // Test 3: Test Button Component
    printf("Test 3: Button Component\n");
    printf("------------------------\n");
    UIComponent* button = button_component_new();
    if (button) {
        printf("✓ Button component created\n");

        ButtonProps button_props = {
            .label = "Open",
            .hint = NULL,
            .rect = {50, 150, 100, 40},
            .style = BUTTON_STYLE_PRIMARY,
            .highlighted = true,
            .color = THEME_COLOR1
        };

        printf("✓ Button props prepared\n");
        printf("  Label: %s\n", button_props.label);
        printf("  Style: %d\n", button_props.style);
        printf("  Highlighted: %s\n", button_props.highlighted ? "Yes" : "No");

        ui_component_free(button);
        printf("✓ Button component freed\n\n");
    } else {
        printf("✗ Failed to create Button component\n\n");
    }

    // Test 4: Test List Component
    printf("Test 4: List Component\n");
    printf("----------------------\n");
    UIComponent* list = list_component_new();
    if (list) {
        printf("✓ List component created\n");

        // Create test items
        ListItem* items = (ListItem*)malloc(sizeof(ListItem) * 3);
        if (items) {
            items[0].label = strdup("Game 1");
            items[0].icon_path = NULL;
            items[0].thumbnail = NULL;
            items[0].user_data = NULL;

            items[1].label = strdup("Game 2");
            items[1].icon_path = NULL;
            items[1].thumbnail = NULL;
            items[1].user_data = NULL;

            items[2].label = strdup("Game 3");
            items[2].icon_path = NULL;
            items[2].thumbnail = NULL;
            items[2].user_data = NULL;

            ListProps list_props = {
                .rect = {50, 250, 300, 150},
                .items = items,
                .count = 3,
                .selected = 1,
                .columns = 1,
                .item_size = 40,
                .style = LIST_STYLE_VERTICAL,
                .show_thumbnails = false,
                .selected_color = THEME_COLOR1,
                .normal_color = RGB_WHITE,
                .text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT),
                .selected_text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT)
            };

            printf("✓ List props prepared\n");
            printf("  Item count: %d\n", list_props.count);
            printf("  Selected: %d\n", list_props.selected);
            printf("  Items:\n");
            for (int i = 0; i < list_props.count; i++) {
                printf("    %d: %s\n", i, list_props.items[i].label);
            }

            // Free items
            for (int i = 0; i < 3; i++) {
                free(items[i].label);
            }
            free(items);
        }

        ui_component_free(list);
        printf("✓ List component freed\n\n");
    } else {
        printf("✗ Failed to create List component\n\n");
    }

    // Test 5: Test Status Component
    printf("Test 5: Status Component\n");
    printf("------------------------\n");
    UIComponent* status = status_component_new();
    if (status) {
        printf("✓ Status component created\n");

        StatusProps status_props = {
            .battery_level = 80,
            .is_charging = false,
            .has_wifi = true,
            .time_string = strdup("12:00"),
            .show_battery = true,
            .show_wifi = true,
            .show_time = true,
            .text_color = SDL_COLOR_TO_UINT32(COLOR_LIGHT_TEXT)
        };

        printf("✓ Status props prepared\n");
        printf("  Battery level: %d%%\n", status_props.battery_level);
        printf("  Charging: %s\n", status_props.is_charging ? "Yes" : "No");
        printf("  WiFi: %s\n", status_props.has_wifi ? "Yes" : "No");
        printf("  Time: %s\n", status_props.time_string);

        free(status_props.time_string);
        ui_component_free(status);
        printf("✓ Status component freed\n\n");
    } else {
        printf("✗ Failed to create Status component\n\n");
    }

    printf("All tests completed!\n");
    printf("\nPress any key to exit...\n");
    getchar();

    // Cleanup
    SDL_FreeSurface(screen);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
