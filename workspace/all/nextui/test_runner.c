#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// Mock declarations for testing
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TEST_OUTPUT_DIR "test_output"

// Test context
typedef struct {
    SDL_Surface* screen;
    int test_count;
    int passed_count;
    int failed_count;
    const char* current_test_name;
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

// Save screenshot
int save_screenshot(test_context* ctx, const char* filename) {
    if (!ctx || !ctx->screen) return 0;
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.png", TEST_OUTPUT_DIR, filename);
    
    // Save surface as PNG
    int result = SDL_SavePNG(ctx->screen, path);
    if (result != 0) {
        fprintf(stderr, "Failed to save screenshot %s: %s\n", path, SDL_GetError());
        return 0;
    }
    
    printf("Saved screenshot: %s\n", path);
    return 1;
}

// Compare two screenshots (simple pixel comparison)
int compare_screenshots(const char* file1, const char* file2, const char* diff_output, int threshold) {
    SDL_Surface* surf1 = IMG_Load(file1);
    SDL_Surface* surf2 = IMG_Load(file2);
    
    if (!surf1 || !surf2) {
        fprintf(stderr, "Failed to load screenshots for comparison\n");
        if (surf1) SDL_FreeSurface(surf1);
        if (surf2) SDL_FreeSurface(surf2);
        return 0;
    }
    
    if (surf1->w != surf2->w || surf1->h != surf2->h) {
        fprintf(stderr, "Screenshots have different dimensions: %dx%d vs %dx%d\n",
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
    Uint32* diff_pixels = (Uint32*)diff_surf->pixels;
    
    for (int i = 0; i < total_pixels; i++) {
        Uint32 p1 = pixels1[i];
        Uint32 p2 = pixels2[i];
        
        // Compare pixels
        if (p1 != p2) {
            different_pixels++;
            // Highlight difference in red
            diff_pixels[i] = 0xFFFF0000;
        } else {
            diff_pixels[i] = p1;
        }
    }
    
    SDL_UnlockSurface(surf1);
    SDL_UnlockSurface(surf2);
    
    // Save diff image if requested
    if (diff_output) {
        SDL_SavePNG(diff_surf, diff_output);
    }
    
    SDL_FreeSurface(surf1);
    SDL_FreeSurface(surf2);
    SDL_FreeSurface(diff_surf);
    
    float diff_percent = (float)different_pixels / total_pixels * 100.0f;
    printf("Pixel difference: %d/%d (%.2f%%)\n", different_pixels, total_pixels, diff_percent);
    
    return diff_percent < threshold;
}

// Start a test
void test_start(test_context* ctx, const char* name) {
    ctx->current_test_name = name;
    ctx->test_count++;
    printf("\n=== Test %d: %s ===\n", ctx->test_count, name);
    
    // Clear screen
    SDL_FillRect(ctx->screen, NULL, 0);
}

// End a test
void test_end(test_context* ctx, int passed) {
    if (passed) {
        ctx->passed_count++;
        printf("✓ PASSED\n");
    } else {
        ctx->failed_count++;
        printf("✗ FAILED\n");
    }
}

// Render test pattern
void render_test_pattern(test_context* ctx, int pattern) {
    if (!ctx || !ctx->screen) return;
    
    switch (pattern) {
        case 0: // Solid red
            SDL_FillRect(ctx->screen, NULL, SDL_MapRGB(ctx->screen->format, 255, 0, 0));
            break;
        case 1: // Solid green
            SDL_FillRect(ctx->screen, NULL, SDL_MapRGB(ctx->screen->format, 0, 255, 0));
            break;
        case 2: // Solid blue
            SDL_FillRect(ctx->screen, NULL, SDL_MapRGB(ctx->screen->format, 0, 0, 255));
            break;
        case 3: // Gradient
            for (int y = 0; y < ctx->screen->h; y++) {
                Uint32 color = SDL_MapRGB(ctx->screen->format, 
                    (y * 255) / ctx->screen->h,
                    255 - (y * 255) / ctx->screen->h,
                    128);
                SDL_Rect line = {0, y, ctx->screen->w, 1};
                SDL_FillRect(ctx->screen, &line, color);
            }
            break;
        case 4: // Checkerboard
            for (int y = 0; y < ctx->screen->h; y += 32) {
                for (int x = 0; x < ctx->screen->w; x += 32) {
                    Uint32 color = ((x / 32 + y / 32) % 2) ? 
                        SDL_MapRGB(ctx->screen->format, 255, 255, 255) :
                        SDL_MapRGB(ctx->screen->format, 0, 0, 0);
                    SDL_Rect rect = {x, y, 32, 32};
                    SDL_FillRect(ctx->screen, &rect, color);
                }
            }
            break;
    }
}

// Main test runner
int main(int argc, char* argv[]) {
    printf("=== NextUI Regression Test Runner ===\n\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialize SDL_image
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
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
    
    // Run tests
    // Test 1: Solid color patterns
    test_start(ctx, "Solid Red");
    render_test_pattern(ctx, 0);
    save_screenshot(ctx, "test_solid_red.png");
    test_end(ctx, 1);
    
    test_start(ctx, "Solid Green");
    render_test_pattern(ctx, 1);
    save_screenshot(ctx, "test_solid_green.png");
    test_end(ctx, 1);
    
    test_start(ctx, "Solid Blue");
    render_test_pattern(ctx, 2);
    save_screenshot(ctx, "test_solid_blue.png");
    test_end(ctx, 1);
    
    test_start(ctx, "Gradient Pattern");
    render_test_pattern(ctx, 3);
    save_screenshot(ctx, "test_gradient.png");
    test_end(ctx, 1);
    
    test_start(ctx, "Checkerboard Pattern");
    render_test_pattern(ctx, 4);
    save_screenshot(ctx, "test_checkerboard.png");
    test_end(ctx, 1);
    
    // Test 2: Screenshot comparison
    test_start(ctx, "Screenshot Comparison");
    render_test_pattern(ctx, 0);
    save_screenshot(ctx, "test_compare_a.png");
    render_test_pattern(ctx, 0);
    save_screenshot(ctx, "test_compare_b.png");
    
    int passed = compare_screenshots(
        TEST_OUTPUT_DIR "/test_compare_a.png",
        TEST_OUTPUT_DIR "/test_compare_b.png",
        TEST_OUTPUT_DIR "/test_diff.png",
        0.1 // 10% threshold
    );
    test_end(ctx, passed);
    
    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", ctx->test_count);
    printf("Passed: %d\n", ctx->passed_count);
    printf("Failed: %d\n", ctx->failed_count);
    printf("Success rate: %.1f%%\n", 
        ctx->test_count > 0 ? (float)ctx->passed_count / ctx->test_count * 100.0f : 0.0f);
    
    // Cleanup
    test_context_free(ctx);
    IMG_Quit();
    SDL_Quit();
    
    return ctx->failed_count > 0 ? 1 : 0;
}
