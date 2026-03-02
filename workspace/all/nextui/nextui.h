#ifndef NEXTUI_H__
#define NEXTUI_H__

#include <SDL2/SDL.h>
#include "../common/api.h"
#include "../common/defines.h"

// Theme colors
#define THEME_COLOR1 0x2C3E50
#define THEME_COLOR2 0x34495E
#define THEME_COLOR3 0x1ABC9C
#define THEME_COLOR4 0x2ECC71
#define THEME_COLOR5 0x3498DB
#define THEME_COLOR6 0x9B59B6
#define THEME_COLOR7 0xE74C3C
#define THEME_COLOR8 0xF39C12

// Theme colors with alpha
#define THEME_COLOR1_255 (THEME_COLOR1 | 0xFF000000)
#define THEME_COLOR2_255 (THEME_COLOR2 | 0xFF000000)
#define THEME_COLOR3_255 (THEME_COLOR3 | 0xFF000000)
#define THEME_COLOR4_255 (THEME_COLOR4 | 0xFF000000)
#define THEME_COLOR5_255 (THEME_COLOR5 | 0xFF000000)
#define THEME_COLOR6_255 (THEME_COLOR6 | 0xFF000000)
#define THEME_COLOR7_255 (THEME_COLOR7 | 0xFF000000)
#define THEME_COLOR8_255 (THEME_COLOR8 | 0xFF000000)

// Utility functions
SDL_Color uintToColour(uint32_t color);

#endif // NEXTUI_H__
