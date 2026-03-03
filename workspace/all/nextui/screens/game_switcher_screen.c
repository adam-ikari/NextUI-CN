#include "game_switcher_screen.h"
#include "../common/api.h"
#include "../common/utils.h"
#include "../common/defines.h"
#include "../common/config.h"
#include "../../i18n/i18n.h"
#include <stdlib.h>
#include <string.h>

// Entry type definition (matching nextui.c)
enum EntryType {
    ENTRY_DIR,
    ENTRY_PAK,
    ENTRY_ROM,
    ENTRY_DIP,
};

typedef struct Entry {
    char* path;
    char* name;
    char* display;
    char* unique;
    int type;
    int alpha;
} Entry;

// Animation constants (matching original nextui.c)
enum {
    ANIM_NONE = 0,
    SLIDE_LEFT = 1,
    SLIDE_RIGHT = 2,
};

// External functions (from nextui.c)
extern Array* recents; // RecentArray
extern void saveRecents(void);
extern Entry* entryFromRecent(void* recent);
extern void readyResume(Entry* entry);
extern void Entry_open(void* entry);
extern void Entry_free(void* entry);

// External variables (from nextui.c)
extern int can_resume;
extern int has_preview;
extern char preview_path[256];

GameSwitcherScreen* game_switcher_screen_new(UIState* state, SDL_Surface* screen) {
    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)malloc(sizeof(GameSwitcherScreen));
    if (!screen_data) return NULL;

    screen_data->state = state;
    screen_data->screen = screen;
    screen_data->tmpOldScreen = NULL;
    
    // Initialize game switcher state (matching original nextui.c)
    screen_data->switcher_selected = 0;
    screen_data->can_resume = 0;
    screen_data->has_preview = 0;
    screen_data->preview_path[0] = '\0';
    screen_data->gsanimdir = ANIM_NONE;
    screen_data->last_screen = SCREEN_GAMELIST;
    
    // Get recents from global state
    screen_data->recents = recents;
    
    // Create black background surface
    screen_data->blackBG = SDL_CreateRGBSurfaceWithFormat(0, screen->w, screen->h, 32, SDL_PIXELFORMAT_RGBA8888);
    if (screen_data->blackBG) {
        SDL_FillRect(screen_data->blackBG, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));
    }
    
    screen_data->switcherSur = NULL;

    return screen_data;
}

