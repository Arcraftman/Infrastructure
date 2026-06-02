#ifndef STK_CORE_UF_H
#define STK_CORE_UF_H



#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 并查集 / 不相交集合 (Union-Find / Disjoint Set Union)
 * 
 * 用于处理元素分组和连通性问题。
 * 支持路径压缩和按秩合并优化。
 * 
 * 时间复杂度:
 *   - find:    O(α(n)) 近似常数
 *   - union:   O(α(n)) 近似常数
 *   - connected: O(α(n)) 近似常数
 * 
 * Basic usage:
 * @code
 *   stk_uf uf;
 *   stk_uf_init(&uf, 10);
 *   stk_uf_union(&uf, 1, 2);
 *   stk_uf_union(&uf, 2, 3);
 *   bool connected = stk_uf_connected(&uf, 1, 3);  // true
 *   stk_uf_free(&uf);
 * @endcode
 * ========================================================================= */

typedef struct {
    size_t* parent;    /* 父节点数组 */
    size_t* rank;      /* 秩（树高度上界） */
    size_t  count;     /* 元素个数 */
    size_t  sets;      /* 当前集合数量 */
} stk_uf;

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

/* 初始化并查集，包含 n 个独立元素 */
STK_API STK_STATUS stk_uf_init(stk_uf* uf, size_t n);

/* 释放并查集内存 */
STK_API STK_STATUS stk_uf_free(stk_uf* uf);

/* 重置并查集为 n 个独立元素 */
STK_API STK_STATUS stk_uf_reset(stk_uf* uf, size_t n);

/* =========================================================================
 * 核心操作
 * ========================================================================= */

/* 查找元素 x 所在集合的代表元（根节点） */
STK_API size_t stk_uf_find(stk_uf* uf, size_t x);

/* 合并元素 x 和 y 所在的集合，返回是否成功合并 */
STK_API STK_STATUS stk_uf_union(stk_uf* uf, size_t x, size_t y);

/* 检查元素 x 和 y 是否在同一个集合中 */
STK_API bool stk_uf_connected(const stk_uf* uf, size_t x, size_t y);

/* 获取元素 x 所在集合的大小 */
STK_API size_t stk_uf_set_size(const stk_uf* uf, size_t x);

/* =========================================================================
 * 查询接口
 * ========================================================================= */

/* 返回元素个数 */
STK_API size_t stk_uf_count(const stk_uf* uf);

/* 返回当前集合数量 */
STK_API size_t stk_uf_sets(const stk_uf* uf);

/* 检查所有元素是否都在同一个集合中 */
STK_API bool stk_uf_all_connected(const stk_uf* uf);

/* =========================================================================
 * 高级操作
 * ========================================================================= */

/* 获取所有集合的代表元（根节点） */
STK_API size_t* stk_uf_roots(const stk_uf* uf);

/* 将所有元素压缩到直接指向根节点 */
STK_API STK_STATUS stk_uf_flatten(stk_uf* uf);

/* 交换两个并查集 */
STK_API STK_STATUS stk_uf_swap(stk_uf* a, stk_uf* b);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_UF_H */