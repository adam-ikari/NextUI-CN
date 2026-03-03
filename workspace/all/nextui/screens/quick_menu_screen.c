#include "quick_menu_screen.h"
#include "../common/api.h"
#include "../common/utils.h"
#include "../common/defines.h"
#include "../common/config.h"
#include "../../i18n/i18n.h"
#include <stdlib.h>
#include <string.h>

// External declarations
extern int GetHDMI(void);
extern int CFG_getShowSetting(void);

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

// Constants from original nextui.c
#define MENU_ITEM_SIZE 72
#define MENU_MARGIN_Y 32
#define MENU_MARGIN_X 40
#define MENU_ITEM_MARGIN 18
#define MENU_TOGGLE_MARGIN 8
#define MENU_LINE_MARGIN 8

// External functions (from nextui.c)
extern Array* getQuickEntries(void);
extern Array* getQuickToggles(void);
extern void Entry_open(void* entry);

QuickMenuScreen* quick_menu_screen_new(UIState* state, SDL_Surface* screen) {
    QuickMenuScreen* screen_data = (QuickMenuScreen*)malloc(sizeof(QuickMenuScreen));
    if (!screen_data) return NULL;

    screen_data->state = state;
    screen_data->screen = screen;
    screen_data->background = NULL;
    screen_data->folderBgPath = NULL;

    // Initialize quick menu state (matching original nextui.c)
    screen_data->qm_row = 0;
    screen_data->qm_col = 0;
    screen_data->qm_slot = 0;
    screen_data->qm_shift = 0;

    // Get quick entries and actions
    screen_data->quick = getQuickEntries();
    screen_data->quickActions = getQuickToggles();

    // Calculate slots
    screen_data->qm_slots = QUICK_SWITCHER_COUNT > screen_data->quick->count 
        ? screen_data->quick->count 
        : QUICK_SWITCHER_COUNT;

    // Allocate background path buffer
    screen_data->folderBgPath = (char*)malloc(MAX_PATH * sizeof(char));
    if (screen_data->folderBgPath) {
        strncpy(screen_data->folderBgPath, "", MAX_PATH - 1);
        screen_data->folderBgPath[MAX_PATH - 1] = '\0';
    }

    return screen_data;
}

