#include "web/session.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* =========================================================================
 * Internal types
 * ========================================================================= */

#define SESSION_HASH_SIZE 256
#define SESSION_ID_LEN    32    /* hex string = 128 bits */
#define SESSION_KEY_MAX   256
#define SESSION_VALUE_MAX 8192

/* Hash chain node */
typedef struct session_data {
    char               *key;
    void               *value;
    size_t              value_len;
    struct session_data *next;
} session_data_t;

typedef struct session_entry {
    char                id[SESSION_ID_LEN + 1];
    session_data_t     *data;
    time_t              created;
    time_t              accessed;
    int                 ttl_sec;          /* per-session TTL override      */
    struct session_entry *hash_next;
    struct session_entry *prev;           /* expiry list */
    struct session_entry *next;
} session_entry_t;

struct web_session_store {
    session_entry_t  *hash_table[SESSION_HASH_SIZE];
    session_entry_t  *expire_head;         /* oldest access first */
    session_entry_t  *expire_tail;
    int               default_ttl;         /* seconds */
    int               cleanup_interval;
    time_t            last_cleanup;
    size_t            count;
    pthread_mutex_t   lock;
};

/* =========================================================================
 * Hash helpers
 * ========================================================================= */

static size_t
hash_id(const char *id)
{
    size_t h = 5381;
    int c;
    while ((c = (unsigned char)*id++))
        h = ((h << 5) + h) + (size_t)c;
    return h % SESSION_HASH_SIZE;
}

/* =========================================================================
 * Session ID generation
 * ========================================================================= */

static void
generate_id(char *buf, size_t len)
{
    static const char HEX[] = "0123456789abcdef";
    size_t i;

    /* Fallback: use time + pid + cheap pseudo-random */
    time_t t = time(NULL);
    unsigned int r = (unsigned int)((t & 0xFFFF) ^ ((t >> 16) & 0xFFFF));
    r ^= (unsigned int)(uintptr_t)buf; /* some ASLR entropy */

    for (i = 0; i < len - 1; i++) {
        r = r * 1103515245U + 12345U;
        buf[i] = HEX[(r >> 16) & 0x0F];
    }
    buf[len - 1] = '\0';
}

/* =========================================================================
 * Entry lifecycle
 * ========================================================================= */

static session_entry_t *
entry_find(web_session_store_t *store, const char *sid)
{
    size_t h = hash_id(sid);
    for (session_entry_t *e = store->hash_table[h]; e; e = e->hash_next)
        if (strcmp(e->id, sid) == 0)
            return e;
    return NULL;
}

static void
entry_link_expire(web_session_store_t *store, session_entry_t *e)
{
    /* Remove from current position */
    if (e->prev) e->prev->next = e->next;
    if (e->next) e->next->prev = e->prev;
    if (store->expire_head == e) store->expire_head = e->next;
    if (store->expire_tail == e) store->expire_tail = e->prev;

    /* Insert at tail */
    e->prev = store->expire_tail;
    e->next = NULL;
    if (store->expire_tail)
        store->expire_tail->next = e;
    else
        store->expire_head = e;
    store->expire_tail = e;
}

static void
entry_destroy(web_session_store_t *store, session_entry_t *e)
{
    if (!e) return;

    /* Unlink from hash */
    size_t h = hash_id(e->id);
    session_entry_t **pp = &store->hash_table[h];
    while (*pp) {
        if (*pp == e) { *pp = e->hash_next; break; }
        pp = &(*pp)->hash_next;
    }

    /* Unlink from expire list */
    if (e->prev) e->prev->next = e->next;
    if (e->next) e->next->prev = e->prev;
    if (store->expire_head == e) store->expire_head = e->next;
    if (store->expire_tail == e) store->expire_tail = e->prev;

    /* Free data */
    session_data_t *d = e->data;
    while (d) {
        session_data_t *next = d->next;
        free(d->key);
        free(d->value);
        free(d);
        d = next;
    }
    free(e);
    store->count--;
}

