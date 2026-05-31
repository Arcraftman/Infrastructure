#ifndef WEB_HTTP_H
#define WEB_HTTP_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * HTTP Methods
 * ========================================================================= */

typedef enum web_method {
    WEB_METHOD_UNKNOWN = 0,
    WEB_METHOD_GET,
    WEB_METHOD_POST,
    WEB_METHOD_PUT,
    WEB_METHOD_DELETE,
    WEB_METHOD_PATCH,
    WEB_METHOD_HEAD,
    WEB_METHOD_OPTIONS,
    WEB_METHOD_CONNECT,
    WEB_METHOD_TRACE,
    WEB_METHOD_COUNT
} web_method_t;

WEB_API const char *
web_method_str(web_method_t method);

WEB_API web_method_t
web_method_from_str(const char *str, size_t len);

/* =========================================================================
 * HTTP Status Codes
 * ========================================================================= */

typedef enum web_status {
    WEB_STATUS_CONTINUE            = 100,
    WEB_STATUS_SWITCHING_PROTOCOLS = 101,

    WEB_STATUS_OK                  = 200,
    WEB_STATUS_CREATED             = 201,
    WEB_STATUS_ACCEPTED            = 202,
    WEB_STATUS_NO_CONTENT          = 204,
    WEB_STATUS_PARTIAL_CONTENT     = 206,

    WEB_STATUS_MOVED_PERMANENTLY   = 301,
    WEB_STATUS_FOUND               = 302,
    WEB_STATUS_NOT_MODIFIED        = 304,

    WEB_STATUS_BAD_REQUEST         = 400,
    WEB_STATUS_UNAUTHORIZED        = 401,
    WEB_STATUS_FORBIDDEN           = 403,
    WEB_STATUS_NOT_FOUND           = 404,
    WEB_STATUS_METHOD_NOT_ALLOWED  = 405,
    WEB_STATUS_REQUEST_TIMEOUT     = 408,
    WEB_STATUS_CONFLICT            = 409,
    WEB_STATUS_GONE                = 410,
    WEB_STATUS_PAYLOAD_TOO_LARGE   = 413,
    WEB_STATUS_UPGRADE_REQUIRED    = 426,
    WEB_STATUS_TOO_MANY_REQUESTS   = 429,

    WEB_STATUS_INTERNAL_ERROR      = 500,
    WEB_STATUS_NOT_IMPLEMENTED     = 501,
    WEB_STATUS_BAD_GATEWAY         = 502,
    WEB_STATUS_SERVICE_UNAVAILABLE = 503,
    WEB_STATUS_GATEWAY_TIMEOUT     = 504,
    WEB_STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
} web_status_t;

WEB_API const char *
web_status_str(web_status_t status);

WEB_API const char *
web_status_reason(web_status_t status);

/* =========================================================================
 * HTTP Header
 * ========================================================================= */

typedef struct web_header {
    char *name;
    char *value;
    struct web_header *next;
} web_header_t;

WEB_API web_header_t *
web_header_new(const char *name, const char *value);

WEB_API void
web_header_free(web_header_t *hdr);

/* =========================================================================
 * HTTP Request
 * ========================================================================= */

typedef struct web_request {
    web_method_t    method;
    char           *path;
    char           *query;
    char           *version;
    web_header_t   *headers;
    unsigned char   *body;
    size_t          body_len;
    /* internal: raw data reference */
    char           *_raw;
    size_t          _raw_len;
} web_request_t;

WEB_API int
web_request_parse(const char *data, size_t len, web_request_t **out);

WEB_API const char *
web_request_header(const web_request_t *req, const char *name);

WEB_API void
web_request_destroy(web_request_t *req);

/* =========================================================================
 * HTTP Response
 * ========================================================================= */

typedef struct web_response {
    web_status_t    status;
    web_header_t   *headers;
    unsigned char   *body;
    size_t          body_len;
    int             owns_body;   /* non-zero if body should be freed */
    int             chunked;     /* non-zero for chunked transfer encoding */
} web_response_t;

WEB_API web_response_t *
web_response_new(web_status_t status);

WEB_API int
web_response_set_header(web_response_t *resp, const char *name, const char *value);

WEB_API int
web_response_set_body(web_response_t *resp, const void *data, size_t len);

WEB_API int
web_response_set_body_copy(web_response_t *resp, const void *data, size_t len);

WEB_API char *
web_response_format(const web_response_t *resp, size_t *out_len);

WEB_API void
web_response_destroy(web_response_t *resp);

/* =========================================================================
 * Convenience response builders
 * ========================================================================= */

WEB_API web_response_t *
web_response_text(web_status_t status, const char *text);

WEB_API web_response_t *
web_response_json(web_status_t status, const char *json_str);

WEB_API web_response_t *
web_response_file(const char *path);

WEB_API web_response_t *
web_response_error(web_status_t status);

/**
 * Format body using printf-style formatting.
 * The response will own the formatted body.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_response_printf(web_response_t *resp, const char *fmt, ...);

/**
 * Enable chunked transfer encoding for a response.
 * When set, web_response_format will produce chunked body output and
 * suppress automatic Content-Length.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_response_set_chunked(web_response_t *resp);

/* =========================================================================
 * Chunked (streaming) response builder
 * ========================================================================= */

/**
 * Create a new empty chunked response.
 * The caller uses web_response_send_chunk() / web_response_end_chunked()
 * to stream body data.
 */
WEB_API web_response_t *
web_response_chunked(web_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* WEB_HTTP_H */
