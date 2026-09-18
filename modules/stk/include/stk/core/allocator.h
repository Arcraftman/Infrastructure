#ifndef STK_CORE_ALLOCATOR_H
#define STK_CORE_ALLOCATOR_H

#include <stdlib.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 分配层：两套互补机制，按需要选用。
 *
 * 1) 编译期宏覆盖（仿 khash / uthash）：默认就是 malloc/free，
 *    想换内存池可在 #include 任意 stk 头【之前】#define：
 *        #define STK_MALLOC(size)   my_pool_alloc(size)
 *        #define STK_FREE(ptr)      my_pool_free(ptr)
 *        #include "stk/core/hashmap.h"
 *
 * 2) 运行时可插拔分配器（仿 libuv / protobuf-c）：把自定义分配器作为
 *    const stk_allocator * 传给容器的 create。传 NULL = 使用上面的默认宏。
 *    适合“整个容器想要用某个内存池”的场景，无需改全局宏。
 */

#ifndef STK_MALLOC
#define STK_MALLOC(size) malloc(size)
#endif
#ifndef STK_FREE
#define STK_FREE(ptr)    free(ptr)
#endif
#ifndef STK_REALLOC
#define STK_REALLOC(ptr, size) realloc(ptr, size)
#endif
#ifndef STK_CALLOC
#define STK_CALLOC(n, size) calloc((n), (size))
#endif

/* 运行时分配器：user 为回调的私有上下文；任一回调为 NULL 时该操作退化为默认宏。
 * 注：调用方须保证 *alloc 在容器生命周期内持续有效（栈上结构体需长活）。 */
typedef struct stk_allocator {
    void *(*alloc)(void *user, size_t size);
    void *(*realloc)(void *user, void *ptr, size_t size);
    void  (*free)(void *user, void *ptr);
    void  *user;
} stk_allocator;

/* 统一分配入口：a 为 NULL（或未提供对应回调）时退化到 STK_* 默认宏。
 * 容器内部一律通过这两个函数分配 / 释放，从而支持自定义内存池。 */
static inline void *stk_alloc(const stk_allocator *a, size_t size) {
    return (a && a->alloc) ? a->alloc(a->user, size) : STK_MALLOC(size);
}
static inline void *stk_realloc(const stk_allocator *a, void *ptr, size_t size) {
    return (a && a->realloc) ? a->realloc(a->user, ptr, size) : STK_REALLOC(ptr, size);
}
static inline void stk_dealloc(const stk_allocator *a, void *ptr) {
    if (!ptr) return;
    if (a && a->free) a->free(a->user, ptr);
    else STK_FREE(ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_ALLOCATOR_H */
