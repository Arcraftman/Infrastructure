/*
 * stk_hashmap — a small, allocation-injectable hash map with two selectable
 * backends (separate chaining / open addressing with tombstones).
 *
 * Engineering notes:
 *   - Every heap allocation goes through an injected stk_allocator, so the
 *     map is testable (counting allocator) and embeddable (arena/pool).
 *   - The backend is chosen once at construction and dispatched via a static
 *     vtable (struct hashmap_ops).  Call sites stay backend-agnostic.
 *   - Iteration is offered two ways: a snapshot-based foreach (push) that is
 *     safe against in-callback removal, and an explicit pull iterator that
 *     supports removing the current entry without dangling.
 *   - The map never owns keys or values; the caller owns them and is
 *     responsible for freeing them (this is why set.c, which uses the map
 *     with NULL values, owns nothing extra).
 */

#include "stk/core/hashmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stk/core/preset.h"
#include "stk/def.h"
#include "stk/utils/logger.h"
#include "stk/utils/status.h"

/* ------------------------------------------------------------------ */
/* Allocator plumbing                                                 */
/* ------------------------------------------------------------------ */

static void* stk_libc_alloc(void* ctx, size_t size) { (void)ctx; return malloc(size); }
static void  stk_libc_free(void* ctx, void* ptr)     { (void)ctx; free(ptr); }

const stk_allocator STK_ALLOCATOR_DEFAULT = {
    .alloc = stk_libc_alloc,
    .free  = stk_libc_free,
    .ctx   = NULL,
};

static inline void* stk_alloc(const stk_hashmap* m, size_t size)
{
    const stk_allocator* a = m->alloc ? m->alloc : &STK_ALLOCATOR_DEFAULT;
    return a->alloc(a->ctx, size);
}

static inline void stk_dealloc(const stk_hashmap* m, void* ptr)
{
    if (!ptr)
        return;
    const stk_allocator* a = m->alloc ? m->alloc : &STK_ALLOCATOR_DEFAULT;
    a->free(a->ctx, ptr);
}

/* ------------------------------------------------------------------ */
/* Default hash / equality functions                                 */
/* ------------------------------------------------------------------ */

uint64_t stk_hashmap_str_hash(const void* key)
{
    if (!key)
        return 0;   /* never dereference a NULL key */
    const unsigned char* p = (const unsigned char*)key;
    uint64_t h = 0xcbf29ce484222325ULL;   /* FNV-1a 64 */
    while (*p) {
        h ^= *p;
        h *= 0x100000001b3ULL;
        p++;
    }
    return h;
}

bool stk_hashmap_str_eq(const void* a, const void* b)
{
    return strcmp((const char*)a, (const char*)b) == 0;
}

static uint64_t ptr_int_hash(const void* key)
{
    uintptr_t v = (uintptr_t)key;
    uint64_t h = 0xcbf29ce484222325ULL;
    h ^= (uint64_t)v;
    h *= 0x100000001b3ULL;
    h ^= (uint64_t)(v >> 32);
    h *= 0x100000001b3ULL;
    return h;
}

uint64_t stk_hashmap_ptr_hash(const void* key) { return ptr_int_hash(key); }
uint64_t stk_hashmap_int_hash(const void* key) { return ptr_int_hash(key); }

bool stk_hashmap_ptr_eq(const void* a, const void* b) { return a == b; }
bool stk_hashmap_int_eq(const void* a, const void* b) { return a == b; }

/* ------------------------------------------------------------------ */
/* Backend strategy (Strategy pattern via a static vtable)            */
/* ------------------------------------------------------------------ */

/* These operate on the common stk_hashmap struct.  open-addressing
 * uses the `entries` array directly; chaining uses `entries[i].next`
 * as bucket heads plus heap-allocated nodes. */

typedef struct {
    /* find the entry holding `key`; returns its index/slot.
     * For chaining this is unused (nodes carry their own links). */
    size_t (*find_slot)(const stk_hashmap* m, const void* key);
    /* locate where to insert `key` (existing key -> update slot). */
    size_t (*insert_slot)(const stk_hashmap* m, const void* key);
    /* rehash all live entries from `old` (cap old_cap) into the already
     * reallocated m (m->entries, m->capacity set to new_cap). */
    void   (*rehash)(stk_hashmap* m, stk_hashmap_entry* old, size_t old_cap);
    void   (*free_nodes)(stk_hashmap* m);          /* free chaining nodes  */
    void   (*clear_nodes)(stk_hashmap* m);         /* clear but keep array */
    bool   (*grow_needed)(const stk_hashmap* m);   /* load-factor check    */
} hashmap_ops;

