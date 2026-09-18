#include "stk/core/hash.h"

/* FNV-1a 64 bit，截断到 size_t（32 位平台自然截断）。 */
static const size_t STK_FNV_OFFSET = 14695981039346656037UL;
static const size_t STK_FNV_PRIME  = 1099511628211UL;

size_t stk_hash_cstr(const void *key) {
    const unsigned char *s = (const unsigned char *)key;
    size_t h = STK_FNV_OFFSET;
    if (!s) return h;
    while (*s) {
        h ^= (size_t)(*s++);
        h *= STK_FNV_PRIME;
    }
    return h;
}

size_t stk_hash_int(const void *key) {
    /* 对整数键做一次简单的可逆混淆（splitmix64 风格）。 */
    size_t x = *(const size_t *)key;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdUL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53UL;
    x ^= x >> 33;
    return x;
}

size_t stk_hash_bytes(const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    size_t h = STK_FNV_OFFSET;
    for (size_t i = 0; i < len; i++) {
        h ^= (size_t)p[i];
        h *= STK_FNV_PRIME;
    }
    return h;
}

int stk_eq_cstr(const void *a, const void *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    const char *sa = (const char *)a;
    const char *sb = (const char *)b;
    while (*sa && *sb) {
        if (*sa++ != *sb++) return 0;
    }
    return *sa == *sb;
}

int stk_eq_ptr(const void *a, const void *b) {
    return a == b;
}

int stk_eq_int(const void *a, const void *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    return *(const size_t *)a == *(const size_t *)b;
}
