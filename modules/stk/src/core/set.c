#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/set.h"
#include "stk/core/hashmap.h"
#include "stk/core/preset.h"

/* =========================================================================
 * 默认哈希/相等函数（复用 hashmap 的实现）
 * ========================================================================= */

uint64_t stk_set_str_hash(const void* key) {
    return stk_hashmap_str_hash(key);
}

bool stk_set_str_eq(const void* a, const void* b) {
    return stk_hashmap_str_eq(a, b);
}

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_set_init(stk_set* set,
                         stk_hashmap_hash_fn hash_fn,
                         stk_hashmap_eq_fn eq_fn) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set init: NULL set pointer");
    
    /* 初始化哈希表，值存储为 NULL */
    stk_hashmap_init(&set->map, 0, hash_fn, eq_fn);
    
    STK_LOG_DEBUG("Set initialized");
    return STK_OK;
}

STK_STATUS stk_set_init_with_capacity(stk_set* set,
                                       size_t initial_capacity,
                                       stk_hashmap_hash_fn hash_fn,
                                       stk_hashmap_eq_fn eq_fn) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set init_with_capacity: NULL set pointer");
    
    stk_hashmap_init(&set->map, initial_capacity, hash_fn, eq_fn);
    
    STK_LOG_DEBUG("Set initialized with capacity: %zu", initial_capacity);
    return STK_OK;
}

STK_STATUS stk_set_free(stk_set* set) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set free: NULL set pointer");
    
    stk_hashmap_free(&set->map);
    
    STK_LOG_DEBUG("Set freed");
    return STK_OK;
}

/* =========================================================================
 * 核心操作
 * ========================================================================= */

STK_STATUS stk_set_insert(stk_set* set, void* value) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set insert: NULL set pointer");
    STK_RETURN_IF(!value, STK_EINVAL, "Set insert: NULL value");
    
    /* 如果已存在，stk_hashmap_set 会更新值（但我们的值是 NULL，所以无影响） */
    stk_hashmap_set(&set->map, value, NULL);
    
    STK_LOG_DEBUG("Set insert: value inserted");
    return STK_OK;
}

STK_STATUS stk_set_remove(stk_set* set, const void* value) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set remove: NULL set pointer");
    STK_RETURN_IF(!value, STK_EINVAL, "Set remove: NULL value");
    
    stk_hashmap_remove(&set->map, value);
    
    STK_LOG_DEBUG("Set remove: value removed");
    return STK_OK;
}

bool stk_set_contains(const stk_set* set, const void* value) {
    if (!set || !value) {
        if (set) STK_LOG_WARN("Set contains: NULL %s", !value ? "value" : "set");
        return false;
    }
    
    return stk_hashmap_has(&set->map, value);
}

STK_STATUS stk_set_clear(stk_set* set) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set clear: NULL set pointer");
    
    stk_hashmap_clear(&set->map);
    
    STK_LOG_DEBUG("Set cleared");
    return STK_OK;
}

/* =========================================================================
 * 集合操作（内部辅助函数）
 * ========================================================================= */

/* 获取哈希表中的值（用于遍历） */
typedef struct {
    stk_set* dst;
    bool in_place;
} set_foreach_ctx;

static bool set_union_callback(void* key, void* value, void* user_data) {
    (void)value;
    stk_set* dst = (stk_set*)user_data;
    stk_set_insert(dst, key);
    return true;
}

static bool set_intersection_callback(void* key, void* value, void* user_data) {
    (void)value;
    stk_set* src = (stk_set*)user_data;
    if (!stk_set_contains(src, key)) {
        stk_set_remove((stk_set*)user_data, key);
    }
    return true;
}

/* =========================================================================
 * 集合操作
 * ========================================================================= */

STK_STATUS stk_set_union(stk_set* dst, const stk_set* src) {
    STK_RETURN_IF(!dst, STK_EINVAL, "Set union: NULL destination set");
    STK_RETURN_IF(!src, STK_EINVAL, "Set union: NULL source set");
    
    stk_hashmap_foreach(&src->map, set_union_callback, dst);
    
    STK_LOG_DEBUG("Set union completed");
    return STK_OK;
}

STK_STATUS stk_set_intersection(stk_set* dst, const stk_set* src) {
    STK_RETURN_IF(!dst, STK_EINVAL, "Set intersection: NULL destination set");
    STK_RETURN_IF(!src, STK_EINVAL, "Set intersection: NULL source set");
    
    /* 创建临时集合来存储交集结果 */
    stk_set result;
    stk_set_init(&result, dst->map.hash_fn, dst->map.eq_fn);
    
    /* 遍历 dst，只保留也在 src 中的元素 */
    stk_hashmap_foreach(&dst->map, set_intersection_callback, (void*)src);
    
    STK_LOG_DEBUG("Set intersection completed");
    return STK_OK;
}

