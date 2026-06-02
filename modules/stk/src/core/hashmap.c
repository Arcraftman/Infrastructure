#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/hashmap.h"

/* Fowler-Noll-Vo 1a 64-bit hash */
uint64_t stk_hashmap_str_hash(const void *key) {
    const unsigned char *p = (const unsigned char *)key;
    uint64_t h = 0xcbf29ce484222325ULL;
    while (*p) { h ^= *p; h *= 0x100000001b3ULL; p++; }
    return h;
}

bool stk_hashmap_str_eq(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b) == 0;
}

static uint64_t hashmap_hash(const stk_hashmap *m, const void *key) {
    uint64_t h = m->hash_fn(key);
    return h & (uint64_t)(m->capacity - 1);
}

static size_t hashmap_probe(const stk_hashmap *m, const void *key) {
    uint64_t idx = hashmap_hash(m, key);
    while (m->entries[idx].occupied || m->entries[idx].tombstone) {
        if (m->entries[idx].occupied && m->eq_fn(m->entries[idx].key, key))
            break;
        idx = (idx + 1) & (uint64_t)(m->capacity - 1);
    }
    return (size_t)idx;
}

static bool hashmap_grow(stk_hashmap *m, size_t new_cap) {
    stk_hashmap_entry *old = m->entries;
    size_t old_cap = m->capacity;
    
    m->entries = (stk_hashmap_entry *)calloc(new_cap, sizeof(stk_hashmap_entry));
    if (!m->entries) {
        STK_LOG_ERROR("Hashmap grow: failed to allocate %zu entries", new_cap);
        return false;
    }
    
    m->capacity   = new_cap;
    m->count      = 0;
    m->tombstones = 0;
    
    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].occupied) {
            stk_hashmap_set(m, old[i].key, old[i].value);
        }
    }
    free(old);
    
    STK_LOG_DEBUG("Hashmap grew: capacity=%zu -> %zu", old_cap, new_cap);
    return true;
}

STK_STATUS stk_hashmap_init(stk_hashmap *m, size_t initial_capacity,
                            stk_hashmap_hash_fn hash_fn, stk_hashmap_eq_fn eq_fn) {
    STK_RETURN_IF(!m, STK_EINVAL, "Hashmap init: NULL hashmap pointer");
    
    if (initial_capacity < 8) initial_capacity = 8;
    size_t cap = 1;
    while (cap < initial_capacity) cap <<= 1;
    
    m->entries = (stk_hashmap_entry *)calloc(cap, sizeof(stk_hashmap_entry));
    if (!m->entries) {
        STK_LOG_ERROR("Hashmap init: failed to allocate %zu entries", cap);
        return STK_ENOMEM;
    }
    
    m->capacity   = cap;
    m->count      = 0;
    m->tombstones = 0;
    m->hash_fn    = hash_fn ? hash_fn : stk_hashmap_str_hash;
    m->eq_fn      = eq_fn   ? eq_fn   : stk_hashmap_str_eq;
    
    STK_LOG_DEBUG("Hashmap init: capacity=%zu", cap);
    return STK_OK;
}

STK_STATUS stk_hashmap_free(stk_hashmap *m) {
    if (!m) {
        STK_LOG_WARN("Hashmap free: NULL hashmap pointer");
        return STK_EINVAL;
    }
    free(m->entries);
    m->entries    = NULL;
    m->capacity   = 0;
    m->count      = 0;
    m->tombstones = 0;
    STK_LOG_DEBUG("Hashmap freed");
    return STK_OK;
}

STK_STATUS stk_hashmap_set(stk_hashmap *m, void *key, void *value) {
    STK_RETURN_IF(!m, STK_EINVAL, "Hashmap set: NULL hashmap pointer");
    STK_RETURN_IF(!key, STK_EINVAL, "Hashmap set: NULL key");
    
    if ((double)(m->count + m->tombstones + 1) / (double)m->capacity > 0.7) {
        if (!hashmap_grow(m, m->capacity * 2)) {
            STK_LOG_ERROR("Hashmap set: failed to grow hashmap");
            return STK_ENOMEM;
        }
    }
    
    size_t idx = hashmap_probe(m, key);
    if (m->entries[idx].occupied) {
        STK_LOG_DEBUG("Hashmap set: updating existing key");
        m->entries[idx].value = value;
        return STK_OK;
    }
    
    m->entries[idx].key       = key;
    m->entries[idx].value     = value;
    m->entries[idx].occupied  = true;
    m->entries[idx].tombstone = false;
    m->count++;
    
    STK_LOG_DEBUG("Hashmap set: added new entry, count=%zu", m->count);
    return STK_OK;
}

void *stk_hashmap_get(const stk_hashmap *m, const void *key) {
    if (!m || !key) {
        if (m) STK_LOG_WARN("Hashmap get: NULL %s", !m ? "hashmap" : "key");
        return NULL;
    }
    size_t idx = hashmap_probe(m, key);
    return m->entries[idx].occupied ? m->entries[idx].value : NULL;
}

bool stk_hashmap_has(const stk_hashmap *m, const void *key) {
    if (!m || !key) return false;
    size_t idx = hashmap_probe(m, key);
    return m->entries[idx].occupied;
}

void *stk_hashmap_remove(stk_hashmap *m, const void *key) {
    if (!m || !key) {
        if (m) STK_LOG_WARN("Hashmap remove: NULL %s", !m ? "hashmap" : "key");
        return NULL;
    }
    
    size_t idx = hashmap_probe(m, key);
    if (!m->entries[idx].occupied) {
        STK_LOG_DEBUG("Hashmap remove: key not found");
        return NULL;
    }
    
    void *val = m->entries[idx].value;
    m->entries[idx].occupied  = false;
    m->entries[idx].tombstone = true;
    m->count--;
    m->tombstones++;
    
    STK_LOG_DEBUG("Hashmap remove: removed entry, count=%zu", m->count);
    return val;
}

STK_STATUS stk_hashmap_clear(stk_hashmap *m) {
    STK_RETURN_IF(!m, STK_EINVAL, "Hashmap clear: NULL hashmap pointer");
    
    memset(m->entries, 0, m->capacity * sizeof(stk_hashmap_entry));
    m->count      = 0;
    m->tombstones = 0;
    
    STK_LOG_DEBUG("Hashmap cleared");
    return STK_OK;
}

STK_STATUS stk_hashmap_foreach(const stk_hashmap *m,
                               bool (*fn)(void *key, void *value, void *ud), void *ud) {
    STK_RETURN_IF(!m, STK_EINVAL, "Hashmap foreach: NULL hashmap pointer");
    STK_RETURN_IF(!fn, STK_EINVAL, "Hashmap foreach: NULL callback function");
    
    for (size_t i = 0; i < m->capacity; i++) {
        if (m->entries[i].occupied) {
            if (!fn(m->entries[i].key, m->entries[i].value, ud))
                break;
        }
    }
    return STK_OK;
}

size_t stk_hashmap_count(const stk_hashmap *m)     { return m ? m->count : 0; }
size_t stk_hashmap_capacity(const stk_hashmap *m)  { return m ? m->capacity : 0; }
double stk_hashmap_load_factor(const stk_hashmap *m) {
    return (m && m->capacity) ? (double)m->count / (double)m->capacity : 0.0;
}