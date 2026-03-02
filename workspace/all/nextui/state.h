#ifndef NEXTUI_STATE_H__
#define NEXTUI_STATE_H__

#include <SDL2/SDL.h>
#include <stdint.h>

// Forward declarations
typedef struct state state;
typedef struct component component;

// State value types
typedef enum {
    STATE_TYPE_INT,
    STATE_TYPE_FLOAT,
    STATE_TYPE_BOOL,
    STATE_TYPE_STRING,
    STATE_TYPE_POINTER,
    STATE_TYPE_ARRAY
} state_type;

// State value union
typedef struct {
    state_type type;
    union {
        int int_val;
        float float_val;
        int bool_val;
        char* string_val;
        void* ptr_val;
        struct {
            void** items;
            int count;
        } array_val;
    } value;
} state_value;

// State change callback
typedef void (*state_change_callback)(state* st, const char* key, state_value* old_val, state_value* new_val, void* user_data);

// Effect cleanup function
typedef void (*effect_cleanup)(void* user_data);

// Effect function
typedef void (*effect_function)(void* user_data);

// State structure
struct state {
    char** keys;
    state_value* values;
    int count;
    int capacity;
    
    // State change callbacks
    state_change_callback* callbacks;
    int callback_count;
    int callback_capacity;
    
    // User data for callbacks
    void** callback_user_data;
    
    // Dirty flag for triggering re-render
    int dirty;
};

// State API

// Create a new state object
state* state_new(void);

// Free a state object
void state_free(state* st);

// Set a state value (immutable update)
void state_set(state* st, const char* key, state_value value);

// Get a state value
state_value state_get(state* st, const char* key);

// Check if a key exists
int state_has(state* st, const char* key);

// Remove a key
void state_remove(state* st, const char* key);

// Register a state change callback
void state_on_change(state* st, state_change_callback callback, void* user_data);

// Remove a state change callback
void state_remove_callback(state* st, state_change_callback callback);

// Check if state is dirty (has pending changes)
int state_is_dirty(state* st);

// Clear dirty flag
void state_clear_dirty(state* st);

// Helper functions for creating state values
state_value state_value_int(int val);
state_value state_value_float(float val);
state_value state_value_bool(int val);
state_value state_value_string(const char* val);
state_value state_value_pointer(void* val);
state_value state_value_array(void** items, int count);

// Clone a state value (deep copy)
state_value state_value_clone(state_value val);

// Free a state value
void state_value_free(state_value val);

// Hook system for React-like patterns

// Effect hook - run side effects when state changes
typedef struct effect_hook {
    effect_function func;
    effect_cleanup cleanup;
    void* user_data;
    
    // Dependency keys
    char** deps;
    int dep_count;
    
    // Track if effect has run
    int has_run;
} effect_hook;

// Effect hook array
typedef struct {
    effect_hook* hooks;
    int count;
    int capacity;
} effect_hooks;

// Create effect hooks
effect_hooks* effect_hooks_new(void);

// Free effect hooks
void effect_hooks_free(effect_hooks* hooks);

// Add an effect
void effect_hooks_add(effect_hooks* hooks, effect_function func, effect_cleanup cleanup, void* user_data, char** deps, int dep_count);

// Run effects that depend on changed state
void effect_hooks_run(effect_hooks* hooks, state* st, const char* changed_key);

// Clear all effects
void effect_hooks_clear(effect_hooks* hooks);

// Memo hook - memoize expensive computations
typedef struct memo_cache {
    char* key;
    void* value;
    void (*free_func)(void*);
    
    // Dependencies
    char** deps;
    int dep_count;
    
    // Hash of dependencies
    uint32_t dep_hash;
} memo_cache;

// Memo cache array
typedef struct {
    memo_cache* caches;
    int count;
    int capacity;
} memo_caches;

// Create memo cache
memo_caches* memo_caches_new(void);

// Free memo cache
void memo_caches_free(memo_caches* caches);

// Get or compute memoized value
void* memo_caches_get(memo_caches* caches, const char* key, void* (*compute_func)(void*), void* user_data, char** deps, int dep_count, void (*free_func)(void*));

// Clear memo cache
void memo_caches_clear(memo_caches* caches);

// Compute hash of dependencies
uint32_t compute_dep_hash(char** deps, int dep_count);

#endif // NEXTUI_STATE_H__