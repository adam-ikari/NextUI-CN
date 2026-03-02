#include <stdio.h>
#include <string.h>
#include "../common/utils.h"

int main() {
    char emu_name[256];
    
    const char* test_names[] = {
        "Game Boy (GB)",
        "Game Boy Advance (GBA)",
        "Game Boy Color (GBC)",
        NULL
    };
    
    for (int i = 0; test_names[i] != NULL; i++) {
        getEmuName(test_names[i], emu_name);
        printf("Input: '%s' -> Output: '%s'\n", test_names[i], emu_name);
    }
    
    return 0;
}
