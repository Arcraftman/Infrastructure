#ifndef STK_CORE_ARENA_H
#define STK_CORE_ARENA_H



#ifdef __cplusplus
extern "C" {
#endif

/* 结构体定义必须在函数声明之前 */
typedef struct stk_arena_block {
    struct stk_arena_block *next;
    size_t                  used;
    size_t                  capacity;
} stk_arena_block;

typedef struct stk_arena {
    stk_arena_block  *head;
    stk_arena_block  *current;
    size_t            block_size;
    size_t            total_allocated;
    void            *(*malloc_fn)(size_t);
    void             (*free_fn)(void *);
} stk_arena;

/* Lifetime */
STK_API STK_STATUS stk_arena_init(stk_arena *a, size_t block_size);
STK_API STK_STATUS stk_arena_init_custom(stk_arena *a, size_t block_size,
                                          void *(*malloc_fn)(size_t),
                                          void (*free_fn)(void *));
STK_API void       stk_arena_free(stk_arena *a);

/* Allocation */
STK_API void      *stk_arena_alloc(stk_arena *a, size_t size);
STK_API void      *stk_arena_alloc_aligned(stk_arena *a, size_t size, size_t align);
STK_API char      *stk_arena_dup_str(stk_arena *a, const char *s);
STK_API char      *stk_arena_dup_strn(stk_arena *a, const char *s, size_t n);

/* Management */
STK_API void       stk_arena_reset(stk_arena *a);
STK_API size_t     stk_arena_total_allocated(const stk_arena *a);
STK_API size_t     stk_arena_block_count(const stk_arena *a);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_ARENA_H */