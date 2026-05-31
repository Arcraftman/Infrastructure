#ifndef STK_HEAP_H
#define STK_HEAP_H

#include "stk/def.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Binary min-heap (priority queue) on void* data.
 *
 * The heap stores pointers in a dynamic array and maintains the
 * min-heap property: parent <= children (as defined by the user's
 * compare function).
 *
 * Basic usage:
 * @code
 *   heap h;
 *   heap_init(&h, my_compare);
 *   heap_push(&h, ptr);
 *   void *min = heap_pop(&h);
 *   heap_free(&h);
 * @endcode
 */

typedef int (*heap_compare_fn)(const void *a, const void *b);

typedef struct {
    void          **data;       /* array of void* in heap order */
    size_t          size;
    size_t          capacity;
    heap_compare_fn cmp;        /* min-heap: cmp(a,b) <= 0 means a <= b */
} heap;

/* Lifetime ------------------------------------------------------------- */

STK_API void   heap_init(heap *h, heap_compare_fn cmp);
STK_API void   heap_init_with_capacity(heap *h, heap_compare_fn cmp,
                                        size_t cap);
STK_API void   heap_free(heap *h);

/* Core operations ------------------------------------------------------ */

STK_API void   heap_push(heap *h, void *val);
STK_API void  *heap_pop(heap *h);
STK_API void  *heap_top(const heap *h);
STK_API void   heap_remove(heap *h, size_t idx);

/* Introspection -------------------------------------------------------- */

STK_API bool   heap_empty(const heap *h);
STK_API size_t heap_size(const heap *h);
STK_API size_t heap_capacity(const heap *h);

#ifdef __cplusplus
}
#endif

#endif /* STK_HEAP_H */
