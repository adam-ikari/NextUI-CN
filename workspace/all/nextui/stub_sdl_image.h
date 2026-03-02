// Stub for SDL2_image.h (for simulator use)
#ifndef STUB_SDL_IMAGE_H
#define STUB_SDL_IMAGE_H

#include <SDL2/SDL.h>

typedef struct SDL_Surface SDL_Surface;

// Stub functions
static inline SDL_Surface* IMG_Load(const char* file) {
    (void)file;
    return NULL;
}

static inline int IMG_Init(int flags) {
    (void)flags;
    return 0;
}

static inline void IMG_Quit(void) {
}

#define IMG_INIT_PNG 0x00000001
#define IMG_INIT_JPG 0x00000002

#endif // STUB_SDL_IMAGE_H