#ifndef WEB_ACCESSLOG_H
#define WEB_ACCESSLOG_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accesslog.h
 * @brief Structured HTTP access logging.
 *
 * Supports Apache Common Log Format (CLF) and Combined Log Format.
 * Logs are written asynchronously via a background writer thread.
 */

typedef struct web_accesslog web_accesslog_t;

/** Log format type. */
typedef enum web_accesslog_format {
    WEB_ACCESSLOG_CLF,       /**< Apache Common Log Format */
    WEB_ACCESSLOG_COMBINED,  /**< Apache Combined Log Format (with Referer + User-Agent) */
    WEB_ACCESSLOG_JSON       /**< JSON line format */
} web_accesslog_format_t;

/**
 * Create an access log writer.
 * @param path   File path to write logs to. Pass NULL for stderr.
 * @param format Log format.
 * @return New access log writer, or NULL on error.
 */
WEB_API web_accesslog_t *
web_accesslog_create(const char *path, web_accesslog_format_t format);

/**
 * Log a single request/response.
 * @param log     Access log writer.
 * @param req     The HTTP request.
 * @param resp    The HTTP response (may be NULL if not yet available).
 * @param status  HTTP status code.
 * @param bytes   Response body size in bytes.
 * @param remote  Remote address string (e.g. "127.0.0.1").
 * @param ms      Request handling time in milliseconds.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_accesslog_log(web_accesslog_t *log,
                   const web_request_t *req,
                   const web_response_t *resp,
                   int status, size_t bytes,
                   const char *remote, long ms);

/**
 * Flush buffered log entries to disk.
 */
WEB_API int
web_accesslog_flush(web_accesslog_t *log);

/**
 * Destroy the access log writer and flush remaining entries.
 */
WEB_API void
web_accesslog_destroy(web_accesslog_t *log);

#ifdef __cplusplus
}
#endif

#endif /* WEB_ACCESSLOG_H */
