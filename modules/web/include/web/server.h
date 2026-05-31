#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Connection & Route Types
 * ========================================================================= */

typedef struct web_connection web_connection_t;

/** Request handler callback. Returns a response that the caller owns. */
typedef web_response_t *(*web_handler_fn)(const web_request_t *req,
                                          void *userdata);

/** Handler destructor — called when a route is removed or server destroyed. */
typedef void (*web_handler_dtor)(void *userdata);

typedef struct web_route {
    char            *method;     /* "GET", "POST", etc., or "*" */
    char            *path;       /* e.g. "/api/users" or wildcard path */
    web_handler_fn   handler;
    void            *userdata;
    web_handler_dtor userdata_dtor;
    struct web_route *next;
} web_route_t;

/* =========================================================================
 * Server
 * ========================================================================= */

typedef struct web_server web_server_t;

/**
 * Create an HTTP server listening on addr:port.
 * @param addr  Address to bind (e.g. "0.0.0.0", "127.0.0.1", "::1").
 *              Pass NULL for "0.0.0.0".
 * @param port  Port number (e.g. 8080).
 * @return New server instance, or NULL on error.
 */
WEB_API web_server_t *
web_server_create(const char *addr, int port);

/**
 * Set the maximum number of pending connections.
 * Must be called before web_server_start().
 */
WEB_API int
web_server_set_backlog(web_server_t *srv, int backlog);

/**
 * Set the maximum number of worker threads (0 = single-threaded).
 * Must be called before web_server_start().
 */
WEB_API int
web_server_set_threads(web_server_t *srv, int threads);

/**
 * Register a route handler.
 * @param method  HTTP method string ("GET", "POST", ...) or "*" for any.
 * @param path    URL path ("/api/foo"). Trailing "*" matches prefix.
 * @param handler Callback function.
 * @param userdata User data passed to handler (may be NULL).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_server_add_route(web_server_t *srv, const char *method,
                     const char *path, web_handler_fn handler,
                     void *userdata);

/**
 * Register a route with a destructor for userdata.
 */
WEB_API int
web_server_add_route_ex(web_server_t *srv, const char *method,
                        const char *path, web_handler_fn handler,
                        void *userdata, web_handler_dtor dtor);

/**
 * Serve static files from a directory under a mount path.
 * @param mount  URL prefix (e.g. "/static").
 * @param dir    Filesystem directory (e.g. "/var/www/static").
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_server_add_static(web_server_t *srv, const char *mount,
                      const char *dir);

/**
 * Start the server. Blocks the calling thread (single-threaded mode)
 * or launches worker threads.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_server_start(web_server_t *srv);

/**
 * Signal the server to stop. If running in a separate thread, call
 * this from a signal handler or another thread, then join.
 */
WEB_API void
web_server_stop(web_server_t *srv);

/**
 * Set connection idle timeout in seconds (default 60).
 * Connections idle longer than this are closed. Set to 0 to disable.
 */
WEB_API int
web_server_set_timeout(web_server_t *srv, int seconds);

/**
 * Enable or disable HTTP/1.1 keep-alive (default enabled).
 * @param enabled  Non-zero to enable, 0 to disable.
 */
WEB_API int
web_server_set_keepalive(web_server_t *srv, int enabled);

/**
 * Destroy the server and free all resources.
 */
WEB_API void
web_server_destroy(web_server_t *srv);

#ifdef __cplusplus
}
#endif

#endif /* WEB_SERVER_H */
