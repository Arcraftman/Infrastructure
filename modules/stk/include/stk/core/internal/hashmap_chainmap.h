#ifndef STK_CORE_INTERNAL_HASHMAP_CHAINMAP_H
#define STK_CORE_INTERNAL_HASHMAP_CHAINMAP_H

#include "hashmap_allocator.h"
#include "hash.h"
#include <stddef.h>

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
    size_t bucket_count;
    size_t count;               /* 已存元素数 */
    stk_hash_fn hash;
    stk_eq_fn   eq;
    const stk_allocator *alloc;
} stk_chainmap;

/* 生命周期 */
void stk_i_chainmap_init(stk_chainmap *m, size_t bucket_count,
                         stk_hash_fn hash, stk_eq_fn eq,
                         const stk_allocator *alloc);
void stk_i_chainmap_free(stk_chainmap *m);

/* 核心操作 */
int  stk_i_chainmap_set(stk_chainmap *m, void *key, void *value); /* 0=新插 1=覆盖 */
void *stk_i_chainmap_get(const stk_chainmap *m, const void *key);  /* NULL=未找到 */
int  stk_i_chainmap_remove(stk_chainmap *m, const void *key);     /* 0=没删到 1=删了 */
void stk_i_chainmap_clear(stk_chainmap *m);

/* 扩容：分配 new_bucket_count 个新桶，原地复用旧节点 rehash 到新桶
 * （节点不 free，只改 next 指针与桶头），最后 free 旧桶数组。
 * new_bucket_count 必须 > 当前 bucket_count，否则返回 0（无操作）。 */
int stk_i_chainmap_reserve(stk_chainmap *m, size_t new_bucket_count);

/* 迭代器：O(1) 前进；remove 通过委托 stk_i_chainmap_remove 安全删除当前项。
 * 用法：
 *   stk_chain_iter it; stk_i_chain_iter_begin(&it, &m);
 *   while (stk_i_chain_iter_next(&it)) {
 *       void *k = stk_i_chain_iter_key(&it);
 *       void *v = stk_i_chain_iter_value(&it);
 *       // 可选：stk_i_chain_iter_remove(&it); // 删当前项，仍可继续 next
 *   }
 */
typedef struct stk_chain_iter {
    const stk_chainmap *map;
    size_t bucket;
    stk_chain_node *cur;   /* 当前返回节点 */
    stk_chain_node *next;  /* 预取的下一个节点（避免删除后悬空） */
} stk_chain_iter;

void  stk_i_chain_iter_begin(stk_chain_iter *it, const stk_chainmap *m);
int   stk_i_chain_iter_next(stk_chain_iter *it); /* 0=结束 1=有元素 */
void *stk_i_chain_iter_key(const stk_chain_iter *it);
void *stk_i_chain_iter_value(const stk_chain_iter *it);
/* 删除当前项：委托 stk_i_chainmap_remove，迭代器内部已 pre-fetch next，可继续。 */
int   stk_i_chain_iter_remove(stk_chain_iter *it);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASHMAP_CHAINMAP_H */
