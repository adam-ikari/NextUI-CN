// Desktop platform stub functions for NextUI
// These functions provide minimal implementations for desktop testing

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Stub implementations for desktop platform

int autoResume(void) {
    // No auto-resume on desktop
    return 0;
}

void initImageLoaderPool(void) {
    // No image loader pool needed on desktop
}

void Menu_init(void) {
    // No menu initialization needed on desktop
}

void destroyImageLoaderPool(void) {
    // No cleanup needed on desktop
}

int getInputBlocking(void) {
    // This should be implemented by the actual input system
    // For now, return a dummy value
    return -1;
}