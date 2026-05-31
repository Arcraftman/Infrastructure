#ifndef WEB_RATELIMIT_H
#define WEB_RATELIMIT_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ratelimit.h
 * @brief Token-bucket rate limiter for connection / request throttling.
 *
 * Each client IP is tracked independently. When tokens run out,
 * requests are denied until the bucket refills.
 */

typedef struct web_ratelimit web_ratelimit_t;

/**
 * Create a rate limiter using the token-bucket algorithm.
 * @param rate      Maximum requests per second per client.
 * @param burst     Maximum burst size (tokens). Must be >= rate.
 * @param cleanup_interval  How often (seconds) to clean up stale entries (0 = 60).
 * @return New rate limiter, or NULL on error.
 */
WEB_API web_ratelimit_t *
web_ratelimit_create(double rate, int burst, int cleanup_interval);

/**
 * Check if a request from the given client should be allowed.
 * @param rl       Rate limiter.
 * @param client_key  Client identifier (e.g. IP address string).
 * @return 1 if allowed, 0 if rate-limited.
 */
WEB_API int
web_ratelimit_allow(web_ratelimit_t *rl, const char *client_key);

/**
 * Get the number of tracked clients.
 */
WEB_API size_t
web_ratelimit_count(const web_ratelimit_t *rl);

/**
 * Reset the rate limiter for a specific client.
 */
WEB_API void
web_ratelimit_reset(web_ratelimit_t *rl, const char *client_key);

/**
 * Destroy the rate limiter.
 */
WEB_API void
web_ratelimit_destroy(web_ratelimit_t *rl);

#ifdef __cplusplus
}
#endif

#endif /* WEB_RATELIMIT_H */
