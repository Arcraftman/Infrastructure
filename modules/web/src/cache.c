#include "web/cache.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =========================================================================
 * Internal types
 * ========================================================================= */

#define CACHE_HASH_SIZE 256

/* Hash chain node (separate from LRU list) */
typedef struct cache_entry {
    char            *key;
    void            *value;
    size_t           value_len;
    time_t           timestamp;
    struct cache_entry *hash_next;  /* next in hash chain */
    struct cache_entry *lru_prev;   /* LRU list */
    struct cache_entry *lru_next;
} cache_entry_t;

struct web_cache {
    cache_entry_t  *hash_table[CACHE_HASH_SIZE];
    cache_entry_t  *lru_head;       /* most recently used */
    cache_entry_t  *lru_tail;       /* least recently used */
    size_t          max_items;
    int             ttl_sec;
    size_t          count;
    web_cache_evict_fn evict_cb;
    pthread_mutex_t lock;
};

/* =========================================================================
 * Hash helpers
 * ========================================================================= */

static size_t
hash_key(const char *key)
{
    size_t h = 5381;
    int c;
    while ((c = (unsigned char)*key++))
        h = ((h << 5) + h) + (size_t)c;
    return h % CACHE_HASH_SIZE;
}

/* Move entry to MRU position */
static void
lru_touch(web_cache_t *cache, cache_entry_t *entry)
{
    if (entry == cache->lru_head) return; /* already MRU */
    /* Unlink from current position */
    if (entry->lru_prev)
        entry->lru_prev->lru_next = entry->lru_next;
    if (entry->lru_next)
        entry->lru_next->lru_prev = entry->lru_prev;
    if (cache->lru_tail == entry)
        cache->lru_tail = entry->lru_prev;

    /* Insert at head */
    entry->lru_next = cache->lru_head;
    entry->lru_prev = NULL;
    if (cache->lru_head)
        cache->lru_head->lru_prev = entry;
    cache->lru_head = entry;
    if (!cache->lru_tail)
        cache->lru_tail = entry;
}

