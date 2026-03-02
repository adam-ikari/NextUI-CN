/**
 * SDL2_image Adaptation Layer
 * 
 * This file provides platform-independent access to SDL2_image functions.
 * If SDL2_image is not available, it provides stub implementations.
 */

#ifndef SDL2_SDL_IMAGE_H
#define SDL2_SDL_IMAGE_H

#include <SDL2/SDL.h>

#ifdef HAS_SDL2_IMAGE
    // Use real SDL2_image if available
    #include_next <SDL2/SDL_image.h>
#else
    // Stub implementations when SDL2_image is not available
    #define IMG_Init(flags) 0
    #define IMG_Quit()
    #define IMG_GetError() "SDL2_image not available"
    #define IMG_INIT_PNG 0x00000001
    #define IMG_INIT_JPG 0x00000002
    #define IMG_INIT_TIF 0x00000004
    #define IMG_INIT_WEBP 0x00000008
    
    static inline SDL_Surface* IMG_Load(const char* file) {
        (void)file;
        return NULL;
    }
    
    static inline SDL_Surface* IMG_Load_RW(SDL_RWops* src, int freesrc) {
        (void)src; (void)freesrc;
        return NULL;
    }
    
    static inline SDL_Surface* IMG_LoadTyped_RW(SDL_RWops* src, int freesrc, const char* type) {
        (void)src; (void)freesrc; (void)type;
        return NULL;
    }
    
    static inline int IMG_SavePNG(SDL_Surface* surface, const char* file) {
        // Fall back to BMP
        return SDL_SaveBMP(surface, file);
    }
#endif

#endif // SDL2_SDL_IMAGE_H