/**
 * SDL2_ttf Adaptation Layer
 * 
 * This file provides platform-independent access to SDL2_ttf functions.
 * If SDL2_ttf is not available, it provides stub implementations.
 */

#ifndef SDL2_SDL_TTF_H
#define SDL2_SDL_TTF_H

#include <SDL2/SDL.h>

#ifdef HAS_SDL2_TTF
    // Use real SDL2_ttf if available
    #include_next <SDL2/SDL_ttf.h>
#else
    // Stub implementations when SDL2_ttf is not available
    #define TTF_MAJOR_VERSION 2
    #define TTF_MINOR_VERSION 0
    #define TTF_PATCHLEVEL 18
    
    typedef void TTF_Font;
    typedef struct SDL_RWops SDL_RWops;
    
    #define TTF_Init() 0
    #define TTF_Quit()
    #define TTF_WasInit() 1
    #define TTF_GetError() SDL_GetError()
    
    // Font loading
    static inline TTF_Font* TTF_OpenFont(const char* file, int ptsize) {
        (void)file; (void)ptsize;
        return NULL;
    }
    static inline TTF_Font* TTF_OpenFontIndex(const char* file, int ptsize, long index) {
        (void)file; (void)ptsize; (void)index;
        return NULL;
    }
    static inline TTF_Font* TTF_OpenFontRW(SDL_RWops* src, int freesrc, int ptsize) {
        (void)src; (void)freesrc; (void)ptsize;
        return NULL;
    }
    static inline TTF_Font* TTF_OpenFontIndexRW(SDL_RWops* src, int freesrc, int ptsize, long index) {
        (void)src; (void)freesrc; (void)ptsize; (void)index;
        return NULL;
    }
    static inline void TTF_CloseFont(TTF_Font* font) {
        (void)font;
    }
    
    // Font information
    static inline int TTF_FontHeight(const TTF_Font* font) {
        (void)font;
        return 16;
    }
    static inline int TTF_FontAscent(const TTF_Font* font) {
        (void)font;
        return 12;
    }
    static inline int TTF_FontDescent(const TTF_Font* font) {
        (void)font;
        return 4;
    }
    static inline int TTF_FontLineSkip(const TTF_Font* font) {
        (void)font;
        return 18;
    }
    static inline long TTF_FontFaces(const TTF_Font* font) {
        (void)font;
        return 1;
    }
    static inline int TTF_FontFaceIsFixedWidth(const TTF_Font* font) {
        (void)font;
        return 0;
    }
    static inline char* TTF_FontFaceFamilyName(const TTF_Font* font) {
        (void)font;
        return "Arial";
    }
    static inline char* TTF_FontFaceStyleName(const TTF_Font* font) {
        (void)font;
        return "Regular";
    }
    
    // Font styles
    #define TTF_STYLE_NORMAL 0
    #define TTF_STYLE_BOLD 1
    #define TTF_STYLE_ITALIC 2
    #define TTF_STYLE_UNDERLINE 4
    #define TTF_STYLE_STRIKETHROUGH 8
    
    static inline int TTF_GetFontStyle(const TTF_Font* font) {
        (void)font;
        return TTF_STYLE_NORMAL;
    }
    static inline void TTF_SetFontStyle(TTF_Font* font, int style) {
        (void)font; (void)style;
    }
    static inline int TTF_GetFontOutline(const TTF_Font* font) {
        (void)font;
        return 0;
    }
    static inline void TTF_SetFontOutline(TTF_Font* font, int outline) {
        (void)font; (void)outline;
    }
    static inline int TTF_GetFontHinting(const TTF_Font* font) {
        (void)font;
        return 0;
    }
    static inline void TTF_SetFontHinting(TTF_Font* font, int hinting) {
        (void)font; (void)hinting;
    }
    static inline int TTF_GetFontKerning(const TTF_Font* font) {
        (void)font;
        return 0;
    }
    static inline void TTF_SetFontKerning(TTF_Font* font, int allowed) {
        (void)font; (void)allowed;
    }
    
    // Text rendering
    static inline int TTF_SizeText(TTF_Font* font, const char* text, int* w, int* h) {
        (void)font; (void)text;
        if (w) *w = 100;
        if (h) *h = 16;
        return 0;
    }
    static inline int TTF_SizeUTF8(TTF_Font* font, const char* text, int* w, int* h) {
        (void)font; (void)text;
        if (w) *w = 100;
        if (h) *h = 16;
        return 0;
    }
    
    static inline SDL_Surface* TTF_RenderText_Solid(TTF_Font* font, const char* text, SDL_Color fg) {
        (void)font; (void)text; (void)fg;
        return NULL;
    }
    static inline SDL_Surface* TTF_RenderUTF8_Solid(TTF_Font* font, const char* text, SDL_Color fg) {
        (void)font; (void)text; (void)fg;
        return NULL;
    }
    static inline SDL_Surface* TTF_RenderText_Blended(TTF_Font* font, const char* text, SDL_Color fg) {
        (void)font; (void)text; (void)fg;
        return NULL;
    }
    static inline SDL_Surface* TTF_RenderUTF8_Blended(TTF_Font* font, const char* text, SDL_Color fg) {
        (void)font; (void)text; (void)fg;
        return NULL;
    }
    static inline SDL_Surface* TTF_RenderText_Shaded(TTF_Font* font, const char* text, SDL_Color fg, SDL_Color bg) {
        (void)font; (void)text; (void)fg; (void)bg;
        return NULL;
    }
    static inline SDL_Surface* TTF_RenderUTF8_Shaded(TTF_Font* font, const char* text, SDL_Color fg, SDL_Color bg) {
        (void)font; (void)text; (void)fg; (void)bg;
        return NULL;
    }
    
    // Hinting
    #define TTF_HINTING_NORMAL 0
    #define TTF_HINTING_LIGHT 1
    #define TTF_HINTING_MONO 2
    #define TTF_HINTING_NONE 3
    
#endif

#endif // SDL2_SDL_TTF_H