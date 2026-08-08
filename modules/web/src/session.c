#include "web/session.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SESSION_ID_LEN 32

typedef struct session_value {
    char* key;
    char* value;
    struct session_value* next;
} session_value_t;

struct web_session {
    char id[SESSION_ID_LEN + 1];
    time_t accessed;
    session_value_t* values;
    struct web_session* next;
};

struct web_session_store {
    web_session_t* sessions;
    long expiry_secs;
    pthread_mutex_t lock;
};

static void make_id(char id[SESSION_ID_LEN + 1])
{
    static const char hex[] = "0123456789abcdef";
    unsigned long seed = (unsigned long)time(NULL) ^ (unsigned long)(uintptr_t)id;
    size_t i;
    for (i = 0; i < SESSION_ID_LEN; ++i) {
        seed = seed * 1103515245UL + 12345UL;
        id[i] = hex[(seed >> 28) & 0x0f];
    }
    id[SESSION_ID_LEN] = '\0';
}

static void values_free(session_value_t* value)
{
    while (value) {
        session_value_t* next = value->next;
        free(value->key);
        free(value->value);
        free(value);
        value = next;
    }
}

static web_session_t* find_session(web_session_store_t* store, const char* id)
{
    web_session_t* session;
    for (session = store->sessions; session; session = session->next) {
        if (strcmp(session->id, id) == 0)
            return session;
    }
    return NULL;
}

WEB_API web_session_store_t* web_session_store_create(long expiry_secs)
{
    web_session_store_t* store = (web_session_store_t*)calloc(1, sizeof(*store));
    if (!store)
        return NULL;
    store->expiry_secs = expiry_secs > 0 ? expiry_secs : 3600;
    if (pthread_mutex_init(&store->lock, NULL) != 0) {
        free(store);
        return NULL;
    }
    return store;
}

WEB_API web_session_t* web_session_get(web_session_store_t* store,
                                        const char* session_id,
                                        const char** new_session_id)
{
    web_session_t* session;
    if (new_session_id)
        *new_session_id = NULL;
    if (!store)
        return NULL;

    pthread_mutex_lock(&store->lock);
    session = session_id ? find_session(store, session_id) : NULL;
    if (session && store->expiry_secs > 0 && time(NULL) - session->accessed >= store->expiry_secs)
        session = NULL;
    if (!session) {
        session = (web_session_t*)calloc(1, sizeof(*session));
        if (!session) {
            pthread_mutex_unlock(&store->lock);
            return NULL;
        }
        make_id(session->id);
        session->next = store->sessions;
        store->sessions = session;
        if (new_session_id)
            *new_session_id = session->id;
    }
    session->accessed = time(NULL);
    pthread_mutex_unlock(&store->lock);
    return session;
}

WEB_API int web_session_set(web_session_store_t* store,
                            web_session_t* session,
                            const char* key,
                            const char* value)
{
    session_value_t* item;
    if (!store || !session || !key || key[0] == '\0')
        return -1;
    pthread_mutex_lock(&store->lock);
    for (item = session->values; item; item = item->next) {
        if (strcmp(item->key, key) == 0) {
            char* copy = value ? strdup(value) : NULL;
            if (value && !copy) {
                pthread_mutex_unlock(&store->lock);
                return -1;
            }
            free(item->value);
            item->value = copy;
            pthread_mutex_unlock(&store->lock);
            return 0;
        }
    }
    if (!value) {
        pthread_mutex_unlock(&store->lock);
        return 0;
    }
    item = (session_value_t*)calloc(1, sizeof(*item));
    if (!item || !(item->key = strdup(key)) || !(item->value = strdup(value))) {
        if (item) {
            free(item->key);
            free(item->value);
            free(item);
        }
        pthread_mutex_unlock(&store->lock);
        return -1;
    }
    item->next = session->values;
    session->values = item;
    pthread_mutex_unlock(&store->lock);
    return 0;
}

WEB_API const char* web_session_get_value(const web_session_t* session, const char* key)
{
    session_value_t* item;
    if (!session || !key)
        return NULL;
    for (item = session->values; item; item = item->next) {
        if (strcmp(item->key, key) == 0)
            return item->value;
    }
    return NULL;
}

WEB_API void web_session_destroy(web_session_store_t* store, web_session_t* session)
{
    web_session_t** link;
    if (!store || !session)
        return;
    pthread_mutex_lock(&store->lock);
    link = &store->sessions;
    while (*link && *link != session)
        link = &(*link)->next;
    if (*link == session) {
        *link = session->next;
        values_free(session->values);
        free(session);
    }
    pthread_mutex_unlock(&store->lock);
}

WEB_API void web_session_store_cleanup(web_session_store_t* store)
{
    web_session_t** link;
    time_t now;
    if (!store)
        return;
    pthread_mutex_lock(&store->lock);
    now = time(NULL);
    link = &store->sessions;
    while (*link) {
        web_session_t* session = *link;
        if (now - session->accessed >= store->expiry_secs) {
            *link = session->next;
            values_free(session->values);
            free(session);
        } else {
            link = &session->next;
        }
    }
    pthread_mutex_unlock(&store->lock);
}

WEB_API void web_session_store_destroy(web_session_store_t* store)
{
    web_session_t* session;
    if (!store)
        return;
    pthread_mutex_lock(&store->lock);
    session = store->sessions;
    while (session) {
        web_session_t* next = session->next;
        values_free(session->values);
        free(session);
        session = next;
    }
    store->sessions = NULL;
    pthread_mutex_unlock(&store->lock);
    pthread_mutex_destroy(&store->lock);
    free(store);
}
