#ifndef STK_CORE_INTERNAL_HASHMAP_OAMAP_H
#define STK_CORE_INTERNAL_HASHMAP_OAMAP_H

#include <stddef.h>
#include "stk/core/hash.h"
#include "stk/core/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 开放寻址：三态槽位。STK_OA_EMPTY 必须为 0，以便 calloc 后天然为空。 */
typedef enum stk_oa_state {
    STK_OA_EMPTY = 0,
    STK_OA_OCCUPIED = 1,
    STK_OA_TOMB = 2
} stk_oa_state;

typedef struct stk_oa_slot {
    void *key;
    void *value;
    stk_oa_state state;
} stk_oa_slot;

typedef struct stk_oamap {
    stk_oa_slot *slots;
    size_t slot_count;   /* 2 的幂 */
    size_t size;         /* 已存（OCCUPIED）元素数 */
    size_t used;         /* OCCUPIED + TOMB，扩容阈值据此计算 */
    stk_hash_fn hash;
    stk_eq_fn   eq;
    const stk_allocator *alloc; /* 用于槽数组分配（NULL=默认） */
} stk_oamap;

/* 生命周期 */
void  stk_i_oamap_init(stk_oamap *m, size_t capacity, stk_hash_fn hash, stk_eq_fn eq, const stk_allocator *alloc);
void  stk_i_oamap_free(stk_oamap *m);

/* 核心操作。out_key / out_value 可取回被替换 / 删除的原始指针（拥有模式下用于释放）。
 * set 返回：0=新插 1=覆盖 -1=分配失败。remove 返回：0=没删到 1=删了。 */
int   stk_i_oamap_set(stk_oamap *m, void *key, void *value, void **out_old);
void *stk_i_oamap_get(const stk_oamap *m, const void *key);  /* NULL=未找到 */
int   stk_i_oamap_remove(stk_oamap *m, const void *key, void **out_key, void **out_value);
void  stk_i_oamap_clear(stk_oamap *m);
size_t stk_i_oamap_size(const stk_oamap *m);

/* 扩容：分配 new_slot_count(向上取 2 的幂) 个全新槽，把所有 OCCUPIED 重新线性探测
 * 插入（墓碑不搬，借此顺带清理墓碑），再 free 旧表。1=成功 0=失败。 */
int   stk_i_oamap_reserve(stk_oamap *m, size_t new_slot_count);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASHMAP_OAMAP_H */
