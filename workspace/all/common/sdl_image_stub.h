#ifndef SDL_IMAGE_STUB_H
#define SDL_IMAGE_STUB_H

#ifdef NO_SDL2_IMAGE

// Stub functions for SDL2_image when it's not available
#define IMG_Load(path) NULL
#define IMG_Load_RW(rw, freesrc) NULL
#define IMG_isPNG(rw) 0
#define IMG_isJPG(rw) 0
#define IMG_isWEBP(rw) 0
#define IMG_GetError() "SDL2_image not available"

#endif // NO_SDL2_IMAGE

#endif // SDL_IMAGE_STUB_H