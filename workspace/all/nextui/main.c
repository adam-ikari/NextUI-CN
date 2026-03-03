#include "app.h"
#include "sdl.h"
#include "defines.h"
#include "api.h"
#include "utils.h"
#include "config.h"
#include "i18n.h"
#include "../common/api.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char *argv[]) {
    // Check for auto-resume mode
    if (autoResume()) return 0;
    
    // Initialize i18n
    LOG_info("NextUI\n");
    I18N_init();
    LOG_info("i18n: %s (%s)\n", TR("recents"), I18N_loadedPath());
    
    // Initialize settings
    InitSettings();
    
    // Startup haptic feedback
    if (CFG_getHaptics())
        VIB_singlePulse(VIB_bootStrength, VIB_bootDuration_ms);
    
    // Initialize graphics
    SDL_Surface* screen = GFX_init(MODE_MAIN);
    if (!screen) {
        LOG_error("Failed to initialize graphics\n");
        return 1;
    }
    
    // Initialize subsystems
    PAD_init();
    VIB_init();
    WIFI_init();
    PWR_init();
    
    // Disable sleep if no power button and not in simple mode
    if (!HAS_POWER_BUTTON && !exists(SIMPLE_MODE_PATH)) {
        PWR_disableSleep();
    }
    
    // Initialize image loader
    initImageLoaderPool();
    Menu_init();
    
    // Clear any active game logs
    system("gametimectl.elf stop_all");
    
    // Set vsync mode
    GFX_setVsync(VSYNC_STRICT);
    
    // Reset input and clear screen
    PAD_reset();
    GFX_clearLayers(LAYER_ALL);
    GFX_clear(screen);
    
    // Create the application
    NextUIApp* app = nextui_app_new(screen);
    if (!app) {
        LOG_error("Failed to create application\n");
        return 1;
    }
    
    // Initialize the application
    nextui_app_init(app);
    
    // Start CPU monitor thread
    pthread_t cpucheckthread;
    pthread_create(&cpucheckthread, NULL, PLAT_cpu_monitor, NULL);
    
    // Run the main application loop
    nextui_app_run(app);
    
    // Cleanup
    nextui_app_free(app);
    
    // Cleanup image loader
    destroyImageLoaderPool();
    
    return 0;
}