static size_t oa_find_slot(const stk_hashmap* m, const void* key)
{
    uint64_t mask = (uint64_t)(m->capacity - 1);
    uint64_t idx = m->hash_fn(key) & mask;
    for (;;) {
        const stk_hashmap_entry* e = &m->entries[idx];
        if (e->occupied && m->eq_fn(e->key, key))
            return (size_t)idx;                 /* found */
        if (!e->occupied && !e->tombstone)
            return (size_t)idx;                 /* first truly empty */
        idx = (idx + 1) & mask;                 /* probe / skip tombstone */
    }
}

static size_t oa_insert_slot(const stk_hashmap* m, const void* key)
{
    uint64_t mask = (uint64_t)(m->capacity - 1);
    uint64_t idx = m->hash_fn(key) & mask;
    size_t first_tomb = (size_t)-1;
    for (;;) {
        const stk_hashmap_entry* e = &m->entries[idx];
        if (e->occupied && m->eq_fn(e->key, key))
            return (size_t)idx;                 /* existing key -> update */
        if (!e->occupied) {
            if (e->tombstone) {
                if (first_tomb == (size_t)-1)
                    first_tomb = (size_t)idx;   /* remember reusable slot */
            } else {
                return (size_t)idx;             /* truly empty slot */
            }
        }
        /* occupied-but-not-match, or a tombstone: keep probing.
         * Terminates because the load factor keeps at least one empty slot,
         * and a pre-existing key is always found before wrapping. */
        idx = (idx + 1) & mask;
    }
}

static void oa_rehash(stk_hashmap* m, stk_hashmap_entry* old, size_t old_cap)
{
    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].occupied)
            stk_hashmap_set(m, old[i].key, old[i].value);
    }
}

static void oa_free_nodes(stk_hashmap* m)   { (void)m; }
static void oa_clear_nodes(stk_hashmap* m)  { memset(m->entries, 0, m->capacity * sizeof(stk_hashmap_entry)); }
static bool oa_grow_needed(const stk_hashmap* m)
{
    return (double)(m->count + m->tombstones + 1) / (double)m->capacity > 0.7;
}

static void ch_free_nodes(stk_hashmap* m)
{
    for (size_t i = 0; i < m->capacity; i++) {
        stk_hashmap_entry* e = m->entries[i].next;
        while (e) {
            stk_hashmap_entry* nx = e->next;
            stk_dealloc(m, e);
            e = nx;
        }
        m->entries[i].next = NULL;
    }
}

static void ch_clear_nodes(stk_hashmap* m) { ch_free_nodes(m); }

static void ch_rehash(stk_hashmap* m, stk_hashmap_entry* old, size_t old_cap)
{
    for (size_t i = 0; i < old_cap; i++) {
        stk_hashmap_entry* e = old[i].next;
        while (e) {
            stk_hashmap_entry* nx = e->next;
            uint64_t idx = m->hash_fn(e->key) & (uint64_t)(m->capacity - 1);
            e->next = m->entries[idx].next;
            m->entries[idx].next = e;
            m->count++;
            e = nx;
        }
    }
}

static bool ch_grow_needed(const stk_hashmap* m)
{
    return (double)m->count / (double)m->capacity > 0.7;
}

static const hashmap_ops OPS[2] = {
    [HASHMAP_MODE_DEFAULT] = {
        .find_slot    = NULL,            /* chaining searches its own list */
        .insert_slot  = NULL,
        .rehash       = ch_rehash,
        .free_nodes   = ch_free_nodes,
        .clear_nodes  = ch_clear_nodes,
        .grow_needed  = ch_grow_needed,
    },
    [HASHMAP_MODE_OPEN_ADDRESS] = {
        .find_slot    = oa_find_slot,
        .insert_slot  = oa_insert_slot,
        .rehash       = oa_rehash,
        .free_nodes   = oa_free_nodes,
        .clear_nodes  = oa_clear_nodes,
        .grow_needed  = oa_grow_needed,
    },
};

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static inline uint64_t hashmap_mask(const stk_hashmap* m) { return (uint64_t)(m->capacity - 1); }

/* Grow to an exact (caller-guaranteed power-of-two) capacity.  Shared by
 * the load-factor auto-grow and by stk_hashmap_reserve. */
