#ifndef STK_CORE_POOL_H
#define STK_CORE_POOL_H

#include "stk/core/preset.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fixed-size object pool (slab allocator).
 *
 * The pool pre-allocates "slabs" of fixed-size elements and hands out
 * individual elements on demand.  Freeing returns the element to the
 * pool for reuse — no system allocator call is made.  This provides
 * O(1) alloc/free with excellent cache locality.
 *
 * All elements are aligned to sizeof(void*).
 *
 * Basic usage:
 * @code
 *   stk_pool p;
 *   stk_pool_init(&p, sizeof(my_struct), 64);
 *   my_struct *s = (my_struct *)stk_pool_alloc(&p);
 *   // ... use s ...
 *   stk_pool_free(&p, s);
 *   stk_pool_destroy(&p);
 * @endcode
 */

typedef struct stk_pool_slab {
    struct stk_pool_slab *next;
} stk_pool_slab;

typedef struct stk_pool_free_node {
    struct stk_pool_free_node *next;
} stk_pool_free_node;

typedef struct {
    size_t              element_size;   /* per-element size (padded to alignment) */
    size_t              slab_capacity;  /* elements per slab */
    stk_pool_slab      *slabs;          /* linked list of all slabs */
    stk_pool_free_node *freelist;       /* singly-linked freelist of freed nodes */
    size_t              allocated;      /* number of elements currently in use */
    size_t              total;          /* total elements available */
} stk_pool;

/* Lifetime ------------------------------------------------------------- */

STK_API void   stk_pool_init(stk_pool *p, size_t element_size, size_t slab_capacity);
STK_API void   stk_pool_destroy(stk_pool *p);

/* Allocation / deallocation -------------------------------------------- */

STK_API void  *stk_pool_alloc(stk_pool *p);
STK_API void   stk_pool_free(stk_pool *p, void *element);

/* Introspection -------------------------------------------------------- */

STK_API size_t stk_pool_element_size(const stk_pool *p);
STK_API size_t stk_pool_allocated(const stk_pool *p);
STK_API size_t stk_pool_available(const stk_pool *p);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_POOL_H */