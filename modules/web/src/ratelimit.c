#include "web/ratelimit.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =========================================================================
 * Internal types
 * ========================================================================= */

#define RL_HASH_SIZE 256

typedef struct rl_entry {
    char               *key;           /* client identifier (IP, API key, …) */
    double              tokens;        /* current token count               */
    double              capacity;      /* max tokens (burst)                */
    double              rate;          /* tokens per second                 */
    time_t              last_refill;   /* last refill timestamp             */
    struct rl_entry    *hash_next;
    struct rl_entry    *prev;          /* expiry list (prev) */
    struct rl_entry    *next;          /* expiry list (next) */
} rl_entry_t;

struct web_ratelimit {
    rl_entry_t  *hash_table[RL_HASH_SIZE];
    rl_entry_t  *expire_head;          /* oldest first */
    rl_entry_t  *expire_tail;
    double       default_rate;         /* tokens/sec  */
    double       default_capacity;     /* burst       */
    int          cleanup_interval;     /* seconds     */
    time_t       last_cleanup;
    size_t       count;
    pthread_mutex_t lock;
};

/* =========================================================================
 * Hash helpers
 * ========================================================================= */

static size_t
hash_key(const char *key)
{
    size_t h = 5381;
    int c;
    while ((c = (unsigned char)*key++))
        h = ((h << 5) + h) + (size_t)c;
    return h % RL_HASH_SIZE;
}

/* =========================================================================
 * Entry lifecycle
 * ========================================================================= */

static rl_entry_t *
entry_find(web_ratelimit_t *rl, const char *key)
{
    size_t h = hash_key(key);
    for (rl_entry_t *e = rl->hash_table[h]; e; e = e->hash_next)
        if (strcmp(e->key, key) == 0)
            return e;
    return NULL;
}

static void
entry_link_expire(web_ratelimit_t *rl, rl_entry_t *e)
{
    /* Insert at tail (most recent) */
    e->prev = rl->expire_tail;
    e->next = NULL;
    if (rl->expire_tail)
        rl->expire_tail->next = e;
    else
        rl->expire_head = e;
    rl->expire_tail = e;
}

static void
entry_unlink_expire(web_ratelimit_t *rl, rl_entry_t *e)
{
    if (e->prev)
        e->prev->next = e->next;
    if (e->next)
        e->next->prev = e->prev;
    if (rl->expire_head == e)
        rl->expire_head = e->next;
    if (rl->expire_tail == e)
        rl->expire_tail = e->prev;
}

static void
entry_destroy(web_ratelimit_t *rl, rl_entry_t *e)
{
    if (!e) return;

    /* Unlink from hash */
    size_t h = hash_key(e->key);
    rl_entry_t **pp = &rl->hash_table[h];
    while (*pp) {
        if (*pp == e) { *pp = e->hash_next; break; }
        pp = &(*pp)->hash_next;
    }

    entry_unlink_expire(rl, e);
    free(e->key);
    free(e);
    rl->count--;
}

/* Refill tokens based on elapsed time */
static void
entry_refill(rl_entry_t *e)
{
    time_t now = time(NULL);
    double elapsed = difftime(now, e->last_refill);
    if (elapsed > 0) {
        e->tokens += elapsed * e->rate;
        if (e->tokens > e->capacity)
            e->tokens = e->capacity;
        e->last_refill = now;
    }
}

/* =========================================================================
 * Cleanup stale entries (entries with full tokens for > cleanup_interval)
 * ========================================================================= */

