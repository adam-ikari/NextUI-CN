#ifndef RT64ES_ZELDA_H
#define RT64ES_ZELDA_H

#include "../rt64es.h"
#include "../n64/rt64es_n64.h"

#ifdef __cplusplus
extern "C" {
#endif

// Zelda64Recomp render hooks
typedef struct {
    void* window;
    void* context;
    int width;
    int height;
    bool fullscreen;
    vsync_enabled;
} RT64ES_WindowConfig;

// Initialize Zelda64Recomp integration
RT64ES_Error rt64es_zelda_init(const RT64ES_WindowConfig* config);

// Shutdown Zelda64Recomp integration
RT64ES_Error rt64es_zelda_shutdown(void);

// Render a frame for Zelda64Recomp
RT64ES_Error rt64es_zelda_render_frame(void);

// Process N64 display list from Zelda64Recomp
RT64ES_Error rt64es_zelda_process_display_list(const uint32_t* dl, uint32_t size);

// Get current framebuffer data
RT64ES_Error rt64es_zelda_get_framebuffer(void** data, int* width, int* height);

// Set screen resolution
RT64ES_Error rt64es_zelda_set_resolution(int width, int height);

// Enable/disable vsync
RT64ES_Error rt64es_zelda_set_vsync(bool enabled);

// Get current FPS
float rt64es_zelda_get_fps(void);

#ifdef __cplusplus
}
#endif

#endif
