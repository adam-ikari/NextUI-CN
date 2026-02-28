#include "defines.h"
#include "api.h"
#include "utils.h"
#include "i18n.h"
#include "lvgl.h"
#include "lv_port_nextui.h"
#include <signal.h>

static bool quit = false;

/* Signal handler */
static void signal_handler(int sig)
{
    quit = true;
}

/* MinUI color palette */
#define MINUI_BG_COLOR         lv_color_hex(0x000000)
#define MINUI_PRIMARY_COLOR    lv_color_hex(0x7A2155)
#define MINUI_TEXT_COLOR       lv_color_hex(0xFFFFFF)
#define MINUI_SELECTED_BG      lv_color_hex(0xFFFFFF)
#define MINUI_SELECTED_TEXT    lv_color_hex(0x000000)
#define MINUI_LIST_BG          lv_color_hex(0x333333)

/* Screen size breakpoints */
typedef enum {
    SCREEN_SIZE_SMALL,   /* < 320px width (e.g., 240x240) */
    SCREEN_SIZE_MEDIUM,  /* 320-480px (e.g., 320x240, 480x272) */
    SCREEN_SIZE_LARGE,   /* 480-720px (e.g., 640x480) */
    SCREEN_SIZE_XLARGE   /* > 720px (e.g., 800x480, 1024x600) */
} ScreenSize;

/* Responsive layout configuration */
typedef struct {
    ScreenSize size;
    int header_height;
    int footer_height;
    int card_height;
    int list_item_height;
    int button_width;
    int button_height;
    int corner_radius;
    int font_small;
    int font_medium;
    int font_large;
    int padding;
    int spacing;
} LayoutConfig;

/* Get screen size based on dimensions */
static ScreenSize get_screen_size(int width, int height)
{
    int min_dim = width < height ? width : height;
    
    if (min_dim < 320) {
        return SCREEN_SIZE_SMALL;
    } else if (min_dim < 480) {
        return SCREEN_SIZE_MEDIUM;
    } else if (min_dim < 720) {
        return SCREEN_SIZE_LARGE;
    } else {
        return SCREEN_SIZE_XLARGE;
    }
}

/* Get layout configuration for screen size */
static void get_layout_config(ScreenSize size, LayoutConfig *config)
{
    switch (size) {
        case SCREEN_SIZE_SMALL:
            config->header_height = 50;
            config->footer_height = 55;
            config->card_height = 80;
            config->list_item_height = 32;
            config->button_width = 120;
            config->button_height = 40;
            config->corner_radius = 6;
            config->font_small = 12;
            config->font_medium = 14;
            config->font_large = 18;
            config->padding = 10;
            config->spacing = 8;
            break;
            
        case SCREEN_SIZE_MEDIUM:
            config->header_height = 55;
            config->footer_height = 60;
            config->card_height = 90;
            config->list_item_height = 36;
            config->button_width = 140;
            config->button_height = 45;
            config->corner_radius = 8;
            config->font_small = 14;
            config->font_medium = 16;
            config->font_large = 20;
            config->padding = 12;
            config->spacing = 12;
            break;
            
        case SCREEN_SIZE_LARGE:
            config->header_height = 60;
            config->footer_height = 65;
            config->card_height = 100;
            config->list_item_height = 40;
            config->button_width = 160;
            config->button_height = 50;
            config->corner_radius = 8;
            config->font_small = 16;
            config->font_medium = 18;
            config->font_large = 24;
            config->padding = 16;
            config->spacing = 16;
            break;
            
        case SCREEN_SIZE_XLARGE:
            config->header_height = 70;
            config->footer_height = 70;
            config->card_height = 120;
            config->list_item_height = 48;
            config->button_width = 180;
            config->button_height = 55;
            config->corner_radius = 12;
            config->font_small = 18;
            config->font_medium = 20;
            config->font_large = 28;
            config->padding = 20;
            config->spacing = 20;
            break;
    }
}

