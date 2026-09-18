#ifndef STK_CORE_ITERATOR_H
#define STK_CORE_ITERATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 通用迭代器（轻量、非类型擦除、无 _Generic）
 * ----------------------------------------------------------------------------
 * 每个容器保留自己的【具体迭代器类型】（如 stk_hashmap_iter），逻辑快、清晰、好调试。
 * 本头只做两件事：
 *
 *  1) 约定契约：所有容器的迭代器都提供同名操作（<t> 为容器名）：
 *       void  <t>_iter_begin(<t>_iter *it, const Container *c);  // 初始化
 *       int   <t>_iter_next(<t>_iter *it);                       // 1=有元素 0=结束
 *       void *<t>_iter_key(<t>_iter *it);                        // 顺序容器返回 NULL
 *       void *<t>_iter_value(<t>_iter *it);
 *       int   <t>_iter_remove(<t>_iter *it);                     // 可选，删当前项
 *     新增容器时，只要它也提供这套同名函数，就能无缝接入下面的通用遍历。
 *
 *  2) 通用遍历 stk_foreach：回调式，调用方显式传入“怎么遍历”的四个操作函数指针，
 *     用同一段代码遍历任意容器——无需 _Generic，也无需 vtable / 不透明类型擦除。
 */

/* 遍历到每个 (key, value) 时调用；user 为用户数据。 */
typedef void (*stk_iter_cb)(void *key, void *value, void *user);

/* 操作函数指针类型（与各容器 *_iter_* 签名对应，it 以 void* 形态统一调度）。 */
typedef void  (*stk_iter_begin_fn)(void *it, const void *container);
typedef int   (*stk_iter_next_fn)(void *it);
typedef void *(*stk_iter_key_fn)(void *it);
typedef void *(*stk_iter_value_fn)(void *it);

/*
 * 通用遍历：it 为调用方栈上的迭代器对象（类型由容器决定），
 * container 为容器指针。begin/next/key/value 为该容器对应的操作函数。
 *
 * 示例（hashmap 已在 hashmap.h 提供 *_op 适配函数）：
 *   stk_hashmap_iter it;
 *   stk_foreach(&it, m,
 *       stk_hashmap_iter_begin_op, stk_hashmap_iter_next_op,
 *       stk_hashmap_iter_key_op,   stk_hashmap_iter_value_op,
 *       my_cb, my_user);
 */
static inline void stk_foreach(void *it, const void *container,
                               stk_iter_begin_fn begin,
                               stk_iter_next_fn next,
                               stk_iter_key_fn key,
                               stk_iter_value_fn value,
                               stk_iter_cb cb, void *user) {
    begin(it, container);
    while (next(it)) {
        cb(key(it), value(it), user);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_ITERATOR_H */
