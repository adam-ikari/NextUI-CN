#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test configuration
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TEST_OUTPUT_DIR "test_output"

// Color definitions
#define COLOR_BG 0x1E1E1E
#define COLOR_PILL_BG 0x3C3C3C
#define COLOR_PILL_SELECTED 0x4A4A4A
#define COLOR_TEXT 0xFFFFFF
#define COLOR_HIGHLIGHT 0x007ACC
#define COLOR_BUTTON_PRIMARY 0x4A90E2
#define COLOR_BUTTON_SECONDARY 0x50E3C2

// Font mock
typedef struct {
    int large;
    int medium;
    int small;
} font_mock;

font_mock font = {24, 18, 14};

// Scale mock
#define SCALE1(x) (x)

// Mock constants
#define PADDING 10
#define BUTTON_PADDING 8
#define PILL_SIZE 40
#define MAX_HINTS 8

// Test context
typedef struct {
    SDL_Surface* screen;
    int test_count;
    int passed_count;
    int failed_count;
} test_context;

// Initialize test context
test_context* test_context_new(void) {
    test_context* ctx = (test_context*)malloc(sizeof(test_context));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(test_context));
    
    ctx->screen = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!ctx->screen) {
        free(ctx);
        return NULL;
    }
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", TEST_OUTPUT_DIR);
    system(cmd);
    
    return ctx;
}

void test_context_free(test_context* ctx) {
    if (!ctx) return;
    if (ctx->screen) SDL_FreeSurface(ctx->screen);
    free(ctx);
}

// Helper functions
Uint32 rgb_to_color(SDL_Surface* surface, int r, int g, int b) {
    return SDL_MapRGB(surface->format, r, g, b);
}

void draw_text(SDL_Surface* surface, const char* text, int x, int y, Uint32 color, int size) {
    // Mock text rendering - draw a colored rectangle
    SDL_Rect rect = {x, y, (int)strlen(text) * (size / 2), size};
    SDL_FillRect(surface, &rect, color);
}

void draw_pill(SDL_Surface* surface, int x, int y, int width, int height, Uint32 color, int selected) {
    SDL_Rect rect = {x, y, width, height};
    Uint32 fill_color = selected ? rgb_to_color(surface, 74, 74, 74) : color;
    SDL_FillRect(surface, &rect, fill_color);
}

void draw_button_hint(SDL_Surface* surface, const char* button, const char* text, int x, int y) {
    // Draw button
    SDL_Rect button_rect = {x, y, 40, 30};
    SDL_FillRect(surface, &button_rect, rgb_to_color(surface, 74, 144, 226));
    
    // Draw button text
    draw_text(surface, button, x + 5, y + 5, COLOR_TEXT, 14);
    
    // Draw action text
    draw_text(surface, text, x + 50, y + 5, COLOR_TEXT, 14);
}

void draw_status_pill(SDL_Surface* surface) {
    // Battery indicator
    SDL_Rect battery_rect = {SCREEN_WIDTH - 120, 10, 100, 30};
    SDL_FillRect(surface, &battery_rect, rgb_to_color(surface, 0, 200, 0));
    draw_text(surface, "100%", SCREEN_WIDTH - 110, 15, COLOR_TEXT, 16);
    
    // WiFi indicator
    SDL_Rect wifi_rect = {SCREEN_WIDTH - 230, 10, 100, 30};
    SDL_FillRect(surface, &wifi_rect, rgb_to_color(surface, 0, 150, 255));
    draw_text(surface, "WiFi", SCREEN_WIDTH - 220, 15, COLOR_TEXT, 16);
    
    // Clock
    SDL_Rect clock_rect = {SCREEN_WIDTH - 340, 10, 100, 30};
    SDL_FillRect(surface, &clock_rect, rgb_to_color(surface, 150, 150, 150));
    draw_text(surface, "12:00", SCREEN_WIDTH - 330, 15, COLOR_TEXT, 16);
}

// Test functions

