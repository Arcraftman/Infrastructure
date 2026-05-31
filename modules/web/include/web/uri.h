#ifndef WEB_URI_H
#define WEB_URI_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uri.h
 * @brief RFC 3986 URI parser and percent-encoding utilities.
 *
 * Parses URIs of the form:
 *   scheme://host[:port]/path[?query][#fragment]
 *
 * All parsed components are NULL-terminated strings allocated from the heap.
 * Free them with web_uri_destroy().
 */

typedef struct web_uri {
    char *scheme;     /**< e.g. "http", "https", "ws" (lowercase). May be NULL. */
    char *host;       /**< Host portion (decoded). May be NULL. */
    int   port;       /**< Port number, or 0 if not specified. */
    char *path;       /**< Path component (decoded, always non-NULL). */
    char *query;      /**< Query string (raw, without leading '?'). May be NULL. */
    char *fragment;   /**< Fragment (raw, without leading '#'). May be NULL. */
    char *userinfo;   /**< userinfo (e.g. "user:pass"). May be NULL. */
} web_uri_t;

/**
 * Parse a URI string according to RFC 3986.
 * @param str  The URI string (null-terminated).
 * @param len  Length of the string, or 0 to use strlen().
 * @param out  On success, receives the allocated URI. Caller must free with web_uri_destroy().
 * @return 0 on success, -1 on parse error.
 */
WEB_API int
web_uri_parse(const char *str, size_t len, web_uri_t **out);

/**
 * Free all resources held by a parsed URI.
 */
WEB_API void
web_uri_destroy(web_uri_t *uri);

/**
 * Reconstruct a URI string from its components.
 * The caller must free the returned string with free().
 * @return The URI string, or NULL on allocation error.
 */
WEB_API char *
web_uri_build(const web_uri_t *uri);

/**
 * Percent-encode a string.
 * Only encodes characters that are not "unreserved" (ALPHA, DIGIT, '-', '.', '_', '~').
 * @param src   The source string.
 * @param len   Length of source, or 0 for strlen().
 * @param out   Receives the encoded string (caller must free).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_uri_encode(const char *src, size_t len, char **out);

/**
 * Percent-decode a string in-place.
 * Converts "%XX" sequences to their byte values.
 * @param str  The string to decode (modified in place).
 * @return The decoded length (may be shorter than strlen(str)).
 */
WEB_API size_t
web_uri_decode(char *str);

#ifdef __cplusplus
}
#endif

#endif /* WEB_URI_H */
