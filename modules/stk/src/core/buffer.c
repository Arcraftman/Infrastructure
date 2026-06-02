#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/buffer.h"

static bool ensure(stk_buffer *b, size_t extra) {
    size_t needed = b->len + extra + 1;
    if (needed <= b->capacity) return true;
    size_t new_cap = b->capacity ? b->capacity : 32;
    while (new_cap < needed) new_cap *= 2;
    char *p = (char *)realloc(b->data, new_cap);
    if (!p) {
        STK_LOG_ERROR("Buffer ensure: realloc failed (size=%zu)", new_cap);
        return false;
    }
    b->data = p;
    b->capacity = new_cap;
    STK_LOG_DEBUG("Buffer ensure: grew to %zu bytes", new_cap);
    return true;
}

STK_STATUS stk_buffer_init(stk_buffer *b) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer init: NULL buffer pointer");
    b->data = NULL;
    b->len = 0;
    b->capacity = 0;
    STK_LOG_DEBUG("Buffer initialized");
    return STK_OK;
}

STK_STATUS stk_buffer_init_with_capacity(stk_buffer *b, size_t cap) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer init: NULL buffer pointer");
    
    b->data = (char *)malloc(cap);
    if (!b->data && cap > 0) {
        STK_LOG_ERROR("Buffer init: failed to allocate %zu bytes", cap);
        return STK_ENOMEM;
    }
    b->len = 0;
    b->capacity = b->data ? cap : 0;
    if (b->data) b->data[0] = '\0';
    
    STK_LOG_DEBUG("Buffer init: capacity=%zu", cap);
    return STK_OK;
}

STK_STATUS stk_buffer_free(stk_buffer *b) {
    if (!b) {
        STK_LOG_WARN("Buffer free: NULL buffer pointer");
        return STK_EINVAL;
    }
    free(b->data);
    b->data = NULL;
    b->len = b->capacity = 0;
    STK_LOG_DEBUG("Buffer freed");
    return STK_OK;
}

STK_STATUS stk_buffer_append(stk_buffer *b, const void *src, size_t n) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer append: NULL buffer pointer");
    STK_RETURN_IF(!src && n > 0, STK_EINVAL, "Buffer append: NULL source with n>0");
    
    if (n == 0) return STK_OK;
    
    if (!ensure(b, n)) {
        STK_LOG_ERROR("Buffer append: failed to ensure space for %zu bytes", n);
        return STK_ENOMEM;
    }
    memcpy(b->data + b->len, src, n);
    b->len += n;
    b->data[b->len] = '\0';
    
    STK_LOG_DEBUG("Buffer append: added %zu bytes, new len=%zu", n, b->len);
    return STK_OK;
}

STK_STATUS stk_buffer_append_cstr(stk_buffer *b, const char *cstr) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer append_cstr: NULL buffer pointer");
    STK_RETURN_IF(!cstr, STK_EINVAL, "Buffer append_cstr: NULL string");
    
    return stk_buffer_append(b, cstr, strlen(cstr));
}

STK_STATUS stk_buffer_append_char(stk_buffer *b, char ch) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer append_char: NULL buffer pointer");
    
    if (!ensure(b, 1)) {
        STK_LOG_ERROR("Buffer append_char: failed to ensure space");
        return STK_ENOMEM;
    }
    b->data[b->len++] = ch;
    b->data[b->len] = '\0';
    
    STK_LOG_DEBUG("Buffer append_char: '%c'", ch);
    return STK_OK;
}

STK_STATUS stk_buffer_append_int(stk_buffer *b, int val) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%d", val);
    return stk_buffer_append(b, tmp, (size_t)n);
}

STK_STATUS stk_buffer_append_uint(stk_buffer *b, unsigned int val) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%u", val);
    return stk_buffer_append(b, tmp, (size_t)n);
}

STK_STATUS stk_buffer_append_hex(stk_buffer *b, const void *data, size_t n) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer append_hex: NULL buffer pointer");
    STK_RETURN_IF(!data && n > 0, STK_EINVAL, "Buffer append_hex: NULL data with n>0");
    
    static const char hex[] = "0123456789abcdef";
    size_t needed = n * 2;
    if (!ensure(b, needed)) {
        STK_LOG_ERROR("Buffer append_hex: failed to ensure space for %zu bytes", needed);
        return STK_ENOMEM;
    }
    const unsigned char *src = (const unsigned char *)data;
    for (size_t i = 0; i < n; i++) {
        b->data[b->len++] = hex[src[i] >> 4];
        b->data[b->len++] = hex[src[i] & 0x0f];
    }
    b->data[b->len] = '\0';
    
    STK_LOG_DEBUG("Buffer append_hex: %zu bytes encoded to hex", n);
    return STK_OK;
}

STK_STATUS stk_buffer_printf(stk_buffer *b, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    STK_STATUS rc = stk_buffer_vprintf(b, fmt, ap);
    va_end(ap);
    return rc;
}

