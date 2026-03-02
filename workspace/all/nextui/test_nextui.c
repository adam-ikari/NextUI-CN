#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations from nextui
typedef struct screen_manager screen_manager;
typedef struct screen screen;

// Mock functions for testing
extern screen_manager* screen_manager_new(void);
extern void screen_manager_free(screen_manager* mgr);
extern void screen_manager_render(screen_manager* mgr, SDL_Surface* surface);
extern screen* game_list_screen_new(void);
extern screen* game_switcher_screen_new(void);
extern screen* ledcontrol_screen_new(void);

// Test configuration
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TEST_OUTPUT_DIR "test_output"
#define MAX_TESTS 100

// Test case structure
typedef struct {
    const char* name;
    const char* baseline_file;
    void (*test_func)(SDL_Surface* surface);
} test_case;

// Test context
typedef struct {
    SDL_Surface* screen;
    test_case tests[MAX_TESTS];
    int test_count;
    int passed_count;
    int failed_count;
} test_context;

// Initialize test context
test_context* test_context_new(void) {
    test_context* ctx = (test_context*)malloc(sizeof(test_context));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(test_context));
    
    // Create offscreen surface for rendering
    ctx->screen = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!ctx->screen) {
        fprintf(stderr, "Failed to create test surface: %s\n", SDL_GetError());
        free(ctx);
        return NULL;
    }
    
    // Create output directory
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", TEST_OUTPUT_DIR);
    system(cmd);
    
    return ctx;
}

// Free test context
void test_context_free(test_context* ctx) {
    if (!ctx) return;
    
    if (ctx->screen) {
        SDL_FreeSurface(ctx->screen);
    }
    
    free(ctx);
}

// Register a test case
void register_test(test_context* ctx, const char* name, const char* baseline_file, void (*test_func)(SDL_Surface*)) {
    if (ctx->test_count >= MAX_TESTS) {
        fprintf(stderr, "Maximum number of tests reached\n");
        return;
    }
    
    ctx->tests[ctx->test_count].name = name;
    ctx->tests[ctx->test_count].baseline_file = baseline_file;
    ctx->tests[ctx->test_count].test_func = test_func;
    ctx->test_count++;
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

// Compare two screenshots
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
    
    // Create diff surface
    SDL_Surface* diff_surf = SDL_CreateRGBSurfaceWithFormat(0, surf1->w, surf1->h, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_LockSurface(diff_surf);
    Uint32* diff_pixels = (Uint32*)diff_surf->pixels;
    
    for (int i = 0; i < total_pixels; i++) {
        Uint32 p1 = pixels1[i];
        Uint32 p2 = pixels2[i];
        
        if (p1 != p2) {
            different_pixels++;
            diff_pixels[i] = 0xFFFF0000; // Red highlight
        } else {
            diff_pixels[i] = p1;
        }
    }
    
    SDL_UnlockSurface(surf1);
    SDL_UnlockSurface(surf2);
    SDL_UnlockSurface(diff_surf);
    
    // Save diff image
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

// Test functions
void test_empty_screen(SDL_Surface* surface) {
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
}

void test_solid_red(SDL_Surface* surface) {
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 0, 0));
}

void test_gradient(SDL_Surface* surface) {
    for (int y = 0; y < surface->h; y++) {
        Uint32 color = SDL_MapRGB(surface->format, 
            (y * 255) / surface->h,
            255 - (y * 255) / surface->h,
            128);
        SDL_Rect line = {0, y, surface->w, 1};
        SDL_FillRect(surface, &line, color);
    }
}

void test_checkerboard(SDL_Surface* surface) {
    int size = 32;
    for (int y = 0; y < surface->h; y += size) {
        for (int x = 0; x < surface->w; x += size) {
            Uint32 color = ((x / size + y / size) % 2) ? 
                SDL_MapRGB(surface->format, 255, 255, 255) :
                SDL_MapRGB(surface->format, 0, 0, 0);
            SDL_Rect rect = {x, y, size, size};
            SDL_FillRect(surface, &rect, color);
        }
    }
}

void test_text_rendering(SDL_Surface* surface) {
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 30, 30, 30));
    // In real test, this would use font rendering
    SDL_Rect rect = {50, 50, 540, 50};
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 255, 255));
}

