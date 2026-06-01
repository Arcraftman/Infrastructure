#ifndef STK_CORE_BUFFER_H
#define STK_CORE_BUFFER_H

#include "stk/core/preset.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Auto-growing byte buffer.
 *
 * stk_buffer is a contiguous, dynamically-growing chunk of memory that
 * supports both raw-byte and formatted-string append operations.
 * It is a building-block for serialization, I/O buffering, and
 * string construction.
 *
 * Basic usage:
 * @code
 *   stk_buffer b;
 *   stk_buffer_init(&b);
 *   stk_buffer_append(&b, "hello ", 6);
 *   stk_buffer_printf(&b, "world %d", 42);
 *   printf("%s", stk_buffer_data(&b));   // "hello world 42"
 *   stk_buffer_free(&b);
 * @code
 */

typedef struct {
    char   *data;        /* underlying byte array (always NUL-terminated) */
    size_t  len;         /* number of bytes written (excluding NUL) */
    size_t  capacity;    /* allocated capacity */
} stk_buffer;

/* Lifetime ------------------------------------------------------------- */

STK_API void    stk_buffer_init(stk_buffer *b);
STK_API void    stk_buffer_init_with_capacity(stk_buffer *b, size_t cap);
STK_API void    stk_buffer_free(stk_buffer *b);

/* Write operations ----------------------------------------------------- */

STK_API void    stk_buffer_append(stk_buffer *b, const void *src, size_t n);
STK_API void    stk_buffer_append_cstr(stk_buffer *b, const char *cstr);
STK_API void    stk_buffer_append_char(stk_buffer *b, char ch);
STK_API void    stk_buffer_append_int(stk_buffer *b, int val);
STK_API void    stk_buffer_append_uint(stk_buffer *b, unsigned int val);
STK_API void    stk_buffer_append_hex(stk_buffer *b, const void *data, size_t n);
STK_API void    stk_buffer_printf(stk_buffer *b, const char *fmt, ...);
STK_API void    stk_buffer_vprintf(stk_buffer *b, const char *fmt, va_list ap);

/* Insert / replace ----------------------------------------------------- */

STK_API void    stk_buffer_insert(stk_buffer *b, size_t pos, const void *src, size_t n);
STK_API void    stk_buffer_overwrite(stk_buffer *b, size_t pos, const void *src, size_t n);

/* Management ----------------------------------------------------------- */

STK_API void    stk_buffer_clear(stk_buffer *b);
STK_API void    stk_buffer_reserve(stk_buffer *b, size_t cap);
STK_API void    stk_buffer_shrink(stk_buffer *b, size_t max_cap);
STK_API void    stk_buffer_erase(stk_buffer *b, size_t pos, size_t n);

/* Accessors ------------------------------------------------------------ */

STK_API const char *stk_buffer_data(const stk_buffer *b);
STK_API size_t      stk_buffer_len(const stk_buffer *b);
STK_API size_t      stk_buffer_capacity(const stk_buffer *b);
STK_API bool        stk_buffer_empty(const stk_buffer *b);

#ifdef __cplusplus
}
#endif

#endif /* STK_BUFFER_H */