STK_STATUS stk_buffer_vprintf(stk_buffer *b, const char *fmt, va_list ap) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer vprintf: NULL buffer pointer");
    STK_RETURN_IF(!fmt, STK_EINVAL, "Buffer vprintf: NULL format string");
    
    va_list copy;
    va_copy(copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    if (needed < 0) {
        STK_LOG_ERROR("Buffer vprintf: vsnprintf failed");
        va_end(copy);
        return STK_ERROR;
    }
    size_t n = (size_t)needed;
    if (!ensure(b, n)) {
        STK_LOG_ERROR("Buffer vprintf: failed to ensure space for %zu bytes", n);
        va_end(copy);
        return STK_ENOMEM;
    }
    vsnprintf(b->data + b->len, b->capacity - b->len, fmt, copy);
    b->len += n;
    va_end(copy);
    
    STK_LOG_DEBUG("Buffer vprintf: added %zu bytes", n);
    return STK_OK;
}

STK_STATUS stk_buffer_insert(stk_buffer *b, size_t pos, const void *src, size_t n) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer insert: NULL buffer pointer");
    STK_RETURN_IF(!src && n > 0, STK_EINVAL, "Buffer insert: NULL source with n>0");
    
    if (pos > b->len) pos = b->len;
    if (n == 0) return STK_OK;
    
    if (!ensure(b, n)) {
        STK_LOG_ERROR("Buffer insert: failed to ensure space for %zu bytes", n);
        return STK_ENOMEM;
    }
    memmove(b->data + pos + n, b->data + pos, b->len - pos);
    memcpy(b->data + pos, src, n);
    b->len += n;
    b->data[b->len] = '\0';
    
    STK_LOG_DEBUG("Buffer insert: inserted %zu bytes at pos %zu", n, pos);
    return STK_OK;
}

STK_STATUS stk_buffer_overwrite(stk_buffer *b, size_t pos, const void *src, size_t n) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer overwrite: NULL buffer pointer");
    STK_RETURN_IF(!src && n > 0, STK_EINVAL, "Buffer overwrite: NULL source with n>0");
    
    if (pos > b->len) pos = b->len;
    size_t end = pos + n;
    if (end > b->len) {
        if (!ensure(b, end - b->len)) {
            STK_LOG_ERROR("Buffer overwrite: failed to ensure space");
            return STK_ENOMEM;
        }
        b->len = end;
    }
    memcpy(b->data + pos, src, n);
    b->data[b->len] = '\0';
    
    STK_LOG_DEBUG("Buffer overwrite: wrote %zu bytes at pos %zu", n, pos);
    return STK_OK;
}

STK_STATUS stk_buffer_clear(stk_buffer *b) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer clear: NULL buffer pointer");
    b->len = 0;
    if (b->data) b->data[0] = '\0';
    STK_LOG_DEBUG("Buffer cleared");
    return STK_OK;
}

STK_STATUS stk_buffer_reserve(stk_buffer *b, size_t cap) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer reserve: NULL buffer pointer");
    
    if (cap <= b->capacity) return STK_OK;
    char *p = (char *)realloc(b->data, cap);
    if (!p) {
        STK_LOG_ERROR("Buffer reserve: realloc failed (size=%zu)", cap);
        return STK_ENOMEM;
    }
    b->data = p;
    b->capacity = cap;
    
    STK_LOG_DEBUG("Buffer reserve: capacity=%zu", cap);
    return STK_OK;
}

STK_STATUS stk_buffer_shrink(stk_buffer *b, size_t max_cap) {
    (void)max_cap;
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer shrink: NULL buffer pointer");
    
    size_t new_cap = b->len + 1;
    if (new_cap < 32) new_cap = 32;
    if (new_cap >= b->capacity) return STK_OK;
    
    char *p = (char *)realloc(b->data, new_cap);
    if (p) {
        b->data = p;
        b->capacity = new_cap;
        STK_LOG_DEBUG("Buffer shrink: capacity=%zu", new_cap);
    }
    return STK_OK;
}

STK_STATUS stk_buffer_erase(stk_buffer *b, size_t pos, size_t n) {
    STK_RETURN_IF(!b, STK_EINVAL, "Buffer erase: NULL buffer pointer");
    
    if (pos >= b->len || n == 0) return STK_OK;
    if (pos + n > b->len) n = b->len - pos;
    memmove(b->data + pos, b->data + pos + n, b->len - pos - n);
    b->len -= n;
    b->data[b->len] = '\0';
    
    STK_LOG_DEBUG("Buffer erase: erased %zu bytes at pos %zu", n, pos);
    return STK_OK;
}

const char *stk_buffer_data(const stk_buffer *b) { 
    return (b && b->data) ? b->data : ""; 
}
size_t stk_buffer_len(const stk_buffer *b) { 
    return b ? b->len : 0; 
}
size_t stk_buffer_capacity(const stk_buffer *b) { 
    return b ? b->capacity : 0; 
}
bool stk_buffer_empty(const stk_buffer *b) { 
    return b ? b->len == 0 : true; 
}