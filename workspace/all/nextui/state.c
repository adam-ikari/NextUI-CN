#include "state.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INITIAL_CAPACITY 16
#define GROW_FACTOR 2

// Hash function for strings (djb2)
static uint32_t string_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

// State implementation

state* state_new(void) {
    state* st = (state*)malloc(sizeof(state));
    if (!st) {
        return NULL;
    }
    
    st->keys = (char**)malloc(INITIAL_CAPACITY * sizeof(char*));
    st->values = (state_value*)malloc(INITIAL_CAPACITY * sizeof(state_value));
    st->callbacks = (state_change_callback*)malloc(INITIAL_CAPACITY * sizeof(state_change_callback));
    st->callback_user_data = (void**)malloc(INITIAL_CAPACITY * sizeof(void*));
    
    if (!st->keys || !st->values || !st->callbacks || !st->callback_user_data) {
        if (st->keys) free(st->keys);
        if (st->values) free(st->values);
        if (st->callbacks) free(st->callbacks);
        if (st->callback_user_data) free(st->callback_user_data);
        free(st);
        return NULL;
    }
    
    st->count = 0;
    st->capacity = INITIAL_CAPACITY;
    st->callback_count = 0;
    st->callback_capacity = INITIAL_CAPACITY;
    st->dirty = 0;
    
    return st;
}

void state_free(state* st) {
    if (!st) return;
    
    // Free all keys and values
    for (int i = 0; i < st->count; i++) {
        free(st->keys[i]);
        state_value_free(st->values[i]);
    }
    
    free(st->keys);
    free(st->values);
    free(st->callbacks);
    free(st->callback_user_data);
    free(st);
}

void state_set(state* st, const char* key, state_value value) {
    if (!st || !key) return;
    
    // Check if key already exists
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->keys[i], key) == 0) {
            // Store old value for callback
            state_value old_val = st->values[i];
            
            // Update value
            st->values[i] = value;
            
            // Mark as dirty
            st->dirty = 1;
            
            // Call callbacks
            for (int j = 0; j < st->callback_count; j++) {
                if (st->callbacks[j]) {
                    st->callbacks[j](st, key, &old_val, &value, st->callback_user_data[j]);
                }
            }
            
            // Free old value
            state_value_free(old_val);
            return;
        }
    }
    
    // Key doesn't exist, add it
    if (st->count >= st->capacity) {
        int new_capacity = st->capacity * GROW_FACTOR;
        char** new_keys = (char**)realloc(st->keys, new_capacity * sizeof(char*));
        state_value* new_values = (state_value*)realloc(st->values, new_capacity * sizeof(state_value));
        
        if (!new_keys || !new_values) {
            // Allocation failed, clean up
            state_value_free(value);
            return;
        }
        
        st->keys = new_keys;
        st->values = new_values;
        st->capacity = new_capacity;
    }
    
    // Duplicate key
    st->keys[st->count] = strdup(key);
    if (!st->keys[st->count]) {
        state_value_free(value);
        return;
    }
    
    st->values[st->count] = value;
    st->count++;
    st->dirty = 1;
    
    // Call callbacks
    for (int j = 0; j < st->callback_count; j++) {
        if (st->callbacks[j]) {
            st->callbacks[j](st, key, NULL, &value, st->callback_user_data[j]);
        }
    }
}

state_value state_get(state* st, const char* key) {
    if (!st || !key) {
        return state_value_int(0);
    }
    
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->keys[i], key) == 0) {
            return st->values[i];
        }
    }
    
    return state_value_int(0);
}

int state_has(state* st, const char* key) {
    if (!st || !key) return 0;
    
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->keys[i], key) == 0) {
            return 1;
        }
    }
    
    return 0;
}

