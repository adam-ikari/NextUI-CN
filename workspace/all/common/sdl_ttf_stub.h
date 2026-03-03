#ifndef SDL_TTF_STUB_H
#define SDL_TTF_STUB_H

#ifdef NO_SDL2_TTF

// Stub type definition for TTF_Font when SDL2_ttf is not available
typedef void TTF_Font;

// Stub constants for SDL2_ttf when it's not available
#define TTF_STYLE_NORMAL 0
#define TTF_STYLE_BOLD 1
#define TTF_STYLE_ITALIC 2
#define TTF_STYLE_UNDERLINE 4
#define TTF_STYLE_STRIKETHROUGH 8

#define TTF_HINTING_NORMAL 0
#define TTF_HINTING_LIGHT 1
#define TTF_HINTING_MONO 2
#define TTF_HINTING_NONE 3

// Stub functions for SDL2_ttf when it's not available
#define TTF_Init() (-1)
#define TTF_Quit() do {} while(0)
#define TTF_WasInit() (0)
#define TTF_OpenFont(file, size) ((TTF_Font*)NULL)
#define TTF_CloseFont(font) do {} while(0)
#define TTF_RenderText_Blended(font, text, fg) ((SDL_Surface*)NULL)
#define TTF_RenderText_Solid(font, text, fg) ((SDL_Surface*)NULL)
#define TTF_RenderUTF8_Blended(font, text, fg) ((SDL_Surface*)NULL)

// Use inline functions for complex stubs to avoid macro issues
static inline int TTF_SizeText(TTF_Font* font, const char* text, int* w, int* h) {
    if (w) *w = 0;
    if (h) *h = 0;
    return 0;
}

static inline int TTF_SizeUTF8(TTF_Font* font, const char* text, int* w, int* h) {
    if (w) *w = 0;
    if (h) *h = 0;
    return 0;
}

static inline int TTF_FontHeight(TTF_Font* font) {
    return 16; // Default font height
}

static inline SDL_Surface* TTF_RenderUTF8_Blended_Wrapped(TTF_Font* font, const char* text, SDL_Color fg, uint32_t wrapLength) {
    return (SDL_Surface*)NULL;
}

#define TTF_SetFontStyle(font, style) do {} while(0)
#define TTF_SetFontHinting(font, hinting) do {} while(0)
#define TTF_GetError() ("SDL2_ttf not available")

#endif // NO_SDL2_TTF

#endif // SDL_TTF_STUB_H