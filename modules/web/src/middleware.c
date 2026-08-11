#include "web/middleware.h"

#include <stdlib.h>
#include <string.h>

typedef struct web_middleware_entry {
    web_middleware_fn fn;
    void* userdata;
    web_middleware_dtor_fn dtor;
} web_middleware_entry_t;

typedef struct web_middleware_value {
    char* key;
    void* value;
    struct web_middleware_value* next;
} web_middleware_value_t;

struct web_middleware {
    web_middleware_entry_t* entries;
    size_t count;
    size_t capacity;
};

struct web_middleware_ctx {
    web_middleware_value_t* values;
};

WEB_API web_middleware_t* web_middleware_create(void)
{
    web_middleware_t* mw = (web_middleware_t*)calloc(1, sizeof(*mw));
    if (!mw)
        return NULL;

    mw->capacity = 8;
    mw->entries = (web_middleware_entry_t*)calloc(mw->capacity, sizeof(*mw->entries));
    if (!mw->entries) {
        free(mw);
        return NULL;
    }
    return mw;
}

WEB_API int web_middleware_use(web_middleware_t* mw,
                               web_middleware_fn fn,
                               void* userdata,
                               web_middleware_dtor_fn dtor)
{
    if (!mw || !fn)
        return -1;

    if (mw->count == mw->capacity) {
        size_t new_capacity = mw->capacity * 2;
        web_middleware_entry_t* entries = (web_middleware_entry_t*)realloc(
            mw->entries, new_capacity * sizeof(*entries));
        if (!entries)
            return -1;
        mw->entries = entries;
        mw->capacity = new_capacity;
    }

    mw->entries[mw->count++] = (web_middleware_entry_t){fn, userdata, dtor};
    return 0;
}

WEB_API int web_middleware_ctx_set(web_middleware_ctx_t* ctx, const char* key, void* value)
{
    web_middleware_value_t* item;

    if (!ctx || !key || key[0] == '\0')
        return -1;

    for (item = ctx->values; item; item = item->next) {
        if (strcmp(item->key, key) == 0) {
            item->value = value;
            return 0;
        }
    }

    item = (web_middleware_value_t*)calloc(1, sizeof(*item));
    if (!item)
        return -1;
    item->key = (char*)malloc(strlen(key) + 1);
    if (!item->key) {
        free(item);
        return -1;
    }
    strcpy(item->key, key);
    item->value = value;
    item->next = ctx->values;
    ctx->values = item;
    return 0;
}

WEB_API void* web_middleware_ctx_get(const web_middleware_ctx_t* ctx, const char* key)
{
    web_middleware_value_t* item;

    if (!ctx || !key)
        return NULL;
    for (item = ctx->values; item; item = item->next) {
        if (strcmp(item->key, key) == 0)
            return item->value;
    }
    return NULL;
}

WEB_API void web_middleware_destroy(web_middleware_t* mw)
{
    size_t i;

    if (!mw)
        return;
    for (i = 0; i < mw->count; ++i) {
        if (mw->entries[i].dtor)
            mw->entries[i].dtor(mw->entries[i].userdata);
    }
    free(mw->entries);
    free(mw);
}
