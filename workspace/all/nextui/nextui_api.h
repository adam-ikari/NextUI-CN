#ifndef __NEXTUI_API_H__
#define __NEXTUI_API_H__

#include "sdl.h"
#include "array.h"
#include "state/ui_state.h"

// Entry type definition (from nextui.c)
enum EntryType {
    ENTRY_DIR,
    ENTRY_PAK,
    ENTRY_ROM,
    ENTRY_DIP,
};

typedef struct Entry {
    char* path;
    char* name;
    char* display;
    char* unique;
    int type;
    int alpha;
} Entry;

// External functions from nextui.c
extern Array* getQuickEntries(void);
extern Array* getQuickToggles(void);
extern void Entry_open(Entry* entry);
extern void Entry_free(Entry* entry);
extern Entry* entryFromRecent(void* recent);
extern void readyResume(Entry* entry);
extern void saveRecents(void);

// External variables from nextui.c
extern Array* recents; // RecentArray
extern int can_resume;
extern int has_preview;
extern char preview_path[256];
extern int should_resume;

// Configuration functions
extern int CFG_getShowSetting(void);

#endif // __NEXTUI_API_H__