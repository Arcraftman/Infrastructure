#ifndef WEB_COOKIE_H
#define WEB_COOKIE_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cookie.h
 * @brief HTTP cookie parsing, serialization, and jar management.
 *
 * Supports Set-Cookie header parsing (RFC 6265) and client-side
 * cookie jar management for outgoing requests.
 */

/* =========================================================================
 * Cookie
 * ========================================================================= */

typedef struct web_cookie {
    char  *name;       /**< Cookie name (decoded). */
    char  *value;      /**< Cookie value (decoded). */
    char  *domain;     /**< Domain scope (may be NULL). */
    char  *path;       /**< Path scope (may be NULL). */
    int    secure;     /**< 1 if Secure flag set. */
    int    httponly;   /**< 1 if HttpOnly flag set. */
    long   max_age;    /**< Max-Age in seconds, or -1 if not set. */
    struct web_cookie *next; /**< Linked list pointer. */
} web_cookie_t;

/**
 * Parse a single "Cookie" header value (semi-colon separated pairs).
 * Returns a linked list of parsed cookies.
 * @param header  The Cookie header value (e.g. "foo=bar; baz=qux").
 * @return Linked list of cookies, or NULL on error / empty input.
 */
WEB_API web_cookie_t *
web_cookie_parse(const char *header);

/**
 * Parse a single "Set-Cookie" header value.
 * @param header  The Set-Cookie header value (e.g. "session=abc; Path=/; HttpOnly").
 * @return A single cookie, or NULL on parse error.
 */
WEB_API web_cookie_t *
web_cookie_parse_set(const char *header);

/**
 * Serialize a cookie for use in a "Cookie" header.
 * Returns a string like "name=value; name2=value2".
 * Caller must free the result with free().
 */
WEB_API char *
web_cookie_serialize(const web_cookie_t *cookie);

/**
 * Serialize a cookie for use in a "Set-Cookie" header.
 * Caller must free the result with free().
 */
WEB_API char *
web_cookie_serialize_set(const web_cookie_t *cookie);

/**
 * Free a single cookie or a linked list of cookies.
 */
WEB_API void
web_cookie_free(web_cookie_t *cookie);

/* =========================================================================
 * Cookie Jar
 * ========================================================================= */

typedef struct web_cookie_jar web_cookie_jar_t;

/**
 * Create an empty cookie jar.
 */
WEB_API web_cookie_jar_t *
web_cookie_jar_create(void);

/**
 * Store cookies from a "Set-Cookie" response header into the jar.
 * @param jar    Cookie jar.
 * @param set_cookie_header  Value of the Set-Cookie header.
 * @param domain The request domain (for domain/path matching).
 * @param path   The request path (for path matching).
 * @return Number of cookies stored, or -1 on error.
 */
WEB_API int
web_cookie_jar_store(web_cookie_jar_t *jar,
                      const char *set_cookie_header,
                      const char *domain, const char *path);

/**
 * Get all cookies in the jar that match the given domain and path.
 * Returns a serialized string suitable for a "Cookie" header.
 * Caller must free the result with free().
 */
WEB_API char *
web_cookie_jar_get(web_cookie_jar_t *jar,
                    const char *domain, const char *path);

/**
 * Clear expired cookies from the jar.
 */
WEB_API void
web_cookie_jar_cleanup(web_cookie_jar_t *jar);

/**
 * Destroy the cookie jar and all stored cookies.
 */
WEB_API void
web_cookie_jar_destroy(web_cookie_jar_t *jar);

#ifdef __cplusplus
}
#endif

#endif /* WEB_COOKIE_H */
