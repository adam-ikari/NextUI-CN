#include "rt64es_zelda.h"
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <time.h>
#include <stdio.h>

static struct {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    N64_RenderState n64_state;
    int width;
    int height;
    bool vsync;
    float fps;
    uint64_t frame_count;
    uint64_t last_time;
    bool initialized;
} g_zelda = {0};

static float calculate_fps(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    
    if (g_zelda.last_time == 0) {
        g_zelda.last_time = now;
        return 0.0f;
    }
    
    g_zelda.frame_count++;
    uint64_t delta = now - g_zelda.last_time;
    
    if (delta >= 1000000000ULL) {
        g_zelda.fps = g_zelda.frame_count * 1000000000.0f / delta;
        g_zelda.frame_count = 0;
        g_zelda.last_time = now;
    }
    
    return g_zelda.fps;
}

RT64ES_Error rt64es_zelda_init(const RT64ES_WindowConfig* config) {
    if (!config) return RT64ES_ERROR_INVALID_PARAM;
    if (g_zelda.initialized) return RT64ES_SUCCESS;
    
    g_zelda.width = config->width > 0 ? config->width : 1024;
    g_zelda.height = config->height > 0 ? config->height : 768;
    g_zelda.vsync = config->vsync_enabled;
    
    RT64ES_Error err = rt64es_init();
    if (err != RT64ES_SUCCESS) return err;
    
    err = rt64es_n64_init(&g_zelda.n64_state);
    if (err != RT64ES_SUCCESS) {
        rt64es_deinit();
        return err;
    }
    
    rt64es_set_viewport(0, 0, g_zelda.width, g_zelda.height);
    
    g_zelda.initialized = true;
    printf("RT64ES Zelda initialized: %dx%d, VSync: %s\n", 
           g_zelda.width, g_zelda.height, g_zelda.vsync ? "ON" : "OFF");
    
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_zelda_shutdown(void) {
    if (!g_zelda.initialized) return RT64ES_SUCCESS;
    
    if (g_zelda.n64_state.texture_memory) {
        free(g_zelda.n64_state.texture_memory);
        g_zelda.n64_state.texture_memory = NULL;
    }
    
    if (g_zelda.n64_state.framebuffer) {
        free(g_zelda.n64_state.framebuffer);
        g_zelda.n64_state.framebuffer = NULL;
    }
    
    if (g_zelda.n64_state.zbuffer) {
        free(g_zelda.n64_state.zbuffer);
        g_zelda.n64_state.zbuffer = NULL;
    }
    
    rt64es_deinit();
    g_zelda.initialized = false;
    
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_zelda_render_frame(void) {
    if (!g_zelda.initialized) return RT64ES_ERROR_NO_GLES;
    
    rt64es_begin_frame();
    rt64es_clear(0xFF000000, 1.0f);
    
    calculate_fps();
    
    rt64es_end_frame();
    rt64es_present();
    
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_zelda_process_display_list(const uint32_t* dl, uint32_t size) {
    if (!g_zelda.initialized || !dl || size == 0) return RT64ES_ERROR_INVALID_PARAM;
    
    return rt64es_n64_process_dl(&g_zelda.n64_state, dl, size);
}

RT64ES_Error rt64es_zelda_get_framebuffer(void** data, int* width, int* height) {
    if (!data || !width || !height) return RT64ES_ERROR_INVALID_PARAM;
    if (!g_zelda.initialized) return RT64ES_ERROR_NO_GLES;
    
    *data = g_zelda.n64_state.framebuffer;
    *width = g_zelda.width;
    *height = g_zelda.height;
    
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_zelda_set_resolution(int width, int height) {
    if (!g_zelda.initialized) return RT64ES_ERROR_NO_GLES;
    if (width <= 0 || height <= 0) return RT64ES_ERROR_INVALID_PARAM;
    
    g_zelda.width = width;
    g_zelda.height = height;
    rt64es_set_viewport(0, 0, width, height);
    
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_zelda_set_vsync(bool enabled) {
    if (!g_zelda.initialized) return RT64ES_ERROR_NO_GLES;
    
    g_zelda.vsync = enabled;
    if (g_zelda.display && g_zelda.surface) {
        eglSwapInterval(g_zelda.display, enabled ? 1 : 0);
    }
    
    return RT64ES_SUCCESS;
}

float rt64es_zelda_get_fps(void) {
    return g_zelda.fps;
}
