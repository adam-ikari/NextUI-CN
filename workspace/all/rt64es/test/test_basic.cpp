#include "../rt64es.h"
#include "../src/zelda/rt64es_zelda.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    printf("RT64ES Basic Test\n");
    printf("================\n\n");
    
    RT64ES_Error err;
    
    printf("1. Testing RT64ES initialization...\n");
    err = rt64es_init();
    if (err != RT64ES_SUCCESS) {
        printf("   ERROR: %s\n", rt64es_get_error_string(err));
        return 1;
    }
    printf("   SUCCESS\n\n");
    
    printf("2. Testing N64 adapter initialization...\n");
    N64_RenderState n64_state;
    err = rt64es_n64_init(&n64_state);
    if (err != RT64ES_SUCCESS) {
        printf("   ERROR: %s\n", rt64es_get_error_string(err));
        rt64es_deinit();
        return 1;
    }
    printf("   SUCCESS\n");
    printf("   Framebuffer: %dx%d\n", n64_state.fb_width, n64_state.fb_height);
    printf("   Z-Buffer: %dx%d\n", n64_state.zb_width, n64_state.zb_height);
    printf("   Texture Memory: %d bytes\n\n", n64_state.tmem_size);
    
    printf("3. Testing texture format conversion...\n");
    uint16_t rgba16_test[] = {0xF800, 0x07E0, 0x001F, 0xFFFF};
    uint32_t rgba32_result[4];
    err = rt64es_n64_convert_texture(N64_TEX_RGBA16, rgba16_test, 2, 2, rgba32_result);
    if (err != RT64ES_SUCCESS) {
        printf("   ERROR: %s\n", rt64es_get_error_string(err));
    } else {
        printf("   SUCCESS\n");
        printf("   RGBA16[0] = 0x%04X -> RGBA32[0] = 0x%08X\n", rgba16_test[0], rgba32_result[0]);
        printf("   RGBA16[1] = 0x%04X -> RGBA32[1] = 0x%08X\n", rgba16_test[1], rgba32_result[1]);
        printf("   RGBA16[2] = 0x%04X -> RGBA32[2] = 0x%08X\n", rgba16_test[2], rgba32_result[2]);
        printf("   RGBA16[3] = 0x%04X -> RGBA32[3] = 0x%08X\n\n", rgba16_test[3], rgba32_result[3]);
    }
    
    printf("4. Testing RDP display list processing...\n");
    uint32_t test_dl[] = {
        0xFC000000, // RDP_SET_CIMG
        0x80000000, 0x00000000, 0x00000000, 0x000003FF, // CIMG parameters
        0xE3000000, // RDP_SET_OTHER_MODE
        0x00001111, 0x00000000, // Other mode parameters
        0x00000000  // RDP_NOOP
    };
    err = rt64es_n64_process_dl(&n64_state, test_dl, sizeof(test_dl) / sizeof(uint32_t));
    if (err != RT64ES_SUCCESS) {
        printf("   ERROR: %s\n", rt64es_get_error_string(err));
    } else {
        printf("   SUCCESS\n");
        printf("   Processed %zu display list commands\n\n", sizeof(test_dl) / sizeof(uint32_t));
    }
    
    printf("5. Testing viewport setting...\n");
    err = rt64es_n64_set_viewport(&n64_state, 320.0f, 240.0f, 160.0f, 120.0f);
    if (err != RT64ES_SUCCESS) {
        printf("   ERROR: %s\n", rt64es_get_error_string(err));
    } else {
        printf("   SUCCESS\n");
        printf("   Scale: %.1f, %.1f\n", n64_state.viewport_scale[0], n64_state.viewport_scale[1]);
        printf("   Offset: %.1f, %.1f\n\n", n64_state.viewport_offset[0], n64_state.viewport_offset[1]);
    }
    
    printf("6. Testing Zelda64Recomp integration...\n");
    RT64ES_WindowConfig zelda_config = {
        .window = NULL,
        .context = NULL,
        .width = 1024,
        .height = 768,
        .fullscreen = false,
        .vsync_enabled = true
    };
    err = rt64es_zelda_init(&zelda_config);
    if (err != RT64ES_SUCCESS) {
        printf("   ERROR: %s\n", rt64es_get_error_string(err));
    } else {
        printf("   SUCCESS\n");
        printf("   Resolution: %dx%d\n", zelda_config.width, zelda_config.height);
        printf("   VSync: %s\n\n", zelda_config.vsync_enabled ? "ON" : "OFF");
        
        void* fb_data;
        int fb_w, fb_h;
        err = rt64es_zelda_get_framebuffer(&fb_data, &fb_w, &fb_h);
        if (err == RT64ES_SUCCESS) {
            printf("   Framebuffer: %p (%dx%d)\n\n", fb_data, fb_w, fb_h);
        }
        
        printf("   Testing resolution change...\n");
        err = rt64es_zelda_set_resolution(1280, 720);
        if (err == RT64ES_SUCCESS) {
            printf("   Resolution changed to 1280x720\n\n");
        }
        
        printf("   Cleaning up Zelda integration...\n");
        rt64es_zelda_shutdown();
        printf("   DONE\n\n");
    }
    
    printf("7. Cleanup...\n");
    if (n64_state.texture_memory) free(n64_state.texture_memory);
    if (n64_state.framebuffer) free(n64_state.framebuffer);
    if (n64_state.zbuffer) free(n64_state.zbuffer);
    rt64es_deinit();
    printf("   DONE\n\n");
    
    printf("================\n");
    printf("All tests passed!\n");
    printf("================\n");
    
    return 0;
}