void state_remove(state* st, const char* key) {
    if (!st || !key) return;
    
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->keys[i], key) == 0) {
            // Store old value for callback
            state_value old_val = st->values[i];
            
            // Free key
            free(st->keys[i]);
            
            // Shift remaining items
            for (int j = i; j < st->count - 1; j++) {
                st->keys[j] = st->keys[j + 1];
                st->values[j] = st->values[j + 1];
            }
            
            st->count--;
            st->dirty = 1;
            
            // Call callbacks
            for (int j = 0; j < st->callback_count; j++) {
                if (st->callbacks[j]) {
                    st->callbacks[j](st, key, &old_val, NULL, st->callback_user_data[j]);
                }
            }
            
            // Free old value
            state_value_free(old_val);
            return;
        }
    }
}

void state_on_change(state* st, state_change_callback callback, void* user_data) {
    if (!st || !callback) return;
    
    if (st->callback_count >= st->callback_capacity) {
        int new_capacity = st->callback_capacity * GROW_FACTOR;
        state_change_callback* new_callbacks = (state_change_callback*)realloc(st->callbacks, new_capacity * sizeof(state_change_callback));
        void** new_user_data = (void**)realloc(st->callback_user_data, new_capacity * sizeof(void*));
        
        if (!new_callbacks || !new_user_data) {
            return;
        }
        
        st->callbacks = new_callbacks;
        st->callback_user_data = new_user_data;
        st->callback_capacity = new_capacity;
    }
    
    st->callbacks[st->callback_count] = callback;
    st->callback_user_data[st->callback_count] = user_data;
    st->callback_count++;
}

void state_remove_callback(state* st, state_change_callback callback) {
    if (!st || !callback) return;
    
    for (int i = 0; i < st->callback_count; i++) {
        if (st->callbacks[i] == callback) {
            // Shift remaining callbacks
            for (int j = i; j < st->callback_count - 1; j++) {
                st->callbacks[j] = st->callbacks[j + 1];
                st->callback_user_data[j] = st->callback_user_data[j + 1];
            }
            st->callback_count--;
            return;
        }
    }
}

int state_is_dirty(state* st) {
    return st ? st->dirty : 0;
}

void state_clear_dirty(state* st) {
    if (st) {
        st->dirty = 0;
    }
}

// Helper functions for creating state values

state_value state_value_int(int val) {
    state_value v;
    v.type = STATE_TYPE_INT;
    v.value.int_val = val;
    return v;
}

state_value state_value_float(float val) {
    state_value v;
    v.type = STATE_TYPE_FLOAT;
    v.value.float_val = val;
    return v;
}

state_value state_value_bool(int val) {
    state_value v;
    v.type = STATE_TYPE_BOOL;
    v.value.bool_val = val;
    return v;
}

state_value state_value_string(const char* val) {
    state_value v;
    v.type = STATE_TYPE_STRING;
    v.value.string_val = val ? strdup(val) : NULL;
    return v;
}

state_value state_value_pointer(void* val) {
    state_value v;
    v.type = STATE_TYPE_POINTER;
    v.value.ptr_val = val;
    return v;
}

state_value state_value_array(void** items, int count) {
    state_value v;
    v.type = STATE_TYPE_ARRAY;
    
    if (items && count > 0) {
        v.value.array_val.items = (void**)malloc(count * sizeof(void*));
        if (v.value.array_val.items) {
            memcpy(v.value.array_val.items, items, count * sizeof(void*));
            v.value.array_val.count = count;
        } else {
            v.value.array_val.items = NULL;
            v.value.array_val.count = 0;
        }
    } else {
        v.value.array_val.items = NULL;
        v.value.array_val.count = 0;
    }
    
    return v;
}

