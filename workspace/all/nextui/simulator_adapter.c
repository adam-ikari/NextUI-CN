#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "defines.h"
#include "api.h"
#include "utils.h"
#include "config.h"
#include "i18n.h"
#include "platform.h"

// Global screen surface for NextUI rendering
SDL_Surface* g_simulator_surface = NULL;
static int g_nextui_initialized = 0;

// Forward declarations from nextui.c (these are static in nextui.c)
// We need to make them accessible or recreate the necessary initialization

// Stub functions for simulator
static void stub_init(void) {
    // Initialize basic systems that don't need hardware
    printf("Initializing NextUI systems for simulator...\n");
    
    // Initialize i18n
    I18N_init();
    printf("  i18n initialized\n");
    
    // Initialize settings (stub)
    InitSettings();
    printf("  settings initialized\n");
    
    g_nextui_initialized = 1;
}

static void stub_quit(void) {
    printf("Shutting down NextUI systems...\n");
    g_nextui_initialized = 0;
}

// Public API for simulator
void init_nextui(void) {
    if (g_nextui_initialized) {
        return;
    }
    stub_init();
}

void quit_nextui(void) {
    if (!g_nextui_initialized) {
        return;
    }
    stub_quit();
}

int main_loop(SDL_Surface* surface) {
    if (!g_nextui_initialized) {
        return -1;
    }
    
    // Update the global surface pointer
    g_simulator_surface = surface;
    
    // Stub: clear surface with background color
    if (surface) {
        // Dark gray background
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 30, 30, 30));
        
        // Draw a simple UI
        SDL_Rect header = {0, 0, surface->w, 40};
        SDL_FillRect(surface, &header, SDL_MapRGB(surface->format, 45, 45, 45));
        
        // Draw "NextUI Simulator" text
        SDL_Rect title_rect = {surface->w/2 - 100, 10, 200, 20};
        // Note: We would need TTF for actual text rendering
        SDL_FillRect(surface, &title_rect, SDL_MapRGB(surface->format, 100, 100, 100));
        
        // Draw some sample menu items
        for (int i = 0; i < 5; i++) {
            int y = 60 + i * 50;
            SDL_Rect item = {20, y, surface->w - 40, 40};
            Uint32 color = (i == 0) ? 
                SDL_MapRGB(surface->format, 70, 70, 70) :  // Selected
                SDL_MapRGB(surface->format, 55, 55, 55);   // Normal
            SDL_FillRect(surface, &item, color);
        }
        
        // Draw status pill
        SDL_Rect status = {surface->w - 120, 5, 110, 30};
        SDL_FillRect(surface, &status, SDL_MapRGB(surface->format, 60, 60, 60));
        
        // Draw button hints
        SDL_Rect hints = {0, surface->h - 30, surface->w, 30};
        SDL_FillRect(surface, &hints, SDL_MapRGB(surface->format, 45, 45, 45));
    }
    
    return 0;
}