void test_button_rendering(SDL_Surface* surface) {
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 30, 30, 30));
    
    // Draw some "buttons"
    SDL_Rect button1 = {50, 50, 200, 50};
    SDL_FillRect(surface, &button1, SDL_MapRGB(surface->format, 100, 100, 255));
    
    SDL_Rect button2 = {50, 120, 200, 50};
    SDL_FillRect(surface, &button2, SDL_MapRGB(surface->format, 100, 255, 100));
    
    SDL_Rect button3 = {50, 190, 200, 50};
    SDL_FillRect(surface, &button3, SDL_MapRGB(surface->format, 255, 100, 100));
}

void test_list_rendering(SDL_Surface* surface) {
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 30, 30, 30));
    
    // Draw list items
    for (int i = 0; i < 5; i++) {
        SDL_Rect item = {50, 50 + i * 60, 540, 50};
        Uint32 color = (i % 2 == 0) ? 
            SDL_MapRGB(surface->format, 60, 60, 60) :
            SDL_MapRGB(surface->format, 80, 80, 80);
        SDL_FillRect(surface, &item, color);
    }
}

void test_status_pill(SDL_Surface* surface) {
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 30, 30, 30));
    
    // Draw status indicators
    SDL_Rect battery = {SCREEN_WIDTH - 150, 10, 100, 30};
    SDL_FillRect(surface, &battery, SDL_MapRGB(surface->format, 0, 255, 0));
    
    SDL_Rect wifi = {SCREEN_WIDTH - 260, 10, 100, 30};
    SDL_FillRect(surface, &wifi, SDL_MapRGB(surface->format, 0, 100, 255));
}

// Run all tests
int run_tests(test_context* ctx, int create_baseline) {
    printf("Running %d tests...\n\n", ctx->test_count);
    
    for (int i = 0; i < ctx->test_count; i++) {
        test_case* test = &ctx->tests[i];
        printf("Test %d: %s\n", i + 1, test->name);
        
        // Clear screen
        SDL_FillRect(ctx->screen, NULL, 0);
        
        // Run test function
        if (test->test_func) {
            test->test_func(ctx->screen);
        }
        
        // Save output
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s_output.png", test->name);
        save_screenshot(ctx, output_file);
        
        if (create_baseline) {
            // Create baseline
            save_screenshot(ctx, test->baseline_file);
            printf("  ✓ Baseline created\n");
            ctx->passed_count++;
        } else {
            // Compare with baseline
            char output_path[512];
            snprintf(output_path, sizeof(output_path), "%s/%s", TEST_OUTPUT_DIR, output_file);
            
            char baseline_path[512];
            snprintf(baseline_path, sizeof(baseline_path), "%s/%s", TEST_OUTPUT_DIR, test->baseline_file);
            
            char diff_path[512];
            snprintf(diff_path, sizeof(diff_path), "%s/%s_diff.png", TEST_OUTPUT_DIR, test->name);
            
            int passed = compare_screenshots(baseline_path, output_path, diff_path, 0.1);
            
            if (passed) {
                printf("  ✓ PASSED\n");
                ctx->passed_count++;
            } else {
                printf("  ✗ FAILED\n");
                ctx->failed_count++;
            }
        }
        
        printf("\n");
    }
    
    return ctx->failed_count == 0 ? 0 : 1;
}

// Print test summary
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
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--create-baseline") == 0) {
            create_baseline = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--create-baseline] [--help]\n", argv[0]);
            printf("  --create-baseline  Create baseline images instead of comparing\n");
            printf("  --help            Show this help message\n");
            return 0;
        }
    }
    
    printf("=== NextUI Regression Test Suite ===\n");
    printf("Mode: %s\n\n", create_baseline ? "Creating baseline" : "Comparing against baseline");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create test context
    test_context* ctx = test_context_new();
    if (!ctx) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Register tests
    register_test(ctx, "empty_screen", "baseline_empty_screen.png", test_empty_screen);
    register_test(ctx, "solid_red", "baseline_solid_red.png", test_solid_red);
    register_test(ctx, "gradient", "baseline_gradient.png", test_gradient);
    register_test(ctx, "checkerboard", "baseline_checkerboard.png", test_checkerboard);
    register_test(ctx, "text_rendering", "baseline_text_rendering.png", test_text_rendering);
    register_test(ctx, "button_rendering", "baseline_button_rendering.png", test_button_rendering);
    register_test(ctx, "list_rendering", "baseline_list_rendering.png", test_list_rendering);
    register_test(ctx, "status_pill", "baseline_status_pill.png", test_status_pill);
    
    // Run tests
    int result = run_tests(ctx, create_baseline);
    
    // Print summary
    print_summary(ctx);
    
    // Cleanup
    test_context_free(ctx);
    IMG_Quit();
    SDL_Quit();
    
    return result;
}
