#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/deque.h"

/* =========================================================================
 * 内部常量定义
 * ========================================================================= */

#define STK_DEQUE_DEFAULT_CAPACITY 16
#define STK_DEQUE_GROW_FACTOR 2
#define STK_DEQUE_SHRINK_FACTOR 4
#define STK_DEQUE_ELEMENT_SIZE sizeof(void*)

/* =========================================================================
 * 内部辅助函数
 * ========================================================================= */

/* 获取真实索引（考虑 head 偏移） */
static inline size_t deque_real_index(const stk_deque* dq, size_t index) {
    return (dq->head + index) % dq->capacity;
}

/* 扩容 - 容量翻倍 */
static bool deque_grow(stk_deque* dq) {
    size_t new_capacity;
    
    if (dq->capacity == 0) {
        new_capacity = STK_DEQUE_DEFAULT_CAPACITY;
    } else if (dq->capacity < (SIZE_MAX / STK_DEQUE_GROW_FACTOR)) {
        new_capacity = dq->capacity * STK_DEQUE_GROW_FACTOR;
    } else {
        STK_LOG_ERROR("Deque grow: cannot grow beyond SIZE_MAX");
        return false;
    }
    
    void** new_data = (void**)malloc(STK_DEQUE_ELEMENT_SIZE * new_capacity);
    if (!new_data) {
        STK_LOG_ERROR("Deque grow: malloc failed (size=%zu)", new_capacity);
        return false;
    }
    
    /* 重新排列元素到新数组的头部（使元素连续） */
    for (size_t i = 0; i < dq->size; i++) {
        new_data[i] = dq->data[deque_real_index(dq, i)];
    }
    
    free(dq->data);
    dq->data = new_data;
    dq->head = 0;
    dq->tail = dq->size;
    dq->capacity = new_capacity;
    
    STK_LOG_DEBUG("Deque grew: capacity=%zu -> %zu", 
                  new_capacity / STK_DEQUE_GROW_FACTOR, new_capacity);
    return true;
}

/* 缩容检查 */
static void deque_shrink_if_needed(stk_deque* dq) {
    if (dq->capacity <= STK_DEQUE_DEFAULT_CAPACITY) {
        return;
    }
    
    if (dq->size > dq->capacity / STK_DEQUE_SHRINK_FACTOR) {
        return;
    }
    
    size_t new_capacity = dq->capacity / STK_DEQUE_GROW_FACTOR;
    if (new_capacity < STK_DEQUE_DEFAULT_CAPACITY) {
        new_capacity = STK_DEQUE_DEFAULT_CAPACITY;
    }
    
    void** new_data = (void**)malloc(STK_DEQUE_ELEMENT_SIZE * new_capacity);
    if (!new_data) {
        return; /* 缩容失败不影响使用 */
    }
    
    for (size_t i = 0; i < dq->size; i++) {
        new_data[i] = dq->data[deque_real_index(dq, i)];
    }
    
    free(dq->data);
    dq->data = new_data;
    dq->head = 0;
    dq->tail = dq->size;
    dq->capacity = new_capacity;
    
    STK_LOG_DEBUG("Deque shrunk: capacity=%zu -> %zu", 
                  new_capacity * STK_DEQUE_GROW_FACTOR, new_capacity);
}

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_deque_init(stk_deque* dq) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque init: NULL deque pointer");
    
    dq->data = NULL;
    dq->head = 0;
    dq->tail = 0;
    dq->size = 0;
    dq->capacity = 0;
    
    STK_LOG_DEBUG("Deque initialized");
    return STK_OK;
}