void quick_menu_screen_render(void* screen_instance, SDL_Surface* surface) {
    if (!screen_instance || !surface) return;

    QuickMenuScreen* screen_data = (QuickMenuScreen*)screen_instance;

    // Clear layers for quick menu
    GFX_clearLayers(LAYER_BACKGROUND);
    GFX_clearLayers(LAYER_THUMBNAIL);

    // Get current selected entry
    Entry* current = screen_data->qm_row == 0 
        ? (Entry*)screen_data->quick->items[screen_data->qm_col] 
        : (Entry*)screen_data->quickActions->items[screen_data->qm_col];

    // Load background based on current selection
    char newBgPath[MAX_PATH];
    char fallbackBgPath[MAX_PATH];
    sprintf(newBgPath, SDCARD_PATH "/.media/quick_%s%s.png", current->name,
        (!strcmp(current->name, "Wifi") && CFG_getWifi() ||
         !strcmp(current->name, "Bluetooth") && CFG_getBluetooth()) ? "_off" : "");
    sprintf(fallbackBgPath, SDCARD_PATH "/.media/quick.png");

    // Use fallback if custom background doesn't exist
    if (!exists(newBgPath)) {
        strncpy(newBgPath, fallbackBgPath, sizeof(newBgPath) - 1);
        newBgPath[sizeof(newBgPath) - 1] = '\0';
    }

    // Update background if changed
    if (strcmp(newBgPath, screen_data->folderBgPath) != 0) {
        strncpy(screen_data->folderBgPath, newBgPath, MAX_PATH - 1);
        screen_data->folderBgPath[MAX_PATH - 1] = '\0';
        
        // Load background image
        if (screen_data->background) {
            SDL_FreeSurface(screen_data->background);
            screen_data->background = NULL;
        }
        screen_data->background = IMG_Load(newBgPath);
    }

    // Draw background
    if (screen_data->background) {
        SDL_BlitSurface(screen_data->background, NULL, surface, NULL);
    }

    // Draw hardware hints if showing setting
    int show_setting = CFG_getShowSetting();
    if (show_setting && !GetHDMI()) {
        GFX_blitHardwareHints(surface, show_setting);
    } else {
        GFX_blitButtonGroup((char*[]){
            (char*)(BTN_SLEEP == BTN_POWER ? TR("common.power") : TR("common.menu")),
            (char*)TR("common.sleep"),
            NULL
        }, 0, surface, 0);
    }

    // Draw action buttons
    GFX_blitButtonGroup((char*[]){
        "B", (char*)TR("common.back"),
        "A", (char*)TR("common.open"),
        NULL
    }, 1, surface, 1);

    // Check if quick switcher UI is enabled
    if (!CFG_getShowQuickswitcherUI()) {
        return;
    }

    // Calculate layout
    int item_space_y = surface->h - SCALE1(PADDING + PILL_SIZE + BUTTON_MARGIN +
        MENU_MARGIN_Y + MENU_LINE_MARGIN + PILL_SIZE + MENU_MARGIN_Y +
        BUTTON_MARGIN + PILL_SIZE + PADDING);
    int item_size = SCALE1(MENU_ITEM_SIZE);
    int item_extra_y = item_space_y - item_size;
    int item_space_x = surface->w - SCALE1(PADDING + MENU_MARGIN_X + MENU_MARGIN_X + PADDING);
    int item_inset_x = (item_space_x - SCALE1(screen_data->qm_slots * MENU_ITEM_SIZE +
        (screen_data->qm_slots - 1) * MENU_ITEM_MARGIN)) / 2;

    // Render primary row (quick entries)
    int ox = SCALE1(PADDING + MENU_MARGIN_X) + item_inset_x;
    int oy = SCALE1(PADDING + PILL_SIZE + BUTTON_MARGIN + MENU_MARGIN_Y) + item_extra_y / 2;
    
    // Apply horizontal shift for scrolling
    ox -= screen_data->qm_shift * (item_size + SCALE1(MENU_ITEM_MARGIN));

    for (int c = 0; c < screen_data->quick->count; c++) {
        SDL_Rect item_rect = {ox, oy, item_size, item_size};
        Entry* item = (Entry*)screen_data->quick->items[c];

        SDL_Color text_color = uintToColour(THEME_COLOR4_255);
        uint32_t item_color = THEME_COLOR3;
        uint32_t icon_color = THEME_COLOR4;

        if (screen_data->qm_row == 0 && screen_data->qm_col == c) {
            text_color = uintToColour(THEME_COLOR5_255);
            item_color = THEME_COLOR1;
            icon_color = THEME_COLOR5;
        }

        // Draw item background
        GFX_blitRectColor(ASSET_STATE_BG, surface, &item_rect, item_color);

        // Load and draw icon
        char icon_path[MAX_PATH];
        sprintf(icon_path, SDCARD_PATH "/.system/res/%s@%ix.png", item->name, FIXED_SCALE);
        SDL_Surface* bmp = IMG_Load(icon_path);
        if (bmp) {
            SDL_Surface* converted = SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_RGBA8888, 0);
            if (converted) {
                SDL_FreeSurface(bmp);
                bmp = converted;
            }
        }

        if (bmp) {
            int x = (item_rect.w - bmp->w) / 2;
            int y = (item_rect.h - SCALE1(FONT_TINY + BUTTON_MARGIN) - bmp->h) / 2;
            SDL_Rect destRect = {ox + x, oy + y, 0, 0};
            GFX_blitSurfaceColor(bmp, NULL, surface, &destRect, icon_color);
            SDL_FreeSurface(bmp);
        }

        // Draw label text
        int w, h;
        const char* label = item->display ? item->display : item->name;
        GFX_sizeText(font.tiny, label, SCALE1(FONT_TINY), &w, &h);
        SDL_Rect text_rect = {
            item_rect.x + (item_size - w) / 2,
            item_rect.y + item_size - h - SCALE1(BUTTON_MARGIN),
            w, h
        };
        GFX_blitText(font.tiny, label, SCALE1(FONT_TINY), text_color, surface, &text_rect);

        ox += item_rect.w + SCALE1(MENU_ITEM_MARGIN);
    }

    // Render secondary row (quick actions)
    ox = SCALE1(PADDING + MENU_MARGIN_X);
    ox += (surface->w - SCALE1(PADDING + MENU_MARGIN_X + MENU_MARGIN_X + PADDING) -
        SCALE1(screen_data->quickActions->count * PILL_SIZE) -
        SCALE1((screen_data->quickActions->count - 1) * MENU_TOGGLE_MARGIN)) / 2;
    oy = SCALE1(PADDING + PILL_SIZE + BUTTON_MARGIN + MENU_MARGIN_Y + MENU_LINE_MARGIN) +
        item_size + item_extra_y / 2;

    for (int c = 0; c < screen_data->quickActions->count; c++) {
        SDL_Rect item_rect = {ox, oy, SCALE1(PILL_SIZE), SCALE1(PILL_SIZE)};
        Entry* item = (Entry*)screen_data->quickActions->items[c];

        SDL_Color text_color = uintToColour(THEME_COLOR4_255);
        uint32_t item_color = THEME_COLOR3;
        uint32_t icon_color = THEME_COLOR4;

        if (screen_data->qm_row == 1 && screen_data->qm_col == c) {
            text_color = uintToColour(THEME_COLOR5_255);
            item_color = THEME_COLOR1;
            icon_color = THEME_COLOR5;
        }

        // Draw pill background
        GFX_blitPillColor(ASSET_WHITE_PILL, surface, &item_rect, item_color, RGB_WHITE);

        // Determine which asset to display
        int asset = ASSET_WIFI;
        if (!strcmp(item->name, "Wifi")) {
            asset = CFG_getWifi() ? ASSET_WIFI_OFF : ASSET_WIFI;
        } else if (!strcmp(item->name, "Bluetooth")) {
            asset = CFG_getBluetooth() ? ASSET_BLUETOOTH_OFF : ASSET_BLUETOOTH;
        } else if (!strcmp(item->name, "Sleep")) {
            asset = ASSET_SUSPEND;
        } else if (!strcmp(item->name, "Reboot")) {
            asset = ASSET_RESTART;
        } else if (!strcmp(item->name, "Poweroff")) {
            asset = ASSET_POWEROFF;
        } else if (!strcmp(item->name, "Settings")) {
            asset = ASSET_SETTINGS;
        } else if (!strcmp(item->name, "Pak Store")) {
            asset = ASSET_STORE;
        }

        // Draw icon centered in pill
        SDL_Rect rect;
        GFX_assetRect(asset, &rect);
        int x = item_rect.x + (SCALE1(PILL_SIZE) - rect.w) / 2;
        int y = item_rect.y + (SCALE1(PILL_SIZE) - rect.h) / 2;
        SDL_Rect destRect = {x, y, rect.w, rect.h};
        GFX_blitAssetColor(asset, NULL, surface, &destRect, icon_color);

        ox += SCALE1(PILL_SIZE) + SCALE1(MENU_TOGGLE_MARGIN);
    }
}