STK_STATUS stk_set_difference(stk_set* dst, const stk_set* src) {
    STK_RETURN_IF(!dst, STK_EINVAL, "Set difference: NULL destination set");
    STK_RETURN_IF(!src, STK_EINVAL, "Set difference: NULL source set");
    
    /* 遍历 src，从 dst 中移除 */
    stk_hashmap_foreach(&src->map, set_intersection_callback, dst);
    
    STK_LOG_DEBUG("Set difference completed");
    return STK_OK;
}

STK_STATUS stk_set_symmetric_difference(stk_set* dst, const stk_set* src) {
    STK_RETURN_IF(!dst, STK_EINVAL, "Set symmetric_difference: NULL destination set");
    STK_RETURN_IF(!src, STK_EINVAL, "Set symmetric_difference: NULL source set");
    
    /* 创建临时集合 */
    stk_set temp;
    stk_set_init(&temp, dst->map.hash_fn, dst->map.eq_fn);
    
    /* 先复制 dst 到 temp */
    stk_hashmap_foreach(&dst->map, set_union_callback, &temp);
    
    /* 对称差 = (dst ∪ src) - (dst ∩ src) */
    /* 先取并集到 dst */
    stk_set_union(dst, src);
    /* 再取交集到 temp */
    /* 最后从 dst 中删除 temp 中的元素 */
    stk_hashmap_foreach(&temp.map, set_intersection_callback, dst);
    
    stk_set_free(&temp);
    
    STK_LOG_DEBUG("Set symmetric_difference completed");
    return STK_OK;
}

bool stk_set_is_subset(const stk_set* a, const stk_set* b) {
    if (!a || !b) {
        STK_LOG_WARN("Set is_subset: NULL %s", !a ? "first set" : "second set");
        return false;
    }
    
    /* 如果 a 的大小大于 b，a 不可能是 b 的子集 */
    if (a->map.count > b->map.count) {
        return false;
    }
    
    /* 遍历 a，检查每个元素是否都在 b 中 */
    for (size_t i = 0; i < a->map.capacity; i++) {
        if (a->map.entries[i].occupied) {
            if (!stk_set_contains(b, a->map.entries[i].key)) {
                return false;
            }
        }
    }
    
    return true;
}

bool stk_set_is_equal(const stk_set* a, const stk_set* b) {
    if (!a || !b) {
        STK_LOG_WARN("Set is_equal: NULL %s", !a ? "first set" : "second set");
        return false;
    }
    
    if (a->map.count != b->map.count) {
        return false;
    }
    
    return stk_set_is_subset(a, b);
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_set_size(const stk_set* set) {
    return set ? set->map.count : 0;
}

bool stk_set_empty(const stk_set* set) {
    return set ? set->map.count == 0 : true;
}

/* =========================================================================
 * 遍历操作
 * ========================================================================= */

STK_STATUS stk_set_foreach(const stk_set* set,
                             bool (*fn)(void* value, void* user_data),
                             void* user_data) {
    STK_RETURN_IF(!set, STK_EINVAL, "Set foreach: NULL set pointer");
    STK_RETURN_IF(!fn, STK_EINVAL, "Set foreach: NULL callback function");
    
    /* 包装回调函数，适配 hashmap 的 foreach */
    bool wrapper(void* key, void* value, void* ud) {
        (void)value;
        return fn(key, ud);
    }
    
    return stk_hashmap_foreach(&set->map, wrapper, user_data);
}

void** stk_set_to_array(const stk_set* set) {
    if (!set) {
        STK_LOG_WARN("Set to_array: NULL set pointer");
        return NULL;
    }
    
    size_t size = stk_set_size(set);
    if (size == 0) {
        return NULL;
    }
    
    void** array = (void**)malloc(sizeof(void*) * size);
    if (!array) {
        STK_LOG_ERROR("Set to_array: malloc failed");
        return NULL;
    }
    
    size_t idx = 0;
    for (size_t i = 0; i < set->map.capacity && idx < size; i++) {
        if (set->map.entries[i].occupied) {
            array[idx++] = set->map.entries[i].key;
        }
    }
    
    STK_LOG_DEBUG("Set to_array: converted %zu elements", size);
    return array;
}

void stk_set_free_array(void** array) {
    if (array) {
        free(array);
    }
}