/**
 * NextUI UI Simulator with Screenshot Mode
 * 
 * A desktop simulator for testing NextUI UI components without hardware.
 * Supports automatic page traversal and screenshot capture for visual regression testing.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// SDL_image stub for systems without SDL2_image installed
#ifdef STUB_SDL_IMAGE
#define IMG_SavePNG(surface, file) save_png_stub(surface, file)
#define IMG_Init(flags) 0
#define IMG_Quit()
#define IMG_INIT_PNG 0x00000001
static inline const char* IMG_GetError(void) { return "SDL_image not available"; }
#else
#include <SDL2/SDL_image.h>
#endif

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

// Screen types
typedef enum {
    SCREEN_GAMELIST,
    SCREEN_QUICKMENU,
    SCREEN_GAMESWITCHER,
    SCREEN_SETTINGS,
    SCREEN_COUNT
} screen_type;

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
    
    // Font
    TTF_Font* font;
    int font_size;
    
    // Screenshot mode
    int screenshot_mode;
    int screenshot_index;
    char screenshot_dir[256];
    
    // Auto-traversal mode
    int auto_traverse;
    int traverse_delay;
    Uint32 last_traverse_time;
    
    // Current screen
    screen_type current_screen;
    
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

// Global simulator state
simulator_state* g_sim = NULL;

// Forward declarations
const char* screen_type_name(screen_type type);
int render_text(SDL_Surface* surface, const char* text, int x, int y, Uint32 color, int size);
void cleanup_font(simulator_state* sim);

#ifdef STUB_SDL_IMAGE
// Simple PNG save stub (saves as BMP instead)
static inline int save_png_stub(SDL_Surface* surface, const char* file) {
    // Convert .png extension to .bmp
    char bmp_file[512];
    strncpy(bmp_file, file, sizeof(bmp_file) - 1);
    char* ext = strstr(bmp_file, ".png");
    if (ext) {
        strcpy(ext, ".bmp");
    }
    return SDL_SaveBMP(surface, bmp_file) == 0 ? 0 : -1;
}
#endif

// Initialize font
int init_font(simulator_state* sim) {
    if (TTF_Init() == -1) {
        fprintf(stderr, "Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return 0;
    }
    
    // Try to load a system font
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/System/Library/Fonts/Helvetica.ttc",  // macOS
        "C:\\Windows\\Fonts\\arial.ttf",  // Windows
        NULL
    };
    
    for (int i = 0; font_paths[i] != NULL; i++) {
        sim->font = TTF_OpenFont(font_paths[i], 16);
        if (sim->font) {
            printf("Loaded font: %s\n", font_paths[i]);
            break;
        }
    }
    
    if (!sim->font) {
        fprintf(stderr, "Warning: Could not load any font. Text will not be displayed.\n");
        fprintf(stderr, "Please install a TTF font (e.g., sudo apt-get install fonts-dejavu)\n");
        return 0;
    }
    
    sim->font_size = 16;
    return 1;
}

// Cleanup font
void cleanup_font(simulator_state* sim) {
    if (sim->font) {
        TTF_CloseFont(sim->font);
        sim->font = NULL;
    }
    TTF_Quit();
}

// Render text to surface
int render_text(SDL_Surface* surface, const char* text, int x, int y, Uint32 color, int size) {
    if (!g_sim || !g_sim->font || !text || !surface) {
        return 0;
    }
    
    SDL_Color text_color = {
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF,
        (color >> 24) & 0xFF
    };
    
    SDL_Surface* text_surface = TTF_RenderText_Blended(g_sim->font, text, text_color);
    if (!text_surface) {
        return 0;
    }
    
    SDL_Rect dest_rect = {x, y, text_surface->w, text_surface->h};
    SDL_BlitSurface(text_surface, NULL, surface, &dest_rect);
    
    SDL_FreeSurface(text_surface);
    return 1;
}

// Save screenshot
int save_screenshot(simulator_state* sim, const char* name) {
    if (!sim || !sim->screen) return 0;
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.png", sim->screenshot_dir, name);
    
    // Create directory if it doesn't exist
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", sim->screenshot_dir);
    system(cmd);
    
    // Save surface as PNG
    int result = IMG_SavePNG(sim->screen, path);
    if (result == 0) {
        printf("  Saved: %s\n", path);
    } else {
        printf("  Failed to save: %s\n", path);
    }
    
    return result == 0;
}

// Initialize SDL
int init_sdl(simulator_state* sim) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 0;
    }
    
#ifndef STUB_SDL_IMAGE
    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
        fprintf(stderr, "Screenshot functionality may not work properly.\n");
    }
#endif
    
    // Initialize SDL_ttf
    if (!init_font(sim)) {
        fprintf(stderr, "Warning: Font initialization failed. Text may not display.\n");
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
        cleanup_font(sim);
#ifndef STUB_SDL_IMAGE
        IMG_Quit();
#endif
        SDL_Quit();
        return 0;
    }
    
    // Get window surface
    sim->screen = SDL_GetWindowSurface(sim->window);
    
    sim->running = 1;
    sim->selected_item = 0;
    sim->menu_open = 0;
    sim->current_screen = SCREEN_GAMELIST;
    
    return 1;
}

// Cleanup SDL
void cleanup_sdl(simulator_state* sim) {
    cleanup_font(sim);
    
    if (sim->window) {
        SDL_DestroyWindow(sim->window);
        sim->window = NULL;
    }
    
#ifndef STUB_SDL_IMAGE
    IMG_Quit();
#endif
    
    SDL_Quit();
}

// Draw a rounded pill
void draw_pill(SDL_Surface* surface, int x, int y, int width, int height, Uint32 color, int selected) {
    SDL_Rect rect = {x, y, width, height};
    SDL_FillRect(surface, &rect, color);
    
    if (selected) {
        // Draw highlight border
        SDL_Rect border = {x + 2, y + 2, width - 4, height - 4};
        SDL_FillRect(surface, &border, SDL_MapRGB(surface->format, 255, 255, 255));
    }
}

// Draw button hint
void draw_button_hint(SDL_Surface* surface, const char* button, const char* text, int x, int y) {
    // Draw button name
    render_text(surface, button, x, y, SDL_MapRGB(surface->format, 100, 180, 255), 16);
    
    // Draw hint text
    render_text(surface, text, x + 50, y, SDL_MapRGB(surface->format, 255, 255, 255), 16);
}

// Draw status pill
void draw_status_pill(SDL_Surface* surface) {
    // Battery indicator
    SDL_Rect battery = {SCREEN_WIDTH - 120, 5, 40, 30};
    SDL_FillRect(surface, &battery, SDL_MapRGB(surface->format, 0, 200, 0));
    
    // WiFi indicator
    SDL_Rect wifi = {SCREEN_WIDTH - 170, 5, 40, 30};
    SDL_FillRect(surface, &wifi, SDL_MapRGB(surface->format, 0, 150, 255));
    
    // Clock
    SDL_Rect clock = {SCREEN_WIDTH - 220, 5, 40, 30};
    SDL_FillRect(surface, &clock, SDL_MapRGB(surface->format, 200, 200, 200));
}

// Render game list screen
void render_game_list(SDL_Surface* surface, simulator_state* sim) {
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw header
    SDL_Rect header = {0, 40, SCREEN_WIDTH, 2};
    SDL_FillRect(surface, &header, SDL_MapRGB(surface->format, 100, 100, 100));
    
    // Draw menu items
    int item_height = 50;
    int start_y = 60;
    
    for (int i = 0; i < 8; i++) {
        int y = start_y + i * item_height;
        int selected = (i == sim->selected_item);
        
        Uint32 pill_color = selected ? 
            SDL_MapRGB(surface->format, 70, 70, 70) : 
            SDL_MapRGB(surface->format, 55, 55, 55);
        
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

// Render game switcher screen
void render_game_switcher(SDL_Surface* surface, simulator_state* sim) {
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw header
    SDL_Rect header = {0, 40, SCREEN_WIDTH, 2};
    SDL_FillRect(surface, &header, SDL_MapRGB(surface->format, 100, 100, 100));
    
    // Draw carousel of recent games
    int item_width = 120;
    int item_height = 100;
    int spacing = 20;
    int start_x = (SCREEN_WIDTH - (3 * item_width + 2 * spacing)) / 2;
    
    for (int i = 0; i < 3; i++) {
        int x = start_x + i * (item_width + spacing);
        int y = (SCREEN_HEIGHT - item_height) / 2;
        int selected = (i == sim->selected_item % 3);
        
        Uint32 item_color = selected ? 
            SDL_MapRGB(surface->format, 100, 180, 255) : 
            SDL_MapRGB(surface->format, 55, 55, 55);
        
        draw_pill(surface, x, y, item_width, item_height, item_color, 0);
        
        // Draw placeholder for game thumbnail
        SDL_Rect thumb = {x + 10, y + 10, item_width - 20, item_height - 30};
        SDL_FillRect(surface, &thumb, SDL_MapRGB(surface->format, 30, 30, 30));
    }
    
    // Draw button hints
    draw_button_hint(surface, "A", "Resume", 10, SCREEN_HEIGHT - 30);
    draw_button_hint(surface, "B", "Close", 100, SCREEN_HEIGHT - 30);
    draw_button_hint(surface, "LEFT/RIGHT", "Switch", 200, SCREEN_HEIGHT - 30);
}

// Render settings screen
void render_settings(SDL_Surface* surface, simulator_state* sim) {
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw header
    SDL_Rect header = {0, 40, SCREEN_WIDTH, 2};
    SDL_FillRect(surface, &header, SDL_MapRGB(surface->format, 100, 100, 100));
    
    // Draw settings options
    int option_count = 4;
    int item_height = 50;
    int start_y = 60;
    
    for (int i = 0; i < option_count; i++) {
        int y = start_y + i * item_height;
        int selected = (i == sim->selected_item);
        
        Uint32 pill_color = selected ? 
            SDL_MapRGB(surface->format, 70, 70, 70) : 
            SDL_MapRGB(surface->format, 55, 55, 55);
        
        draw_pill(surface, 10, y, SCREEN_WIDTH - 20, item_height, pill_color, 0);
        
        // Draw option name placeholder
        SDL_Rect name = {30, y + item_height / 2 - 5, 100, 10};
        SDL_FillRect(surface, &name, SDL_MapRGB(surface->format, 255, 255, 255));
    }
    
    // Draw button hints
    draw_button_hint(surface, "A", "Select", 10, SCREEN_HEIGHT - 30);
    draw_button_hint(surface, "B", "Back", 100, SCREEN_HEIGHT - 30);
}

// Main render function
void render(simulator_state* sim) {
    // Clear screen
    Uint32 bg_color = SDL_MapRGB(sim->screen->format, 30, 30, 30);
    SDL_FillRect(sim->screen, NULL, bg_color);
    
    // Render appropriate screen
    switch (sim->current_screen) {
        case SCREEN_GAMELIST:
            render_game_list(sim->screen, sim);
            break;
        case SCREEN_QUICKMENU:
            render_quick_menu(sim->screen, sim);
            break;
        case SCREEN_GAMESWITCHER:
            render_game_switcher(sim->screen, sim);
            break;
        case SCREEN_SETTINGS:
            render_settings(sim->screen, sim);
            break;
        default:
            break;
    }
    
    // Update window
    SDL_UpdateWindowSurface(sim->window);
}

// Auto-traverse and capture screenshots
void auto_traverse_and_capture(simulator_state* sim) {
    Uint32 now = SDL_GetTicks();
    
    if ((Sint32)(now - sim->last_traverse_time) < sim->traverse_delay) {
        return;
    }
    
    sim->last_traverse_time = now;
    
    // Capture screenshot for current state
    char screenshot_name[256];
    snprintf(screenshot_name, sizeof(screenshot_name), "%03d_%s_item%d", 
             sim->screenshot_index,
             screen_type_name(sim->current_screen),
             sim->selected_item);
    
    save_screenshot(sim, screenshot_name);
    sim->screenshot_index++;
    
    // Navigate to next state
    sim->selected_item++;
    
    int max_items = 0;
    switch (sim->current_screen) {
        case SCREEN_GAMELIST:
            max_items = 8;
            if (sim->selected_item >= max_items) {
                sim->selected_item = 0;
                sim->current_screen = SCREEN_QUICKMENU;
            }
            break;
        case SCREEN_QUICKMENU:
            max_items = 4;
            if (sim->selected_item >= max_items) {
                sim->selected_item = 0;
                sim->current_screen = SCREEN_GAMESWITCHER;
            }
            break;
        case SCREEN_GAMESWITCHER:
            max_items = 3;
            if (sim->selected_item >= max_items) {
                sim->selected_item = 0;
                sim->current_screen = SCREEN_SETTINGS;
            }
            break;
        case SCREEN_SETTINGS:
            max_items = 4;
            if (sim->selected_item >= max_items) {
                sim->selected_item = 0;
                sim->current_screen = SCREEN_GAMELIST;
                // All screens traversed, exit
                sim->running = 0;
            }
            break;
    }
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
                case SDLK_LEFT:
                    if (!sim->left_pressed) {
                        sim->selected_item = (sim->selected_item > 0) ? sim->selected_item - 1 : 2;
                        sim->left_pressed = 1;
                    }
                    break;
                case SDLK_RIGHT:
                    if (!sim->right_pressed) {
                        sim->selected_item = (sim->selected_item < 2) ? sim->selected_item + 1 : 0;
                        sim->right_pressed = 1;
                    }
                    break;
                case SDLK_a:
                    sim->a_pressed = 1;
                    // Handle selection
                    printf("Selected item: %d\n", sim->selected_item);
                    break;
                case SDLK_b:
                    sim->b_pressed = 1;
                    // Go back to game list
                    if (sim->current_screen != SCREEN_GAMELIST) {
                        sim->current_screen = SCREEN_GAMELIST;
                        sim->selected_item = 0;
                    }
                    break;
                case SDLK_MENU:
                case SDLK_m:
                    if (!sim->menu_pressed) {
                        if (sim->current_screen == SCREEN_GAMELIST) {
                            sim->current_screen = SCREEN_QUICKMENU;
                        } else if (sim->current_screen == SCREEN_QUICKMENU) {
                            sim->current_screen = SCREEN_GAMELIST;
                        }
                        sim->selected_item = 0;
                        sim->menu_pressed = 1;
                    }
                    break;
                case SDLK_s:
                    // Manual screenshot
                    if (!sim->screenshot_mode) {
                        char screenshot_name[256];
                        snprintf(screenshot_name, sizeof(screenshot_name), "manual_%s_item%d", 
                                 screen_type_name(sim->current_screen),
                                 sim->selected_item);
                        save_screenshot(sim, screenshot_name);
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
                case SDLK_LEFT:
                    sim->left_pressed = 0;
                    break;
                case SDLK_RIGHT:
                    sim->right_pressed = 0;
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
        if (sim->auto_traverse) {
            auto_traverse_and_capture(sim);
        } else {
            handle_input(sim);
        }
        
        // Render
        render(sim);
        
        // Cap FPS
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 1000 / FPS) {
            SDL_Delay((1000 / FPS) - frame_time);
        }
    }
}

// Get screen type name
const char* screen_type_name(screen_type type) {
    switch (type) {
        case SCREEN_GAMELIST: return "gamelist";
        case SCREEN_QUICKMENU: return "quickmenu";
        case SCREEN_GAMESWITCHER: return "gameswitcher";
        case SCREEN_SETTINGS: return "settings";
        case SCREEN_COUNT: return "unknown";
        default: return "unknown";
    }
}

// Print usage
void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("OPTIONS:\n");
    printf("  --screenshot DIR  Enable screenshot mode and save to DIR\n");
    printf("  --auto            Enable automatic traversal mode\n");
    printf("  --delay MS        Delay between screenshots in auto mode (default: 500)\n");
    printf("  --help            Show this help message\n\n");
    printf("CONTROLS:\n");
    printf("  UP/DOWN/LEFT/RIGHT - Navigate\n");
    printf("  A                  - Select\n");
    printf("  B                  - Back\n");
    printf("  MENU/M             - Toggle quick menu\n");
    printf("  S                  - Manual screenshot\n");
    printf("  ESC                - Exit\n\n");
    printf("SCREENS:\n");
    printf("  Game List        - Main game list screen\n");
    printf("  Quick Menu       - Quick menu overlay\n");
    printf("  Game Switcher    - Recent games carousel\n");
    printf("  Settings         - Settings screen\n");
}

// Main entry point
int main(int argc, char* argv[]) {
    printf("=== NextUI UI Simulator ===\n\n");
    
    simulator_state sim;
    memset(&sim, 0, sizeof(sim));
    g_sim = &sim;
    
    // Default values
    snprintf(sim.screenshot_dir, sizeof(sim.screenshot_dir), "screenshots");
    sim.auto_traverse = 0;
    sim.traverse_delay = 500;
    sim.screenshot_mode = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            snprintf(sim.screenshot_dir, sizeof(sim.screenshot_dir), "%s", argv[++i]);
            sim.screenshot_mode = 1;
        } else if (strcmp(argv[i], "--auto") == 0) {
            sim.auto_traverse = 1;
            sim.screenshot_mode = 1;
        } else if (strcmp(argv[i], "--delay") == 0 && i + 1 < argc) {
            sim.traverse_delay = atoi(argv[++i]);
        }
    }
    
    // Print mode information
    if (sim.auto_traverse) {
        printf("Mode: Auto-traversal with screenshots\n");
        printf("Output directory: %s\n", sim.screenshot_dir);
        printf("Delay: %d ms\n\n");
    } else if (sim.screenshot_mode) {
        printf("Mode: Manual screenshots\n");
        printf("Output directory: %s\n", sim.screenshot_dir);
        printf("Press 'S' to capture screenshot\n\n");
    } else {
        printf("Mode: Interactive\n\n");
        print_usage(argv[0]);
        printf("\nStarting interactive mode...\n\n");
    }
    
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
    if (sim.screenshot_mode) {
        printf("Screenshots saved to: %s\n", sim.screenshot_dir);
    }
    
    return 0;
}
