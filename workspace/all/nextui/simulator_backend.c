/**
 * Desktop Backend for NextUI Simulator
 * 
 * Provides SDL2 off-screen rendering backend for NextUI framework.
 * This replaces the hardware-specific GFX layer with SDL2 surface rendering.
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "platform.h"
#include "api.h"

// Off-screen surface for NextUI rendering
static SDL_Surface* g_offscreen_surface = NULL;
static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_texture = NULL;

// Screen dimensions
static int g_screen_width = 640;
static int g_screen_height = 480;

// Display surface for actual display
static SDL_Surface* g_display_surface = NULL;

// Initialize the desktop backend
int desktop_init(void) {
    printf("Initializing desktop backend for NextUI simulator...\n");
    
    // Initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 0;
    }
    
    // Create window
    g_window = SDL_CreateWindow(
        "NextUI Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        g_screen_width,
        g_screen_height,
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
    
    // Create off-screen surface (this is what NextUI will render to)
    g_offscreen_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        g_screen_width,
        g_screen_height,
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
    
    // Create display surface (for window content)
    g_display_surface = SDL_GetWindowSurface(g_window);
    
    // Create texture for display
    g_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        g_screen_width,
        g_screen_height
    );
    
    if (!g_texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(g_offscreen_surface);
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }
    
    printf("Desktop backend initialized successfully\n");
    printf("  Screen: %dx%d\n", g_screen_width, g_screen_height);
    printf("  Off-screen surface created\n");
    printf("  Window created\n");
    
    return 1;
}

// Get the off-screen surface for NextUI rendering
SDL_Surface* desktop_get_surface(void) {
    return g_offscreen_surface;
}

// Update the display from off-screen surface
void desktop_flip(void) {
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

// Cleanup desktop backend
void desktop_quit(void) {
    printf("Shutting down desktop backend...\n");
    
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
    
    SDL_Quit();
    printf("Desktop backend shutdown complete\n");
}

// Save screenshot of current state
int desktop_save_screenshot(const char* filename) {
    if (!g_offscreen_surface) {
        return 0;
    }
    
    char path[512];
    snprintf(path, sizeof(path), "screenshots/%s", filename);
    
    // Create directory if it doesn't exist
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p screenshots");
    system(cmd);
    
    // Save as BMP (simpler than PNG)
    int result = SDL_SaveBMP(g_offscreen_surface, path);
    if (result == 0) {
        printf("  Saved screenshot: %s\n", path);
    } else {
        printf("  Failed to save screenshot: %s\n", path);
    }
    
    return result == 0;
}

// Handle SDL events
int desktop_poll_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 0; // Quit signal
        }
    }
    return 1; // Continue running
}

// Wait for events (with timeout)
int desktop_wait_events(int timeout_ms) {
    SDL_Event event;
    int result = SDL_WaitEventTimeout(&event, timeout_ms);
    
    if (result == 0) {
        return 1; // Timeout
    } else if (result == 1) {
        if (event.type == SDL_QUIT) {
            return 0; // Quit
        }
        return 1; // Event received
    }
    
    return 0; // Error
}

// Get screen dimensions
void desktop_get_size(int* width, int* height) {
    if (width) *width = g_screen_width;
    if (height) *height = g_screen_height;
}

// Set screen dimensions
void desktop_set_size(int width, int height) {
    g_screen_width = width;
    g_screen_height = height;
    
    if (g_window) {
        SDL_SetWindowSize(g_window, width, height);
    }
}