void test_empty_directory(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw empty folder message
    draw_text(surface, "Empty Folder", SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 - 10, COLOR_TEXT, 24);
    
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw button hints
    draw_button_hint(surface, "A", "Open", 10, SCREEN_HEIGHT - 40);
    draw_button_hint(surface, "B", "Back", 10, SCREEN_HEIGHT - 80);
}

void test_game_list(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw list items
    const char* games[] = {"Game 1", "Game 2", "Game 3", "Game 4", "Game 5"};
    int selected = 1;
    
    for (int i = 0; i < 5; i++) {
        int y = 50 + i * 60;
        int is_selected = (i == selected);
        
        // Draw pill background
        draw_pill(surface, 10, y, SCREEN_WIDTH - 20, 50, 
                  rgb_to_color(surface, 60, 60, 60), is_selected);
        
        // Draw game name
        draw_text(surface, games[i], 30, y + 15, COLOR_TEXT, 18);
    }
    
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw button hints
    draw_button_hint(surface, "A", "Open", 10, SCREEN_HEIGHT - 40);
    draw_button_hint(surface, "B", "Back", 10, SCREEN_HEIGHT - 80);
    draw_button_hint(surface, "X", "Resume", 120, SCREEN_HEIGHT - 40);
}

void test_game_list_with_thumbnail(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw thumbnail on the right
    SDL_Rect thumb_rect = {SCREEN_WIDTH - 180, 50, 160, 120};
    SDL_FillRect(surface, &thumb_rect, rgb_to_color(surface, 100, 100, 100));
    draw_text(surface, "Preview", SCREEN_WIDTH - 170, 100, COLOR_TEXT, 16);
    
    // Draw list items
    const char* games[] = {"Game 1", "Game 2", "Game 3"};
    int selected = 0;
    
    for (int i = 0; i < 3; i++) {
        int y = 50 + i * 60;
        int is_selected = (i == selected);
        
        // Draw pill background
        int width = SCREEN_WIDTH - 200;
        draw_pill(surface, 10, y, width, 50, 
                  rgb_to_color(surface, 60, 60, 60), is_selected);
        
        // Draw game name
        draw_text(surface, games[i], 30, y + 15, COLOR_TEXT, 18);
    }
    
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw button hints
    draw_button_hint(surface, "A", "Open", 10, SCREEN_HEIGHT - 40);
    draw_button_hint(surface, "B", "Back", 10, SCREEN_HEIGHT - 80);
}

void test_game_switcher(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw title pill
    draw_pill(surface, 10, 10, SCREEN_WIDTH - 20, 40, rgb_to_color(surface, 60, 60, 60), 0);
    draw_text(surface, "Recent Games", 30, 20, COLOR_TEXT, 18);
    
    // Draw large preview area
    SDL_Rect preview_rect = {20, 70, SCREEN_WIDTH - 40, SCREEN_HEIGHT - 180};
    SDL_FillRect(surface, &preview_rect, rgb_to_color(surface, 50, 50, 50));
    draw_text(surface, "Game Preview", SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2, COLOR_TEXT, 20);
    
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw button hints
    draw_button_hint(surface, "A", "Resume", 10, SCREEN_HEIGHT - 40);
    draw_button_hint(surface, "Y", "Remove", 120, SCREEN_HEIGHT - 40);
    draw_button_hint(surface, "B", "Back", 10, SCREEN_HEIGHT - 80);
}

void test_game_switcher_empty(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw title pill
    draw_pill(surface, 10, 10, SCREEN_WIDTH - 20, 40, rgb_to_color(surface, 60, 60, 60), 0);
    draw_text(surface, "Recent Games", 30, 20, COLOR_TEXT, 18);
    
    // Draw empty message
    draw_text(surface, "No recent games", SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2, COLOR_TEXT, 20);
    
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw button hints
    draw_button_hint(surface, "B", "Back", 10, SCREEN_HEIGHT - 40);
}

