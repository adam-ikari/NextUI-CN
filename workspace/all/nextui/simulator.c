/**
 * NextUI UI Simulator
 * 
 * A desktop simulator for testing NextUI UI components without hardware.
 * This allows for faster development and testing of UI changes.
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Simulator configuration
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FPS 60

// Color definitions
#define COLOR_BG 0x1E1E1E
#define COLOR_PILL_BG 0x3C3C3C
#define COLOR_PILL_SELECTED 0x4A4A4A
#define COLOR_TEXT 0xFFFFFF
#define COLOR_HIGHLIGHT 0x007ACC
#define COLOR_BUTTON_PRIMARY 0x4A90E2
#define COLOR_BUTTON_SECONDARY 0x50E3C2

// Key mappings (keyboard to controller)
typedef struct {
    SDL_Keycode up;
    SDL_Keycode down;
    SDL_Keycode left;
    SDL_Keycode right;
    SDL_Keycode a;
    SDL_Keycode b;
    SDL_Keycode x;
    SDL_Keycode y;
    SDL_Keycode select;
    SDL_Keycode start;
    SDL_Keycode menu;
} key_mapping;

// Simulator state
typedef struct {
    SDL_Window* window;
    SDL_Surface* screen;
    int running;
    
    // Input state
    int up_pressed;
    int down_pressed;
    int left_pressed;
    int right_pressed;
    int a_pressed;
    int b_pressed;
    int x_pressed;
    int y_pressed;
    int select_pressed;
    int start_pressed;
    int menu_pressed;
    
    // Menu state
    int selected_item;
    int menu_open;
} simulator_state;

// Initialize SDL
int init_sdl(simulator_state* sim) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 0;
    }
    
    // Create window
    sim->window = SDL_CreateWindow(
        "NextUI Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!sim->window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    // Get window surface
    sim->screen = SDL_GetWindowSurface(sim->window);
    
    sim->running = 1;
    sim->selected_item = 0;
    sim->menu_open = 0;
    
    return 1;
}

// Cleanup SDL
void cleanup_sdl(simulator_state* sim) {
    if (sim->window) {
        SDL_DestroyWindow(sim->window);
    }
    SDL_Quit();
}

// Draw rounded rectangle (pill)
void draw_pill(SDL_Surface* surface, int x, int y, int width, int height, Uint32 color, int selected) {
    SDL_Rect rect = {x, y, width, height};
    
    // Draw rounded corners approximation
    int radius = height / 2;
    
    // Draw center rectangle
    SDL_Rect center = {x + radius, y, width - 2 * radius, height};
    SDL_FillRect(surface, &center, color);
    
    // Draw left and right circles
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrt(radius * radius - dy * dy);
        
        // Left circle
        int px = x + radius;
        int py = y + radius + dy;
        if (py >= y && py < y + height) {
            SDL_Rect left = {px - dx, py, 2 * dx, 1};
            SDL_FillRect(surface, &left, color);
        }
        
        // Right circle
        px = x + width - radius;
        if (py >= y && py < y + height) {
            SDL_Rect right = {px - dx, py, 2 * dx, 1};
            SDL_FillRect(surface, &right, color);
        }
    }
    
    // Draw highlight if selected
    if (selected) {
        Uint32 highlight = SDL_MapRGB(surface->format, 100, 180, 255);
        SDL_Rect highlight_rect = {x + 4, y + 4, width - 8, height - 8};
        draw_pill(surface, x + 4, y + 4, width - 8, height - 8, highlight, 0);
    }
}

// Draw button hint
void draw_button_hint(SDL_Surface* surface, const char* button, const char* text, int x, int y) {
    // Draw button label (simple text)
    Uint32 text_color = SDL_MapRGB(surface->format, 255, 255, 255);
    
    // Draw button box
    SDL_Rect button_rect = {x, y, 30, 20};
    SDL_FillRect(surface, &button_rect, SDL_MapRGB(surface->format, 100, 100, 100));
    
    // Note: In a real implementation, you would use SDL_ttf for proper text rendering
    // For now, we just draw a placeholder
}

// Draw status pill
void draw_status_pill(SDL_Surface* surface, int x, int y, int width, int height) {
    Uint32 pill_color = SDL_MapRGB(surface->format, 60, 60, 60);
    draw_pill(surface, x, y, width, height, pill_color, 0);
    
    // Draw status indicators (placeholders)
    // Battery indicator
    SDL_Rect battery = {x + 10, y + height / 2 - 5, 20, 10};
    SDL_FillRect(surface, &battery, SDL_MapRGB(surface->format, 0, 255, 0));
    
    // WiFi indicator
    SDL_Rect wifi = {x + 40, y + height / 2 - 5, 10, 10};
    SDL_FillRect(surface, &wifi, SDL_MapRGB(surface->format, 0, 200, 255));
    
    // Clock indicator
    SDL_Rect clock = {x + width - 40, y + height / 2 - 5, 30, 10};
    SDL_FillRect(surface, &clock, SDL_MapRGB(surface->format, 255, 255, 255));
}

// Render game list screen
void render_game_list(SDL_Surface* surface, simulator_state* sim) {
    // Clear background
    Uint32 bg_color = SDL_MapRGB(surface->format, 30, 30, 30);
    SDL_FillRect(surface, NULL, bg_color);
    
    // Draw status pill
    draw_status_pill(surface, 10, 10, SCREEN_WIDTH - 20, 40);
    
    // Draw menu items
    int item_count = 8;
    int start_y = 60;
    int item_height = 50;
    int margin = 10;
    
    for (int i = 0; i < item_count; i++) {
        int y = start_y + i * (item_height + margin);
        int selected = (i == sim->selected_item);
        
        Uint32 pill_color = selected ? 
            SDL_MapRGB(surface->format, 74, 144, 226) : 
            SDL_MapRGB(surface->format, 60, 60, 60);
        
        draw_pill(surface, 10, y, SCREEN_WIDTH - 20, item_height, pill_color, 0);
        
        // Draw item number as placeholder
        SDL_Rect item_num = {30, y + item_height / 2 - 5, 20, 10};
        SDL_FillRect(surface, &item_num, SDL_MapRGB(surface->format, 255, 255, 255));
    }
    
    // Draw button hints
    draw_button_hint(surface, "A", "Select", 10, SCREEN_HEIGHT - 30);
    draw_button_hint(surface, "B", "Back", 100, SCREEN_HEIGHT - 30);
    draw_button_hint(surface, "MENU", "Quick Menu", 200, SCREEN_HEIGHT - 30);
}

// Render quick menu screen
void render_quick_menu(SDL_Surface* surface, simulator_state* sim) {
    // Draw semi-transparent overlay
    Uint32 overlay_color = SDL_MapRGBA(surface->format, 0, 0, 0, 180);
    SDL_FillRect(surface, NULL, overlay_color);
    
    // Draw menu background
    int menu_width = 300;
    int menu_height = 200;
    int menu_x = (SCREEN_WIDTH - menu_width) / 2;
    int menu_y = (SCREEN_HEIGHT - menu_height) / 2;
    
    Uint32 menu_color = SDL_MapRGB(surface->format, 50, 50, 50);
    draw_pill(surface, menu_x, menu_y, menu_width, menu_height, menu_color, 0);
    
    // Draw menu title
    SDL_Rect title = {
        menu_x + 50,
        menu_y + 20,
        menu_width - 100,
        20
    };
    SDL_FillRect(surface, &title, SDL_MapRGB(surface->format, 255, 255, 255));
    
    // Draw menu items
    int quick_selected = sim->selected_item % 4;
    
    for (int i = 0; i < 4; i++) {
        int y = menu_y + 60 + i * 30;
        int selected = (i == quick_selected);
        
        Uint32 item_color = selected ? 
            SDL_MapRGB(surface->format, 100, 180, 255) : 
            SDL_MapRGB(surface->format, 200, 200, 200);
        
        SDL_Rect item = {
            menu_x + 50,
            y,
            menu_width - 100,
            20
        };
        SDL_FillRect(surface, &item, item_color);
    }
}

// Main render function
void render(simulator_state* sim) {
    // Clear screen
    Uint32 bg_color = SDL_MapRGB(sim->screen->format, 30, 30, 30);
    SDL_FillRect(sim->screen, NULL, bg_color);
    
    // Render appropriate screen
    if (sim->menu_open) {
        render_quick_menu(sim->screen, sim);
    } else {
        render_game_list(sim->screen, sim);
    }
    
    // Update window
    SDL_UpdateWindowSurface(sim->window);
}

// Handle keyboard input
void handle_input(simulator_state* sim) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            sim->running = 0;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (!sim->up_pressed) {
                        sim->selected_item = (sim->selected_item > 0) ? sim->selected_item - 1 : 7;
                        sim->up_pressed = 1;
                    }
                    break;
                case SDLK_DOWN:
                    if (!sim->down_pressed) {
                        sim->selected_item = (sim->selected_item < 7) ? sim->selected_item + 1 : 0;
                        sim->down_pressed = 1;
                    }
                    break;
                case SDLK_a:
                    sim->a_pressed = 1;
                    // Handle selection
                    printf("Selected item: %d\n", sim->selected_item);
                    break;
                case SDLK_b:
                    sim->b_pressed = 1;
                    // Go back
                    sim->menu_open = 0;
                    break;
                case SDLK_MENU:
                case SDLK_m:
                    if (!sim->menu_pressed) {
                        sim->menu_open = !sim->menu_open;
                        sim->selected_item = 0;
                        sim->menu_pressed = 1;
                    }
                    break;
                case SDLK_ESCAPE:
                    sim->running = 0;
                    break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    sim->up_pressed = 0;
                    break;
                case SDLK_DOWN:
                    sim->down_pressed = 0;
                    break;
                case SDLK_a:
                    sim->a_pressed = 0;
                    break;
                case SDLK_b:
                    sim->b_pressed = 0;
                    break;
                case SDLK_MENU:
                case SDLK_m:
                    sim->menu_pressed = 0;
                    break;
            }
        }
    }
}

// Main loop
void main_loop(simulator_state* sim) {
    Uint32 frame_start;
    int frame_time;
    
    while (sim->running) {
        frame_start = SDL_GetTicks();
        
        // Handle input
        handle_input(sim);
        
        // Render
        render(sim);
        
        // Cap FPS
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 1000 / FPS) {
            SDL_Delay((1000 / FPS) - frame_time);
        }
    }
}

// Global simulator state
simulator_state* g_sim = NULL;

int main(int argc, char* argv[]) {
    printf("=== NextUI UI Simulator ===\n");
    printf("Controls:\n");
    printf("  UP/DOWN    - Navigate menu\n");
    printf("  A          - Select item\n");
    printf("  B          - Go back\n");
    printf("  MENU/M    - Toggle quick menu\n");
    printf("  ESC        - Exit\n");
    printf("==========================\n\n");
    
    simulator_state sim;
    memset(&sim, 0, sizeof(sim));
    g_sim = &sim;
    
    // Initialize SDL
    if (!init_sdl(&sim)) {
        return 1;
    }
    
    printf("Simulator started. Press ESC to exit.\n");
    
    // Run main loop
    main_loop(&sim);
    
    // Cleanup
    cleanup_sdl(&sim);
    
    printf("Simulator closed.\n");
    return 0;
}