/* =========================================================================
 * Cleanup expired sessions
 * ========================================================================= */

static void
cleanup_expired(web_session_store_t *store)
{
    time_t now = time(NULL);
    if (store->cleanup_interval > 0 &&
        (now - store->last_cleanup) < store->cleanup_interval)
        return;

    store->last_cleanup = now;

    /* Walk expire list from head (oldest) */
    session_entry_t *e = store->expire_head;
    while (e) {
        session_entry_t *next = e->next;
        int ttl = e->ttl_sec > 0 ? e->ttl_sec : store->default_ttl;
        if (ttl > 0 && (now - e->accessed) > ttl)
            entry_destroy(store, e);
        e = next;
    }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

WEB_API web_session_store_t *
web_session_store_create(int default_ttl, int cleanup_interval)
{
    web_session_store_t *store = (web_session_store_t *)calloc(1, sizeof(*store));
    if (!store) return NULL;

    store->default_ttl     = default_ttl > 0 ? default_ttl : 1800;  /* 30 min */
    store->cleanup_interval = cleanup_interval > 0 ? cleanup_interval : 60;
    store->last_cleanup    = time(NULL);
    pthread_mutex_init(&store->lock, NULL);
    return store;
}

WEB_API char *
web_session_create(web_session_store_t *store)
{
    if (!store) { errno = EINVAL; return NULL; }

    char sid[SESSION_ID_LEN + 1];
    session_entry_t *e = NULL;

    /* Retry loop to avoid ID collision */
    for (int attempt = 0; attempt < 10; attempt++) {
        generate_id(sid, sizeof(sid));

        pthread_mutex_lock(&store->lock);
        if (!entry_find(store, sid)) {
            e = (session_entry_t *)calloc(1, sizeof(*e));
            if (e) {
                memcpy(e->id, sid, SESSION_ID_LEN + 1);
                e->created  = time(NULL);
                e->accessed = e->created;
                e->ttl_sec  = 0; /* use default */

                /* Hash insert */
                size_t h = hash_id(sid);
                e->hash_next = store->hash_table[h];
                store->hash_table[h] = e;

                /* Expire list (tail) */
                e->prev = store->expire_tail;
                e->next = NULL;
                if (store->expire_tail)
                    store->expire_tail->next = e;
                else
                    store->expire_head = e;
                store->expire_tail = e;

                store->count++;
            }
            pthread_mutex_unlock(&store->lock);
            if (e) break;
        } else {
            pthread_mutex_unlock(&store->lock);
        }
    }

    if (!e) return NULL;
    return strdup(e->id);
}

WEB_API int
web_session_set(web_session_store_t *store, const char *session_id,
                const char *key, const void *value, size_t value_len)
{
    if (!store || !session_id || !key) return -1;
    if (strlen(key) >= SESSION_KEY_MAX) { errno = EINVAL; return -1; }
    if (value_len >= SESSION_VALUE_MAX) { errno = EINVAL; return -1; }

    pthread_mutex_lock(&store->lock);

    cleanup_expired(store);

    session_entry_t *e = entry_find(store, session_id);
    if (!e) {
        pthread_mutex_unlock(&store->lock);
        errno = ENOENT;
        return -1;
    }
    e->accessed = time(NULL);
    entry_link_expire(store, e);

    /* Find or create data entry */
    session_data_t **pp = &e->data;
    while (*pp) {
        if (strcmp((*pp)->key, key) == 0) {
            /* Update existing */
            free((*pp)->value);
            (*pp)->value = NULL;
            (*pp)->value_len = 0;
            if (value && value_len > 0) {
                (*pp)->value = malloc(value_len);
                if ((*pp)->value) {
                    memcpy((*pp)->value, value, value_len);
                    (*pp)->value_len = value_len;
                }
            }
            pthread_mutex_unlock(&store->lock);
            return 0;
        }
        pp = &(*pp)->next;
    }

    /* Create new data entry */
    session_data_t *d = (session_data_t *)calloc(1, sizeof(*d));
    if (!d) { pthread_mutex_unlock(&store->lock); return -1; }
    d->key = strdup(key);
    if (!d->key) { free(d); pthread_mutex_unlock(&store->lock); return -1; }
    if (value && value_len > 0) {
        d->value = malloc(value_len);
        if (d->value) {
            memcpy(d->value, value, value_len);
            d->value_len = value_len;
        }
    }
    d->next = e->data;
    e->data = d;

    pthread_mutex_unlock(&store->lock);
    return 0;
}

WEB_API int
web_session_get(web_session_store_t *store, const char *session_id,
                const char *key, void **value, size_t *value_len)
{
    if (!store || !session_id || !key || !value || !value_len) return -1;

    pthread_mutex_lock(&store->lock);

    cleanup_expired(store);

    session_entry_t *e = entry_find(store, session_id);
    if (!e) {
        pthread_mutex_unlock(&store->lock);
        errno = ENOENT;
        return -1;
    }

    /* Check TTL */
    int ttl = e->ttl_sec > 0 ? e->ttl_sec : store->default_ttl;
    if (ttl > 0) {
        time_t now = time(NULL);
        if (now - e->accessed > ttl) {
            entry_destroy(store, e);
            pthread_mutex_unlock(&store->lock);
            errno = ENOENT;
            return -1;
        }
    }

    e->accessed = time(NULL);
    entry_link_expire(store, e);

    for (session_data_t *d = e->data; d; d = d->next) {
        if (strcmp(d->key, key) == 0) {
            *value = d->value;
            *value_len = d->value_len;
            pthread_mutex_unlock(&store->lock);
            return 0;
        }
    }

    *value = NULL;
    *value_len = 0;
    pthread_mutex_unlock(&store->lock);
    return -1;
}

WEB_API int
web_session_delete(web_session_store_t *store, const char *session_id)
{
    if (!store || !session_id) return -1;

    pthread_mutex_lock(&store->lock);

    session_entry_t *e = entry_find(store, session_id);
    if (!e) {
        pthread_mutex_unlock(&store->lock);
        return -1;
    }

    entry_destroy(store, e);
    pthread_mutex_unlock(&store->lock);
    return 0;
}

WEB_API int
web_session_touch(web_session_store_t *store, const char *session_id)
{
    if (!store || !session_id) return -1;

    pthread_mutex_lock(&store->lock);

    session_entry_t *e = entry_find(store, session_id);
    if (!e) {
        pthread_mutex_unlock(&store->lock);
        return -1;
    }

    e->accessed = time(NULL);
    entry_link_expire(store, e);
    pthread_mutex_unlock(&store->lock);
    return 0;
}

WEB_API size_t
web_session_count(const web_session_store_t *store)
{
    if (!store) return 0;
    size_t cnt;
    pthread_mutex_lock(&((web_session_store_t *)store)->lock);
    cnt = store->count;
    pthread_mutex_unlock(&((web_session_store_t *)store)->lock);
    return cnt;
}

WEB_API void
web_session_store_destroy(web_session_store_t *store)
{
    if (!store) return;

    pthread_mutex_lock(&store->lock);

    for (size_t i = 0; i < SESSION_HASH_SIZE; i++) {
        session_entry_t *e = store->hash_table[i];
        while (e) {
            session_entry_t *next = e->hash_next;
            session_data_t *d = e->data;
            while (d) {
                session_data_t *dnext = d->next;
                free(d->key);
                free(d->value);
                free(d);
                d = dnext;
            }
            free(e);
            e = next;
        }
        store->hash_table[i] = NULL;
    }
    store->expire_head = NULL;
    store->expire_tail = NULL;
    store->count = 0;

    pthread_mutex_unlock(&store->lock);
    pthread_mutex_destroy(&store->lock);
    memset(store, 0, sizeof(*store));
    free(store);
}
