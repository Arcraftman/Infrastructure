#ifndef STK_CORE_STACK_H
#define STK_CORE_STACK_H



#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 动态栈 (LIFO)
 * 
 * 基于动态数组实现的栈，支持自动扩容。
 * 存储 void* 类型，可存放任意数据指针。
 * 
 * 时间复杂度:
 *   - push:    O(1) 均摊
 *   - pop:     O(1)
 *   - top:     O(1)
 *   - reserve: O(n)
 * 
 * Basic usage:
 * @code
 *   stk_stack s;
 *   stk_stack_init(&s);
 *   stk_stack_push(&s, ptr);
 *   void *top = stk_stack_top(&s);
 *   void *val = stk_stack_pop(&s);
 *   stk_stack_free(&s);
 * @endcode
 * ========================================================================= */

typedef struct {
    void** data;       /* 元素数组 */
    size_t size;       /* 当前元素数量 */
    size_t capacity;   /* 当前容量 */
} stk_stack;

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化空栈 */
STK_API STK_STATUS stk_stack_init(stk_stack* s);

/* 初始化并预分配容量 */
STK_API STK_STATUS stk_stack_init_with_capacity(stk_stack* s, size_t capacity);

/* 释放栈内存 */
STK_API STK_STATUS stk_stack_free(stk_stack* s);

/* =========================================================================
 * 核心操作
 * ========================================================================= */

/* 压入元素到栈顶 */
STK_API STK_STATUS stk_stack_push(stk_stack* s, void* value);

/* 弹出栈顶元素（不返回值） */
STK_API STK_STATUS stk_stack_pop(stk_stack* s);

/* 获取栈顶元素（不移除），栈空时返回 NULL */
STK_API void* stk_stack_top(const stk_stack* s);

/* 弹出栈顶元素并返回其值 */
STK_API void* stk_stack_pop_value(stk_stack* s);

/* 清空所有元素（不释放内存） */
STK_API STK_STATUS stk_stack_clear(stk_stack* s);

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回栈中元素个数 */
STK_API size_t stk_stack_size(const stk_stack* s);

/* 检查栈是否为空 */
STK_API bool stk_stack_empty(const stk_stack* s);

/* 返回当前容量 */
STK_API size_t stk_stack_capacity(const stk_stack* s);

/* =========================================================================
 * 内存管理
 * ========================================================================= */

/* 预分配容量，避免多次扩容 */
STK_API STK_STATUS stk_stack_reserve(stk_stack* s, size_t capacity);

/* 收缩内存到刚好容纳当前元素 */
STK_API STK_STATUS stk_stack_shrink(stk_stack* s);

/* 交换两个栈的内容 */
STK_API STK_STATUS stk_stack_swap(stk_stack* a, stk_stack* b);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_STACK_H */