void test_button_hints_default(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw title
    draw_text(surface, "Default Button Hints", 20, 20, COLOR_TEXT, 24);
    
    // Draw primary hints (top)
    draw_button_hint(surface, "A", "Open", 20, 80);
    draw_button_hint(surface, "B", "Back", 150, 80);
    
    // Draw secondary hints (bottom)
    draw_button_hint(surface, "START", "Menu", 20, 150);
    draw_button_hint(surface, "SELECT", "Info", 150, 150);
}

void test_button_hints_menu_held(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw title
    draw_text(surface, "Combined Key Hints (Menu Held)", 20, 20, COLOR_TEXT, 24);
    
    // Draw brightness/volume hints
    draw_button_hint(surface, "START", "Brightness", 20, 80);
    draw_button_hint(surface, "SELECT", "Color Temp", 20, 130);
}

void test_status_pill_only(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw only status pill
    draw_status_pill(surface);
    
    // Draw message
    draw_text(surface, "Status Pill Test", SCREEN_WIDTH/2 - 70, SCREEN_HEIGHT/2, COLOR_TEXT, 20);
}

void test_status_pill_with_hints(SDL_Surface* surface) {
    // Clear screen
    SDL_FillRect(surface, NULL, rgb_to_color(surface, 30, 30, 30));
    
    // Draw status pill
    draw_status_pill(surface);
    
    // Draw hints
    draw_button_hint(surface, "A", "Action", 10, SCREEN_HEIGHT - 40);
    draw_button_hint(surface, "B", "Back", 120, SCREEN_HEIGHT - 40);
    
    // Draw message
    draw_text(surface, "Status + Hints Test", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2, COLOR_TEXT, 20);
}

// Save screenshot
int save_screenshot(test_context* ctx, const char* filename) {
    if (!ctx || !ctx->screen) return 0;
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.png", TEST_OUTPUT_DIR, filename);
    
    int result = SDL_SavePNG(ctx->screen, path);
    if (result != 0) {
        fprintf(stderr, "Failed to save screenshot %s: %s\n", path, SDL_GetError());
        return 0;
    }
    
    printf("  Saved: %s\n", path);
    return 1;
}

