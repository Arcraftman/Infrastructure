#include "stk/core/internal/hash.h"

#include <string.h>

/* FNV-1a 64bit，截断到 size_t（LP64 下 size_t 是 64bit，无损失）。 */
size_t stk_i_hash_bytes(const void *key, size_t len) {
    const unsigned char *p = (const unsigned char *)key;
    size_t h = 1469598103934665603ULL; /* FNV offset basis */
    for (size_t i = 0; i < len; i++) {
        h ^= (size_t)p[i];
        h *= 1099511628211ULL;         /* FNV prime */
    }
    return h;
}

size_t stk_i_hash_cstr(const void *key) {
    const unsigned char *p = (const unsigned char *)key;
    size_t h = 1469598103934665603ULL;
    if (p == NULL) return h;
    while (*p) {
        h ^= (size_t)(*p++);
        h *= 1099511628211ULL;
    }
    return h;
}

size_t stk_i_hash_ptr(const void *key) {
    /* 直接拿指针值，混合一下低位（指针常按对齐，低位多为 0）。 */
    size_t h = (size_t)key;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    return h;
}

int stk_i_eq_cstr(const void *a, const void *b) {
    if (a == b) return 1;              /* 同为同一指针（含都 NULL）即相等 */
    if (a == NULL || b == NULL) return 0;
    return strcmp((const char *)a, (const char *)b) == 0;
}

int stk_i_eq_ptr(const void *a, const void *b) {
    return a == b;
}
