#ifndef STK_CORE_DEQUE_H
#define STK_CORE_DEQUE_H

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 双端队列 (Deque)
 * 
 * 基于循环数组实现，支持在头部和尾部进行 O(1) 的插入和删除。
 * 存储 void* 类型，可存放任意数据指针。
 * 
 * 时间复杂度:
 *   - push_front/push_back: O(1) 均摊
 *   - pop_front/pop_back:   O(1)
 *   - front/back:           O(1)
 *   - get/set:              O(1)
 *   - insert/erase:         O(n)
 * 
 * Basic usage:
 * @code
 *   stk_deque dq;
 *   stk_deque_init(&dq);
 *   stk_deque_push_back(&dq, ptr);
 *   stk_deque_push_front(&dq, ptr);
 *   void *front = stk_deque_front(&dq);
 *   void *back = stk_deque_back(&dq);
 *   stk_deque_free(&dq);
 * @endcode
 * ========================================================================= */

typedef struct {
    void** data;       /* 元素数组 */
    size_t head;       /* 头部索引 */
    size_t tail;       /* 尾部索引（指向下一个空位） */
    size_t size;       /* 当前元素数量 */
    size_t capacity;   /* 当前容量 */
} stk_deque;

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化空双端队列 */
STK_API STK_STATUS stk_deque_init(stk_deque* dq);

/* 初始化并预分配容量 */
STK_API STK_STATUS stk_deque_init_with_capacity(stk_deque* dq, size_t capacity);

/* 释放双端队列内存 */
STK_API STK_STATUS stk_deque_free(stk_deque* dq);

/* =========================================================================
 * 头部操作 (O(1))
 * ========================================================================= */

/* 在头部插入元素 */
STK_API STK_STATUS stk_deque_push_front(stk_deque* dq, void* value);

/* 移除头部元素 */
STK_API STK_STATUS stk_deque_pop_front(stk_deque* dq);

/* 获取头部元素（不移除），队列空时返回 NULL */
STK_API void* stk_deque_front(const stk_deque* dq);

/* 移除并返回头部元素 */
STK_API void* stk_deque_pop_front_value(stk_deque* dq);

/* =========================================================================
 * 尾部操作 (O(1))
 * ========================================================================= */

/* 在尾部插入元素 */
STK_API STK_STATUS stk_deque_push_back(stk_deque* dq, void* value);

/* 移除尾部元素 */
STK_API STK_STATUS stk_deque_pop_back(stk_deque* dq);

/* 获取尾部元素（不移除），队列空时返回 NULL */
STK_API void* stk_deque_back(const stk_deque* dq);

/* 移除并返回尾部元素 */
STK_API void* stk_deque_pop_back_value(stk_deque* dq);

/* =========================================================================
 * 随机访问 (O(1))
 * ========================================================================= */

/* 获取指定索引的元素（0-based），越界返回 NULL */
STK_API void* stk_deque_get(const stk_deque* dq, size_t index);

/* 设置指定索引的元素值，越界返回错误 */
STK_API STK_STATUS stk_deque_set(stk_deque* dq, size_t index, void* value);

/* =========================================================================
 * 插入/删除 (O(n))
 * ========================================================================= */

/* 在指定索引处插入元素 */
STK_API STK_STATUS stk_deque_insert(stk_deque* dq, size_t index, void* value);

/* 删除指定索引处的元素 */
STK_API STK_STATUS stk_deque_erase(stk_deque* dq, size_t index);

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回队列中元素个数 */
STK_API size_t stk_deque_size(const stk_deque* dq);

/* 检查队列是否为空 */
STK_API bool stk_deque_empty(const stk_deque* dq);

/* 返回当前容量 */
STK_API size_t stk_deque_capacity(const stk_deque* dq);

/* =========================================================================
 * 内存管理
 * ========================================================================= */

/* 预分配容量 */
STK_API STK_STATUS stk_deque_reserve(stk_deque* dq, size_t capacity);

/* 收缩内存到刚好容纳当前元素 */
STK_API STK_STATUS stk_deque_shrink(stk_deque* dq);

/* 清空所有元素（不释放内存） */
STK_API STK_STATUS stk_deque_clear(stk_deque* dq);

/* 交换两个双端队列 */
STK_API STK_STATUS stk_deque_swap(stk_deque* a, stk_deque* b);

/* =========================================================================
 * 遍历操作
 * ========================================================================= */

/* 遍历队列，对每个元素执行函数（返回 false 停止遍历） */
STK_API STK_STATUS stk_deque_foreach(const stk_deque* dq,
                                       bool (*fn)(void* data, void* user_data),
                                       void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_DEQUE_H */