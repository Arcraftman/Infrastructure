#ifndef STK_CORE_ARENA_H
#define STK_CORE_ARENA_H

#include "stk/core/preset.h"
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
 *   stk_arena a;
 *   stk_arena_init(&a, 0);
 *   int *nums = (int *)stk_arena_alloc(&a, 10 * sizeof(int));
 *   char *dup = stk_arena_dup_str(&a, "hello");
 *   stk_arena_reset(&a);   // re-use underlying blocks
 *   stk_arena_free(&a);    // release everything
 * @code
 */

typedef struct stk_arena_block {
    struct stk_arena_block *next;
    size_t                  used;
    size_t                  capacity;
} stk_arena_block;

typedef struct {
    stk_arena_block  *head;
    stk_arena_block  *current;
    size_t            block_size;
    size_t            total_allocated;
    void            *(*malloc_fn)(size_t);
    void             (*free_fn)(void *);
} stk_arena;

/* Lifetime ------------------------------------------------------------- */

STK_API void   stk_arena_init(stk_arena *a, size_t block_size);
STK_API void   stk_arena_init_custom(stk_arena *a, size_t block_size,
                                      void *(*malloc_fn)(size_t),
                                      void (*free_fn)(void *));
STK_API void   stk_arena_free(stk_arena *a);

/* Allocation ----------------------------------------------------------- */

STK_API void  *stk_arena_alloc(stk_arena *a, size_t size);
STK_API void  *stk_arena_alloc_aligned(stk_arena *a, size_t size, size_t align);
STK_API char  *stk_arena_dup_str(stk_arena *a, const char *s);
STK_API char  *stk_arena_dup_strn(stk_arena *a, const char *s, size_t n);

/* Management ---------------------------------------------------------- */

STK_API void   stk_arena_reset(stk_arena *a);
STK_API size_t stk_arena_total_allocated(const stk_arena *a);
STK_API size_t stk_arena_block_count(const stk_arena *a);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_ARENA_H */