/* Get font for layout */
static const lv_font_t * get_font_for_size(int size)
{
    if (size <= 12) return &lv_font_montserrat_12;
    if (size <= 14) return &lv_font_montserrat_14;
    if (size <= 16) return &lv_font_montserrat_16;
    if (size <= 18) return &lv_font_montserrat_18;
    if (size <= 20) return &lv_font_montserrat_20;
    if (size <= 24) return &lv_font_montserrat_24;
    return &lv_font_montserrat_28;
}

/* Create MinUI-style header bar (responsive) */
static lv_obj_t * create_minui_header(lv_obj_t *parent, const char *title, const LayoutConfig *config)
{
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, lv_pct(100), config->header_height);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, MINUI_PRIMARY_COLOR, 0);
    lv_obj_set_style_radius(header, config->corner_radius, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, config->padding, 0);
    
    /* Title */
    lv_obj_t *title_label = lv_label_create(header);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, get_font_for_size(config->font_large), 0);
    lv_obj_set_style_text_color(title_label, MINUI_TEXT_COLOR, 0);
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);
    
    /* Time display (hidden on very small screens) */
    if (config->size >= SCREEN_SIZE_MEDIUM) {
        int time_box_w = 70 + (config->size - SCREEN_SIZE_MEDIUM) * 5;
        int time_box_h = 35 + (config->size - SCREEN_SIZE_MEDIUM) * 3;
        
        lv_obj_t *time_box = lv_obj_create(header);
        lv_obj_set_size(time_box, time_box_w, time_box_h);
        lv_obj_set_style_bg_color(time_box, MINUI_PRIMARY_COLOR, 0);
        lv_obj_set_style_border_color(time_box, MINUI_TEXT_COLOR, 0);
        lv_obj_set_style_border_width(time_box, 2, 0);
        lv_obj_set_style_border_side(time_box, LV_BORDER_SIDE_FULL, 0);
        lv_obj_set_style_radius(time_box, 6, 0);
        lv_obj_align(time_box, LV_ALIGN_RIGHT_MID, -config->padding, 0);
        
        lv_obj_t *time_label = lv_label_create(time_box);
        lv_label_set_text(time_label, "12:34");
        lv_obj_set_style_text_font(time_label, get_font_for_size(config->font_medium), 0);
        lv_obj_set_style_text_color(time_label, MINUI_TEXT_COLOR, 0);
        lv_obj_center(time_label);
    }
    
    return header;
}

/* Create MinUI-style button (responsive) */
static lv_obj_t * create_minui_button(lv_obj_t *parent, const char *icon, const char *text, int x, int y, const LayoutConfig *config)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, config->button_width, config->button_height);
    
    /* Purple-red background */
    lv_obj_set_style_bg_color(btn, MINUI_PRIMARY_COLOR, 0);
    lv_obj_set_style_radius(btn, config->button_height / 2, 0); /* Pill shape */
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, config->padding / 2, 0);
    
    /* Icon box */
    int icon_box_size = 30 + (config->size - SCREEN_SIZE_SMALL) * 2;
    if (icon_box_size > 40) icon_box_size = 40;
    
    lv_obj_t *icon_box = lv_obj_create(btn);
    lv_obj_set_size(icon_box, icon_box_size, icon_box_size);
    lv_obj_set_style_bg_color(icon_box, MINUI_TEXT_COLOR, 0);
    lv_obj_set_style_radius(icon_box, 6, 0);
    lv_obj_set_style_border_width(icon_box, 2, 0);
    lv_obj_set_style_border_color(icon_box, MINUI_TEXT_COLOR, 0);
    lv_obj_align(icon_box, LV_ALIGN_LEFT_MID, config->padding / 2, 0);
    
    lv_obj_t *icon_label = lv_label_create(icon_box);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_font(icon_label, get_font_for_size(config->font_medium), 0);
    lv_obj_set_style_text_color(icon_label, lv_color_black(), 0);
    lv_obj_center(icon_label);
    
    /* Text (hidden on very small screens) */
    if (config->size >= SCREEN_SIZE_MEDIUM) {
        lv_obj_t *text_label = lv_label_create(btn);
        lv_label_set_text(text_label, text);
        lv_obj_set_style_text_font(text_label, get_font_for_size(config->font_medium), 0);
        lv_obj_set_style_text_color(text_label, MINUI_TEXT_COLOR, 0);
        lv_obj_align(text_label, LV_ALIGN_LEFT_MID, icon_box_size + config->padding, 0);
    }
    
    return btn;
}