static bool hashmap_grow_to(stk_hashmap* m, size_t new_cap)
{
    stk_hashmap_entry* old = m->entries;
    size_t old_cap = m->capacity;
    m->entries = (stk_hashmap_entry*)stk_alloc(m, new_cap * sizeof(stk_hashmap_entry));
    if (!m->entries) {
        m->entries = old;
        return false;
    }
    memset(m->entries, 0, new_cap * sizeof(stk_hashmap_entry));
    m->capacity = new_cap;
    m->count = 0;
    m->tombstones = 0;
    OPS[m->mode].rehash(m, old, old_cap);
    stk_dealloc(m, old);
    return true;
}

static bool hashmap_grow(stk_hashmap* m)
{
    size_t new_cap = m->capacity * 2;
    stk_hashmap_entry* old = m->entries;
    size_t old_cap = m->capacity;

    m->entries = (stk_hashmap_entry*)stk_alloc(m, new_cap * sizeof(stk_hashmap_entry));
    if (!m->entries) {
        STK_LOG_ERROR("hashmap: grow failed allocating %zu slots", new_cap);
        m->entries = old;   /* keep the map usable at the old capacity */
        return false;
    }
    memset(m->entries, 0, new_cap * sizeof(stk_hashmap_entry));
    m->capacity = new_cap;
    m->count = 0;
    m->tombstones = 0;

    OPS[m->mode].rehash(m, old, old_cap);
    stk_dealloc(m, old);
    STK_LOG_DEBUG("hashmap: grew to %zu slots", new_cap);
    return true;
}

/* ------------------------------------------------------------------ */
/* Construction / destruction                                         */
/* ------------------------------------------------------------------ */

STK_STATUS stk_hashmap_create_with_alloc(stk_hashmap* m,
                                         HASHMAP_MODE mode,
                                         size_t initial_capacity,
                                         stk_hashmap_hash_fn hash_fn,
                                         stk_hashmap_eq_fn eq_fn,
                                         const stk_allocator* alloc)
{
    STK_RETURN_IF(!m, STK_EINVAL, "hashmap: NULL self");
    if ((unsigned)mode > HASHMAP_MODE_OPEN_ADDRESS)
        return STK_EINVAL;

    m->alloc = alloc ? alloc : &STK_ALLOCATOR_DEFAULT;

    size_t cap = 8;
    while (cap < initial_capacity)
        cap <<= 1;

    m->entries = (stk_hashmap_entry*)stk_alloc(m, cap * sizeof(stk_hashmap_entry));
    if (!m->entries) {
        STK_LOG_ERROR("hashmap: alloc failed for %zu slots", cap);
        return STK_ENOMEM;
    }
    memset(m->entries, 0, cap * sizeof(stk_hashmap_entry));

    m->capacity  = cap;
    m->count     = 0;
    m->tombstones = 0;
    m->mode      = mode;
    m->hash_fn   = hash_fn ? hash_fn : stk_hashmap_str_hash;
    m->eq_fn     = eq_fn ? eq_fn : stk_hashmap_str_eq;
    return STK_OK;
}

STK_STATUS stk_hashmap_create(stk_hashmap* m,
                              HASHMAP_MODE mode,
                              size_t initial_capacity,
                              stk_hashmap_hash_fn hash_fn,
                              stk_hashmap_eq_fn eq_fn)
{
    return stk_hashmap_create_with_alloc(m, mode, initial_capacity, hash_fn, eq_fn, NULL);
}

STK_STATUS stk_hashmap_free(stk_hashmap* m)
{
    if (!m) {
        STK_LOG_WARN("hashmap: free(NULL)");
        return STK_EINVAL;
    }
    OPS[m->mode].free_nodes(m);
    stk_dealloc(m, m->entries);
    memset(m, 0, sizeof(*m));   /* invalidate the handle */
    return STK_OK;
}

/* ------------------------------------------------------------------ */
/* Core operations                                                    */
/* ------------------------------------------------------------------ */

