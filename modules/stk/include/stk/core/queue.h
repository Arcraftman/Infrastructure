#ifndef STK_CORE_QUEUE_H
#define STK_CORE_QUEUE_H



#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 动态队列 (FIFO)
 * 
 * 基于循环数组实现的队列，支持自动扩容。
 * 存储 void* 类型，可存放任意数据指针。
 * 
 * 时间复杂度:
 *   - enqueue:  O(1) 均摊
 *   - dequeue:  O(1)
 *   - front:    O(1)
 *   - back:     O(1)
 * 
 * Basic usage:
 * @code
 *   stk_queue q;
 *   stk_queue_init(&q);
 *   stk_queue_enqueue(&q, ptr);
 *   void *front = stk_queue_front(&q);
 *   void *val = stk_queue_dequeue(&q);
 *   stk_queue_free(&q);
 * @endcode
 * ========================================================================= */

typedef struct {
    void** data;       /* 元素数组 */
    size_t head;       /* 头部索引（出队位置） */
    size_t tail;       /* 尾部索引（入队位置） */
    size_t size;       /* 当前元素数量 */
    size_t capacity;   /* 当前容量 */
} stk_queue;

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化空队列 */
STK_API STK_STATUS stk_queue_init(stk_queue* q);

/* 初始化并预分配容量 */
STK_API STK_STATUS stk_queue_init_with_capacity(stk_queue* q, size_t capacity);

/* 释放队列内存 */
STK_API STK_STATUS stk_queue_free(stk_queue* q);

/* =========================================================================
 * 核心操作
 * ========================================================================= */

/* 入队：将元素添加到队尾 */
STK_API STK_STATUS stk_queue_enqueue(stk_queue* q, void* value);

/* 出队：移除队首元素 */
STK_API STK_STATUS stk_queue_dequeue(stk_queue* q);

/* 获取队首元素（不移除），队列空时返回 NULL */
STK_API void* stk_queue_front(const stk_queue* q);

/* 获取队尾元素（不移除），队列空时返回 NULL */
STK_API void* stk_queue_back(const stk_queue* q);

/* 出队并返回队首元素值 */
STK_API void* stk_queue_dequeue_value(stk_queue* q);

/* 清空所有元素（不释放内存） */
STK_API STK_STATUS stk_queue_clear(stk_queue* q);

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回队列中元素个数 */
STK_API size_t stk_queue_size(const stk_queue* q);

/* 检查队列是否为空 */
STK_API bool stk_queue_empty(const stk_queue* q);

/* 返回当前容量 */
STK_API size_t stk_queue_capacity(const stk_queue* q);

/* =========================================================================
 * 内存管理
 * ========================================================================= */

/* 预分配容量，避免多次扩容 */
STK_API STK_STATUS stk_queue_reserve(stk_queue* q, size_t capacity);

/* 收缩内存到刚好容纳当前元素 */
STK_API STK_STATUS stk_queue_shrink(stk_queue* q);

/* 交换两个队列的内容 */
STK_API STK_STATUS stk_queue_swap(stk_queue* a, stk_queue* b);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_QUEUE_H */