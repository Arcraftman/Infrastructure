#ifndef STK_BUFFER_H
#define STK_BUFFER_H

#include "stk/def.h"
#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Auto-growing byte buffer.
 *
 * buffer is a contiguous, dynamically-growing chunk of memory that
 * supports both raw-byte and formatted-string append operations.
 * It is a building-block for serialization, I/O buffering, and
 * string construction.
 *
 * Basic usage:
 * @code
 *   buffer b;
 *   buffer_init(&b);
 *   buffer_append(&b, "hello ", 6);
 *   buffer_printf(&b, "world %d", 42);
 *   printf("%s", buffer_data(&b));   // "hello world 42"
 *   buffer_free(&b);
 * @code
 */

typedef struct {
    char   *data;        /* underlying byte array (always NUL-terminated) */
    size_t  len;         /* number of bytes written (excluding NUL) */
    size_t  capacity;    /* allocated capacity */
} buffer;

/* Lifetime ------------------------------------------------------------- */

STK_API void    buffer_init(buffer *b);
STK_API void    buffer_init_with_capacity(buffer *b, size_t cap);
STK_API void    buffer_free(buffer *b);

/* Write operations ----------------------------------------------------- */

STK_API void    buffer_append(buffer *b, const void *src, size_t n);
STK_API void    buffer_append_cstr(buffer *b, const char *cstr);
STK_API void    buffer_append_char(buffer *b, char ch);
STK_API void    buffer_append_int(buffer *b, int val);
STK_API void    buffer_append_uint(buffer *b, unsigned int val);
STK_API void    buffer_append_hex(buffer *b, const void *data, size_t n);
STK_API void    buffer_printf(buffer *b, const char *fmt, ...);
STK_API void    buffer_vprintf(buffer *b, const char *fmt, va_list ap);

/* Insert / replace ----------------------------------------------------- */

STK_API void    buffer_insert(buffer *b, size_t pos, const void *src, size_t n);
STK_API void    buffer_overwrite(buffer *b, size_t pos, const void *src, size_t n);

/* Management ----------------------------------------------------------- */

STK_API void    buffer_clear(buffer *b);
STK_API void    buffer_reserve(buffer *b, size_t cap);
STK_API void    buffer_shrink(buffer *b, size_t max_cap);
STK_API void    buffer_erase(buffer *b, size_t pos, size_t n);

/* Accessors ------------------------------------------------------------ */

STK_API const char *buffer_data(const buffer *b);
STK_API size_t      buffer_len(const buffer *b);
STK_API size_t      buffer_capacity(const buffer *b);
STK_API bool        buffer_empty(const buffer *b);

#ifdef __cplusplus
}
#endif

#endif /* STK_BUFFER_H */
