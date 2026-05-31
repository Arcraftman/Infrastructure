#ifndef WEB_ROUTER_H
#define WEB_ROUTER_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file router.h
 * @brief Enhanced request router with path parameters and middleware support.
 *
 * Supports route patterns like:
 *   - Exact:  "/users/profile"
 *   - Param:  "/users/:id"        -> captures "id" from /users/42
 *   - Wild:   "/static/*path"     -> captures everything after /static/
 *
 * Route matching is priority-ordered: exact > param > wild.
 */

typedef struct web_router      web_router_t;
typedef struct web_route_match web_route_match_t;

/**
 * Route handler callback.
 * @param req   The incoming HTTP request.
 * @param match Route match context (use web_route_match_param() to get path params).
 * @param userdata User data registered with the route.
 * @return A response to send, or NULL to pass to the next middleware/route.
 */
typedef web_response_t *(*web_route_handler_fn)(const web_request_t *req,
                                                 const web_route_match_t *match,
                                                 void *userdata);

/** Destructor for handler userdata. */
typedef void (*web_route_dtor_fn)(void *userdata);

/**
 * Create a new router instance.
 * @return New router, or NULL on allocation failure.
 */
WEB_API web_router_t *
web_router_create(void);

/**
 * Register a route handler.
 * @param router  Router instance.
 * @param method  HTTP method ("GET", "POST", etc.) or "*" for any.
 * @param pattern Route pattern (e.g. "/users/:id", "/static/*path").
 * @param handler Callback.
 * @param userdata User data passed to handler.
 * @param dtor    Destructor for userdata (may be NULL).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_router_add(web_router_t *router, const char *method,
               const char *pattern, web_route_handler_fn handler,
               void *userdata, web_route_dtor_fn dtor);

/**
 * Route an incoming request.
 * @param router Router instance.
 * @param method HTTP method string.
 * @param path   Request path.
 * @param match  [out] On success, populated with match info.
 *               Caller must free with web_route_match_free().
 * @return 0 if a match was found, -1 if no match.
 */
WEB_API int
web_router_route(const web_router_t *router, const char *method,
                 const char *path, web_route_match_t **match);

/**
 * Get a path parameter value from a route match.
 * @param match Route match from web_router_route().
 * @param name  Parameter name (e.g. "id" for route "/users/:id").
 * @return The parameter value, or NULL if not found.
 */
WEB_API const char *
web_route_match_param(const web_route_match_t *match, const char *name);

/**
 * Get the handler registered for this match.
 */
WEB_API web_route_handler_fn
web_route_match_handler(const web_route_match_t *match);

/**
 * Get the userdata registered for this match.
 */
WEB_API void *
web_route_match_userdata(const web_route_match_t *match);

/**
 * Free a route match.
 */
WEB_API void
web_route_match_free(web_route_match_t *match);

/**
 * Destroy the router and all registered routes.
 */
WEB_API void
web_router_destroy(web_router_t *router);

#ifdef __cplusplus
}
#endif

#endif /* WEB_ROUTER_H */