STK_STATUS stk_hashmap_set(stk_hashmap* m, void* key, void* value)
{
    STK_RETURN_IF(!m, STK_EINVAL, "hashmap: set NULL self");
    STK_RETURN_IF(!key, STK_EINVAL, "hashmap: set NULL key");

    if (OPS[m->mode].grow_needed(m)) {
        if (!hashmap_grow(m)) {
            STK_LOG_ERROR("hashmap: set failed to grow");
            return STK_ENOMEM;
        }
    }

    if (m->mode == HASHMAP_MODE_DEFAULT) {
        uint64_t idx = m->hash_fn(key) & hashmap_mask(m);
        for (stk_hashmap_entry* e = m->entries[idx].next; e; e = e->next) {
            if (m->eq_fn(e->key, key)) {
                e->value = value;          /* update in place */
                return STK_OK;
            }
        }
        stk_hashmap_entry* node = (stk_hashmap_entry*)stk_alloc(m, sizeof(stk_hashmap_entry));
        if (!node)
            return STK_ENOMEM;
        node->key = key;
        node->value = value;
        node->occupied = true;
        node->tombstone = false;
        node->next = m->entries[idx].next;   /* head insert */
        m->entries[idx].next = node;
        m->count++;
        return STK_OK;
    }

    /* open addressing */
    size_t idx = OPS[m->mode].insert_slot(m, key);
    if (m->entries[idx].occupied) {
        m->entries[idx].value = value;        /* update */
        return STK_OK;
    }
    m->entries[idx].key = key;
    m->entries[idx].value = value;
    m->entries[idx].occupied = true;
    m->entries[idx].tombstone = false;
    m->count++;
    return STK_OK;
}

void* stk_hashmap_get(const stk_hashmap* m, const void* key)
{
    if (!m || !key)
        return NULL;

    if (m->mode == HASHMAP_MODE_DEFAULT) {
        uint64_t idx = m->hash_fn(key) & hashmap_mask(m);
        for (stk_hashmap_entry* e = m->entries[idx].next; e; e = e->next)
            if (m->eq_fn(e->key, key))
                return e->value;
        return NULL;
    }
    size_t idx = OPS[m->mode].find_slot(m, key);
    return m->entries[idx].occupied ? m->entries[idx].value : NULL;
}

bool stk_hashmap_has(const stk_hashmap* m, const void* key)
{
    if (!m || !key)
        return false;
    if (m->mode == HASHMAP_MODE_DEFAULT) {
        uint64_t idx = m->hash_fn(key) & hashmap_mask(m);
        for (stk_hashmap_entry* e = m->entries[idx].next; e; e = e->next)
            if (m->eq_fn(e->key, key))
                return true;
        return false;
    }
    size_t idx = OPS[m->mode].find_slot(m, key);
    return m->entries[idx].occupied;
}

void* stk_hashmap_remove(stk_hashmap* m, const void* key)
{
    if (!m || !key)
        return NULL;

    if (m->mode == HASHMAP_MODE_DEFAULT) {
        uint64_t idx = m->hash_fn(key) & hashmap_mask(m);
        stk_hashmap_entry* prev = &m->entries[idx];
        for (stk_hashmap_entry* e = prev->next; e; e = e->next) {
            if (m->eq_fn(e->key, key)) {
                void* val = e->value;
                prev->next = e->next;
                stk_dealloc(m, e);
                m->count--;
                return val;
            }
            prev = e;
        }
        return NULL;
    }

    size_t idx = OPS[m->mode].find_slot(m, key);
    if (!m->entries[idx].occupied)
        return NULL;
    void* val = m->entries[idx].value;
    m->entries[idx].occupied = false;
    m->entries[idx].tombstone = true;
    m->count--;
    m->tombstones++;
    return val;
}

STK_STATUS stk_hashmap_clear(stk_hashmap* m)
{
    STK_RETURN_IF(!m, STK_EINVAL, "hashmap: clear NULL self");
    OPS[m->mode].clear_nodes(m);
    m->count = 0;
    m->tombstones = 0;
    return STK_OK;
}

STK_STATUS stk_hashmap_reserve(stk_hashmap* m, size_t capacity)
{
    STK_RETURN_IF(!m, STK_EINVAL, "hashmap: reserve NULL self");
    if (capacity <= m->capacity)
        return STK_OK;
    size_t cap = m->capacity;
    while (cap < capacity)
        cap <<= 1;
    return hashmap_grow_to(m, cap) ? STK_OK : STK_ENOMEM;
}

STK_STATUS stk_hashmap_put_if_absent(stk_hashmap* m, void* key, void* value, void** existing)
{
    STK_RETURN_IF(!m, STK_EINVAL, "hashmap: put_if_absent NULL self");
    STK_RETURN_IF(!key, STK_EINVAL, "hashmap: put_if_absent NULL key");
    if (existing)
        *existing = NULL;

    if (stk_hashmap_has(m, key)) {
        if (existing)
            *existing = stk_hashmap_get(m, key);
        return STK_OK;                 /* key already present, no insert */
    }
    return stk_hashmap_set(m, key, value);
}

/* ------------------------------------------------------------------ */
/* Iteration                                                          */
/* ------------------------------------------------------------------ */