state_value state_value_clone(state_value val) {
    state_value clone;
    clone.type = val.type;
    
    switch (val.type) {
        case STATE_TYPE_INT:
            clone.value.int_val = val.value.int_val;
            break;
        case STATE_TYPE_FLOAT:
            clone.value.float_val = val.value.float_val;
            break;
        case STATE_TYPE_BOOL:
            clone.value.bool_val = val.value.bool_val;
            break;
        case STATE_TYPE_STRING:
            clone.value.string_val = val.value.string_val ? strdup(val.value.string_val) : NULL;
            break;
        case STATE_TYPE_POINTER:
            clone.value.ptr_val = val.value.ptr_val;
            break;
        case STATE_TYPE_ARRAY:
            if (val.value.array_val.items && val.value.array_val.count > 0) {
                clone.value.array_val.items = (void**)malloc(val.value.array_val.count * sizeof(void*));
                if (clone.value.array_val.items) {
                    memcpy(clone.value.array_val.items, val.value.array_val.items, val.value.array_val.count * sizeof(void*));
                    clone.value.array_val.count = val.value.array_val.count;
                } else {
                    clone.value.array_val.items = NULL;
                    clone.value.array_val.count = 0;
                }
            } else {
                clone.value.array_val.items = NULL;
                clone.value.array_val.count = 0;
            }
            break;
    }
    
    return clone;
}

void state_value_free(state_value val) {
    switch (val.type) {
        case STATE_TYPE_STRING:
            if (val.value.string_val) {
                free(val.value.string_val);
            }
            break;
        case STATE_TYPE_ARRAY:
            if (val.value.array_val.items) {
                free(val.value.array_val.items);
            }
            break;
        default:
            break;
    }
}

// Hook system implementation

effect_hooks* effect_hooks_new(void) {
    effect_hooks* hooks = (effect_hooks*)malloc(sizeof(effect_hooks));
    if (!hooks) {
        return NULL;
    }
    
    hooks->hooks = (effect_hook*)malloc(INITIAL_CAPACITY * sizeof(effect_hook));
    if (!hooks->hooks) {
        free(hooks);
        return NULL;
    }
    
    hooks->count = 0;
    hooks->capacity = INITIAL_CAPACITY;
    
    return hooks;
}

void effect_hooks_free(effect_hooks* hooks) {
    if (!hooks) return;
    
    // Cleanup all effects
    for (int i = 0; i < hooks->count; i++) {
        if (hooks->hooks[i].cleanup && hooks->hooks[i].has_run) {
            hooks->hooks[i].cleanup(hooks->hooks[i].user_data);
        }
        if (hooks->hooks[i].deps) {
            free(hooks->hooks[i].deps);
        }
    }
    
    free(hooks->hooks);
    free(hooks);
}

void effect_hooks_add(effect_hooks* hooks, effect_function func, effect_cleanup cleanup, void* user_data, char** deps, int dep_count) {
    if (!hooks || !func) return;
    
    if (hooks->count >= hooks->capacity) {
        int new_capacity = hooks->capacity * GROW_FACTOR;
        effect_hook* new_hooks = (effect_hook*)realloc(hooks->hooks, new_capacity * sizeof(effect_hook));
        if (!new_hooks) {
            return;
        }
        hooks->hooks = new_hooks;
        hooks->capacity = new_capacity;
    }
    
    hooks->hooks[hooks->count].func = func;
    hooks->hooks[hooks->count].cleanup = cleanup;
    hooks->hooks[hooks->count].user_data = user_data;
    hooks->hooks[hooks->count].has_run = 0;
    
    // Copy dependencies
    if (deps && dep_count > 0) {
        hooks->hooks[hooks->count].deps = (char**)malloc(dep_count * sizeof(char*));
        if (hooks->hooks[hooks->count].deps) {
            for (int i = 0; i < dep_count; i++) {
                hooks->hooks[hooks->count].deps[i] = deps[i] ? strdup(deps[i]) : NULL;
            }
            hooks->hooks[hooks->count].dep_count = dep_count;
        } else {
            hooks->hooks[hooks->count].deps = NULL;
            hooks->hooks[hooks->count].dep_count = 0;
        }
    } else {
        hooks->hooks[hooks->count].deps = NULL;
        hooks->hooks[hooks->count].dep_count = 0;
    }
    
    hooks->count++;
}