void game_switcher_screen_render(void* screen_instance, SDL_Surface* surface) {
    if (!screen_instance || !surface) return;

    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)screen_instance;

    // Capture old screen for animation
    if (screen_data->tmpOldScreen) {
        SDL_FreeSurface(screen_data->tmpOldScreen);
    }
    screen_data->tmpOldScreen = GFX_captureRendererToSurface();
    if (screen_data->tmpOldScreen) {
        SDL_SetSurfaceBlendMode(screen_data->tmpOldScreen, SDL_BLENDMODE_BLEND);
    }

    // Clear all layers
    GFX_clearLayers(LAYER_ALL);
    
    int ox = 0;
    int oy = 0;

    // Render recent games carousel
    if (screen_data->recents && screen_data->recents->count > 0) {
        // Get selected entry and check resume state
        Entry* selectedEntry = entryFromRecent(screen_data->recents->items[screen_data->switcher_selected]);
        if (selectedEntry) {
            readyResume(selectedEntry);
            
            // Use global variables set by readyResume
            screen_data->can_resume = can_resume;
            screen_data->has_preview = has_preview;
            strncpy(screen_data->preview_path, preview_path, sizeof(screen_data->preview_path) - 1);
            screen_data->preview_path[sizeof(screen_data->preview_path) - 1] = '\0';
        }

        // Draw title pill
        {
            int ow = GFX_blitHardwareGroup(surface, CFG_getShowSetting());
            int max_width = surface->w - SCALE1(PADDING * 2) - ow;

            char display_name[256];
            const char* name = selectedEntry ? (selectedEntry->display ? selectedEntry->display : selectedEntry->name) : "";
            int text_width = GFX_truncateText(font.large, name, display_name, max_width, SCALE1(BUTTON_PADDING * 2));
            max_width = MIN(max_width, text_width);

            SDL_Surface* text;
            SDL_Color textColor = uintToColour(THEME_COLOR6_255);
            text = TTF_RenderUTF8_Blended(font.large, display_name, textColor);
            
            if (text) {
                const int text_offset_y = (SCALE1(PILL_SIZE) - text->h + 1) / 2 + SCALE1(TEXT_Y_OFFSET);
                GFX_blitPillLight(ASSET_WHITE_PILL, surface, &(SDL_Rect){
                    SCALE1(PADDING),
                    SCALE1(PADDING),
                    max_width,
                    SCALE1(PILL_SIZE)
                });
                
                SDL_BlitSurface(text, &(SDL_Rect){
                    0, 0,
                    max_width - SCALE1(BUTTON_PADDING * 2),
                    text->h
                }, surface, &(SDL_Rect){
                    SCALE1(PADDING + BUTTON_PADDING),
                    SCALE1(PADDING) + text_offset_y,
                });
                SDL_FreeSurface(text);
            }
        }

        // Draw button hints
        if (screen_data->can_resume) {
            GFX_blitButtonGroup((char*[]){
                "B", (char*)TR("common.back"),
                NULL
            }, 0, surface, 0);
        } else {
            GFX_blitButtonGroup((char*[]){
                (char*)(BTN_SLEEP == BTN_POWER ? TR("common.power") : TR("common.menu")),
                (char*)TR("common.sleep"),
                NULL
            }, 0, surface, 0);
        }

        GFX_blitButtonGroup((char*[]){
            "Y", (char*)TR("common.remove"),
            "A", (char*)TR("common.resume"),
            NULL
        }, 1, surface, 1);

        // Draw preview image or placeholder
        if (screen_data->has_preview) {
            SDL_Surface* bmp = IMG_Load(screen_data->preview_path);
            if (bmp) {
                SDL_Surface* raw_preview = SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_RGBA8888, 0);
                if (raw_preview) {
                    SDL_FreeSurface(bmp);
                    bmp = raw_preview;
                }
            }

            if (bmp) {
                // Calculate aspect ratio and position
                int aw = surface->w;
                int ah = surface->h;
                int ax = 0;
                int ay = 0;

                float aspectRatio = (float)bmp->w / (float)bmp->h;
                float screenRatio = (float)surface->w / (float)surface->h;

                if (screenRatio > aspectRatio) {
                    aw = (int)(surface->h * aspectRatio);
                    ah = surface->h;
                } else {
                    aw = surface->w;
                    ah = (int)(surface->w / aspectRatio);
                }
                ax = (surface->w - aw) / 2;
                ay = (surface->h - ah) / 2;

                // Handle animation based on last screen
                if (screen_data->last_screen == SCREEN_GAME) {
                    // Fade in
                    GFX_flipHidden();
                    GFX_animateSurfaceOpacity(bmp, 0, 0, surface->w, surface->h, 0, 255,
                        CFG_getMenuTransitions() ? 150 : 20, LAYER_ALL);
                } else if (screen_data->last_screen == SCREEN_GAMELIST) {
                    // Slide up
                    GFX_drawOnLayer(screen_data->blackBG, 0, 0, surface->w, surface->h, 1.0f, 0, LAYER_BACKGROUND);
                    GFX_drawOnLayer(bmp, ax, ay, aw, ah, 1.0f, 0, LAYER_BACKGROUND);
                    GFX_flipHidden();
                    
                    SDL_Surface* tmpNewScreen = GFX_captureRendererToSurface();
                    GFX_clearLayers(LAYER_ALL);
                    
                    GFX_drawOnLayer(screen_data->tmpOldScreen, 0, 0, surface->w, surface->h, 1.0f, 0, LAYER_ALL);
                    GFX_animateSurface(tmpNewScreen, 0, 0 - surface->h, 0, 0, surface->w, surface->h,
                        CFG_getMenuTransitions() ? 100 : 20, 255, 255, LAYER_BACKGROUND);
                    SDL_FreeSurface(tmpNewScreen);
                } else if (screen_data->last_screen == SCREEN_GAMESWITCHER) {
                    // Slide left/right
                    GFX_flipHidden();
                    GFX_drawOnLayer(screen_data->blackBG, 0, 0, surface->w, surface->h, 1.0f, 0, LAYER_BACKGROUND);
                    
                    if (screen_data->gsanimdir == SLIDE_LEFT) {
                        GFX_animateSurface(bmp, ax + surface->w, ay, ax, ay, aw, ah,
                            CFG_getMenuTransitions() ? 80 : 20, 0, 255, LAYER_ALL);
                    } else if (screen_data->gsanimdir == SLIDE_RIGHT) {
                        GFX_animateSurface(bmp, ax - surface->w, ay, ax, ay, aw, ah,
                            CFG_getMenuTransitions() ? 80 : 20, 0, 255, LAYER_ALL);
                    }
                    
                    GFX_drawOnLayer(bmp, ax, ay, aw, ah, 1.0f, 0, LAYER_BACKGROUND);
                } else if (screen_data->last_screen == SCREEN_QUICKMENU) {
                    // Simple fade
                    GFX_flipHidden();
                    GFX_drawOnLayer(screen_data->blackBG, 0, 0, surface->w, surface->h, 1.0f, 0, LAYER_BACKGROUND);
                    GFX_drawOnLayer(bmp, ax, ay, aw, ah, 1.0f, 0, LAYER_BACKGROUND);
                }

                SDL_FreeSurface(bmp);
            }
        } else {
            // No preview - draw black screen with message
            SDL_Rect preview_rect = {ox, oy, surface->w, surface->h};
            
            if (screen_data->last_screen == SCREEN_GAME) {
                // Fade to black
                GFX_animateSurfaceOpacity(screen_data->blackBG, 0, 0, surface->w, surface->h, 255, 0,
                    CFG_getMenuTransitions() ? 150 : 20, LAYER_BACKGROUND);
            } else if (screen_data->last_screen == SCREEN_GAMELIST) {
                // Slide up
                GFX_animateSurface(screen_data->blackBG, 0, 0 - surface->h, 0, 0, surface->w, surface->h,
                    CFG_getMenuTransitions() ? 100 : 20, 255, 255, LAYER_ALL);
            } else if (screen_data->last_screen == SCREEN_GAMESWITCHER) {
                // Slide left/right
                GFX_flipHidden();
                if (screen_data->gsanimdir == SLIDE_LEFT) {
                    GFX_animateSurface(screen_data->blackBG, 0 + surface->w, 0, 0, 0, surface->w, surface->h,
                        CFG_getMenuTransitions() ? 80 : 20, 0, 255, LAYER_ALL);
                } else if (screen_data->gsanimdir == SLIDE_RIGHT) {
                    GFX_animateSurface(screen_data->blackBG, 0 - surface->w, 0, 0, 0, surface->w, surface->h,
                        CFG_getMenuTransitions() ? 80 : 20, 0, 255, LAYER_ALL);
                }
            }
            
            // Draw "no preview" message
            GFX_blitMessage(font.large, (char*)TR("common.no_preview"), surface, &preview_rect);
        }

        // Free selected entry
        if (selectedEntry) {
            Entry_free(selectedEntry);
        }
    } else {
        // No recent games
        SDL_Rect preview_rect = {ox, oy, surface->w, surface->h};
        SDL_FillRect(surface, &preview_rect, 0);
        GFX_blitMessage(font.large, (char*)TR("common.no_recents"), surface, &preview_rect);
        GFX_blitButtonGroup((char*[]){
            "B", (char*)TR("common.back"),
            NULL
        }, 1, surface, 1);
    }

    GFX_flipHidden();

    // Cache current screen
    if (screen_data->switcherSur) {
        SDL_FreeSurface(screen_data->switcherSur);
        screen_data->switcherSur = NULL;
    }
    screen_data->switcherSur = GFX_captureRendererToSurface();
    screen_data->last_screen = SCREEN_GAMESWITCHER;
}

