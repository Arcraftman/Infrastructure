#include "stk/core/hashmap.h"
#include <stdlib.h>
#include <string.h>

/* Fowler-Noll-Vo 1a 64-bit hash */
uint64_t hashmap_str_hash(const void *key) {
    const unsigned char *p = (const unsigned char *)key;
    uint64_t h = 0xcbf29ce484222325ULL;
    while (*p) { h ^= *p; h *= 0x100000001b3ULL; p++; }
    return h;
}

bool hashmap_str_eq(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b) == 0;
}

static uint64_t hashmap_hash(const hashmap *m, const void *key) {
    uint64_t h = m->hash_fn(key);
    /* Ensure no tombstone bit-flag conflicts: mask to capacity-1 */
    return h & (uint64_t)(m->capacity - 1);
}

static size_t hashmap_probe(const hashmap *m, const void *key) {
    uint64_t idx = hashmap_hash(m, key);
    while (m->entries[idx].occupied || m->entries[idx].tombstone) {
        if (m->entries[idx].occupied && m->eq_fn(m->entries[idx].key, key))
            break;
        idx = (idx + 1) & (uint64_t)(m->capacity - 1);
    }
    return (size_t)idx;
}

static bool hashmap_grow(hashmap *m, size_t new_cap) {
    hashmap_entry *old = m->entries;
    size_t old_cap = m->capacity;
    m->entries = (hashmap_entry *)calloc(new_cap, sizeof(hashmap_entry));
    if (!m->entries) return false;
    m->capacity   = new_cap;
    m->count      = 0;
    m->tombstones = 0;
    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].occupied) {
            hashmap_set(m, old[i].key, old[i].value);
        }
    }
    free(old);
    return true;
}

void hashmap_init(hashmap *m, size_t initial_capacity,
                  hashmap_hash_fn hash_fn, hashmap_eq_fn eq_fn) {
    if (initial_capacity < 8) initial_capacity = 8;
    /* Round up to next power of two */
    size_t cap = 1;
    while (cap < initial_capacity) cap <<= 1;
    m->entries    = (hashmap_entry *)calloc(cap, sizeof(hashmap_entry));
    m->capacity   = m->entries ? cap : 0;
    m->count      = 0;
    m->tombstones = 0;
    m->hash_fn    = hash_fn ? hash_fn : hashmap_str_hash;
    m->eq_fn      = eq_fn   ? eq_fn   : hashmap_str_eq;
}

void hashmap_free(hashmap *m) {
    free(m->entries);
    m->entries    = NULL;
    m->capacity   = 0;
    m->count      = 0;
    m->tombstones = 0;
}

void hashmap_set(hashmap *m, void *key, void *value) {
    if ((double)(m->count + m->tombstones + 1) / (double)m->capacity > 0.7) {
        if (!hashmap_grow(m, m->capacity * 2)) return;
    }
    size_t idx = hashmap_probe(m, key);
    if (m->entries[idx].occupied) {
        m->entries[idx].value = value;
        return;
    }
    m->entries[idx].key      = key;
    m->entries[idx].value    = value;
    m->entries[idx].occupied = true;
    m->entries[idx].tombstone = false;
    m->count++;
}

void *hashmap_get(const hashmap *m, const void *key) {
    size_t idx = hashmap_probe(m, key);
    return m->entries[idx].occupied ? m->entries[idx].value : NULL;
}

bool hashmap_has(const hashmap *m, const void *key) {
    size_t idx = hashmap_probe(m, key);
    return m->entries[idx].occupied;
}

void *hashmap_remove(hashmap *m, const void *key) {
    size_t idx = hashmap_probe(m, key);
    if (!m->entries[idx].occupied) return NULL;
    void *val = m->entries[idx].value;
    m->entries[idx].occupied  = false;
    m->entries[idx].tombstone = true;
    m->count--;
    m->tombstones++;
    return val;
}

void hashmap_clear(hashmap *m) {
    memset(m->entries, 0, m->capacity * sizeof(hashmap_entry));
    m->count      = 0;
    m->tombstones = 0;
}

void hashmap_foreach(const hashmap *m,
                     bool (*fn)(void *key, void *value, void *ud), void *ud) {
    for (size_t i = 0; i < m->capacity; i++) {
        if (m->entries[i].occupied) {
            if (!fn(m->entries[i].key, m->entries[i].value, ud))
                break;
        }
    }
}

size_t hashmap_count(const hashmap *m)     { return m->count; }
size_t hashmap_capacity(const hashmap *m)  { return m->capacity; }
double hashmap_load_factor(const hashmap *m) {
    return m->capacity ? (double)m->count / (double)m->capacity : 0.0;
}
