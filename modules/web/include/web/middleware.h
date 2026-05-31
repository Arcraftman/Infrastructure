#ifndef WEB_MIDDLEWARE_H
#define WEB_MIDDLEWARE_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file middleware.h
 * @brief Middleware pipeline for request/response processing.
 *
 * Middleware runs in a chain before the route handler. Each middleware
 * can inspect/modify the request, short-circuit with a response, or
 * pass to the next middleware by returning NULL.
 *
 * Call order:  M1 → M2 → ... → Mn → handler → Mn_out → ... → M1_out
 */

typedef struct web_middleware     web_middleware_t;
typedef struct web_middleware_ctx web_middleware_ctx_t;

/**
 * Middleware callback.
 * @param req   The current request (may be modified).
 * @param ctx   Context for passing data and flow control.
 * @param userdata User data registered with this middleware.
 *
 * To pass to the next middleware/handler, return NULL.
 * To short-circuit, return a web_response_t (caller will send it).
 */
typedef web_response_t *(*web_middleware_fn)(const web_request_t *req,
                                              web_middleware_ctx_t *ctx,
                                              void *userdata);

/** Destructor for middleware userdata. */
typedef void (*web_middleware_dtor_fn)(void *userdata);

/**
 * Create a middleware pipeline.
 */
WEB_API web_middleware_t *
web_middleware_create(void);

/**
 * Append a middleware to the chain.
 * Middleware are executed in registration order.
 * @param mw      Middleware pipeline.
 * @param fn      Middleware callback.
 * @param userdata User data (may be NULL).
 * @param dtor    Destructor for userdata (may be NULL).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_middleware_use(web_middleware_t *mw, web_middleware_fn fn,
                   void *userdata, web_middleware_dtor_fn dtor);

/**
 * Set a key-value pair on the middleware context.
 * This allows middleware to pass data (e.g. authenticated user)
 * to downstream handlers.
 * @param ctx   Middleware context.
 * @param key   Key string (copied internally).
 * @param value Value pointer (not copied, stored as-is).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_middleware_ctx_set(web_middleware_ctx_t *ctx,
                       const char *key, void *value);

/**
 * Get a value from the middleware context by key.
 * @return The stored value, or NULL if not found.
 */
WEB_API void *
web_middleware_ctx_get(const web_middleware_ctx_t *ctx, const char *key);

/**
 * Destroy middleware pipeline and all registered middleware.
 */
WEB_API void
web_middleware_destroy(web_middleware_t *mw);

#ifdef __cplusplus
}
#endif

#endif /* WEB_MIDDLEWARE_H */
