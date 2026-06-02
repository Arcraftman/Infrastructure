#ifndef STK_CORE_SLIST_H
#define STK_CORE_SLIST_H



#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 单向链表 (Singly Linked List)
 * 
 * 每个节点只指向下一个节点，不能反向遍历。
 * 存储 void* 类型，可存放任意数据指针。
 * 
 * 时间复杂度:
 *   - push_front:  O(1)
 *   - pop_front:   O(1)
 *   - push_back:   O(n) (需要遍历到尾部)
 *   - pop_back:    O(n)
 *   - insert:      O(n)
 *   - erase:       O(n)
 *   - find:        O(n)
 * 
 * 优点: 实现简单，内存占用少（比双向链表少一个指针）
 * 缺点: 无法反向遍历，删除尾部需要遍历
 * 
 * Basic usage:
 * @code
 *   stk_slist list;
 *   stk_slist_init(&list);
 *   stk_slist_push_front(&list, ptr);
 *   void *front = stk_slist_front(&list);
 *   stk_slist_pop_front(&list);
 *   stk_slist_free(&list);
 * @endcode
 * ========================================================================= */

/* 单向链表节点 */
typedef struct stk_snode {
    void* data;                 /* 存储的数据 */
    struct stk_snode* next;     /* 指向下一个节点 */
} stk_snode;

/* 单向链表结构 */
typedef struct {
    stk_snode* head;            /* 头节点指针 */
    stk_snode* tail;            /* 尾节点指针（可选，用于快速尾部操作） */
    size_t size;                /* 节点数量 */
} stk_slist;

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化空链表 */
STK_API STK_STATUS stk_slist_init(stk_slist* list);

/* 释放链表所有节点（不释放数据） */
STK_API STK_STATUS stk_slist_free(stk_slist* list);

/* 释放链表并释放数据（使用自定义释放函数） */
STK_API STK_STATUS stk_slist_free_custom(stk_slist* list, void (*free_data)(void*));

/* =========================================================================
 * 头部操作 (O(1))
 * ========================================================================= */

/* 在头部插入元素 */
STK_API STK_STATUS stk_slist_push_front(stk_slist* list, void* value);

/* 移除头部元素 */
STK_API STK_STATUS stk_slist_pop_front(stk_slist* list);

/* 获取头部元素（不移除），链表空时返回 NULL */
STK_API void* stk_slist_front(const stk_slist* list);

/* 移除并返回头部元素 */
STK_API void* stk_slist_pop_front_value(stk_slist* list);

/* =========================================================================
 * 尾部操作 (push_back 需要遍历，pop_back 需要遍历到倒数第二个节点)
 * ========================================================================= */

/* 在尾部插入元素 */
STK_API STK_STATUS stk_slist_push_back(stk_slist* list, void* value);

/* 移除尾部元素 */
STK_API STK_STATUS stk_slist_pop_back(stk_slist* list);

/* 获取尾部元素（不移除），链表空时返回 NULL */
STK_API void* stk_slist_back(const stk_slist* list);

/* 移除并返回尾部元素 */
STK_API void* stk_slist_pop_back_value(stk_slist* list);

/* =========================================================================
 * 随机访问 (需要遍历，O(n))
 * ========================================================================= */

/* 获取指定索引的元素（0-based），越界返回 NULL */
STK_API void* stk_slist_get(const stk_slist* list, size_t index);

/* 设置指定索引的元素值，越界返回错误 */
STK_API STK_STATUS stk_slist_set(stk_slist* list, size_t index, void* value);

/* 在指定索引处插入元素 */
STK_API STK_STATUS stk_slist_insert(stk_slist* list, size_t index, void* value);

/* 删除指定索引处的元素 */
STK_API STK_STATUS stk_slist_erase(stk_slist* list, size_t index);

/* =========================================================================
 * 查找操作
 * ========================================================================= */

/* 查找第一个匹配值的节点索引，未找到返回 (size_t)-1 */
STK_API size_t stk_slist_find(const stk_slist* list, const void* value, 
                               bool (*equal)(const void* a, const void* b));

/* 检查链表是否包含某个值 */
STK_API bool stk_slist_contains(const stk_slist* list, const void* value,
                                 bool (*equal)(const void* a, const void* b));

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回链表中元素个数 */
STK_API size_t stk_slist_size(const stk_slist* list);

/* 检查链表是否为空 */
STK_API bool stk_slist_empty(const stk_slist* list);

/* =========================================================================
 * 修改操作
 * ========================================================================= */

/* 清空所有节点（不释放数据） */
STK_API STK_STATUS stk_slist_clear(stk_slist* list);

/* 清空并释放数据 */
STK_API STK_STATUS stk_slist_clear_custom(stk_slist* list, void (*free_data)(void*));

/* 反转链表 */
STK_API STK_STATUS stk_slist_reverse(stk_slist* list);

/* 交换两个链表 */
STK_API STK_STATUS stk_slist_swap(stk_slist* a, stk_slist* b);

/* =========================================================================
 * 遍历操作
 * ========================================================================= */

/* 遍历链表，对每个元素执行函数（返回 false 停止遍历） */
STK_API STK_STATUS stk_slist_foreach(const stk_slist* list, 
                                      bool (*fn)(void* data, void* user_data),
                                      void* user_data);

/* 遍历链表（可修改），对每个元素执行函数 */
STK_API STK_STATUS stk_slist_foreach_mut(stk_slist* list,
                                          bool (*fn)(void** data, void* user_data),
                                          void* user_data);

/* =========================================================================
 * 高级操作
 * ========================================================================= */

/* 将两个链表合并（将 src 的所有节点移动到 dst 尾部） */
STK_API STK_STATUS stk_slist_merge(stk_slist* dst, stk_slist* src);

/* 移除所有匹配的值 */
STK_API size_t stk_slist_remove_all(stk_slist* list, const void* value,
                                     bool (*equal)(const void* a, const void* b));

/* 获取节点地址（用于高级操作） */
STK_API stk_snode* stk_slist_node_at(const stk_slist* list, size_t index);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_SLIST_H */