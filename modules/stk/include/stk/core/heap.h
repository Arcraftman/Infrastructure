#ifndef STK_CORE_HEAP_H
#define STK_CORE_HEAP_H
#include "stk/core/preset.h"

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
 *   stk_heap h;
 *   stk_heap_init(&h, my_compare);
 *   stk_heap_push(&h, ptr);
 *   void *min = stk_heap_pop(&h);
 *   stk_heap_free(&h);
 * @endcode
 */

typedef int (*stk_heap_compare_fn)(const void *a, const void *b);

typedef struct {
    void            **data;       /* array of void* in heap order */
    size_t            size;
    size_t            capacity;
    stk_heap_compare_fn cmp;        /* min-heap: cmp(a,b) <= 0 means a <= b */
} stk_heap;

/* Lifetime ------------------------------------------------------------- */

STK_API void   stk_heap_init(stk_heap *h, stk_heap_compare_fn cmp);
STK_API void   stk_heap_init_with_capacity(stk_heap *h, stk_heap_compare_fn cmp,
                                            size_t cap);
STK_API void   stk_heap_free(stk_heap *h);

/* Core operations ------------------------------------------------------ */

STK_API void   stk_heap_push(stk_heap *h, void *val);
STK_API void  *stk_heap_pop(stk_heap *h);
STK_API void  *stk_heap_top(const stk_heap *h);
STK_API void   stk_heap_remove(stk_heap *h, size_t idx);

/* Introspection -------------------------------------------------------- */

STK_API bool   stk_heap_empty(const stk_heap *h);
STK_API size_t stk_heap_size(const stk_heap *h);
STK_API size_t stk_heap_capacity(const stk_heap *h);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_HEAP_H */