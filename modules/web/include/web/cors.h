#ifndef WEB_CORS_H
#define WEB_CORS_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cors.h
 * @brief CORS (Cross-Origin Resource Sharing) helper functions.
 *
 * Provides utilities to set the correct CORS headers on responses
 * and to handle preflight (OPTIONS) requests.
 */

/**
 * CORS configuration.
 * All string fields are copied internally. NULL values omit the header.
 */
typedef struct web_cors_config {
    char   *origin;          /**< Access-Control-Allow-Origin (e.g. "*" or "https://example.com") */
    char   *methods;         /**< Access-Control-Allow-Methods (e.g. "GET, POST, PUT, DELETE") */
    char   *headers;         /**< Access-Control-Allow-Headers (e.g. "Content-Type, Authorization") */
    char   *expose_headers;  /**< Access-Control-Expose-Headers (may be NULL). */
    int     allow_credentials; /**< Access-Control-Allow-Credentials: true */
    long    max_age;          /**< Access-Control-Max-Age in seconds (0 to omit). */
} web_cors_config_t;

/**
 * Apply CORS headers to a response based on the given configuration.
 * @param resp Response to modify.
 * @param config  CORS configuration.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_cors_apply(web_response_t *resp, const web_cors_config_t *config);

/**
 * Handle a CORS preflight (OPTIONS) request.
 * Creates a 204 No Content response with the appropriate CORS headers.
 * If the request is not an OPTIONS request, returns NULL.
 * @param req    The incoming request.
 * @param config CORS configuration.
 * @return A 204 response with CORS headers, or NULL if not a preflight.
 */
WEB_API web_response_t *
web_cors_handle_preflight(const web_request_t *req,
                           const web_cors_config_t *config);

/**
 * Fill a cors_config with default permissive values.
 * Fields: origin="*", methods="GET, POST, PUT, DELETE, PATCH, OPTIONS",
 *         headers="Content-Type, Authorization", allow_credentials=0, max_age=86400.
 */
WEB_API void
web_cors_config_default(web_cors_config_t *config);

/**
 * Free strings held by a CORS config (the struct itself is not freed).
 */
WEB_API void
web_cors_config_destroy(web_cors_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* WEB_CORS_H */