void game_switcher_screen_handle_input(void* screen_instance, int input) {
    if (!screen_instance) return;

    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)screen_instance;

    switch (input) {
        case BTN_B:
        case BTN_SELECT:
            // Return to game list
            screen_data->switcher_selected = 0;
            ui_state_set_screen(screen_data->state, SCREEN_GAMELIST);
            ui_state_mark_dirty(screen_data->state);
            break;

        case BTN_A:
            // Resume selected game
            if (screen_data->recents && screen_data->recents->count > 0) {
                // Set persistence flag
                putFile(GAME_SWITCHER_PERSIST_PATH, "unused");
                
                // Get selected entry
                Entry* selectedEntry = entryFromRecent(screen_data->recents->items[screen_data->switcher_selected]);
                if (selectedEntry) {
                    // Set resume flag
                    extern int should_resume;
                    should_resume = screen_data->can_resume;
                    
                    // Open game
                    Entry_open(selectedEntry);
                    ui_state_mark_dirty(screen_data->state);
                    Entry_free(selectedEntry);
                }
            }
            break;

        case BTN_Y:
            // Remove from recents
            if (screen_data->recents && screen_data->recents->count > 0) {
                void* recentEntry = screen_data->recents->items[screen_data->switcher_selected--];
                Array_remove(screen_data->recents, recentEntry);
                // Note: Don't free recentEntry here as it's managed by the original code
                saveRecents();
                
                if (screen_data->switcher_selected < 0) {
                    screen_data->switcher_selected = screen_data->recents->count - 1;
                }
                ui_state_mark_dirty(screen_data->state);
            }
            break;

        case BTN_RIGHT:
            // Next game
            if (screen_data->recents && screen_data->recents->count > 0) {
                screen_data->switcher_selected++;
                if (screen_data->switcher_selected == screen_data->recents->count) {
                    screen_data->switcher_selected = 0;
                }
                screen_data->gsanimdir = SLIDE_LEFT;
                ui_state_mark_dirty(screen_data->state);
            }
            break;

        case BTN_LEFT:
            // Previous game
            if (screen_data->recents && screen_data->recents->count > 0) {
                screen_data->switcher_selected--;
                if (screen_data->switcher_selected < 0) {
                    screen_data->switcher_selected = screen_data->recents->count - 1;
                }
                screen_data->gsanimdir = SLIDE_RIGHT;
                ui_state_mark_dirty(screen_data->state);
            }
            break;
    }
}

void game_switcher_screen_update(void* screen_instance) {
    // Update logic if needed
}

void game_switcher_screen_destroy(void* screen_instance) {
    if (!screen_instance) return;

    GameSwitcherScreen* screen_data = (GameSwitcherScreen*)screen_instance;

    if (screen_data->blackBG) {
        SDL_FreeSurface(screen_data->blackBG);
        screen_data->blackBG = NULL;
    }

    if (screen_data->switcherSur) {
        SDL_FreeSurface(screen_data->switcherSur);
        screen_data->switcherSur = NULL;
    }

    if (screen_data->tmpOldScreen) {
        SDL_FreeSurface(screen_data->tmpOldScreen);
        screen_data->tmpOldScreen = NULL;
    }

    // Don't free recents - managed by Menu_init/Menu_quit
    screen_data->recents = NULL;

    free(screen_data);
}

ScreenModule* game_switcher_screen_module_new(UIState* state, SDL_Surface* screen) {
    GameSwitcherScreen* screen_data = game_switcher_screen_new(state, screen);
    if (!screen_data) return NULL;

    return screen_module_new(
        game_switcher_screen_render,
        game_switcher_screen_handle_input,
        game_switcher_screen_destroy,
        game_switcher_screen_update,
        screen_data
    );
}