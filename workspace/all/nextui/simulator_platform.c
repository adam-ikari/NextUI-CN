/**
 * Simulator Platform Implementation
 * 
 * Provides SDL2-based off-screen rendering for NextUI simulator.
 */

#include "simulator_platform.h"
#include <stdio.h>
#include <stdlib.h>

// Simulator state
static SDL_Surface* g_offscreen_surface = NULL;
static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_texture = NULL;
static int g_initialized = 0;

// Screen dimensions
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int simulator_init(void) {
    printf("Initializing simulator platform...\n");
    
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return 0;
    }
    
#ifdef HAS_SDL2_IMAGE
    IMG_Init(IMG_INIT_PNG);
#endif

#ifdef HAS_SDL2_TTF
    TTF_Init();
#endif
    
    // Create window (visible for debugging)
    g_window = SDL_CreateWindow(
        "NextUI Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!g_window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    // Create renderer
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }
    
    // Create off-screen surface for NextUI rendering
    g_offscreen_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        32,
        SDL_PIXELFORMAT_RGBA8888
    );
    
    if (!g_offscreen_surface) {
        fprintf(stderr, "Failed to create offscreen surface: %s\n", SDL_GetError());
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }
    
    // Create texture for display
    g_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    
    if (!g_texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(g_offscreen_surface);
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }
    
    g_initialized = 1;
    printf("Simulator platform initialized successfully\n");
    printf("  Screen: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    
    return 1;
}

void simulator_cleanup(void) {
    if (!g_initialized) return;
    
    printf("Cleaning up simulator platform...\n");
    
    if (g_texture) {
        SDL_DestroyTexture(g_texture);
        g_texture = NULL;
    }
    
    if (g_offscreen_surface) {
        SDL_FreeSurface(g_offscreen_surface);
        g_offscreen_surface = NULL;
    }
    
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
    
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    
#ifdef HAS_SDL2_TTF
    TTF_Quit();
#endif

#ifdef HAS_SDL2_IMAGE
    IMG_Quit();
#endif

    SDL_Quit();
    g_initialized = 0;
    printf("Simulator platform cleanup complete\n");
}

int simulator_poll_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 0;
        }
    }
    return 1;
}

SDL_Surface* simulator_get_surface(void) {
    return g_offscreen_surface;
}

void simulator_flip(void) {
    if (!g_offscreen_surface || !g_renderer || !g_texture) {
        return;
    }
    
    // Update texture from offscreen surface
    SDL_LockTexture(g_texture, NULL, &g_offscreen_surface->pixels, &g_offscreen_surface->pitch);
    SDL_UnlockTexture(g_texture);
    
    // Render to screen
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
}

int simulator_save_screenshot(const char* filename) {
    if (!g_offscreen_surface) {
        return 0;
    }
    
    char path[512];
    snprintf(path, sizeof(path), "screenshots/%s", filename);
    
    // Create directory if it doesn't exist
    system("mkdir -p screenshots");
    
    // Save as BMP (works without SDL2_image)
    int result = SDL_SaveBMP(g_offscreen_surface, path);
    if (result == 0) {
        printf("  Saved screenshot: %s\n", path);
    } else {
        printf("  Failed to save screenshot: %s\n", path);
    }
    
    return result == 0;
}