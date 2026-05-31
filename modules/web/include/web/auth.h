#ifndef WEB_AUTH_H
#define WEB_AUTH_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file auth.h
 * @brief HTTP authentication helpers (Basic and Bearer).
 *
 * Provides utilities for parsing Authorization headers and
 * generating WWW-Authenticate responses.
 */

/* =========================================================================
 * Basic Auth
 * ========================================================================= */

/**
 * Parse a Basic Authorization header value.
 * @param header  The Authorization header value (e.g. "Basic base64string").
 * @param[out] username  Receives the decoded username (caller must free).
 * @param[out] password  Receives the decoded password (caller must free).
 * @return 0 on success, -1 on parse error.
 */
WEB_API int
web_auth_basic_parse(const char *header, char **username, char **password);

/**
 * Encode credentials for a Basic Authorization header value.
 * Returns a string like "Basic base64string". Caller must free.
 */
WEB_API char *
web_auth_basic_encode(const char *username, const char *password);

/**
 * Create a 401 Unauthorized response with a WWW-Authenticate: Basic header.
 * @param realm  The authentication realm string.
 */
WEB_API web_response_t *
web_auth_basic_challenge(const char *realm);

/* =========================================================================
 * Bearer Token
 * ========================================================================= */

/**
 * Parse a Bearer token from an Authorization header.
 * @param header  The Authorization header value (e.g. "Bearer eyJ...")
 * @return The token string (caller must free), or NULL on error.
 */
WEB_API char *
web_auth_bearer_parse(const char *header);

/**
 * Create a 401 Unauthorized response with a WWW-Authenticate: Bearer header.
 * @param scope  Optional scope string (may be NULL).
 */
WEB_API web_response_t *
web_auth_bearer_challenge(const char *scope);

#ifdef __cplusplus
}
#endif

#endif /* WEB_AUTH_H */