/* Evict the LRU entry */
static void
evict_one(web_cache_t *cache)
{
    cache_entry_t *victim = cache->lru_tail;
    if (!victim) return;

    /* Unlink from LRU */
    if (victim->lru_prev)
        victim->lru_prev->lru_next = NULL;
    else
        cache->lru_head = NULL;
    cache->lru_tail = victim->lru_prev;

    /* Unlink from hash */
    size_t h = hash_key(victim->key);
    cache_entry_t **pp = &cache->hash_table[h];
    while (*pp) {
        if (*pp == victim) {
            *pp = victim->hash_next;
            break;
        }
        pp = &(*pp)->hash_next;
    }

    cache->count--;

    if (cache->evict_cb)
        cache->evict_cb(victim->key, victim->value, victim->value_len);

    free(victim->key);
    free(victim->value);
    free(victim);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

WEB_API web_cache_t *
web_cache_create(size_t max_items, int ttl_sec)
{
    web_cache_t *cache = (web_cache_t *)calloc(1, sizeof(*cache));
    if (!cache) return NULL;

    cache->max_items = max_items > 0 ? max_items : 256;
    cache->ttl_sec   = ttl_sec;
    pthread_mutex_init(&cache->lock, NULL);
    return cache;
}

WEB_API int
web_cache_set(web_cache_t *cache, const char *key,
              void *value, size_t value_len)
{
    if (!cache || !key) return -1;

    pthread_mutex_lock(&cache->lock);

    size_t h = hash_key(key);

    /* Look for existing entry */
    for (cache_entry_t *e = cache->hash_table[h]; e; e = e->hash_next) {
        if (strcmp(e->key, key) == 0) {
            /* Update in place */
            if (cache->evict_cb)
                cache->evict_cb(e->key, e->value, e->value_len);
            free(e->value);
            e->value = NULL;
            e->value_len = 0;
            if (value && value_len > 0) {
                e->value = malloc(value_len);
                if (e->value) {
                    memcpy(e->value, value, value_len);
                    e->value_len = value_len;
                }
            }
            e->timestamp = time(NULL);
            lru_touch(cache, e);
            pthread_mutex_unlock(&cache->lock);
            return 0;
        }
    }

    /* Evict if cache is full */
    while (cache->count >= cache->max_items)
        evict_one(cache);

    /* Create new entry */
    cache_entry_t *e = (cache_entry_t *)calloc(1, sizeof(*e));
    if (!e) {
        pthread_mutex_unlock(&cache->lock);
        return -1;
    }
    e->key = strdup(key);
    if (!e->key) { free(e); pthread_mutex_unlock(&cache->lock); return -1; }
    if (value && value_len > 0) {
        e->value = malloc(value_len);
        if (e->value) {
            memcpy(e->value, value, value_len);
            e->value_len = value_len;
        }
    }
    e->timestamp = time(NULL);

    /* Insert into hash chain */
    e->hash_next = cache->hash_table[h];
    cache->hash_table[h] = e;

    /* Insert at LRU head */
    e->lru_next = cache->lru_head;
    e->lru_prev = NULL;
    if (cache->lru_head)
        cache->lru_head->lru_prev = e;
    cache->lru_head = e;
    if (!cache->lru_tail)
        cache->lru_tail = e;

    cache->count++;
    pthread_mutex_unlock(&cache->lock);
    return 0;
}

WEB_API int
web_cache_get(web_cache_t *cache, const char *key,
              void **value, size_t *value_len)
{
    if (!cache || !key || !value || !value_len) return -1;

    pthread_mutex_lock(&cache->lock);

    size_t h = hash_key(key);
    for (cache_entry_t *e = cache->hash_table[h]; e; e = e->hash_next) {
        if (strcmp(e->key, key) == 0) {
            /* Check TTL */
            if (cache->ttl_sec > 0) {
                time_t now = time(NULL);
                if (now - e->timestamp > cache->ttl_sec) {
                    /* Expired — treat as miss */
                    *value = NULL;
                    *value_len = 0;
                    pthread_mutex_unlock(&cache->lock);
                    return -1;
                }
            }
            lru_touch(cache, e);
            *value = e->value;
            *value_len = e->value_len;
            pthread_mutex_unlock(&cache->lock);
            return 0;
        }
    }

    *value = NULL;
    *value_len = 0;
    pthread_mutex_unlock(&cache->lock);
    return -1;
}

WEB_API void
web_cache_delete(web_cache_t *cache, const char *key)
{
    if (!cache || !key) return;

    pthread_mutex_lock(&cache->lock);

    size_t h = hash_key(key);
    cache_entry_t **pp = &cache->hash_table[h];
    while (*pp) {
        if (strcmp((*pp)->key, key) == 0) {
            cache_entry_t *e = *pp;
            *pp = e->hash_next;

            /* Unlink from LRU */
            if (e->lru_prev)
                e->lru_prev->lru_next = e->lru_next;
            if (e->lru_next)
                e->lru_next->lru_prev = e->lru_prev;
            if (cache->lru_head == e)
                cache->lru_head = e->lru_next;
            if (cache->lru_tail == e)
                cache->lru_tail = e->lru_prev;

            cache->count--;

            if (cache->evict_cb)
                cache->evict_cb(e->key, e->value, e->value_len);

            free(e->key);
            free(e->value);
            free(e);
            pthread_mutex_unlock(&cache->lock);
            return;
        }
        pp = &(*pp)->hash_next;
    }

    pthread_mutex_unlock(&cache->lock);
}

WEB_API void
web_cache_clear(web_cache_t *cache)
{
    if (!cache) return;

    pthread_mutex_lock(&cache->lock);

    for (size_t i = 0; i < CACHE_HASH_SIZE; i++) {
        cache_entry_t *e = cache->hash_table[i];
        while (e) {
            cache_entry_t *next = e->hash_next;
            if (cache->evict_cb)
                cache->evict_cb(e->key, e->value, e->value_len);
            free(e->key);
            free(e->value);
            free(e);
            e = next;
        }
        cache->hash_table[i] = NULL;
    }
    cache->lru_head = NULL;
    cache->lru_tail = NULL;
    cache->count    = 0;

    pthread_mutex_unlock(&cache->lock);
}

WEB_API void
web_cache_set_evict_cb(web_cache_t *cache, web_cache_evict_fn cb)
{
    if (!cache) return;
    pthread_mutex_lock(&cache->lock);
    cache->evict_cb = cb;
    pthread_mutex_unlock(&cache->lock);
}

WEB_API size_t
web_cache_count(const web_cache_t *cache)
{
    if (!cache) return 0;
    size_t cnt;
    pthread_mutex_lock(&((web_cache_t *)cache)->lock);
    cnt = cache->count;
    pthread_mutex_unlock(&((web_cache_t *)cache)->lock);
    return cnt;
}

WEB_API void
web_cache_destroy(web_cache_t *cache)
{
    if (!cache) return;

    web_cache_clear(cache);
    pthread_mutex_destroy(&cache->lock);
    memset(cache, 0, sizeof(*cache));
    free(cache);
}
