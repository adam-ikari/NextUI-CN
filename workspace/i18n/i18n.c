#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h" // for SDCARD_PATH

// Where we look for language files on-device.
// Keep this in a downstream-owned location to avoid upstream churn.
#ifndef I18N_DEFAULT_LANG_PATH
#define I18N_DEFAULT_LANG_PATH SDCARD_PATH "/.system/i18n/zh_CN.lang"
#endif

typedef struct I18N_Entry {
	char *key;
	char *val;
	struct I18N_Entry *next;
} I18N_Entry;

static I18N_Entry *g_table = NULL;
static char g_loaded_path[512] = {0};

const char *I18N_loadedPath(void) {
	return g_loaded_path;
}

static void I18N_freeTable(void) {
	I18N_Entry *e = g_table;
	while (e) {
		I18N_Entry *n = e->next;
		free(e->key);
		free(e->val);
		free(e);
		e = n;
	}
	g_table = NULL;
}

static void trim_inplace(char *s) {
	if (!s) return;
	// left trim
	char *p = s;
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
	if (p != s) memmove(s, p, strlen(p) + 1);
	// right trim
	size_t len = strlen(s);
	while (len > 0) {
		char c = s[len - 1];
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
			s[len - 1] = 0;
			len--;
		} else {
			break;
		}
	}
}

// Unescape \n, \t, \\ for values.
static void unescape_inplace(char *s) {
	if (!s) return;
	char *r = s;
	char *w = s;
	while (*r) {
		if (*r == '\\') {
			r++;
			if (*r == 'n') { *w++ = '\n'; if (*r) r++; continue; }
			if (*r == 't') { *w++ = '\t'; if (*r) r++; continue; }
			if (*r == '\\') { *w++ = '\\'; if (*r) r++; continue; }
			// Unknown escape, keep the backslash and current char
			*w++ = '\\';
			if (*r) *w++ = *r++;
			continue;
		}
		*w++ = *r++;
	}
	*w = 0;
}

static char *i18n_strdup(const char *s) {
	if (!s) return NULL;
	// strdup is POSIX; some environments (notably MSVC) use _strdup.
	// For maximum portability across toolchains, use a small local implementation.
	size_t n = strlen(s) + 1;
	char *d = (char*)malloc(n);
	if (!d) return NULL;
	memcpy(d, s, n);
	return d;
}

static void I18N_put(const char *key, const char *val) {
	if (!key || !*key) return;
	if (!val) val = "";

	I18N_Entry *e = (I18N_Entry*)calloc(1, sizeof(I18N_Entry));
	if (!e) return;
	e->key = i18n_strdup(key);
	e->val = i18n_strdup(val);
	if (!e->key || !e->val) {
		free(e->key);
		free(e->val);
		free(e);
		return;
	}
	e->next = g_table;
	g_table = e;
}

int I18N_load(const char *lang_file_path) {
	if (!lang_file_path || !*lang_file_path) return -1;

	FILE *f = fopen(lang_file_path, "rb");
	if (!f) return -2;

	I18N_freeTable();
	strncpy(g_loaded_path, lang_file_path, sizeof(g_loaded_path) - 1);

	char line[1024];
	while (fgets(line, sizeof(line), f)) {
		// skip BOM if present
		if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
			memmove(line, line + 3, strlen(line + 3) + 1);
		}

		trim_inplace(line);
		if (!line[0]) continue;
		if (line[0] == '#') continue;

		char *eq = strchr(line, '=');
		if (!eq) continue;
		*eq = 0;
		char *k = line;
		char *v = eq + 1;
		trim_inplace(k);
		trim_inplace(v);
		unescape_inplace(v);
		I18N_put(k, v);
	}
	fclose(f);
	return 0;
}

void I18N_init(void) {
	// Load default language file if any. Safe even on failure.
	if (g_loaded_path[0]) return;
	I18N_load(I18N_DEFAULT_LANG_PATH);
}

const char *I18N_tr(const char *key) {
	if (!key) return "";
	if (!g_table) return key;
	for (I18N_Entry *e = g_table; e; e = e->next) {
		if (strcmp(e->key, key) == 0) return e->val;
	}
	return key;
}
