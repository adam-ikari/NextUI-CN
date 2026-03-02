/**
 * Platform Adaptation Layer for NextUI Simulator
 * 
 * This file provides platform-specific adaptations for the simulator,
 * ensuring the NextUI framework remains platform-independent.
 */

#ifndef SIMULATOR_PLATFORM_H
#define SIMULATOR_PLATFORM_H

#include <SDL2/SDL.h>

// SDL2_image adaptation - handle missing SDL2_image gracefully
#ifdef HAS_SDL2_IMAGE
    #include <SDL2/SDL_image.h>
#else
    // Stub functions for SDL2_image
    static inline SDL_Surface* IMG_Load(const char* file) {
        (void)file;
        return NULL;
    }
    static inline int IMG_SavePNG(SDL_Surface* surface, const char* file) {
        // Fall back to SDL_SaveBMP
        return SDL_SaveBMP(surface, file);
    }
    #define IMG_Init(flags) 0
    #define IMG_Quit()
#endif

// SDL2_ttf adaptation - handle missing SDL2_ttf gracefully
#ifdef HAS_SDL2_TTF
    #include <SDL2/SDL_ttf.h>
#else
    // Basic stub for SDL2_ttf
    typedef void TTF_Font;
    static inline int TTF_Init(void) { return 0; }
    static inline void TTF_Quit(void) {}
    static inline TTF_Font* TTF_OpenFont(const char* file, int ptsize) {
        (void)file; (void)ptsize; return NULL;
    }
    static inline void TTF_CloseFont(TTF_Font* font) { (void)font; }
    static inline int TTF_FontHeight(const TTF_Font* font) {
        (void)font; return 16;
    }
    #define TTF_STYLE_NORMAL 0
    #define TTF_STYLE_BOLD 1
    #define TTF_STYLE_ITALIC 2
#endif

// Simulator-specific functions
int simulator_init(void);
void simulator_cleanup(void);
int simulator_poll_events(void);
SDL_Surface* simulator_get_surface(void);
void simulator_flip(void);
int simulator_save_screenshot(const char* filename);

#endif // SIMULATOR_PLATFORM_H