#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file client.h
 * @brief Simple blocking HTTP client for making outgoing requests.
 *
 * Usage:
 *   web_client_t *c = web_client_create();
 *   web_response_t *resp = web_client_request(c, "GET", "http://example.com/", NULL, 0, NULL);
 *   // use resp ...
 *   web_response_destroy(resp);
 *   web_client_destroy(c);
 */

typedef struct web_client web_client_t;

/** Callback for response body chunks (streaming). Return 0 to continue, -1 to abort. */
typedef int (*web_client_chunk_cb)(const void *data, size_t len, void *userdata);

/**
 * Create an HTTP client.
 * @param connect_timeout  Connection timeout in seconds (0 for default 10).
 * @param request_timeout  Request timeout in seconds (0 for default 30).
 * @return New client, or NULL on allocation failure.
 */
WEB_API web_client_t *
web_client_create(long connect_timeout, long request_timeout);

/**
 * Perform a synchronous HTTP request.
 * @param client    HTTP client.
 * @param method    HTTP method string (e.g. "GET", "POST").
 * @param url       Full URL (e.g. "http://example.com/api").
 * @param headers   Request headers (may be NULL).
 * @param body      Request body (may be NULL).
 * @param body_len  Body length.
 * @return Parsed response (caller must destroy), or NULL on error.
 *         On error, call web_client_error() for the message.
 */
WEB_API web_response_t *
web_client_request(web_client_t *client,
                    const char *method, const char *url,
                    const web_header_t *headers,
                    const void *body, size_t body_len);

/**
 * Perform a streaming HTTP request.
 * Response body chunks are passed to `chunk_cb` as they arrive.
 * @return Response with headers parsed but body may be incomplete (depends on chunk_cb).
 */
WEB_API web_response_t *
web_client_request_stream(web_client_t *client,
                           const char *method, const char *url,
                           const web_header_t *headers,
                           const void *body, size_t body_len,
                           web_client_chunk_cb chunk_cb, void *cb_userdata);

/**
 * Get the last error message.
 * @return A string describing the last error (valid until next client call).
 */
WEB_API const char *
web_client_error(const web_client_t *client);

/**
 * Set a proxy for all requests.
 * @param proxy_url  e.g. "http://proxy:8080". Pass NULL to clear.
 */
WEB_API int
web_client_set_proxy(web_client_t *client, const char *proxy_url);

/**
 * Destroy the HTTP client.
 */
WEB_API void
web_client_destroy(web_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* WEB_CLIENT_H */
