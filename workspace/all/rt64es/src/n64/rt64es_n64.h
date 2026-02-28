#ifndef RT64ES_N64_H
#define RT64ES_N64_H

#include "../rt64es.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// N64 RDP commands (simplified subset for OoT)
typedef enum {
    RDP_NOOP            = 0x00,
    RDP_FILL_RECT       = 0xF8,
    RDP_SET_OTHER_MODE  = 0xE3,
    RDP_LOAD_TILE       = 0x35,
    RDP_LOAD_BLOCK      = 0x33,
    RDP_LOAD_TLUT       = 0x30,
    RDP_SET_TILE        = 0x32,
    RDP_SET_TIMG        = 0xFD,
    RDP_SET_ZIMG        = 0xFF,
    RDP_SET_CIMG        = 0xFC,
    RDP_SET_COMBINE     = 0xC3,
    RDP_SET_COLOR       = 0xFB,
    RDP_SET_ENVCOLOR    = 0xFA,
    RDP_TRIFILL         = 0x08,
    RDP_TRITEX          = 0x07,
    RDP_TRISHADE        = 0x06,
    RDP_TRI_SHADE_Z     = 0x15,
    RDP_TRI_TEX_Z       = 0x14,
    RDP_TRI_TEX Shade   = 0x16,
    RDP_TRIFILL_Z       = 0x09,
} RDP_Command;

// N64 texture formats
typedef enum {
    N64_TEX_RGBA16      = 0,
    N64_TEX_RGBA32      = 1,
    N64_TEX_IA4         = 2,
    N64_TEX_IA8         = 3,
    N64_TEX_IA16        = 4,
    N64_TEX_I4          = 5,
    N64_TEX_I8          = 6,
    N64_TEX_CI4         = 7,
    N64_TEX_CI8         = 8,
} N64_TextureFormat;

// N64 cycle types
typedef enum {
    N64_CYCLE_1         = 0,
    N64_CYCLE_2         = 1,
    N64_CYCLE_COPY      = 2,
    N64_CYCLE_FILL      = 3,
} N64_CycleType;

// N64 other mode H bits
typedef struct {
    uint32_t cycle_type : 2;
    uint32_t tex_lod_en : 1;
    uint32_t tex_sharp_en : 1;
    uint32_t rgb_dither_sel : 2;
    uint32_t alpha_dither_sel : 2;
    uint32_t blend_key : 1;
    uint32_t blend_en : 1;
    uint32_t read_en : 1;
    uint32_t update_en : 1;
    uint32_t fog_en : 1;
    uint32_t blend_mask : 1;
    uint32_t alpha_cvg_sel : 1;
    uint32_t cvg_times_alpha : 1;
    uint32_t z_mode : 2;
    uint32_t cvg_dest : 2;
    uint32_t color_on_cvg : 1;
    uint32_t image_read_en : 1;
    uint32_t z_update_en : 1;
    uint32_t z_compare_en : 1;
    uint32_t antialias_en : 1;
    uint32_t z_source_sel : 1;
    uint32_t dither_alpha_en : 1;
    uint32_t alpha_compare_en : 1;
} N64_OtherModeH;

// N64 tile descriptor
typedef struct {
    uint32_t format : 3;
    uint32_t size : 2;
    uint32_t line : 9;
    uint32_t tmem : 9;
    uint32_t palette : 4;
    uint32_t clamp_t : 1;
    uint32_t mirror_t : 1;
    uint32_t mask_t : 4;
    uint32_t shift_t : 4;
    uint32_t clamp_s : 1;
    uint32_t mirror_s : 1;
    uint32_t mask_s : 4;
    uint32_t shift_s : 4;
} N64_Tile;

// N64 render state
typedef struct {
    uint32_t* framebuffer;
    uint32_t fb_width;
    uint32_t fb_height;
    
    uint32_t* zbuffer;
    uint32_t zb_width;
    uint32_t zb_height;
    
    uint8_t* texture_memory;
    uint32_t tmem_size;
    
    N64_Tile tiles[8];
    N64_OtherModeH other_mode_h;
    uint32_t other_mode_l;
    
    uint32_t combine_color;
    uint32_t env_color;
    uint32_t prim_color;
    uint32_t fog_color;
    uint32_t blend_color;
    
    float viewport_scale[2];
    float viewport_offset[2];
    
    uint32_t texture_width[8];
    uint32_t texture_height[8];
} N64_RenderState;

// Initialize N64 state
RT64ES_Error rt64es_n64_init(N64_RenderState* state);

// Process RDP display list
RT64ES_Error rt64es_n64_process_dl(N64_RenderState* state, const uint32_t* dl, uint32_t size);

// Convert N64 texture format to RT64ES format
RT64ES_Error rt64es_n64_convert_texture(N64_TextureFormat n64_fmt, const void* src, 
                                        uint32_t width, uint32_t height, void* dst);

// Set viewport
RT64ES_Error rt64es_n64_set_viewport(N64_RenderState* state, float scale_x, float scale_y,
                                     float offset_x, float offset_y);

#ifdef __cplusplus
}
#endif

#endif
