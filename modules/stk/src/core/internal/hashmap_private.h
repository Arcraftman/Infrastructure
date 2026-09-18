#ifndef STK_CORE_INTERNAL_HASHMAP_PRIVATE_H
#define STK_CORE_INTERNAL_HASHMAP_PRIVATE_H

#include "stk/core/hashmap.h"
#include "hashmap_chainmap.h"
#include "hashmap_oamap.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 公开句柄 stk_hashmap 的真身：按创建时的 mode 二选一，两套后端各自独立。 */
struct stk_hashmap {
    stk_hashmap_mode mode;
    uint32_t version;        /* 每次写操作 ++，迭代器据此做失败快检 */
    const stk_allocator *alloc;  /* 用于释放 stk_hashmap 自身（NULL=默认） */
    stk_destroy_fn key_free; /* 非 NULL 时表拥有 key 生命周期 */
    stk_destroy_fn val_free; /* 非 NULL 时表拥有 value 生命周期 */
    union {
        stk_chainmap chain;
        stk_oamap    oa;
    } u;
};

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASHMAP_PRIVATE_H */
