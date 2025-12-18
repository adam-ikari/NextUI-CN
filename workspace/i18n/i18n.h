#ifndef NEXTUI_I18N_H
#define NEXTUI_I18N_H

// Lightweight translation layer for downstream localization forks.
//
// Design goals:
// - Minimal dependencies (C stdlib only)
// - Safe fallback if no language file exists
// - Stable keys to minimize re-translation during upstream updates
//
// Usage:
//   I18N_init();
//   printf("%s\n", TR("settings"));

#ifdef __cplusplus
extern "C" {
#endif

// Initialize translation table.
// Safe to call multiple times (it will reload if language file changes).
void I18N_init(void);

// Explicitly (re)load a language file.
// Returns 0 on success, non-zero on failure.
int I18N_load(const char *lang_file_path);

// Lookup translation by key. Never returns NULL.
const char *I18N_tr(const char *key);

// Convenience macro.
#define TR(KEY) I18N_tr((KEY))

#ifdef __cplusplus
}
#endif

#endif
