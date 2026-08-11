#ifndef STK_CORE_INTERNAL_HASHMAP_ALLOCATOR_H
#define STK_CORE_INTERNAL_HASHMAP_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 分配器契约：所有集合共用，不依赖具体后端。
 * alloc 返回 NULL 表示分配失败（调用方决定如何处理，本库不 abort）。
 * free 对 NULL 必须安全（noop）。 */
typedef struct stk_allocator {
    void *(*alloc)(void *ctx, size_t size);
    void  (*free)(void *ctx, void *ptr);
    void  *ctx;
} stk_allocator;

/* 默认分配器：直接 malloc/free，ctx = NULL */
extern const stk_allocator STK_ALLOCATOR_DEFAULT;

/* 内部 helper：通过分配器分配/释放。alloc 失败返回 NULL。 */
void *stk_i_alloc(const stk_allocator *a, size_t size);
void  stk_i_free(const stk_allocator *a, void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASHMAP_ALLOCATOR_H */