/* Create MinUI-style list item (responsive) */
static lv_obj_t * create_minui_list_item(lv_obj_t *parent, const char *text, bool selected, const LayoutConfig *config)
{
    lv_obj_t *item = lv_obj_create(parent);
    lv_obj_set_size(item, lv_pct(100), config->list_item_height);
    
    if (selected) {
        lv_obj_set_style_bg_color(item, MINUI_SELECTED_BG, 0);
        lv_obj_set_style_radius(item, 6, 0);
    } else {
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
    }
    
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_pad_all(item, config->padding, 0);
    
    lv_obj_t *label = lv_label_create(item);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, get_font_for_size(config->font_medium), 0);
    
    if (selected) {
        lv_obj_set_style_text_color(label, MINUI_SELECTED_TEXT, 0);
    } else {
        lv_obj_set_style_text_color(label, MINUI_TEXT_COLOR, 0);
    }
    
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    
    return item;
}

/* Create MinUI-style info card (responsive) */
static lv_obj_t * create_minui_card(lv_obj_t *parent, const char *title, const char *content, int x, int y, int w, int h, const LayoutConfig *config)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    
    lv_obj_set_style_bg_color(card, MINUI_LIST_BG, 0);
    lv_obj_set_style_radius(card, config->corner_radius, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, config->padding, 0);
    
    /* Title */
    lv_obj_t *title_label = lv_label_create(card);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, get_font_for_size(config->font_medium), 0);
    lv_obj_set_style_text_color(title_label, MINUI_PRIMARY_COLOR, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);
    
    /* Content (hidden on very small screens) */
    if (config->size >= SCREEN_SIZE_MEDIUM) {
        lv_obj_t *content_label = lv_label_create(card);
        lv_label_set_text(content_label, content);
        lv_obj_set_style_text_font(content_label, get_font_for_size(config->font_small), 0);
        lv_obj_set_style_text_color(content_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(content_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    }
    
    return card;
}

int main(int argc, char *argv[])
{
    /* Initialize NextUI systems */
    GFX_init();
    PAD_init();
    PWR_init();
    
    /* Initialize LVGL */
    lv_init();
    
    /* Initialize NextUI display driver */
    lv_port_nextui_init();
    
    /* Setup signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Create UI elements */
    lv_obj_t *screen_obj = lv_screen_active();
    
    /* Get screen dimensions */
    int screen_w = screen->w;
    int screen_h = screen->h;
    
    /* Determine screen size and get layout config */
    ScreenSize screen_size = get_screen_size(screen_w, screen_h);
    LayoutConfig config;
    get_layout_config(screen_size, &config);
    config.size = screen_size;
    
    /* Print screen info */
    printf("LVGL 9 MinUI-style demo started\n");
    printf("Screen size: %dx%d (", screen_w, screen_h);
    switch (screen_size) {
        case SCREEN_SIZE_SMALL: printf("SMALL"); break;
        case SCREEN_SIZE_MEDIUM: printf("MEDIUM"); break;
        case SCREEN_SIZE_LARGE: printf("LARGE"); break;
        case SCREEN_SIZE_XLARGE: printf("XLARGE"); break;
    }
    printf(")\n");
    
    /* Set pure black background like MinUI */
    lv_obj_set_style_bg_color(screen_obj, MINUI_BG_COLOR, 0);
    lv_obj_set_style_bg_opa(screen_obj, LV_OPA_COVER, 0);
    
    /* Create header bar */
    create_minui_header(screen_obj, TR("lvgl9.title"), &config);
    
    /* Calculate positions */
    int content_y = config.header_height + config.spacing;
    int footer_y = screen_h - config.footer_height - config.spacing;
    int available_height = footer_y - content_y;
    
    /* Determine card layout based on screen size */
    int num_cards = 3;
    if (screen_size == SCREEN_SIZE_SMALL) {
        num_cards = 1; /* Stack vertically on small screens */
    }
    
    int card_w = (screen_w - config.spacing * (num_cards + 1)) / num_cards;
    int card_h = config.card_height;
    int total_card_h = card_h + config.spacing;
    
    /* Create feature cards */
    for (int i = 0; i < 3; i++) {
        int card_x, card_y;
        
        if (screen_size == SCREEN_SIZE_SMALL) {
            /* Vertical stacking */
            card_x = config.spacing;
            card_y = content_y + i * total_card_h;
        } else {
            /* Horizontal layout */
            card_x = config.spacing + i * (card_w + config.spacing);
            card_y = content_y;
        }
        
        char title_key[32], desc_key[32];
        sprintf(title_key, "lvgl9.feature%d.title", i + 1);
        sprintf(desc_key, "lvgl9.feature%d.desc", i + 1);
        
        create_minui_card(screen_obj, TR(title_key), TR(desc_key), card_x, card_y, card_w, card_h, &config);
    }
    
    /* Calculate list position */
    int list_y;
    if (screen_size == SCREEN_SIZE_SMALL) {
        list_y = content_y + 3 * total_card_h;
    } else {
        list_y = content_y + card_h + config.spacing * 2;
    }
    
    /* Create MinUI-style list */
    int list_item_count = 6;
    int list_height = available_height - (list_y - content_y) - config.spacing;
    
    for (int i = 0; i < list_item_count; i++) {
        char key[32];
        sprintf(key, "lvgl9.list.item%d", i + 1);
        bool selected = (i == 0);
        create_minui_list_item(screen_obj, TR(key), selected, &config);
    }
    
    /* Create MinUI-style footer buttons */
    int btn_y = footer_y;
    int btn_x_start;
    
    if (screen_size == SCREEN_SIZE_SMALL) {
        /* Single button on small screens */
        btn_x_start = (screen_w - config.button_width) / 2;
        create_minui_button(screen_obj, "A", TR("lvgl9.open"), btn_x_start, btn_y, &config);
    } else {
        /* Two buttons on medium+ screens */
        int total_btn_w = config.button_width * 2 + config.spacing;
        btn_x_start = (screen_w - total_btn_w) / 2;
        
        create_minui_button(screen_obj, "⏻", TR("lvgl9.sleep"), btn_x_start, btn_y, &config);
        create_minui_button(screen_obj, "A", TR("lvgl9.open"), btn_x_start + config.button_width + config.spacing, btn_y, &config);
    }
    
    printf("Press B to exit.\n");
    
    /* Main loop */
    bool dirty = true;
    uint32_t last_tick = SDL_GetTicks();
    
    while (!quit) {
        PAD_poll();
        
        /* Handle gamepad input */
        if (PAD_justPressed(BTN_B)) {
            quit = true;
        }
        
        /* Update power */
        PWR_update(&dirty, NULL, NULL, NULL);
        
        /* Calculate elapsed time */
        uint32_t current_tick = SDL_GetTicks();
        uint32_t elapsed = current_tick - last_tick;
        
        /* Increment LVGL tick */
        lv_tick_inc(elapsed);
        
        /* Handle LVGL timer */
        lv_timer_handler();
        
        last_tick = current_tick;
        
        /* Flip screen if dirty */
        if (dirty) {
            GFX_flip(screen);
            dirty = false;
        } else {
            GFX_sync();
        }
        
        /* Frame limiting */
        SDL_Delay(5);
    }
    
    /* Cleanup */
    lv_port_nextui_deinit();
    PWR_quit();
    PAD_quit();
    GFX_quit();
    
    printf("LVGL 9 MinUI-style demo exited.\n");
    
    return EXIT_SUCCESS;
}