void quick_menu_screen_handle_input(void* screen_instance, int input) {
    if (!screen_instance) return;

    QuickMenuScreen* screen_data = (QuickMenuScreen*)screen_instance;
    int qm_total = screen_data->qm_row == 0 
        ? screen_data->quick->count 
        : screen_data->quickActions->count;

    switch (input) {
        case BTN_B:
        case BTN_MENU:
            // Return to game list
            ui_state_set_screen(screen_data->state, SCREEN_GAMELIST);
            break;

        case BTN_A:
            // Open selected entry
            {
                Entry* selected = screen_data->qm_row == 0 
                    ? (Entry*)screen_data->quick->items[screen_data->qm_col]
                    : (Entry*)screen_data->quickActions->items[screen_data->qm_col];
                
                if (selected->type != ENTRY_DIP) {
                    // Return to game list and open entry
                    ui_state_set_screen(screen_data->state, SCREEN_GAMELIST);
                    // Note: The original code resets game list state here
                    // We'll need to handle this through the renderer/app
                    Entry_open(selected);
                } else {
                    // Handle DIP (toggle) entries
                    Entry_open(selected);
                }
            }
            break;

        case BTN_RIGHT:
            if (screen_data->qm_row == 0 && qm_total > screen_data->qm_slots) {
                screen_data->qm_col++;
                if (screen_data->qm_col >= qm_total) {
                    screen_data->qm_col = 0;
                    screen_data->qm_shift = 0;
                    screen_data->qm_slot = 0;
                } else {
                    screen_data->qm_slot++;
                    if (screen_data->qm_slot >= screen_data->qm_slots) {
                        screen_data->qm_slot = screen_data->qm_slots - 1;
                        screen_data->qm_shift++;
                    }
                }
                ui_state_mark_dirty(screen_data->state);
                            } else if (screen_data->qm_row == 1) {
                            screen_data->qm_col += 1;
                            if (screen_data->qm_col >= qm_total) {
                                screen_data->qm_col = 0;
                            }
                            ui_state_mark_dirty(screen_data->state);            }
            break;

        case BTN_LEFT:
            if (screen_data->qm_row == 0 && qm_total > screen_data->qm_slots) {
                screen_data->qm_col -= 1;
                if (screen_data->qm_col < 0) {
                    screen_data->qm_col = qm_total - 1;
                    screen_data->qm_shift = qm_total - screen_data->qm_slots;
                    screen_data->qm_slot = screen_data->qm_slots - 1;
                } else {
                    screen_data->qm_slot--;
                    if (screen_data->qm_slot < 0) {
                        screen_data->qm_slot = 0;
                        screen_data->qm_shift--;
                    }
                }
                ui_state_mark_dirty(screen_data->state);
            } else if (screen_data->qm_row == 1) {
                screen_data->qm_col -= 1;
                if (screen_data->qm_col < 0) {
                    screen_data->qm_col = qm_total - 1;
                }
                ui_state_mark_dirty(screen_data->state);
            }
            break;

        case BTN_DOWN:
            if (screen_data->qm_row == 0) {
                screen_data->qm_row = 1;
                screen_data->qm_col = 0;
                ui_state_mark_dirty(screen_data->state);
            }
            break;

        case BTN_UP:
            if (screen_data->qm_row == 1) {
                screen_data->qm_row = 0;
                screen_data->qm_col = screen_data->qm_slot + screen_data->qm_shift;
                ui_state_mark_dirty(screen_data->state);
            }
            break;
    }
}

void quick_menu_screen_update(void* screen_instance) {
    // Update logic if needed (e.g., background loading)
}

void quick_menu_screen_destroy(void* screen_instance) {
    if (!screen_instance) return;

    QuickMenuScreen* screen_data = (QuickMenuScreen*)screen_instance;

    if (screen_data->background) {
        SDL_FreeSurface(screen_data->background);
        screen_data->background = NULL;
    }

    if (screen_data->folderBgPath) {
        free(screen_data->folderBgPath);
        screen_data->folderBgPath = NULL;
    }

    if (screen_data->quick) {
        // Don't free - managed by Menu_init/Menu_quit
        screen_data->quick = NULL;
    }

    if (screen_data->quickActions) {
        // Don't free - managed by Menu_init/Menu_quit
        screen_data->quickActions = NULL;
    }

    free(screen_data);
}

ScreenModule* quick_menu_screen_module_new(UIState* state, SDL_Surface* screen) {
    QuickMenuScreen* screen_data = quick_menu_screen_new(state, screen);
    if (!screen_data) return NULL;

    return screen_module_new(
        quick_menu_screen_render,
        quick_menu_screen_handle_input,
        quick_menu_screen_destroy,
        quick_menu_screen_update,
        screen_data
    );
}