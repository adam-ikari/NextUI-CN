#ifndef STUB_SDL_IMAGE_H
#define STUB_SDL_IMAGE_H

#include <SDL2/SDL.h>

// Stub for SDL_image functions
#define IMG_Init(flags) 0
#define IMG_Quit() ((void)0)
#define IMG_GetError() SDL_GetError()

// Stub for image loading
#define IMG_LoadPNG_RW(src) NULL
#define IMG_LoadJPG_RW(src) NULL
#define IMG_LoadTyped_RW(src, freesrc, type) NULL

// Stub for image saving - use SDL_SaveBMP instead
static inline int IMG_SavePNG(SDL_Surface *surface, const char *file) {
    // Save as BMP instead of PNG
    char bmp_file[512];
    snprintf(bmp_file, sizeof(bmp_file), "%s", file);
    // If filename ends with .png, replace with .bmp
    size_t len = strlen(file);
    if (len > 4 && strcmp(file + len - 4, ".png") == 0) {
        strncpy(bmp_file, file, len - 4);
        bmp_file[len - 4] = '\0';
        strcat(bmp_file, ".bmp");
    }
    return SDL_SaveBMP(surface, bmp_file);
}

#endif // STUB_SDL_IMAGE_H