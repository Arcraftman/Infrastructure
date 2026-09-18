#ifndef STK_CORE_HASHMAP_H
#define STK_CORE_HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include "stk/core/hash.h"
#include "stk/core/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 两种后端：仅作为创建参数。两套底层各自独立，顶层函数按 mode 分发。 */
typedef enum stk_hashmap_mode {
    STK_HASHMAP_CHAIN = 0,   /* 链地址法：节点独立，缓存命中一般 */
    STK_HASHMAP_OA    = 1    /* 开放寻址：并行槽位，缓存友好（推荐默认） */
} stk_hashmap_mode;

/* 公开句柄：完全不透明，真身在 hashmap_private.h。 */
typedef struct stk_hashmap stk_hashmap;

/* 析构回调：表以"拥有"模式创建时，删除 / 销毁元素会调用它释放 key / value。
 * 传 NULL = 非拥有，调用者自行负责 key / value 的生命周期。 */
typedef void (*stk_destroy_fn)(void *ptr);

/* 创建（非拥有模式）：capacity 为期望容量（实际取 >=capacity 的最小 2 的幂）。
 * 表不负责释放 key / value。hash / eq 必须提供，否则返回 NULL。
 * alloc 传 NULL = 使用默认分配器（STK_MALLOC 等宏，可编译期覆盖）。 */
stk_hashmap *stk_hashmap_create(stk_hashmap_mode mode, size_t capacity,
                                stk_hash_fn hash, stk_eq_fn eq,
                                const stk_allocator *alloc);

/* 创建（拥有模式）：提供 key / value 析构回调，表在删除条目与销毁时负责释放。
 * 任一回调传 NULL 则该侧按非拥有处理。alloc 传 NULL = 默认分配器。 */
stk_hashmap *stk_hashmap_create_full(stk_hashmap_mode mode, size_t capacity,
                                     stk_hash_fn hash, stk_eq_fn eq,
                                     stk_destroy_fn key_free,
                                     stk_destroy_fn val_free,
                                     const stk_allocator *alloc);

void stk_hashmap_destroy(stk_hashmap *m);

/* 返回 0=新插入；1=覆盖了已有 key 的值；-1=分配失败。 */
int  stk_hashmap_set(stk_hashmap *m, void *key, void *value);
void *stk_hashmap_get(const stk_hashmap *m, const void *key); /* NULL=未找到 */
int  stk_hashmap_remove(stk_hashmap *m, const void *key);     /* 0=没删到 1=删了 */
int  stk_hashmap_contains(const stk_hashmap *m, const void *key);
size_t stk_hashmap_size(const stk_hashmap *m);
void stk_hashmap_clear(stk_hashmap *m);
int  stk_hashmap_reserve(stk_hashmap *m, size_t new_capacity); /* 1=成功 0=失败 */

/* 迭代器：扁平栈对象，调用方无需 destroy（不持有自有资源）。
 *
 * 失败快检（仿 GLib GHashTableIter）：表每次写操作都会让 version++，
 * 迭代器在 begin 时记录 version 快照；若迭代中途表被增 / 删修改，
 * next() 检测到 version 不符会立即停止——专门防止"迭代中改表导致静默损坏"。
 * 因此：迭代期间不要用哈希表自身的 set/remove/clear，除非通过 iter_remove。 */
struct stk_hashmap_iter {
    const stk_hashmap *map;
    size_t            idx;     /* chain: 当前桶号 / oa: 下一个待查槽 */
    const void       *node;    /* chain: 当前节点（不透明指针） */
    const void       *prev;    /* chain: 当前节点前驱（O(1) 删除用） */
    void             *key;     /* 缓存的当前键 */
    void             *value;   /* 缓存的当前值 */
    uint32_t         version; /* begin 时记录的表 version 快照 */
};
typedef struct stk_hashmap_iter stk_hashmap_iter;

void  stk_hashmap_iter_begin(stk_hashmap_iter *it, const stk_hashmap *m);
int   stk_hashmap_iter_next(stk_hashmap_iter *it);   /* 0=结束 1=有元素 */
void *stk_hashmap_iter_key(const stk_hashmap_iter *it);
void *stk_hashmap_iter_value(const stk_hashmap_iter *it);
int   stk_hashmap_iter_remove(stk_hashmap_iter *it); /* 删当前项，仍可继续 next；0/1 */

/* 供 iterator.h 的 stk_foreach 使用：把具体迭代器的强类型操作，
 * 以 void* 形态暴露给通用遍历（无 _Generic、无 vtable）。 */
static inline void stk_hashmap_iter_begin_op(void *it, const void *c) {
    stk_hashmap_iter_begin((stk_hashmap_iter *)it, (const stk_hashmap *)c);
}
static inline int stk_hashmap_iter_next_op(void *it) {
    return stk_hashmap_iter_next((stk_hashmap_iter *)it);
}
static inline void *stk_hashmap_iter_key_op(void *it) {
    return stk_hashmap_iter_key((const stk_hashmap_iter *)it);
}
static inline void *stk_hashmap_iter_value_op(void *it) {
    return stk_hashmap_iter_value((const stk_hashmap_iter *)it);
}

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_HASHMAP_H */
