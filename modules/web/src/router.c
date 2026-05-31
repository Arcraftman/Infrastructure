#include "web/router.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Trie-based URL router.
 *
 * Each node stores a path segment and optionally a handler.
 * A trailing '*' segment acts as a wildcard / catch-all.
 * The '*' must appear at the end of the route pattern.
 */

/* =========================================================================
 * Node
 * ========================================================================= */

typedef struct trie_node {
    char              *segment;   /* path segment text */
    int                is_wild;   /* 1 if segment is "*" */
    void              *handler;   /* user handler (web_route_handler) */
    struct trie_node  *child;     /* first child (sibling list) */
    struct trie_node  *next;      /* next sibling */
} trie_node_t;

struct web_router {
    trie_node_t       *root;
    int                ignore_case; /* 1 = case-insensitive matching */
    char               error_buf[256]; /* last route_add error */
};

/* =========================================================================
 * Helper: split path into segments
 * ========================================================================= */

static int
split_path(const char *path, size_t len, char ***segs_out, int *count_out)
{
    if (!path || !len) len = strlen(path);
    if (!len || path[0] != '/') return -1;

    /* count segments */
    int cap = 8, cnt = 0;
    char **segs = (char **)malloc((size_t)cap * sizeof(char *));
    if (!segs) return -1;

    size_t i = 1; /* skip leading '/' */
    while (i < len) {
        size_t start = i;
        while (i < len && path[i] != '/') i++;
        if (i > start) {
            if (cnt >= cap) {
                cap *= 2;
                char **tmp = (char **)realloc(segs, (size_t)cap * sizeof(char *));
                if (!tmp) { for (int j = 0; j < cnt; j++) free(segs[j]); free(segs); return -1; }
                segs = tmp;
            }
            segs[cnt] = strndup(path + start, i - start);
            if (!segs[cnt]) { for (int j = 0; j < cnt; j++) free(segs[j]); free(segs); return -1; }
            cnt++;
        }
        if (i < len) i++;
    }

    *segs_out = segs;
    *count_out = cnt;
    return 0;
}

/* =========================================================================
 * Router create / destroy
 * ========================================================================= */

WEB_API web_router_t *
web_router_create(void)
{
    web_router_t *r = (web_router_t *)calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->root = (trie_node_t *)calloc(1, sizeof(trie_node_t));
    if (!r->root) { free(r); return NULL; }
    r->root->segment = strdup("/");
    r->ignore_case = 0;
    return r;
}

WEB_API void
web_router_set_ignore_case(web_router_t *router, int ignore)
{
    if (router) router->ignore_case = ignore ? 1 : 0;
}

/* =========================================================================
 * Add route
 * ========================================================================= */

static int
seg_cmp(const trie_node_t *n, const char *seg, int ignore_case)
{
    if (n->is_wild) return 0; /* wild matches any */
    if (ignore_case)
        return strcasecmp(n->segment, seg);
    return strcmp(n->segment, seg);
}

static trie_node_t *
node_alloc(const char *seg)
{
    trie_node_t *n = (trie_node_t *)calloc(1, sizeof(*n));
    if (!n) return NULL;
    n->segment = strdup(seg);
    n->is_wild = (strcmp(seg, "*") == 0);
    return n;
}

static trie_node_t *
node_find_child(trie_node_t *parent, const char *seg, int ignore_case)
{
    for (trie_node_t *c = parent->child; c; c = c->next) {
        if (seg_cmp(c, seg, ignore_case) == 0 && c->is_wild == (strcmp(seg, "*") == 0))
            return c;
    }
    return NULL;
}

WEB_API int
web_router_add(web_router_t *router, const char *method,
               const char *path, web_route_handler handler)
{
    if (!router || !path || !handler) return -1;
    if (path[0] != '/') return -1;

    /* split path */
    char **segs = NULL;
    int nsegs = 0;
    if (split_path(path, 0, &segs, &nsegs) != 0) {
        snprintf(router->error_buf, sizeof(router->error_buf),
                 "Invalid path: %s", path);
        return -1;
    }

    /* Validate wildcard position */
    for (int i = 0; i < nsegs; i++) {
        if (strcmp(segs[i], "*") == 0 && i != nsegs - 1) {
            for (int j = 0; j < nsegs; j++) free(segs[j]);
            free(segs);
            snprintf(router->error_buf, sizeof(router->error_buf),
                     "Wildcard must be the last segment");
            return -1;
        }
    }

    /* Walk the trie, creating nodes as needed */
    trie_node_t *cur = router->root;
    for (int i = 0; i < nsegs; i++) {
        trie_node_t *child = node_find_child(cur, segs[i], router->ignore_case);
        if (!child) {
            child = node_alloc(segs[i]);
            if (!child) {
                for (int j = 0; j < nsegs; j++) free(segs[j]);
                free(segs);
                return -1;
            }
            child->next = cur->child;
            cur->child  = child;
        }
        cur = child;
    }

    free(segs);

    /* Store as method:handler pair using method-list */
    /* We store a simple linked list of method-handler structs at the node. */
    typedef struct mh_pair { char method[8]; web_route_handler handler; struct mh_pair *next; } mh_pair_t;

    /* Check if method already registered */
    mh_pair_t *prev = NULL;
    mh_pair_t *mh = (mh_pair_t *)cur->handler;
    while (mh) {
        if (strcmp(mh->method, method) == 0) {
            mh->handler = handler; /* override */
            return 0;
        }
        prev = mh;
        mh = mh->next;
    }

    mh_pair_t *new_mh = (mh_pair_t *)malloc(sizeof(*new_mh));
    if (!new_mh) return -1;
    size_t mlen = strlen(method);
    if (mlen >= sizeof(new_mh->method)) mlen = sizeof(new_mh->method) - 1;
    memcpy(new_mh->method, method, mlen);
    new_mh->method[mlen] = '\0';
    new_mh->handler = handler;
    new_mh->next = NULL;

    if (prev)
        prev->next = new_mh;
    else
        cur->handler = new_mh;

    return 0;
}