void effect_hooks_run(effect_hooks* hooks, state* st, const char* changed_key) {
    if (!hooks || !st || !changed_key) return;
    
    for (int i = 0; i < hooks->count; i++) {
        effect_hook* hook = &hooks->hooks[i];
        
        // Check if this effect depends on the changed key
        int should_run = 0;
        if (hook->dep_count == 0) {
            // No dependencies, run on every change
            should_run = 1;
        } else {
            // Check if changed_key is in dependencies
            for (int j = 0; j < hook->dep_count; j++) {
                if (hook->deps[j] && strcmp(hook->deps[j], changed_key) == 0) {
                    should_run = 1;
                    break;
                }
            }
        }
        
        if (should_run && hook->func) {
            // Cleanup previous effect if it has run
            if (hook->has_run && hook->cleanup) {
                hook->cleanup(hook->user_data);
            }
            
            // Run effect
            hook->func(hook->user_data);
            hook->has_run = 1;
        }
    }
}

void effect_hooks_clear(effect_hooks* hooks) {
    if (!hooks) return;
    
    // Cleanup all effects
    for (int i = 0; i < hooks->count; i++) {
        if (hooks->hooks[i].cleanup && hooks->hooks[i].has_run) {
            hooks->hooks[i].cleanup(hooks->hooks[i].user_data);
        }
        if (hooks->hooks[i].deps) {
            for (int j = 0; j < hooks->hooks[i].dep_count; j++) {
                if (hooks->hooks[i].deps[j]) {
                    free(hooks->hooks[i].deps[j]);
                }
            }
            free(hooks->hooks[i].deps);
        }
    }
    
    hooks->count = 0;
}

// Memo cache implementation

memo_caches* memo_caches_new(void) {
    memo_caches* caches = (memo_caches*)malloc(sizeof(memo_caches));
    if (!caches) {
        return NULL;
    }
    
    caches->caches = (memo_cache*)malloc(INITIAL_CAPACITY * sizeof(memo_cache));
    if (!caches->caches) {
        free(caches);
        return NULL;
    }
    
    caches->count = 0;
    caches->capacity = INITIAL_CAPACITY;
    
    return caches;
}

void memo_caches_free(memo_caches* caches) {
    if (!caches) return;
    
    // Free all caches
    for (int i = 0; i < caches->count; i++) {
        if (caches->caches[i].key) {
            free(caches->caches[i].key);
        }
        if (caches->caches[i].value && caches->caches[i].free_func) {
            caches->caches[i].free_func(caches->caches[i].value);
        }
        if (caches->caches[i].deps) {
            for (int j = 0; j < caches->caches[i].dep_count; j++) {
                if (caches->caches[i].deps[j]) {
                    free(caches->caches[i].deps[j]);
                }
            }
            free(caches->caches[i].deps);
        }
    }
    
    free(caches->caches);
    free(caches);
}

uint32_t compute_dep_hash(char** deps, int dep_count) {
    if (!deps || dep_count == 0) {
        return 0;
    }
    
    uint32_t hash = 0;
    for (int i = 0; i < dep_count; i++) {
        if (deps[i]) {
            hash ^= string_hash(deps[i]);
        }
    }
    return hash;
}