// Compare screenshots
int compare_screenshots(const char* file1, const char* file2, const char* diff_output, float threshold) {
    SDL_Surface* surf1 = IMG_Load(file1);
    SDL_Surface* surf2 = IMG_Load(file2);
    
    if (!surf1 || !surf2) {
        fprintf(stderr, "  Failed to load screenshots for comparison\n");
        if (surf1) SDL_FreeSurface(surf1);
        if (surf2) SDL_FreeSurface(surf2);
        return 0;
    }
    
    if (surf1->w != surf2->w || surf1->h != surf2->h) {
        fprintf(stderr, "  Screenshots have different dimensions: %dx%d vs %dx%d\n",
                surf1->w, surf1->h, surf2->w, surf2->h);
        SDL_FreeSurface(surf1);
        SDL_FreeSurface(surf2);
        return 0;
    }
    
    int different_pixels = 0;
    int total_pixels = surf1->w * surf1->h;
    
    SDL_LockSurface(surf1);
    SDL_LockSurface(surf2);
    
    Uint32* pixels1 = (Uint32*)surf1->pixels;
    Uint32* pixels2 = (Uint32*)surf2->pixels;
    
    SDL_Surface* diff_surf = SDL_CreateRGBSurfaceWithFormat(0, surf1->w, surf1->h, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_LockSurface(diff_surf);
    Uint32* diff_pixels = (Uint32*)diff_surf->pixels;
    
    for (int i = 0; i < total_pixels; i++) {
        Uint32 p1 = pixels1[i];
        Uint32 p2 = pixels2[i];
        
        if (p1 != p2) {
            different_pixels++;
            diff_pixels[i] = 0xFFFF0000;
        } else {
            diff_pixels[i] = p1;
        }
    }
    
    SDL_UnlockSurface(surf1);
    SDL_UnlockSurface(surf2);
    SDL_UnlockSurface(diff_surf);
    
    if (diff_output) {
        SDL_SavePNG(diff_surf, diff_output);
        printf("  Diff saved: %s\n", diff_output);
    }
    
    SDL_FreeSurface(surf1);
    SDL_FreeSurface(surf2);
    SDL_FreeSurface(diff_surf);
    
    float diff_percent = (float)different_pixels / total_pixels * 100.0f;
    printf("  Difference: %.2f%%\n", diff_percent);
    
    return diff_percent < threshold;
}

// Run all tests
int run_page_tests(test_context* ctx, int create_baseline) {
    printf("Running page tests...\n\n");
    
    struct {
        const char* name;
        const char* baseline;
        void (*test_func)(SDL_Surface*);
    } tests[] = {
        {"empty_directory", "baseline_empty_directory.png", test_empty_directory},
        {"game_list", "baseline_game_list.png", test_game_list},
        {"game_list_with_thumbnail", "baseline_game_list_with_thumbnail.png", test_game_list_with_thumbnail},
        {"game_switcher", "baseline_game_switcher.png", test_game_switcher},
        {"game_switcher_empty", "baseline_game_switcher_empty.png", test_game_switcher_empty},
        {"button_hints_default", "baseline_button_hints_default.png", test_button_hints_default},
        {"button_hints_menu_held", "baseline_button_hints_menu_held.png", test_button_hints_menu_held},
        {"status_pill_only", "baseline_status_pill_only.png", test_status_pill_only},
        {"status_pill_with_hints", "baseline_status_pill_with_hints.png", test_status_pill_with_hints},
    };
    
    int test_count = sizeof(tests) / sizeof(tests[0]);
    
    for (int i = 0; i < test_count; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].name);
        
        SDL_FillRect(ctx->screen, NULL, 0);
        
        if (tests[i].test_func) {
            tests[i].test_func(ctx->screen);
        }
        
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s_output.png", tests[i].name);
        save_screenshot(ctx, output_file);
        
        if (create_baseline) {
            save_screenshot(ctx, tests[i].baseline);
            printf("  ✓ Baseline created\n");
            ctx->passed_count++;
        } else {
            char output_path[512];
            snprintf(output_path, sizeof(output_path), "%s/%s", TEST_OUTPUT_DIR, output_file);
            
            char baseline_path[512];
            snprintf(baseline_path, sizeof(baseline_path), "%s/%s", TEST_OUTPUT_DIR, tests[i].baseline);
            
            char diff_path[512];
            snprintf(diff_path, sizeof(diff_path), "%s/%s_diff.png", TEST_OUTPUT_DIR, tests[i].name);
            
            int passed = compare_screenshots(baseline_path, output_path, diff_path, 0.1);
            
            if (passed) {
                printf("  ✓ PASSED\n");
                ctx->passed_count++;
            } else {
                printf("  ✗ FAILED\n");
                ctx->failed_count++;
            }
        }
        
        ctx->test_count++;
        printf("\n");
    }
    
    return ctx->failed_count == 0 ? 0 : 1;
}

void print_summary(test_context* ctx) {
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", ctx->test_count);
    printf("Passed: %d\n", ctx->passed_count);
    printf("Failed: %d\n", ctx->failed_count);
    printf("Success rate: %.1f%%\n", 
        ctx->test_count > 0 ? (float)ctx->passed_count / ctx->test_count * 100.0f : 0.0f);
}

int main(int argc, char* argv[]) {
    int create_baseline = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--create-baseline") == 0) {
            create_baseline = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--create-baseline] [--help]\n", argv[0]);
            printf("  --create-baseline  Create baseline images\n");
            printf("  --help            Show this help\n");
            return 0;
        }
    }
    
    printf("=== NextUI Page Regression Tests ===\n");
    printf("Mode: %s\n\n", create_baseline ? "Creating baseline" : "Comparing against baseline");
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    
    test_context* ctx = test_context_new();
    if (!ctx) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    int result = run_page_tests(ctx, create_baseline);
    print_summary(ctx);
    
    test_context_free(ctx);
    IMG_Quit();
    SDL_Quit();
    
    return result;
}