/* =========================================================================
 * Route lookup
 * ========================================================================= */

WEB_API web_route_handler
web_router_match(const web_router_t *router, const char *method,
                 const char *path, web_match_params_t *params)
{
    if (!router || !method || !path) return NULL;

    char **segs = NULL;
    int nsegs = 0;
    if (split_path(path, 0, &segs, &nsegs) != 0)
        return NULL;

    /* Allocate params slots */
    if (params) {
        params->count = 0;
        for (int i = 0; i < nsegs; i++)
            params->keys[i] = NULL;
    }

    typedef struct mh_pair { char method[8]; web_route_handler handler; struct mh_pair *next; } mh_pair_t;

    /* Traverse */
    const trie_node_t *cur = router->root;
    int si = 0;

    while (cur && si < nsegs) {
        web_route_handler wild_handler = NULL;
        const trie_node_t *exact = NULL;

        for (const trie_node_t *c = cur->child; c; c = c->next) {
            if (c->is_wild) {
                wild_handler = (web_route_handler)(uintptr_t)c->handler;
                /* save all remaining segments as wildcard param */
                if (params) {
                    int pi = params->count;
                    if (pi < WEB_ROUTER_MAX_PARAMS) {
                        size_t remaining = 0;
                        for (int j = si; j < nsegs; j++) {
                            remaining += strlen(segs[j]) + 1;
                        }
                        char *val = remaining > 0 ? (char *)malloc(remaining) : NULL;
                        if (val) {
                            val[0] = '\0';
                            for (int j = si; j < nsegs; j++) {
                                if (j > si) strcat(val, "/");
                                strcat(val, segs[j]);
                            }
                        }
                        params->keys[pi] = (char *)"*";
                        params->values[pi] = val;
                        params->count++;
                    }
                }
                /* found a wildcard match */
                break;
            }
            if (seg_cmp(c, segs[si], router->ignore_case) == 0) {
                exact = c;
            }
        }

        /* found a param pattern segment (:foo) */
        for (const trie_node_t *c = cur->child; c; c = c->next) {
            if (c->segment && c->segment[0] == ':' && !c->is_wild) {
                if (params && params->count < WEB_ROUTER_MAX_PARAMS) {
                    int pi = params->count;
                    params->keys[pi]   = (char *)c->segment + 1; /* skip ':' */
                    params->values[pi] = strdup(segs[si]);
                    params->count++;
                }
                exact = c;
                break; /* param match overrides exact segment match */
            }
        }

        if (exact) {
            cur = exact;
            si++;
        } else {
            /* No match — try wildcard */
            if (wild_handler) {
                cur->handler = (void *)(uintptr_t)wild_handler;
                goto done;
            }
            goto nomatch;
        }
    }

    /* If we consumed all segments, check if this node has a handler */
    /* Also handle trailing '*' in pattern where path has more segments */
    done:
    if (cur) {
        mh_pair_t *mh = (mh_pair_t *)cur->handler;
        while (mh) {
            if (strcmp(mh->method, method) == 0 ||
                strcmp(mh->method, "*") == 0) {
                for (int j = 0; j < nsegs; j++) free(segs[j]);
                free(segs);
                return mh->handler;
            }
            mh = mh->next;
        }
    }

nomatch:
    /* Check for wildcard child that we might have missed (path shorter than pattern) */
    for (const trie_node_t *c = cur ? cur->child : NULL; c; c = c->next) {
        if (c->is_wild && c->handler) {
            mh_pair_t *mh = (mh_pair_t *)c->handler;
            while (mh) {
                if (strcmp(mh->method, method) == 0 || strcmp(mh->method, "*") == 0) {
                    for (int j = 0; j < nsegs; j++) free(segs[j]);
                    free(segs);
                    return mh->handler;
                }
                mh = mh->next;
            }
        }
    }

    for (int j = 0; j < nsegs; j++) free(segs[j]);
    free(segs);
    return NULL;
}

/* =========================================================================
 * Get error
 * ========================================================================= */

WEB_API const char *
web_router_error(const web_router_t *router)
{
    return router ? router->error_buf : NULL;
}

/* =========================================================================
 * Destroy
 * ========================================================================= */

static void
node_destroy(trie_node_t *n)
{
    if (!n) return;
    for (trie_node_t *c = n->child; c; ) {
        trie_node_t *next = c->next;
        node_destroy(c);
        c = next;
    }
    free(n->segment);
    /* free method-handler pairs */
    typedef struct mh_pair { char method[8]; web_route_handler handler; struct mh_pair *next; } mh_pair_t;
    mh_pair_t *mh = (mh_pair_t *)n->handler;
    while (mh) {
        mh_pair_t *tmp = mh->next;
        free(mh);
        mh = tmp;
    }
    free(n);
}

WEB_API void
web_router_destroy(web_router_t *router)
{
    if (!router) return;
    if (router->root)
        node_destroy(router->root);
    free(router);
}
