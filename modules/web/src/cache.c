#include "web/cache.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct cache_entry {
    char* key;
    web_response_t* response;
    char* etag;
    time_t created;
    struct cache_entry* next;
} cache_entry_t;

struct web_cache {
    cache_entry_t* entries;
    size_t max_entries;
    long ttl_secs;
    size_t count;
    pthread_mutex_t lock;
};

static void entry_free(cache_entry_t* entry)
{
    if (!entry)
        return;
    free(entry->key);
    free(entry->etag);
    web_response_destroy(entry->response);
    free(entry);
}

WEB_API web_cache_t* web_cache_create(size_t max_entries, long ttl_secs)
{
    web_cache_t* cache = (web_cache_t*)calloc(1, sizeof(*cache));
    if (!cache)
        return NULL;
    cache->max_entries = max_entries ? max_entries : 256;
    cache->ttl_secs = ttl_secs ? ttl_secs : 60;
    if (pthread_mutex_init(&cache->lock, NULL) != 0) {
        free(cache);
        return NULL;
    }
    return cache;
}

WEB_API int web_cache_set(web_cache_t* cache,
                          const char* key,
                          web_response_t* response,
                          const char* etag)
{
    cache_entry_t* entry;
    cache_entry_t** link;
    if (!cache || !key || !response)
        return -1;

    pthread_mutex_lock(&cache->lock);
    for (entry = cache->entries; entry; entry = entry->next) {
        if (strcmp(entry->key, key) == 0) {
            web_response_destroy(entry->response);
            free(entry->etag);
            entry->response = response;
            entry->etag = etag ? strdup(etag) : NULL;
            entry->created = time(NULL);
            pthread_mutex_unlock(&cache->lock);
            return entry->etag || !etag ? 0 : -1;
        }
    }

    while (cache->count >= cache->max_entries) {
        link = &cache->entries;
        while ((*link)->next)
            link = &(*link)->next;
        entry = *link;
        *link = entry->next;
        entry_free(entry);
        --cache->count;
    }

    entry = (cache_entry_t*)calloc(1, sizeof(*entry));
    if (!entry)
        goto fail;
    entry->key = strdup(key);
    entry->etag = etag ? strdup(etag) : NULL;
    entry->response = response;
    entry->created = time(NULL);
    if (!entry->key || (etag && !entry->etag)) {
        entry_free(entry);
        goto fail;
    }
    entry->next = cache->entries;
    cache->entries = entry;
    ++cache->count;
    pthread_mutex_unlock(&cache->lock);
    return 0;

fail:
    pthread_mutex_unlock(&cache->lock);
    return -1;
}

WEB_API const web_response_t* web_cache_get(web_cache_t* cache,
                                             const char* key,
                                             const char* ims,
                                             const char* inmatch,
                                             int* hit)
{
    cache_entry_t* entry;
    time_t now;
    (void)ims;
    if (hit)
        *hit = 0;
    if (!cache || !key)
        return NULL;

    pthread_mutex_lock(&cache->lock);
    now = time(NULL);
    for (entry = cache->entries; entry; entry = entry->next) {
        if (strcmp(entry->key, key) != 0)
            continue;
        if (cache->ttl_secs > 0 && now - entry->created >= cache->ttl_secs) {
            pthread_mutex_unlock(&cache->lock);
            return NULL;
        }
        if (hit)
            *hit = 1;
        if (inmatch && entry->etag && strcmp(inmatch, entry->etag) == 0 && hit)
            *hit = 2;
        pthread_mutex_unlock(&cache->lock);
        return entry->response;
    }
    pthread_mutex_unlock(&cache->lock);
    return NULL;
}

WEB_API void web_cache_invalidate(web_cache_t* cache, const char* key)
{
    cache_entry_t** link;
    cache_entry_t* entry;
    if (!cache)
        return;
    pthread_mutex_lock(&cache->lock);
    link = &cache->entries;
    while (*link) {
        entry = *link;
        if (!key || strcmp(entry->key, key) == 0) {
            *link = entry->next;
            entry_free(entry);
            --cache->count;
            if (key)
                break;
        } else {
            link = &entry->next;
        }
    }
    pthread_mutex_unlock(&cache->lock);
}

WEB_API size_t web_cache_count(const web_cache_t* cache)
{
    size_t count;
    if (!cache)
        return 0;
    pthread_mutex_lock(&((web_cache_t*)cache)->lock);
    count = cache->count;
    pthread_mutex_unlock(&((web_cache_t*)cache)->lock);
    return count;
}

WEB_API void web_cache_destroy(web_cache_t* cache)
{
    if (!cache)
        return;
    web_cache_invalidate(cache, NULL);
    pthread_mutex_destroy(&cache->lock);
    free(cache);
}