STK_STATUS stk_deque_init_with_capacity(stk_deque* dq, size_t capacity) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque init_with_capacity: NULL deque pointer");
    
    if (capacity == 0) {
        dq->data = NULL;
        dq->head = dq->tail = dq->size = dq->capacity = 0;
        return STK_OK;
    }
    
    dq->data = (void**)malloc(STK_DEQUE_ELEMENT_SIZE * capacity);
    if (!dq->data) {
        STK_LOG_ERROR("Deque init_with_capacity: malloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    dq->head = 0;
    dq->tail = 0;
    dq->size = 0;
    dq->capacity = capacity;
    
    STK_LOG_DEBUG("Deque init_with_capacity: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_deque_free(stk_deque* dq) {
    if (!dq) {
        STK_LOG_WARN("Deque free: NULL deque pointer");
        return STK_EINVAL;
    }
    
    if (dq->data) {
        free(dq->data);
        dq->data = NULL;
    }
    dq->head = dq->tail = dq->size = dq->capacity = 0;
    
    STK_LOG_DEBUG("Deque freed");
    return STK_OK;
}

/* =========================================================================
 * 头部操作
 * ========================================================================= */

STK_STATUS stk_deque_push_front(stk_deque* dq, void* value) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque push_front: NULL deque pointer");
    
    if (dq->size >= dq->capacity) {
        if (!deque_grow(dq)) {
            STK_LOG_ERROR("Deque push_front: failed to grow deque");
            return STK_ENOMEM;
        }
    }
    
    dq->head = (dq->head == 0) ? dq->capacity - 1 : dq->head - 1;
    dq->data[dq->head] = value;
    dq->size++;
    
    STK_LOG_DEBUG("Deque push_front: size=%zu, capacity=%zu", dq->size, dq->capacity);
    return STK_OK;
}

STK_STATUS stk_deque_pop_front(stk_deque* dq) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque pop_front: NULL deque pointer");
    STK_RETURN_IF(dq->size == 0, STK_EMPTY, "Deque pop_front: deque is empty");
    
    dq->head = (dq->head + 1) % dq->capacity;
    dq->size--;
    
    deque_shrink_if_needed(dq);
    
    STK_LOG_DEBUG("Deque pop_front: size=%zu", dq->size);
    return STK_OK;
}

void* stk_deque_front(const stk_deque* dq) {
    if (!dq) {
        STK_LOG_WARN("Deque front: NULL deque pointer");
        return NULL;
    }
    if (dq->size == 0) {
        STK_LOG_WARN("Deque front: deque is empty");
        return NULL;
    }
    return dq->data[dq->head];
}

void* stk_deque_pop_front_value(stk_deque* dq) {
    if (!dq) {
        STK_LOG_WARN("Deque pop_front_value: NULL deque pointer");
        return NULL;
    }
    if (dq->size == 0) {
        STK_LOG_WARN("Deque pop_front_value: deque is empty");
        return NULL;
    }
    
    void* value = dq->data[dq->head];
    dq->head = (dq->head + 1) % dq->capacity;
    dq->size--;
    
    deque_shrink_if_needed(dq);
    
    STK_LOG_DEBUG("Deque pop_front_value: size=%zu", dq->size);
    return value;
}

/* =========================================================================
 * 尾部操作
 * ========================================================================= */

STK_STATUS stk_deque_push_back(stk_deque* dq, void* value) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque push_back: NULL deque pointer");
    
    if (dq->size >= dq->capacity) {
        if (!deque_grow(dq)) {
            STK_LOG_ERROR("Deque push_back: failed to grow deque");
            return STK_ENOMEM;
        }
    }
    
    dq->data[dq->tail] = value;
    dq->tail = (dq->tail + 1) % dq->capacity;
    dq->size++;
    
    STK_LOG_DEBUG("Deque push_back: size=%zu, capacity=%zu", dq->size, dq->capacity);
    return STK_OK;
}

STK_STATUS stk_deque_pop_back(stk_deque* dq) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque pop_back: NULL deque pointer");
    STK_RETURN_IF(dq->size == 0, STK_EMPTY, "Deque pop_back: deque is empty");
    
    dq->tail = (dq->tail == 0) ? dq->capacity - 1 : dq->tail - 1;
    dq->size--;
    
    deque_shrink_if_needed(dq);
    
    STK_LOG_DEBUG("Deque pop_back: size=%zu", dq->size);
    return STK_OK;
}

