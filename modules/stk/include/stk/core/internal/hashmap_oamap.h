#ifndef STK_CORE_INTERNAL_HASHMAP_OAMAP_H
#define STK_CORE_INTERNAL_HASHMAP_OAMAP_H

#include "hashmap_allocator.h"
#include "hash.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 开放寻址：三态槽位。OA_EMPTY 必须为 0，以便 calloc 后天然为空。 */
typedef enum stk_oa_state {
    OA_EMPTY = 0,
    OA_OCCUPIED = 1,
    OA_TOMB = 2
} stk_oa_state;

typedef struct stk_oa_slot {
    void *key;
    void *value;
    stk_oa_state state;
} stk_oa_slot;

typedef struct stk_oamap {
    stk_oa_slot *slots;
    size_t slot_count;
    size_t count;          /* 已存元素（不含墓碑） */
    size_t tombstones;     /* 墓碑数 */
    stk_hash_fn hash;
    stk_eq_fn   eq;
    const stk_allocator *alloc;
} stk_oamap;

/* 生命周期 */
void stk_i_oamap_init(stk_oamap *m, size_t slot_count,
                      stk_hash_fn hash, stk_eq_fn eq,
                      const stk_allocator *alloc);
void stk_i_oamap_free(stk_oamap *m);

/* 核心操作 */
int  stk_i_oamap_set(stk_oamap *m, void *key, void *value); /* 0=新插 1=覆盖 */
void *stk_i_oamap_get(const stk_oamap *m, const void *key);  /* NULL=未找到 */
int  stk_i_oamap_remove(stk_oamap *m, const void *key);     /* 0=没删到 1=删了 */
void stk_i_oamap_clear(stk_oamap *m);

/* 扩容：分配 new_slot_count 个新 slots（全 EMPTY），把所有 OCCUPIED 重新线性探测
 * 插入新表（墓碑不搬）。完成后整体替换 slots 并 free 旧表。
 * new_slot_count 必须足够容纳当前所有元素（含预留），否则返回 0（无操作）。 */
int stk_i_oamap_reserve(stk_oamap *m, size_t new_slot_count);

/* 迭代器：线性扫描 slots，跳过 EMPTY/TOMB。
 * 用法同 chainmap：begin / next / key / value / remove。
 * remove 当前项会把该槽置 TOMB，不影响后续扫描游标（游标已前进）。 */
typedef struct stk_oa_iter {
    const stk_oamap *map;
    size_t slot;       /* 下一个待检查槽 */
    size_t current;    /* 当前返回槽（-1 表示无） */
} stk_oa_iter;

void  stk_i_oa_iter_begin(stk_oa_iter *it, const stk_oamap *m);
int   stk_i_oa_iter_next(stk_oa_iter *it); /* 0=结束 1=有元素 */
void *stk_i_oa_iter_key(const stk_oa_iter *it);
void *stk_i_oa_iter_value(const stk_oa_iter *it);
int   stk_i_oa_iter_remove(stk_oa_iter *it);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASHMAP_OAMAP_H */
