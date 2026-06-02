#ifndef STK_CORE_SET_H
#define STK_CORE_SET_H



#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 集合 (Set)
 * 
 * 基于哈希表实现，存储唯一值。
 * 存储 void* 类型，可存放任意数据指针。
 * 
 * 时间复杂度:
 *   - insert:   O(1) 均摊
 *   - remove:   O(1) 均摊
 *   - contains: O(1) 均摊
 * 
 * Basic usage:
 * @code
 *   stk_set set;
 *   stk_set_init(&set, NULL, NULL);
 *   stk_set_insert(&set, ptr);
 *   bool exists = stk_set_contains(&set, ptr);
 *   stk_set_remove(&set, ptr);
 *   stk_set_free(&set);
 * @endcode
 * ========================================================================= */

/* 集合结构（基于哈希表） */
typedef struct {
    stk_hashmap map;   /* 底层使用哈希表，值存储为 NULL */
} stk_set;

/* 默认哈希/相等函数（用于 C 字符串键） */
STK_API uint64_t stk_set_str_hash(const void* key);
STK_API bool     stk_set_str_eq(const void* a, const void* b);

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化空集合 */
STK_API STK_STATUS stk_set_init(stk_set* set,
                                 stk_hashmap_hash_fn hash_fn,
                                 stk_hashmap_eq_fn eq_fn);

/* 初始化并预分配容量 */
STK_API STK_STATUS stk_set_init_with_capacity(stk_set* set,
                                               size_t initial_capacity,
                                               stk_hashmap_hash_fn hash_fn,
                                               stk_hashmap_eq_fn eq_fn);

/* 释放集合内存 */
STK_API STK_STATUS stk_set_free(stk_set* set);

/* =========================================================================
 * 核心操作
 * ========================================================================= */

/* 插入元素到集合（如果已存在则不操作） */
STK_API STK_STATUS stk_set_insert(stk_set* set, void* value);

/* 从集合中移除元素 */
STK_API STK_STATUS stk_set_remove(stk_set* set, const void* value);

/* 检查集合是否包含元素 */
STK_API bool stk_set_contains(const stk_set* set, const void* value);

/* 清空集合 */
STK_API STK_STATUS stk_set_clear(stk_set* set);

/* =========================================================================
 * 集合操作
 * ========================================================================= */

/* 并集：将 src 中所有元素插入到 dst */
STK_API STK_STATUS stk_set_union(stk_set* dst, const stk_set* src);

/* 交集：dst 中只保留同时存在于 src 中的元素 */
STK_API STK_STATUS stk_set_intersection(stk_set* dst, const stk_set* src);

/* 差集：dst 中移除也存在于 src 中的元素 */
STK_API STK_STATUS stk_set_difference(stk_set* dst, const stk_set* src);

/* 对称差集：dst 中保留只存在于其中一个集合的元素 */
STK_API STK_STATUS stk_set_symmetric_difference(stk_set* dst, const stk_set* src);

/* 检查是否为子集 */
STK_API bool stk_set_is_subset(const stk_set* a, const stk_set* b);

/* 检查两个集合是否相等 */
STK_API bool stk_set_is_equal(const stk_set* a, const stk_set* b);

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回集合中元素个数 */
STK_API size_t stk_set_size(const stk_set* set);

/* 检查集合是否为空 */
STK_API bool stk_set_empty(const stk_set* set);

/* =========================================================================
 * 遍历操作
 * ========================================================================= */

/* 遍历集合，对每个元素执行函数（返回 false 停止遍历） */
STK_API STK_STATUS stk_set_foreach(const stk_set* set,
                                     bool (*fn)(void* value, void* user_data),
                                     void* user_data);

/* 转换为数组 */
STK_API void** stk_set_to_array(const stk_set* set);

/* 释放数组（由 to_array 返回的数组） */
STK_API void stk_set_free_array(void** array);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_SET_H */