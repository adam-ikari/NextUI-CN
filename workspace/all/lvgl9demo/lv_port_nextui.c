#include "lv_port_nextui.h"
#include "defines.h"
#include "api.h"
#include "utils.h"
#include <string.h>

static lv_display_t *lv_display = NULL;
static lv_color_t *draw_buf1 = NULL;
static lv_color_t *draw_buf2 = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static uint32_t hor_res = 0;
static uint32_t ver_res = 0;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    /* Update using GPU-accelerated texture */
    SDL_Rect r;
    r.x = area->x1;
    r.y = area->y1;
    r.w = area->x2 - area->x1 + 1;
    r.h = area->y2 - area->y1 + 1;

    /* Update texture region using GPU */
    if (texture) {
        /* Lock texture for direct pixel access */
        void *pixels;
        int pitch;
        
        if (SDL_LockTexture(texture, &r, &pixels, &pitch) == 0) {
            /* Copy pixels directly to texture memory */
            uint32_t *src = (uint32_t *)px_map;
            uint32_t *dst = (uint32_t *)pixels;
            
            /* Fast copy with proper pitch handling */
            for (int y = 0; y < r.h; y++) {
                memcpy(dst, src, r.w * 4);
                src += r.w;
                dst += pitch / 4;
            }
            
            SDL_UnlockTexture(texture);
            
            /* Render using GPU */
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }

    lv_display_flush_ready(disp);
}

void lv_port_nextui_init(void)
{
    hor_res = screen->w;
    ver_res = screen->h;

    /* Create GPU-accelerated renderer */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        /* Fallback to software rendering */
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer) {
            printf("SDL_CreateRenderer (software) failed: %s\n", SDL_GetError());
            return;
        }
        printf("Using software rendering as fallback\n");
    } else {
        printf("Using GPU-accelerated rendering\n");
    }

    /* Create texture for GPU rendering */
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                SDL_TEXTUREACCESS_STREAMING, hor_res, ver_res);
    if (!texture) {
        printf("SDL_CreateTexture failed: %s\n", SDL_GetError());
        return;
    }

    /* Enable texture blending for smooth rendering */
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    /* Create draw buffers */
    draw_buf1 = lv_malloc(sizeof(lv_color_t) * hor_res * 50);
    draw_buf2 = lv_malloc(sizeof(lv_color_t) * hor_res * 50);

    /* Create LVGL display */
    lv_display = lv_display_create(hor_res, ver_res);
    if (!lv_display) {
        printf("lv_display_create failed\n");
        return;
    }

    /* Set flush callback */
    lv_display_set_flush_cb(lv_display, flush_cb);

    /* Set draw buffers with GPU-friendly mode */
    lv_display_set_buffers(lv_display, draw_buf1, draw_buf2, 
                          hor_res * 50 * sizeof(lv_color_t), 
                          LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Set default screen */
    lv_obj_t *scr = lv_screen_create();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
}

void lv_port_nextui_deinit(void)
{
    if (draw_buf1) {
        lv_free(draw_buf1);
        draw_buf1 = NULL;
    }
    if (draw_buf2) {
        lv_free(draw_buf2);
        draw_buf2 = NULL;
    }
    if (lv_display) {
        lv_display_delete(lv_display);
        lv_display = NULL;
    }
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
}