static void
cleanup_stale(web_ratelimit_t *rl)
{
    time_t now = time(NULL);
    if (rl->cleanup_interval <= 0) return;
    if (now - rl->last_cleanup < rl->cleanup_interval) return;
    rl->last_cleanup = now;

    rl_entry_t *e = rl->expire_head;
    while (e) {
        rl_entry_t *next = e->next;
        /* Remove entries that are idle (tokens at capacity and old) */
        if (e->tokens >= e->capacity && (now - e->last_refill) > rl->cleanup_interval)
            entry_destroy(rl, e);
        e = next;
    }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

WEB_API web_ratelimit_t *
web_ratelimit_create(double default_rate, double default_capacity)
{
    if (default_rate <= 0 || default_capacity <= 0) {
        errno = EINVAL;
        return NULL;
    }

    web_ratelimit_t *rl = (web_ratelimit_t *)calloc(1, sizeof(*rl));
    if (!rl) return NULL;

    rl->default_rate     = default_rate;
    rl->default_capacity = default_capacity;
    rl->cleanup_interval = 300; /* default: 5 minutes */
    rl->last_cleanup     = time(NULL);
    pthread_mutex_init(&rl->lock, NULL);
    return rl;
}

WEB_API int
web_ratelimit_set_cleanup_interval(web_ratelimit_t *rl, int interval_sec)
{
    if (!rl) return -1;
    pthread_mutex_lock(&rl->lock);
    rl->cleanup_interval = interval_sec > 0 ? interval_sec : 300;
    pthread_mutex_unlock(&rl->lock);
    return 0;
}

WEB_API int
web_ratelimit_allow(web_ratelimit_t *rl, const char *key)
{
    return web_ratelimit_consume(rl, key, 1.0);
}

WEB_API int
web_ratelimit_consume(web_ratelimit_t *rl, const char *key, double cost)
{
    if (!rl || !key) return -1;

    pthread_mutex_lock(&rl->lock);

    cleanup_stale(rl);

    rl_entry_t *e = entry_find(rl, key);
    if (!e) {
        /* New client — create entry */
        e = (rl_entry_t *)calloc(1, sizeof(*e));
        if (!e) { pthread_mutex_unlock(&rl->lock); return -1; }
        e->key      = strdup(key);
        if (!e->key) { free(e); pthread_mutex_unlock(&rl->lock); return -1; }
        e->tokens    = rl->default_capacity;
        e->capacity  = rl->default_capacity;
        e->rate      = rl->default_rate;
        e->last_refill = time(NULL);

        /* Hash insert */
        size_t h = hash_key(key);
        e->hash_next = rl->hash_table[h];
        rl->hash_table[h] = e;

        entry_link_expire(rl, e);
        rl->count++;
    } else {
        entry_refill(e);

        /* Move to expire list tail (recently active) */
        entry_unlink_expire(rl, e);
        entry_link_expire(rl, e);
    }

    int allowed = (e->tokens >= cost) ? 1 : 0;
    if (allowed)
        e->tokens -= cost;

    pthread_mutex_unlock(&rl->lock);
    return allowed;
}

WEB_API int
web_ratelimit_reset(web_ratelimit_t *rl, const char *key)
{
    if (!rl || !key) return -1;

    pthread_mutex_lock(&rl->lock);

    rl_entry_t *e = entry_find(rl, key);
    if (e) {
        e->tokens = e->capacity;
        e->last_refill = time(NULL);
        pthread_mutex_unlock(&rl->lock);
        return 0;
    }

    pthread_mutex_unlock(&rl->lock);
    return -1;
}

WEB_API size_t
web_ratelimit_count(const web_ratelimit_t *rl)
{
    if (!rl) return 0;
    size_t cnt;
    pthread_mutex_lock(&((web_ratelimit_t *)rl)->lock);
    cnt = rl->count;
    pthread_mutex_unlock(&((web_ratelimit_t *)rl)->lock);
    return cnt;
}

WEB_API void
web_ratelimit_destroy(web_ratelimit_t *rl)
{
    if (!rl) return;

    pthread_mutex_lock(&rl->lock);

    for (size_t i = 0; i < RL_HASH_SIZE; i++) {
        rl_entry_t *e = rl->hash_table[i];
        while (e) {
            rl_entry_t *next = e->hash_next;
            free(e->key);
            free(e);
            e = next;
        }
        rl->hash_table[i] = NULL;
    }
    rl->expire_head = NULL;
    rl->expire_tail = NULL;
    rl->count = 0;

    pthread_mutex_unlock(&rl->lock);
    pthread_mutex_destroy(&rl->lock);
    memset(rl, 0, sizeof(*rl));
    free(rl);
}
