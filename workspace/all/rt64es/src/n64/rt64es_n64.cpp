#include "rt64es_n64.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static N64_RenderState* g_n64_state = NULL;

RT64ES_Error rt64es_n64_init(N64_RenderState* state) {
    if (!state) return RT64ES_ERROR_INVALID_PARAM;
    
    memset(state, 0, sizeof(N64_RenderState));
    state->fb_width = 640;
    state->fb_height = 480;
    state->zb_width = 640;
    state->zb_height = 480;
    state->tmem_size = 4096;
    
    state->texture_memory = (uint8_t*)malloc(state->tmem_size);
    if (!state->texture_memory) return RT64ES_ERROR_OUT_OF_MEMORY;
    
    state->framebuffer = (uint32_t*)malloc(state->fb_width * state->fb_height * 4);
    if (!state->framebuffer) {
        free(state->texture_memory);
        return RT64ES_ERROR_OUT_OF_MEMORY;
    }
    
    state->zbuffer = (uint32_t*)malloc(state->zb_width * state->zb_height * 4);
    if (!state->zbuffer) {
        free(state->texture_memory);
        free(state->framebuffer);
        return RT64ES_ERROR_OUT_OF_MEMORY;
    }
    
    g_n64_state = state;
    return RT64ES_SUCCESS;
}

static void process_set_tile(N64_RenderState* state, uint32_t cmd) {
    uint32_t tile_idx = (cmd >> 24) & 0x07;
    N64_Tile* tile = &state->tiles[tile_idx];
    memcpy(tile, &cmd, sizeof(N64_Tile));
}

static void process_set_cimg(N64_RenderState* state, uint32_t addr, uint32_t fmt, uint32_t size, uint32_t width) {
    state->fb_width = width + 1;
    state->fb_height = state->fb_width * 3 / 4;
}

static void process_set_zimg(N64_RenderState* state, uint32_t addr) {
    state->zb_width = state->fb_width;
    state->zb_height = state->fb_height;
}

static void process_set_other_modes(N64_RenderState* state, uint32_t mode_h, uint32_t mode_l) {
    memcpy(&state->other_mode_h, &mode_h, sizeof(N64_OtherModeH));
    state->other_mode_l = mode_l;
}

static void process_set_combine(N64_RenderState* state, uint32_t cmd0, uint32_t cmd1) {
    state->combine_color = cmd0;
}

static void process_set_color(N64_RenderState* state, uint32_t cmd) {
    uint32_t color_idx = (cmd >> 24) & 0x0F;
    uint32_t color = cmd & 0x00FFFFFF;
    
    switch (color_idx) {
        case 0x00: state->combine_color = color; break;
        case 0x01: state->prim_color = color; break;
        case 0x02: state->blend_color = color; break;
        case 0x04: state->env_color = color; break;
        case 0x05: state->fog_color = color; break;
    }
}

RT64ES_Error rt64es_n64_process_dl(N64_RenderState* state, const uint32_t* dl, uint32_t size) {
    if (!state || !dl || size == 0) return RT64ES_ERROR_INVALID_PARAM;
    
    uint32_t idx = 0;
    while (idx < size) {
        uint32_t cmd = dl[idx];
        uint8_t opcode = (cmd >> 24) & 0xFF;
        
        switch (opcode) {
            case RDP_SET_TILE:
                process_set_tile(state, cmd);
                idx += 2;
                break;
                
            case RDP_SET_CIMG:
                process_set_cimg(state, dl[idx+1], dl[idx+2], dl[idx+3], dl[idx+4]);
                idx += 6;
                break;
                
            case RDP_SET_ZIMG:
                process_set_zimg(state, dl[idx+1]);
                idx += 3;
                break;
                
            case RDP_SET_TIMG:
                idx += 5;
                break;
                
            case RDP_SET_OTHER_MODE:
                process_set_other_modes(state, dl[idx+1], dl[idx+2]);
                idx += 4;
                break;
                
            case RDP_SET_COMBINE:
                process_set_combine(state, dl[idx+1], dl[idx+2]);
                idx += 4;
                break;
                
            case RDP_SET_COLOR:
                process_set_color(state, cmd);
                idx += 2;
                break;
                
            case RDP_NOOP:
                idx++;
                break;
                
            case RDP_TRIFILL:
            case RDP_TRITEX:
            case RDP_TRISHADE:
            case RDP_TRIFILL_Z:
            case RDP_TRI_TEX_Z:
            case RDP_TRI_SHADE_Z:
                idx += 4;
                break;
                
            default:
                idx++;
                break;
        }
    }
    
    return RT64ES_SUCCESS;
}

static void rgba16_to_rgba32(const uint16_t* src, uint32_t* dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        uint16_t c = src[i];
        uint8_t r = ((c >> 11) & 0x1F) * 255 / 31;
        uint8_t g = ((c >> 6) & 0x1F) * 255 / 31;
        uint8_t b = ((c >> 1) & 0x1F) * 255 / 31;
        uint8_t a = (c & 0x01) ? 255 : 0;
        dst[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
}

static void ia8_to_rgba32(const uint8_t* src, uint32_t* dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        uint8_t ia = src[i];
        uint8_t i_val = (ia >> 4) * 17;
        uint8_t a_val = (ia & 0x0F) * 17;
        dst[i] = (a_val << 24) | (i_val << 16) | (i_val << 8) | i_val;
    }
}

static void i8_to_rgba32(const uint8_t* src, uint32_t* dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        uint8_t val = src[i];
        dst[i] = 0xFF000000 | (val << 16) | (val << 8) | val;
    }
}

RT64ES_Error rt64es_n64_convert_texture(N64_TextureFormat n64_fmt, const void* src, 
                                        uint32_t width, uint32_t height, void* dst) {
    if (!src || !dst || width == 0 || height == 0) return RT64ES_ERROR_INVALID_PARAM;
    
    uint32_t pixel_count = width * height;
    
    switch (n64_fmt) {
        case N64_TEX_RGBA16:
            rgba16_to_rgba32((const uint16_t*)src, (uint32_t*)dst, pixel_count);
            break;
            
        case N64_TEX_IA8:
            ia8_to_rgba32((const uint8_t*)src, (uint32_t*)dst, pixel_count);
            break;
            
        case N64_TEX_I8:
            i8_to_rgba32((const uint8_t*)src, (uint32_t*)dst, pixel_count);
            break;
            
        default:
            return RT64ES_ERROR_INVALID_PARAM;
    }
    
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_n64_set_viewport(N64_RenderState* state, float scale_x, float scale_y,
                                     float offset_x, float offset_y) {
    if (!state) return RT64ES_ERROR_INVALID_PARAM;
    
    state->viewport_scale[0] = scale_x;
    state->viewport_scale[1] = scale_y;
    state->viewport_offset[0] = offset_x;
    state->viewport_offset[1] = offset_y;
    
    return RT64ES_SUCCESS;
}
