#include "stk/core/internal/hashmap_allocator.h"

#include <stdlib.h>

static void *default_alloc(void *ctx, size_t size) {
    (void)ctx;
    if (size == 0) return NULL;
    return malloc(size);
}

static void default_free(void *ctx, void *ptr) {
    (void)ctx;
    free(ptr); /* free(NULL) 安全 */
}

const stk_allocator STK_ALLOCATOR_DEFAULT = {
    .alloc = default_alloc,
    .free  = default_free,
    .ctx   = NULL,
};

void *stk_i_alloc(const stk_allocator *a, size_t size) {
    if (a == NULL) a = &STK_ALLOCATOR_DEFAULT;
    return a->alloc(a->ctx, size);
}

void stk_i_free(const stk_allocator *a, void *ptr) {
    if (ptr == NULL) return; /* free(NULL) 安全，提前返回避免空指针解引用 */
    if (a == NULL) a = &STK_ALLOCATOR_DEFAULT;
    a->free(a->ctx, ptr);
}
