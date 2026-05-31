#include "web/middleware.h"

#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Middleware chain
 * ========================================================================= */

struct web_middleware_chain {
    web_middleware_t    **entries;   /* array of entries */
    size_t                count;
    size_t                capacity;
};

/* =========================================================================
 * Create chain
 * ========================================================================= */

WEB_API web_middleware_chain_t *
web_middleware_chain_create(void)
{
    web_middleware_chain_t *chain =
        (web_middleware_chain_t *)calloc(1, sizeof(*chain));
    if (!chain) return NULL;
    chain->capacity = 8;
    chain->entries = (web_middleware_t **)malloc(
        chain->capacity * sizeof(web_middleware_t *));
    if (!chain->entries) { free(chain); return NULL; }
    return chain;
}

/* =========================================================================
 * Add middleware
 * ========================================================================= */

WEB_API int
web_middleware_use(web_middleware_chain_t *chain,
                   web_middleware_t *middleware)
{
    if (!chain || !middleware) return -1;

    if (chain->count >= chain->capacity) {
        size_t new_cap = chain->capacity * 2;
        web_middleware_t **tmp = (web_middleware_t **)realloc(
            chain->entries, new_cap * sizeof(web_middleware_t *));
        if (!tmp) return -1;
        chain->entries  = tmp;
        chain->capacity = new_cap;
    }

    chain->entries[chain->count++] = middleware;
    return 0;
}

/* =========================================================================
 * Execute chain
 * ========================================================================= */

WEB_API int
web_middleware_execute(web_middleware_chain_t *chain,
                       web_request_t *req, web_response_t *resp,
                       web_route_handler route_handler,
                       void *route_ctx)
{
    if (!chain || !req || !resp) return -1;

    /* Build a simple recursive on-stack execution */
    /* We'll call middleware entries one by one, each can call "next" */
    /* Define a "next" function pointer type internally. */
    typedef int (*mw_next_t)(int idx, web_request_t *req,
                             web_response_t *resp, void *ctx);

    /* We allocate a small struct on the heap to capture chain + handler */
    struct exec_ctx {
        web_middleware_chain_t *chain;
        web_route_handler       handler;
        void                   *handler_ctx;
    };

    /* Build the next-callback logic */
    /* The closure captures: which index we're at */
    /* We'll implement via a simple loop with a "skip_to_handler" flag */
    int idx = 0;
    while (idx < (int)chain->count) {
        web_middleware_t *mw = chain->entries[idx];
        if (!mw) { idx++; continue; }

        /* "skip" flag */
        int skip = 0;

        /* The middleware decides whether to call "next" */
        /* We'll wrap this: each middleware calls next() by returning a specific int */
        /* Simpler approach: middleware returns:
         *   WEB_MW_NEXT    → continue chain
         *   WEB_MW_STOP    → stop chain, run route handler
         *   WEB_MW_ERROR   → abort with error
         *   WEB_MW_DONE    → response already written, stop
         */
        int mw_action = mw(req, resp);

        switch (mw_action) {
            case WEB_MW_NEXT:
                idx++;
                break;
            case WEB_MW_STOP:
                /* skip remaining middleware and run route handler */
                if (chain->next_handler)
                    return chain->next_handler(req, resp);
                return 0;
            case WEB_MW_DONE:
                return 0; /* middleware wrote the response */
            case WEB_MW_ERROR:
                return -1;
            default:
                /* unknown return, treat as NEXT for forward-compat */
                idx++;
                break;
        }
    }

    /* All middleware passed; run the route handler if any */
    if (route_handler)
        return route_handler(req, resp, route_ctx);

    return 0;
}

/* =========================================================================
 * Destroy
 * ========================================================================= */

WEB_API void
web_middleware_chain_destroy(web_middleware_chain_t *chain)
{
    if (!chain) return;
    free(chain->entries);
    free(chain);
}