STK_STATUS stk_hashmap_foreach(const stk_hashmap* m,
                               bool (*fn)(void* key, void* value, void* ud),
                               void* ud)
{
    STK_RETURN_IF(!m, STK_EINVAL, "hashmap: foreach NULL self");
    STK_RETURN_IF(!fn, STK_EINVAL, "hashmap: foreach NULL fn");

    size_t n = m->count;
    void** kv = NULL;
    if (n) {
        kv = (void**)stk_alloc(m, n * 2 * sizeof(void*));
        if (!kv)
            return STK_ENOMEM;
    }
    size_t k = 0;
    for (size_t i = 0; i < m->capacity && k < n; i++) {
        if (m->mode == HASHMAP_MODE_DEFAULT) {
            for (stk_hashmap_entry* e = m->entries[i].next; e; e = e->next) {
                kv[k * 2]     = e->key;
                kv[k * 2 + 1] = e->value;
                k++;
            }
        } else if (m->entries[i].occupied) {
            kv[k * 2]     = m->entries[i].key;
            kv[k * 2 + 1] = m->entries[i].value;
            k++;
        }
    }
    for (size_t i = 0; i < k; i++) {
        if (!fn(kv[i * 2], kv[i * 2 + 1], ud))
            break;
    }
    stk_dealloc(m, kv);
    return STK_OK;
}

void stk_hashmap_iter_init(stk_hashmap_iter* it, const stk_hashmap* m)
{
    if (!it)
        return;
    it->m = m;
    it->bucket = 0;
    it->slot = 0;
    it->node = NULL;
    it->curr = NULL;
    it->valid = false;   /* no entry fetched yet */
}

bool stk_hashmap_iter_next(stk_hashmap_iter* it, void** key, void** value)
{
    if (!it || !it->m)
        return false;
    const stk_hashmap* m = it->m;

    if (m->mode == HASHMAP_MODE_DEFAULT) {
        /* node == next entry to return; advance it on every successful
         * return so that skipping (without iter_remove) still progresses. */
        if (!it->node) {
            while (it->bucket < m->capacity) {
                it->node = m->entries[it->bucket].next;
                it->bucket++;
                if (it->node)
                    break;
            }
        }
        if (!it->node)
            return false;
        it->curr = it->node;        /* remember for iter_remove */
        it->node = it->node->next;  /* advance to the next entry */
        if (key)   *key = it->curr->key;
        if (value) *value = it->curr->value;
        it->valid = true;
        return true;
    }

    while (it->slot < m->capacity) {
        stk_hashmap_entry* e = &m->entries[it->slot];
        if (e->occupied) {
            if (key)   *key = e->key;
            if (value) *value = e->value;
            it->node = e;          /* remember for iter_remove */
            it->slot++;            /* advance past the returned slot */
            it->valid = true;
            return true;
        }
        it->slot++;
    }
    it->node = NULL;
    return false;
}

void* stk_hashmap_iter_remove(stk_hashmap_iter* it)
{
    if (!it || !it->valid)
        return NULL;
    stk_hashmap* m = (stk_hashmap*)it->m;
    void* val;

    if (m->mode == HASHMAP_MODE_DEFAULT) {
        /* it->curr is the entry just returned.  Delegate to stk_hashmap_remove
         * so the node is correctly unlinked from its bucket chain (the
         * iterator only holds the node, not its predecessor).  it->node was
         * already advanced to the successor by iter_next, so the walk
         * continues seamlessly. */
        if (!it->curr)
            return NULL;
        void* key = it->curr->key;
        val = it->curr->value;
        it->curr = NULL;
        stk_hashmap_remove(m, key);   /* unlinks + frees the node, count-- */
    } else {
        stk_hashmap_entry* e = it->node;   /* recorded by iter_next */
        if (!e || !e->occupied)
            return NULL;
        val = e->value;
        e->occupied = false;
        e->tombstone = true;            /* in place; slot already advanced */
        m->count--;
        m->tombstones++;
    }
    it->valid = false;
    return val;
}

/* ------------------------------------------------------------------ */
/* Introspection                                                      */
/* ------------------------------------------------------------------ */

size_t stk_hashmap_count(const stk_hashmap* m)    { return m ? m->count : 0; }
size_t stk_hashmap_capacity(const stk_hashmap* m)  { return m ? m->capacity : 0; }
double stk_hashmap_load_factor(const stk_hashmap* m)
{
    return (m && m->capacity) ? (double)m->count / (double)m->capacity : 0.0;
}
