#ifndef STK_CORE_INTERNAL_HASHMAP_CHAINMAP_H
#define STK_CORE_INTERNAL_HASHMAP_CHAINMAP_H

#include <stddef.h>
#include "stk/core/hash.h"
#include "stk/core/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 链地址法：节点完全独立，不与其他后端共享 entry 结构。 */
typedef struct stk_chain_node {
    void *key;
    void *value;
    struct stk_chain_node *next;
} stk_chain_node;

typedef struct stk_chainmap {
    stk_chain_node **buckets;   /* bucket_count 个指针，NULL = 空桶 */
    size_t bucket_count;        /* 2 的幂 */
    size_t size;                /* 元素数 */
    stk_hash_fn hash;
    stk_eq_fn   eq;
    const stk_allocator *alloc; /* 用于节点 / 桶数组分配（NULL=默认） */
} stk_chainmap;

/* 生命周期 */
void  stk_i_chainmap_init(stk_chainmap *m, size_t capacity, stk_hash_fn hash, stk_eq_fn eq, const stk_allocator *alloc);
void  stk_i_chainmap_free(stk_chainmap *m);

/* 核心操作。out_key / out_value 可取回被替换 / 删除的原始指针（拥有模式下用于释放）。
 * set 返回：0=新插 1=覆盖 -1=分配失败。remove 返回：0=没删到 1=删了。 */
int   stk_i_chainmap_set(stk_chainmap *m, void *key, void *value, void **out_old);
void *stk_i_chainmap_get(const stk_chainmap *m, const void *key);  /* NULL=未找到 */
int   stk_i_chainmap_remove(stk_chainmap *m, const void *key, void **out_key, void **out_value);
void  stk_i_chainmap_clear(stk_chainmap *m);
size_t stk_i_chainmap_size(const stk_chainmap *m);

/* 扩容：分配 new_bucket_count(向上取 2 的幂) 个新桶，原地复用旧节点 rehash
 * （节点不 free，只改 next 指针与桶头），最后 free 旧桶数组。1=成功 0=失败。 */
int   stk_i_chainmap_reserve(stk_chainmap *m, size_t new_bucket_count);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASHMAP_CHAINMAP_H */