void* stk_deque_back(const stk_deque* dq) {
    if (!dq) {
        STK_LOG_WARN("Deque back: NULL deque pointer");
        return NULL;
    }
    if (dq->size == 0) {
        STK_LOG_WARN("Deque back: deque is empty");
        return NULL;
    }
    size_t back_idx = (dq->tail == 0) ? dq->capacity - 1 : dq->tail - 1;
    return dq->data[back_idx];
}

void* stk_deque_pop_back_value(stk_deque* dq) {
    if (!dq) {
        STK_LOG_WARN("Deque pop_back_value: NULL deque pointer");
        return NULL;
    }
    if (dq->size == 0) {
        STK_LOG_WARN("Deque pop_back_value: deque is empty");
        return NULL;
    }
    
    dq->tail = (dq->tail == 0) ? dq->capacity - 1 : dq->tail - 1;
    void* value = dq->data[dq->tail];
    dq->size--;
    
    deque_shrink_if_needed(dq);
    
    STK_LOG_DEBUG("Deque pop_back_value: size=%zu", dq->size);
    return value;
}

/* =========================================================================
 * 随机访问
 * ========================================================================= */

void* stk_deque_get(const stk_deque* dq, size_t index) {
    if (!dq) {
        STK_LOG_WARN("Deque get: NULL deque pointer");
        return NULL;
    }
    if (index >= dq->size) {
        STK_LOG_WARN("Deque get: index %zu out of range (size=%zu)", index, dq->size);
        return NULL;
    }
    return dq->data[deque_real_index(dq, index)];
}

STK_STATUS stk_deque_set(stk_deque* dq, size_t index, void* value) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque set: NULL deque pointer");
    
    if (index >= dq->size) {
        STK_LOG_ERROR("Deque set: index %zu out of range (size=%zu)", index, dq->size);
        return STK_ERANGE;
    }
    
    dq->data[deque_real_index(dq, index)] = value;
    return STK_OK;
}

/* =========================================================================
 * 插入/删除
 * ========================================================================= */

STK_STATUS stk_deque_insert(stk_deque* dq, size_t index, void* value) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque insert: NULL deque pointer");
    
    if (index > dq->size) {
        STK_LOG_ERROR("Deque insert: index %zu out of range (size=%zu)", index, dq->size);
        return STK_ERANGE;
    }
    
    /* 头尾插入使用 O(1) 操作 */
    if (index == 0) {
        return stk_deque_push_front(dq, value);
    }
    if (index == dq->size) {
        return stk_deque_push_back(dq, value);
    }
    
    /* 中间插入：选择移动较少的一端 */
    if (index < dq->size / 2) {
        /* 移动头部元素 */
        stk_deque_push_front(dq, dq->data[dq->head]);
        for (size_t i = 0; i < index; i++) {
            size_t src = deque_real_index(dq, i + 1);
            size_t dst = deque_real_index(dq, i);
            dq->data[dst] = dq->data[src];
        }
        size_t pos = deque_real_index(dq, index);
        dq->data[pos] = value;
    } else {
        /* 移动尾部元素 */
        stk_deque_push_back(dq, dq->data[deque_real_index(dq, dq->size - 1)]);
        for (size_t i = dq->size - 2; i > index; i--) {
            size_t src = deque_real_index(dq, i - 1);
            size_t dst = deque_real_index(dq, i);
            dq->data[dst] = dq->data[src];
        }
        size_t pos = deque_real_index(dq, index);
        dq->data[pos] = value;
    }
    
    STK_LOG_DEBUG("Deque insert: inserted at index %zu, size=%zu", index, dq->size);
    return STK_OK;
}

