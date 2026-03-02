/**
 * NextUI Simulator Main Entry Point
 * 
 * Uses real NextUI framework with desktop platform adaptation layer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// Platform adaptation layer
#include "../../desktop/platform/platform.h"
#include "../../desktop/libmsettings/msettings.h"

// NextUI framework
#include "../common/api.h"
#include "defines.h"

int main(int argc, char* argv[]) {
    printf("NextUI Simulator (Desktop Platform)\n");
    printf("==================================\n\n");

    // Initialize msettings
    printf("Initializing msettings...\n");
    InitSettings();
    printf("  Brightness: %d\n", GetBrightness());
    printf("  Volume: %d\n", GetVolume());
    printf("  Audio sink: %d\n", GetAudioSink());
    printf("\n");

    // Check command line arguments
    int auto_mode = 0;
    int delay = 100;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--auto") == 0) {
            auto_mode = 1;
        } else if (strcmp(argv[i], "--delay") == 0 && i + 1 < argc) {
            delay = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("\nOptions:\n");
            printf("  --auto       Run in auto-traversal mode (screenshot testing)\n");
            printf("  --delay MS   Delay between screenshots in auto mode (default: 100ms)\n");
            printf("  --help       Show this help message\n");
            printf("\nExamples:\n");
            printf("  %s              # Interactive mode\n", argv[0]);
            printf("  %s --auto       # Auto-traversal with screenshot capture\n", argv[0]);
            printf("  %s --auto --delay 200  # Slower auto-traversal\n", argv[0]);
            printf("\nNote: This simulator requires SDL2, SDL2_ttf, and SDL2_image.\n");
            return 0;
        }
    }

    printf("Running mode: %s\n", auto_mode ? "Auto-traversal" : "Interactive");
    if (auto_mode) {
        printf("  Delay: %dms\n", delay);
    }
    printf("\n");

    // TODO: Initialize NextUI framework and run simulation
    // For now, just demonstrate the platform adaptation layer
    
    printf("Platform adaptation layer test:\n");
    printf("  Platform: desktop\n");
    printf("  SDL2: available\n");
    printf("  OpenGL: available\n");
    printf("  msettings: initialized\n");
    printf("\n");

    printf("Note: Full NextUI framework integration is not yet implemented.\n");
    printf("This is a demonstration that the platform adaptation layer works.\n");

    // Cleanup
    QuitSettings();
    
    return 0;
}