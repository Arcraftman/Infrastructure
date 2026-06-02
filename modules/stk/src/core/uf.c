#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/uf.h"

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_uf_init(stk_uf* uf, size_t n) {
    STK_RETURN_IF(!uf, STK_EINVAL, "UF init: NULL uf pointer");
    STK_RETURN_IF(n == 0, STK_EINVAL, "UF init: n must be > 0");
    
    uf->parent = (size_t*)malloc(sizeof(size_t) * n);
    uf->rank = (size_t*)malloc(sizeof(size_t) * n);
    
    if (!uf->parent || !uf->rank) {
        STK_LOG_ERROR("UF init: malloc failed (n=%zu)", n);
        free(uf->parent);
        free(uf->rank);
        return STK_ENOMEM;
    }
    
    /* 初始化：每个元素的父节点是自己，秩为 0 */
    for (size_t i = 0; i < n; i++) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }
    
    uf->count = n;
    uf->sets = n;
    
    STK_LOG_DEBUG("UF initialized: n=%zu", n);
    return STK_OK;
}

STK_STATUS stk_uf_free(stk_uf* uf) {
    if (!uf) {
        STK_LOG_WARN("UF free: NULL uf pointer");
        return STK_EINVAL;
    }
    
    free(uf->parent);
    free(uf->rank);
    uf->parent = NULL;
    uf->rank = NULL;
    uf->count = 0;
    uf->sets = 0;
    
    STK_LOG_DEBUG("UF freed");
    return STK_OK;
}

STK_STATUS stk_uf_reset(stk_uf* uf, size_t n) {
    STK_RETURN_IF(!uf, STK_EINVAL, "UF reset: NULL uf pointer");
    STK_RETURN_IF(n == 0, STK_EINVAL, "UF reset: n must be > 0");
    
    /* 重新分配内存（如果需要） */
    if (uf->count != n) {
        size_t* new_parent = (size_t*)realloc(uf->parent, sizeof(size_t) * n);
        size_t* new_rank = (size_t*)realloc(uf->rank, sizeof(size_t) * n);
        
        if (!new_parent || !new_rank) {
            STK_LOG_ERROR("UF reset: realloc failed (n=%zu)", n);
            return STK_ENOMEM;
        }
        
        uf->parent = new_parent;
        uf->rank = new_rank;
        uf->count = n;
    }
    
    /* 重新初始化 */
    for (size_t i = 0; i < n; i++) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }
    uf->sets = n;
    
    STK_LOG_DEBUG("UF reset: n=%zu", n);
    return STK_OK;
}

/* =========================================================================
 * 核心操作
 * ========================================================================= */

size_t stk_uf_find(stk_uf* uf, size_t x) {
    if (!uf || x >= uf->count) {
        if (uf) STK_LOG_WARN("UF find: invalid index %zu (max=%zu)", x, uf->count);
        return (size_t)-1;
    }
    
    /* 路径压缩：将路径上的所有节点直接指向根节点 */
    if (uf->parent[x] != x) {
        uf->parent[x] = stk_uf_find(uf, uf->parent[x]);
    }
    
    return uf->parent[x];
}

STK_STATUS stk_uf_union(stk_uf* uf, size_t x, size_t y) {
    STK_RETURN_IF(!uf, STK_EINVAL, "UF union: NULL uf pointer");
    STK_RETURN_IF(x >= uf->count, STK_ERANGE, "UF union: x=%zu out of range (max=%zu)", x, uf->count - 1);
    STK_RETURN_IF(y >= uf->count, STK_ERANGE, "UF union: y=%zu out of range (max=%zu)", y, uf->count - 1);
    
    size_t root_x = stk_uf_find(uf, x);
    size_t root_y = stk_uf_find(uf, y);
    
    /* 如果已经在同一个集合中，不需要合并 */
    if (root_x == root_y) {
        return STK_OK;
    }
    
    /* 按秩合并：将秩较小的树接到秩较大的树上 */
    if (uf->rank[root_x] < uf->rank[root_y]) {
        uf->parent[root_x] = root_y;
    } else if (uf->rank[root_x] > uf->rank[root_y]) {
        uf->parent[root_y] = root_x;
    } else {
        /* 秩相等时，任意合并，并增加新根的秩 */
        uf->parent[root_y] = root_x;
        uf->rank[root_x]++;
    }
    
    uf->sets--;
    
    STK_LOG_DEBUG("UF union: merged %zu and %zu, sets=%zu", x, y, uf->sets);
    return STK_OK;
}

bool stk_uf_connected(const stk_uf* uf, size_t x, size_t y) {
    if (!uf || x >= uf->count || y >= uf->count) {
        if (uf) STK_LOG_WARN("UF connected: invalid index");
        return false;
    }
    
    return stk_uf_find((stk_uf*)uf, x) == stk_uf_find((stk_uf*)uf, y);
}

size_t stk_uf_set_size(const stk_uf* uf, size_t x) {
    if (!uf || x >= uf->count) {
        if (uf) STK_LOG_WARN("UF set_size: invalid index %zu", x);
        return 0;
    }
    
    size_t root = stk_uf_find((stk_uf*)uf, x);
    size_t size = 0;
    
    for (size_t i = 0; i < uf->count; i++) {
        if (stk_uf_find((stk_uf*)uf, i) == root) {
            size++;
        }
    }
    
    return size;
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_uf_count(const stk_uf* uf) {
    return uf ? uf->count : 0;
}

size_t stk_uf_sets(const stk_uf* uf) {
    return uf ? uf->sets : 0;
}

bool stk_uf_all_connected(const stk_uf* uf) {
    return uf ? uf->sets == 1 : false;
}

/* =========================================================================
 * 高级操作
 * ========================================================================= */

size_t* stk_uf_roots(const stk_uf* uf) {
    if (!uf) {
        STK_LOG_WARN("UF roots: NULL uf pointer");
        return NULL;
    }
    
    size_t* roots = (size_t*)malloc(sizeof(size_t) * uf->sets);
    if (!roots) {
        STK_LOG_ERROR("UF roots: malloc failed");
        return NULL;
    }
    
    size_t idx = 0;
    bool* visited = (bool*)calloc(uf->count, sizeof(bool));
    if (!visited) {
        free(roots);
        STK_LOG_ERROR("UF roots: calloc failed");
        return NULL;
    }
    
    for (size_t i = 0; i < uf->count; i++) {
        size_t root = stk_uf_find((stk_uf*)uf, i);
        if (!visited[root]) {
            visited[root] = true;
            roots[idx++] = root;
        }
    }
    
    free(visited);
    return roots;
}

STK_STATUS stk_uf_flatten(stk_uf* uf) {
    STK_RETURN_IF(!uf, STK_EINVAL, "UF flatten: NULL uf pointer");
    
    /* 对所有元素执行 find，触发路径压缩 */
    for (size_t i = 0; i < uf->count; i++) {
        stk_uf_find(uf, i);
    }
    
    STK_LOG_DEBUG("UF flattened");
    return STK_OK;
}

STK_STATUS stk_uf_swap(stk_uf* a, stk_uf* b) {
    STK_RETURN_IF(!a, STK_EINVAL, "UF swap: NULL first uf pointer");
    STK_RETURN_IF(!b, STK_EINVAL, "UF swap: NULL second uf pointer");
    
    stk_uf tmp = *a;
    *a = *b;
    *b = tmp;
    
    STK_LOG_DEBUG("UF swapped");
    return STK_OK;
}