#ifndef STK_ARENA_H
#define STK_ARENA_H

#include "stk/def.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Linear arena allocator.
 *
 * An arena (sometimes called a "region" or "bump allocator") allocates
 * memory in large blocks and hands out chunks sequentially.  Freeing an
 * individual allocation is not supported — instead the entire arena is
 * reset or freed at once.  This makes allocation O(1) and gives excellent
 * cache performance for scratch-workloads.
 *
 * Basic usage:
 * @code
 *   arena a;
 *   arena_init(&a, 0);
 *   int *nums = (int *)arena_alloc(&a, 10 * sizeof(int));
 *   char *dup = arena_dup_str(&a, "hello");
 *   arena_reset(&a);   // re-use underlying blocks
 *   arena_free(&a);    // release everything
 * @code
 */

typedef struct arena_block {
    struct arena_block *next;
    size_t              used;
    size_t              capacity;
} arena_block;

typedef struct {
    arena_block  *head;
    arena_block  *current;
    size_t        block_size;
    size_t        total_allocated;
    void        *(*malloc_fn)(size_t);
    void         (*free_fn)(void *);
} arena;

/* Lifetime ------------------------------------------------------------- */

STK_API void   arena_init(arena *a, size_t block_size);
STK_API void   arena_init_custom(arena *a, size_t block_size,
                                  void *(*malloc_fn)(size_t),
                                  void (*free_fn)(void *));
STK_API void   arena_free(arena *a);

/* Allocation ----------------------------------------------------------- */

STK_API void  *arena_alloc(arena *a, size_t size);
STK_API void  *arena_alloc_aligned(arena *a, size_t size, size_t align);
STK_API char  *arena_dup_str(arena *a, const char *s);
STK_API char  *arena_dup_strn(arena *a, const char *s, size_t n);

/* Management ---------------------------------------------------------- */

STK_API void   arena_reset(arena *a);
STK_API size_t arena_total_allocated(const arena *a);
STK_API size_t arena_block_count(const arena *a);

#ifdef __cplusplus
}
#endif

#endif /* STK_ARENA_H */
