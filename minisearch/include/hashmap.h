
#ifndef HASHMAP_H
#define HASHMAP_H
#include <stddef.h>
#include <stdbool.h>

typedef struct HashMap HashMap;

// Simple hashmap from const char* (owned copy) -> void* (value pointer).
HashMap* hm_create(size_t initial_capacity);
void hm_free(HashMap *hm, void (*free_value)(void*));

// Inserts or finds existing. If key exists, returns existing value pointer and sets *inserted=false.
// Otherwise inserts value (caller provided) and sets *inserted=true.
void* hm_put(HashMap *hm, const char *key, void *value, bool *inserted);

// Returns value or NULL.
void* hm_get(HashMap *hm, const char *key);

// Iterate all keys/values. The callback may not modify the map structure.
typedef void (*hm_iter_cb)(const char *key, void *value, void *user);
void hm_foreach(HashMap *hm, hm_iter_cb cb, void *user);

size_t hm_size(HashMap *hm);
#endif
