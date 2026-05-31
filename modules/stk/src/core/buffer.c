#include "stk/core/buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static bool ensure(buffer *b, size_t extra) {
    size_t needed = b->len + extra + 1;
    if (needed <= b->capacity) return true;
    size_t new_cap = b->capacity ? b->capacity : 32;
    while (new_cap < needed) new_cap *= 2;
    char *p = (char *)realloc(b->data, new_cap);
    if (!p) return false;
    b->data = p;
    b->capacity = new_cap;
    return true;
}

void buffer_init(buffer *b) {
    b->data     = NULL;
    b->len      = 0;
    b->capacity = 0;
}

void buffer_init_with_capacity(buffer *b, size_t cap) {
    b->data     = (char *)malloc(cap);
    b->len      = 0;
    b->capacity = b->data ? cap : 0;
    if (b->data) b->data[0] = '\0';
}

void buffer_free(buffer *b) {
    free(b->data);
    b->data = NULL;
    b->len = b->capacity = 0;
}

void buffer_append(buffer *b, const void *src, size_t n) {
    if (!ensure(b, n)) return;
    memcpy(b->data + b->len, src, n);
    b->len += n;
    b->data[b->len] = '\0';
}

void buffer_append_cstr(buffer *b, const char *cstr) {
    buffer_append(b, cstr, strlen(cstr));
}

void buffer_append_char(buffer *b, char ch) {
    if (!ensure(b, 1)) return;
    b->data[b->len++] = ch;
    b->data[b->len] = '\0';
}

void buffer_append_int(buffer *b, int val) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%d", val);
    buffer_append(b, tmp, (size_t)n);
}

void buffer_append_uint(buffer *b, unsigned int val) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%u", val);
    buffer_append(b, tmp, (size_t)n);
}

void buffer_append_hex(buffer *b, const void *data, size_t n) {
    static const char hex[] = "0123456789abcdef";
    size_t needed = n * 2;
    if (!ensure(b, needed)) return;
    const unsigned char *src = (const unsigned char *)data;
    for (size_t i = 0; i < n; i++) {
        b->data[b->len++] = hex[src[i] >> 4];
        b->data[b->len++] = hex[src[i] & 0x0f];
    }
    b->data[b->len] = '\0';
}

void buffer_printf(buffer *b, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    buffer_vprintf(b, fmt, ap);
    va_end(ap);
}

void buffer_vprintf(buffer *b, const char *fmt, va_list ap) {
    va_list copy;
    va_copy(copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    if (needed < 0) { va_end(copy); return; }
    size_t n = (size_t)needed;
    if (!ensure(b, n)) { va_end(copy); return; }
    vsnprintf(b->data + b->len, b->capacity - b->len, fmt, copy);
    b->len += n;
    va_end(copy);
}

void buffer_insert(buffer *b, size_t pos, const void *src, size_t n) {
    if (pos > b->len) pos = b->len;
    if (!ensure(b, n)) return;
    memmove(b->data + pos + n, b->data + pos, b->len - pos);
    memcpy(b->data + pos, src, n);
    b->len += n;
    b->data[b->len] = '\0';
}

void buffer_overwrite(buffer *b, size_t pos, const void *src, size_t n) {
    if (pos > b->len) pos = b->len;
    size_t end = pos + n;
    if (end > b->len) {
        if (!ensure(b, end - b->len)) return;
        b->len = end;
    }
    memcpy(b->data + pos, src, n);
    b->data[b->len] = '\0';
}

void buffer_clear(buffer *b) {
    b->len = 0;
    if (b->data) b->data[0] = '\0';
}

void buffer_reserve(buffer *b, size_t cap) {
    if (cap <= b->capacity) return;
    char *p = (char *)realloc(b->data, cap);
    if (p) {
        b->data = p;
        b->capacity = cap;
    }
}

void buffer_shrink(buffer *b, size_t max_cap) {
    (void)max_cap;
    /* Shrink to fit, but no smaller than 32 bytes */
    size_t new_cap = b->len + 1;
    if (new_cap < 32) new_cap = 32;
    if (new_cap >= b->capacity) return;
    char *p = (char *)realloc(b->data, new_cap);
    if (p) {
        b->data = p;
        b->capacity = new_cap;
    }
}

void buffer_erase(buffer *b, size_t pos, size_t n) {
    if (pos >= b->len || n == 0) return;
    if (pos + n > b->len) n = b->len - pos;
    memmove(b->data + pos, b->data + pos + n, b->len - pos - n);
    b->len -= n;
    b->data[b->len] = '\0';
}

const char *buffer_data(const buffer *b) { return b->data ? b->data : ""; }
size_t      buffer_len(const buffer *b)      { return b->len; }
size_t      buffer_capacity(const buffer *b)  { return b->capacity; }
bool        buffer_empty(const buffer *b)     { return b->len == 0; }
