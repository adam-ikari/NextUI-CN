#ifndef RT64ES_H
#define RT64ES_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RT64ES_VERSION_MAJOR 0
#define RT64ES_VERSION_MINOR 1
#define RT64ES_VERSION_PATCH 0
#define RT64ES_MAX_TEXTURES 256
#define RT64ES_MAX_VERTEX_BUFFER (64 * 1024)
#define RT64ES_MAX_INDEX_BUFFER (128 * 1024)
#define RT64ES_MAX_DRAW_CALLS 1024
typedef enum {
    RT64ES_SUCCESS = 0,
    RT64ES_ERROR_NO_GLES,
    RT64ES_ERROR_NO_ES3_2,
    RT64ES_ERROR_OUT_OF_MEMORY,
    RT64ES_ERROR_INVALID_PARAM,
    RT64ES_ERROR_SHADER_COMPILE,
} RT64ES_Error;
typedef struct {
    int width;
    int height;
    int format;
    int min_filter;
    int mag_filter;
} RT64ES_TextureConfig;
typedef struct {
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t texture_id;
    int primitive_type;
    bool depth_test;
    bool alpha_blend;
} RT64ES_DrawCall;
RT64ES_Error rt64es_init(void);
RT64ES_Error rt64es_deinit(void);
RT64ES_Error rt64es_begin_frame(void);
RT64ES_Error rt64es_end_frame(void);
RT64ES_Error rt64es_submit_draw_call(const RT64ES_DrawCall* call);
RT64ES_Error rt64es_clear(uint32_t color, float depth);
RT64ES_Error rt64es_set_viewport(int x, int y, int width, int height);
RT64ES_Error rt64es_present(void);
RT64ES_Error rt64es_create_texture(const RT64ES_TextureConfig* config, const void* data, uint32_t* out_id);
RT64ES_Error rt64es_destroy_texture(uint32_t id);
const char* rt64es_get_error_string(RT64ES_Error error);
#ifdef __cplusplus
}
#endif
#endif