void* memo_caches_get(memo_caches* caches, const char* key, void* (*compute_func)(void*), void* user_data, char** deps, int dep_count, void (*free_func)(void*)) {
    if (!caches || !key || !compute_func) {
        return NULL;
    }
    
    uint32_t dep_hash = compute_dep_hash(deps, dep_count);
    
    // Check if cache exists and dependencies haven't changed
    for (int i = 0; i < caches->count; i++) {
        if (strcmp(caches->caches[i].key, key) == 0 && caches->caches[i].dep_hash == dep_hash) {
            return caches->caches[i].value;
        }
    }
    
    // Compute new value
    void* value = compute_func(user_data);
    
    // Check if key already exists
    for (int i = 0; i < caches->count; i++) {
        if (strcmp(caches->caches[i].key, key) == 0) {
            // Store old value and deps for cleanup if needed
            void* old_value = caches->caches[i].value;
            void (*old_free_func)(void*) = caches->caches[i].free_func;
            char** old_deps = caches->caches[i].deps;
            int old_dep_count = caches->caches[i].dep_count;
            
            // Prepare new dependencies first
            char** new_deps = NULL;
            int new_dep_count = 0;
            
            if (deps && dep_count > 0) {
                new_deps = (char**)malloc(dep_count * sizeof(char*));
                if (!new_deps) {
                    // Allocation failed, free new value and return NULL
                    if (value && free_func) {
                        free_func(value);
                    }
                    return NULL;
                }
                
                for (int j = 0; j < dep_count; j++) {
                    new_deps[j] = deps[j] ? strdup(deps[j]) : NULL;
                    if (deps[j] && !new_deps[j]) {
                        // strdup failed, clean up and return NULL
                        for (int k = 0; k < j; k++) {
                            if (new_deps[k]) free(new_deps[k]);
                        }
                        free(new_deps);
                        if (value && free_func) {
                            free_func(value);
                        }
                        return NULL;
                    }
                }
                new_dep_count = dep_count;
            }
            
            // All allocations succeeded, now update cache
            caches->caches[i].value = value;
            caches->caches[i].free_func = free_func;
            caches->caches[i].dep_hash = dep_hash;
            caches->caches[i].deps = new_deps;
            caches->caches[i].dep_count = new_dep_count;
            
            // Free old value and deps
            if (old_value && old_free_func) {
                old_free_func(old_value);
            }
            if (old_deps) {
                for (int j = 0; j < old_dep_count; j++) {
                    if (old_deps[j]) {
                        free(old_deps[j]);
                    }
                }
                free(old_deps);
            }
            
            return value;
        }
    }
    
    // Add new cache entry
    if (caches->count >= caches->capacity) {
        int new_capacity = caches->capacity * GROW_FACTOR;
        memo_cache* new_caches = (memo_cache*)realloc(caches->caches, new_capacity * sizeof(memo_cache));
        if (!new_caches) {
            if (value && free_func) {
                free_func(value);
            }
            return NULL;
        }
        caches->caches = new_caches;
        caches->capacity = new_capacity;
    }
    
    caches->caches[caches->count].key = strdup(key);
    caches->caches[caches->count].value = value;
    caches->caches[caches->count].free_func = free_func;
    caches->caches[caches->count].dep_hash = dep_hash;
    
    if (deps && dep_count > 0) {
        caches->caches[caches->count].deps = (char**)malloc(dep_count * sizeof(char*));
        if (caches->caches[caches->count].deps) {
            for (int j = 0; j < dep_count; j++) {
                caches->caches[caches->count].deps[j] = deps[j] ? strdup(deps[j]) : NULL;
            }
            caches->caches[caches->count].dep_count = dep_count;
        } else {
            caches->caches[caches->count].deps = NULL;
            caches->caches[caches->count].dep_count = 0;
        }
    } else {
        caches->caches[caches->count].deps = NULL;
        caches->caches[caches->count].dep_count = 0;
    }
    
    caches->count++;
    
    return value;
}

void memo_caches_clear(memo_caches* caches) {
    if (!caches) return;
    
    // Free all caches
    for (int i = 0; i < caches->count; i++) {
        if (caches->caches[i].key) {
            free(caches->caches[i].key);
        }
        if (caches->caches[i].value && caches->caches[i].free_func) {
            caches->caches[i].free_func(caches->caches[i].value);
        }
        if (caches->caches[i].deps) {
            for (int j = 0; j < caches->caches[i].dep_count; j++) {
                if (caches->caches[i].deps[j]) {
                    free(caches->caches[i].deps[j]);
                }
            }
            free(caches->caches[i].deps);
        }
    }
    
    caches->count = 0;
}