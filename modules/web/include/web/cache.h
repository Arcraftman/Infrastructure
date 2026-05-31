#ifndef WEB_CACHE_H
#define WEB_CACHE_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cache.h
 * @brief In-memory response cache with ETag/Last-Modified support.
 *
 * Uses a simple LRU eviction policy. Cache entries are keyed by
 * HTTP method + URL path. Supports conditional requests via
 * If-None-Match and If-Modified-Since headers.
 */

typedef struct web_cache web_cache_t;

/**
 * Create a response cache.
 * @param max_entries  Maximum number of entries (0 for default 256).
 * @param ttl_secs     Time-to-live in seconds (0 for default 60).
 * @return New cache, or NULL on error.
 */
WEB_API web_cache_t *
web_cache_create(size_t max_entries, long ttl_secs);

/**
 * Store a response in the cache.
 * The cache takes ownership of the response (will destroy it on eviction).
 * @param cache Cache instance.
 * @param key   Cache key (e.g. "GET /api/users").
 * @param resp  Response to store.
 * @param etag  ETag string (may be NULL). Copied internally.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_cache_set(web_cache_t *cache, const char *key,
              web_response_t *resp, const char *etag);

/**
 * Retrieve a response from the cache.
 * @param cache    Cache instance.
 * @param key      Cache key.
 * @param ims      If-Modified-Since header value (may be NULL).
 * @param inmatch  If-None-Match header value (may be NULL).
 * @param[out] hit Set to 1 if cache hit, 0 if miss.
 * @return Cached response (still owned by cache) if hit, NULL if miss.
 *         The response must NOT be freed by the caller.
 *         If the cached response is fresh (not modified), returns the response
 *         but hit is set to 0 and WEB_STATUS_NOT_MODIFIED is signaled via
 *         a special response.
 */
WEB_API const web_response_t *
web_cache_get(web_cache_t *cache, const char *key,
              const char *ims, const char *inmatch, int *hit);

/**
 * Invalidate a cached response.
 * @param cache Cache instance.
 * @param key   Cache key. If NULL, all entries are invalidated.
 */
WEB_API void
web_cache_invalidate(web_cache_t *cache, const char *key);

/**
 * Get the current number of entries in the cache.
 */
WEB_API size_t
web_cache_count(const web_cache_t *cache);

/**
 * Destroy the cache and all cached responses.
 */
WEB_API void
web_cache_destroy(web_cache_t *cache);

#ifdef __cplusplus
}
#endif

#endif /* WEB_CACHE_H */
