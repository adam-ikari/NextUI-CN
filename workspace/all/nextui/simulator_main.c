#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "defines.h"
#include "api.h"
#include "utils.h"
#include "config.h"
#include "i18n.h"

// Screen dimensions (can be adjusted)
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Simulator state
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Surface* offscreen_surface = NULL;
static SDL_Texture* texture = NULL;
static int running = 1;

// Forward declarations from nextui.c
extern int main_loop(SDL_Surface* surface);
extern void init_nextui(void);
extern void quit_nextui(void);

// Initialize SDL for simulator
static int init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 0;
    }
    
    // Create window
    window = SDL_CreateWindow(
        "NextUI Simulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    
    // Create offscreen surface (this is what NextUI will render to)
    offscreen_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        32,
        SDL_PIXELFORMAT_RGBA8888
    );
    
    if (!offscreen_surface) {
        fprintf(stderr, "Failed to create offscreen surface: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    
    // Create texture from surface for display
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    
    if (!texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(offscreen_surface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    
    printf("SDL initialized successfully\n");
    return 1;
}

// Cleanup SDL resources
static void quit_sdl(void) {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (offscreen_surface) {
        SDL_FreeSurface(offscreen_surface);
        offscreen_surface = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

// Main entry point for simulator
int main(int argc, char* argv[]) {
    printf("NextUI UI Simulator\n");
    printf("====================\n\n");
    
    // Initialize SDL
    if (!init_sdl()) {
        return 1;
    }
    
    // Initialize NextUI
    printf("Initializing NextUI...\n");
    init_nextui();
    
    printf("Starting main loop...\n");
    printf("Controls:\n");
    printf("  ESC: Exit\n");
    printf("\n");
    
    // Main loop
    Uint32 last_time = SDL_GetTicks();
    const Uint32 target_fps = 60;
    const Uint32 frame_time = 1000 / target_fps;
    
    while (running) {
        SDL_Event event;
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
                // Pass key events to NextUI if needed
            }
        }
        
        // Run NextUI main loop (this will render to offscreen_surface)
        if (main_loop(offscreen_surface) != 0) {
            running = 0;
        }
        
        // Update texture from offscreen surface
        SDL_LockTexture(texture, NULL, &offscreen_surface->pixels, &offscreen_surface->pitch);
        SDL_UnlockTexture(texture);
        
        // Render to screen
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        // Frame rate control
        Uint32 current_time = SDL_GetTicks();
        Uint32 elapsed = current_time - last_time;
        if (elapsed < frame_time) {
            SDL_Delay(frame_time - elapsed);
        }
        last_time = SDL_GetTicks();
    }
    
    // Cleanup
    printf("Shutting down...\n");
    quit_nextui();
    quit_sdl();
    
    printf("Simulator exited\n");
    return 0;
}