STK_STATUS stk_deque_erase(stk_deque* dq, size_t index) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque erase: NULL deque pointer");
    
    if (index >= dq->size) {
        STK_LOG_ERROR("Deque erase: index %zu out of range (size=%zu)", index, dq->size);
        return STK_ERANGE;
    }
    
    /* 头尾删除使用 O(1) 操作 */
    if (index == 0) {
        return stk_deque_pop_front(dq);
    }
    if (index == dq->size - 1) {
        return stk_deque_pop_back(dq);
    }
    
    /* 中间删除：选择移动较少的一端 */
    if (index < dq->size / 2) {
        for (size_t i = index; i > 0; i--) {
            size_t src = deque_real_index(dq, i - 1);
            size_t dst = deque_real_index(dq, i);
            dq->data[dst] = dq->data[src];
        }
        stk_deque_pop_front(dq);
    } else {
        for (size_t i = index; i < dq->size - 1; i++) {
            size_t src = deque_real_index(dq, i + 1);
            size_t dst = deque_real_index(dq, i);
            dq->data[dst] = dq->data[src];
        }
        stk_deque_pop_back(dq);
    }
    
    STK_LOG_DEBUG("Deque erase: erased index %zu, size=%zu", index, dq->size);
    return STK_OK;
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_deque_size(const stk_deque* dq) {
    return dq ? dq->size : 0;
}

bool stk_deque_empty(const stk_deque* dq) {
    return dq ? dq->size == 0 : true;
}

size_t stk_deque_capacity(const stk_deque* dq) {
    return dq ? dq->capacity : 0;
}

/* =========================================================================
 * 内存管理
 * ========================================================================= */

STK_STATUS stk_deque_reserve(stk_deque* dq, size_t capacity) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque reserve: NULL deque pointer");
    
    if (capacity <= dq->capacity) {
        return STK_OK;
    }
    
    void** new_data = (void**)malloc(STK_DEQUE_ELEMENT_SIZE * capacity);
    if (!new_data) {
        STK_LOG_ERROR("Deque reserve: malloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    for (size_t i = 0; i < dq->size; i++) {
        new_data[i] = dq->data[deque_real_index(dq, i)];
    }
    
    free(dq->data);
    dq->data = new_data;
    dq->head = 0;
    dq->tail = dq->size;
    dq->capacity = capacity;
    
    STK_LOG_DEBUG("Deque reserve: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_deque_shrink(stk_deque* dq) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque shrink: NULL deque pointer");
    
    if (dq->size == dq->capacity) {
        return STK_OK;
    }
    
    if (dq->size == 0) {
        free(dq->data);
        dq->data = NULL;
        dq->head = dq->tail = dq->capacity = 0;
    } else {
        void** new_data = (void**)malloc(STK_DEQUE_ELEMENT_SIZE * dq->size);
        if (!new_data) {
            STK_LOG_WARN("Deque shrink: malloc failed");
            return STK_OK;
        }
        
        for (size_t i = 0; i < dq->size; i++) {
            new_data[i] = dq->data[deque_real_index(dq, i)];
        }
        
        free(dq->data);
        dq->data = new_data;
        dq->head = 0;
        dq->tail = dq->size;
        dq->capacity = dq->size;
    }
    
    STK_LOG_DEBUG("Deque shrink: new capacity=%zu", dq->capacity);
    return STK_OK;
}

STK_STATUS stk_deque_clear(stk_deque* dq) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque clear: NULL deque pointer");
    
    dq->head = 0;
    dq->tail = 0;
    dq->size = 0;
    
    STK_LOG_DEBUG("Deque cleared");
    return STK_OK;
}

STK_STATUS stk_deque_swap(stk_deque* a, stk_deque* b) {
    STK_RETURN_IF(!a, STK_EINVAL, "Deque swap: NULL first deque pointer");
    STK_RETURN_IF(!b, STK_EINVAL, "Deque swap: NULL second deque pointer");
    
    stk_deque tmp = *a;
    *a = *b;
    *b = tmp;
    
    STK_LOG_DEBUG("Deque swapped");
    return STK_OK;
}

/* =========================================================================
 * 遍历操作
 * ========================================================================= */

STK_STATUS stk_deque_foreach(const stk_deque* dq,
                               bool (*fn)(void* data, void* user_data),
                               void* user_data) {
    STK_RETURN_IF(!dq, STK_EINVAL, "Deque foreach: NULL deque pointer");
    STK_RETURN_IF(!fn, STK_EINVAL, "Deque foreach: NULL callback function");
    
    for (size_t i = 0; i < dq->size; i++) {
        if (!fn(dq->data[deque_real_index(dq, i)], user_data)) {
            break;
        }
    }
    return STK_OK;
}