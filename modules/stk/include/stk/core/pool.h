#ifndef STK_POOL_H
#define STK_POOL_H

#include "stk/def.h"
#include <stddef.h>

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
 *   pool p;
 *   pool_init(&p, sizeof(my_struct), 64);
 *   my_struct *s = (my_struct *)pool_alloc(&p);
 *   // ... use s ...
 *   pool_free(&p, s);
 *   pool_destroy(&p);
 * @endcode
 */

typedef struct pool_slab {
    struct pool_slab *next;
} pool_slab;

typedef struct pool_free_node {
    struct pool_free_node *next;
} pool_free_node;

typedef struct {
    size_t        element_size;   /* per-element size (padded to alignment) */
    size_t        slab_capacity;  /* elements per slab */
    pool_slab    *slabs;          /* linked list of all slabs */
    pool_free_node *freelist;     /* singly-linked freelist of freed nodes */
    size_t        allocated;      /* number of elements currently in use */
    size_t        total;          /* total elements available */
} pool;

/* Lifetime ------------------------------------------------------------- */

STK_API void   pool_init(pool *p, size_t element_size, size_t slab_capacity);
STK_API void   pool_destroy(pool *p);

/* Allocation / deallocation -------------------------------------------- */

STK_API void  *pool_alloc(pool *p);
STK_API void   pool_free(pool *p, void *element);

/* Introspection -------------------------------------------------------- */

STK_API size_t pool_element_size(const pool *p);
STK_API size_t pool_allocated(const pool *p);
STK_API size_t pool_available(const pool *p);

#ifdef __cplusplus
}
#endif

#endif /* STK_POOL_H */
