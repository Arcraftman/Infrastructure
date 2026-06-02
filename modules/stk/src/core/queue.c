#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/queue.h"

/* =========================================================================
 * 内部常量定义
 * ========================================================================= */

#define STK_QUEUE_DEFAULT_CAPACITY 16
#define STK_QUEUE_GROW_FACTOR 2
#define STK_QUEUE_SHRINK_FACTOR 4      /* 当使用率低于 1/4 时缩容 */
#define STK_QUEUE_ELEMENT_SIZE sizeof(void*)

/* =========================================================================
 * 内部辅助函数
 * ========================================================================= */

/* 获取下一个索引（循环） */
static inline size_t queue_next_index(const stk_queue* q, size_t idx) {
    return (idx + 1) % q->capacity;
}

/* 扩容 - 容量翻倍，并重新排列元素 */
static bool queue_grow(stk_queue* q) {
    size_t new_capacity;
    
    if (q->capacity == 0) {
        new_capacity = STK_QUEUE_DEFAULT_CAPACITY;
    } else if (q->capacity < (SIZE_MAX / STK_QUEUE_GROW_FACTOR)) {
        new_capacity = q->capacity * STK_QUEUE_GROW_FACTOR;
    } else {
        STK_LOG_ERROR("Queue grow: cannot grow beyond SIZE_MAX");
        return false;
    }
    
    void** new_data = (void**)malloc(STK_QUEUE_ELEMENT_SIZE * new_capacity);
    if (!new_data) {
        STK_LOG_ERROR("Queue grow: malloc failed (size=%zu)", new_capacity);
        return false;
    }
    
    /* 重新排列元素到新数组的头部（使元素连续） */
    for (size_t i = 0; i < q->size; i++) {
        new_data[i] = q->data[(q->head + i) % q->capacity];
    }
    
    free(q->data);
    q->data = new_data;
    q->head = 0;
    q->tail = q->size;
    q->capacity = new_capacity;
    
    STK_LOG_DEBUG("Queue grew: capacity=%zu -> %zu, size=%zu", 
                  new_capacity / STK_QUEUE_GROW_FACTOR, new_capacity, q->size);
    return true;
}

/* 缩容 - 当使用率低于 1/4 时减半容量 */
static bool queue_shrink_if_needed(stk_queue* q) {
    if (q->capacity <= STK_QUEUE_DEFAULT_CAPACITY) {
        return true;  /* 不低于默认容量 */
    }
    
    /* 使用率 = size / capacity */
    if (q->size > q->capacity / STK_QUEUE_SHRINK_FACTOR) {
        return true;  /* 使用率足够高，不需要缩容 */
    }
    
    size_t new_capacity = q->capacity / STK_QUEUE_GROW_FACTOR;
    if (new_capacity < STK_QUEUE_DEFAULT_CAPACITY) {
        new_capacity = STK_QUEUE_DEFAULT_CAPACITY;
    }
    
    void** new_data = (void**)malloc(STK_QUEUE_ELEMENT_SIZE * new_capacity);
    if (!new_data) {
        /* 缩容失败不影响队列的正常使用 */
        STK_LOG_WARN("Queue shrink: malloc failed, keeping current capacity");
        return true;
    }
    
    /* 重新排列元素 */
    for (size_t i = 0; i < q->size; i++) {
        new_data[i] = q->data[(q->head + i) % q->capacity];
    }
    
    free(q->data);
    q->data = new_data;
    q->head = 0;
    q->tail = q->size;
    q->capacity = new_capacity;
    
    STK_LOG_DEBUG("Queue shrunk: capacity=%zu -> %zu, size=%zu", 
                  new_capacity * STK_QUEUE_GROW_FACTOR, new_capacity, q->size);
    return true;
}

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_queue_init(stk_queue* q) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue init: NULL queue pointer");
    
    q->data = NULL;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->capacity = 0;
    
    STK_LOG_DEBUG("Queue initialized");
    return STK_OK;
}

STK_STATUS stk_queue_init_with_capacity(stk_queue* q, size_t capacity) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue init_with_capacity: NULL queue pointer");
    
    if (capacity == 0) {
        q->data = NULL;
        q->head = q->tail = q->size = q->capacity = 0;
        return STK_OK;
    }
    
    q->data = (void**)malloc(STK_QUEUE_ELEMENT_SIZE * capacity);
    if (!q->data) {
        STK_LOG_ERROR("Queue init_with_capacity: malloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->capacity = capacity;
    
    STK_LOG_DEBUG("Queue init_with_capacity: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_queue_free(stk_queue* q) {
    if (!q) {
        STK_LOG_WARN("Queue free: NULL queue pointer");
        return STK_EINVAL;
    }
    
    if (q->data) {
        free(q->data);
        q->data = NULL;
    }
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->capacity = 0;
    
    STK_LOG_DEBUG("Queue freed");
    return STK_OK;
}

/* =========================================================================
 * 核心操作
 * ========================================================================= */

STK_STATUS stk_queue_enqueue(stk_queue* q, void* value) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue enqueue: NULL queue pointer");
    
    /* 检查是否需要扩容 */
    if (q->size >= q->capacity) {
        if (!queue_grow(q)) {
            STK_LOG_ERROR("Queue enqueue: failed to grow queue");
            return STK_ENOMEM;
        }
    }
    
    q->data[q->tail] = value;
    q->tail = queue_next_index(q, q->tail);
    q->size++;
    
    STK_LOG_DEBUG("Queue enqueue: size=%zu, capacity=%zu", q->size, q->capacity);
    return STK_OK;
}

STK_STATUS stk_queue_dequeue(stk_queue* q) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue dequeue: NULL queue pointer");
    STK_RETURN_IF(q->size == 0, STK_EMPTY, "Queue dequeue: queue is empty");
    
    q->head = queue_next_index(q, q->head);
    q->size--;
    
    /* 尝试缩容 */
    queue_shrink_if_needed(q);
    
    STK_LOG_DEBUG("Queue dequeue: size=%zu, capacity=%zu", q->size, q->capacity);
    return STK_OK;
}

void* stk_queue_front(const stk_queue* q) {
    if (!q) {
        STK_LOG_WARN("Queue front: NULL queue pointer");
        return NULL;
    }
    if (q->size == 0) {
        STK_LOG_WARN("Queue front: queue is empty");
        return NULL;
    }
    return q->data[q->head];
}

void* stk_queue_back(const stk_queue* q) {
    if (!q) {
        STK_LOG_WARN("Queue back: NULL queue pointer");
        return NULL;
    }
    if (q->size == 0) {
        STK_LOG_WARN("Queue back: queue is empty");
        return NULL;
    }
    /* 获取尾索引（tail 指向下一个空位，所以实际尾元素是 tail-1） */
    size_t back_idx = (q->tail == 0) ? q->capacity - 1 : q->tail - 1;
    return q->data[back_idx];
}

void* stk_queue_dequeue_value(stk_queue* q) {
    if (!q) {
        STK_LOG_WARN("Queue dequeue_value: NULL queue pointer");
        return NULL;
    }
    if (q->size == 0) {
        STK_LOG_WARN("Queue dequeue_value: queue is empty");
        return NULL;
    }
    
    void* value = q->data[q->head];
    q->head = queue_next_index(q, q->head);
    q->size--;
    
    /* 尝试缩容 */
    queue_shrink_if_needed(q);
    
    STK_LOG_DEBUG("Queue dequeue_value: size=%zu", q->size);
    return value;
}

STK_STATUS stk_queue_clear(stk_queue* q) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue clear: NULL queue pointer");
    
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    /* 注意：不清空内存，下次入队会直接覆盖 */
    
    STK_LOG_DEBUG("Queue cleared: size=%zu, capacity=%zu", q->size, q->capacity);
    return STK_OK;
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_queue_size(const stk_queue* q) {
    return q ? q->size : 0;
}

bool stk_queue_empty(const stk_queue* q) {
    return q ? q->size == 0 : true;
}

size_t stk_queue_capacity(const stk_queue* q) {
    return q ? q->capacity : 0;
}

/* =========================================================================
 * 内存管理
 * ========================================================================= */

STK_STATUS stk_queue_reserve(stk_queue* q, size_t capacity) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue reserve: NULL queue pointer");
    
    if (capacity <= q->capacity) {
        return STK_OK;
    }
    
    void** new_data = (void**)malloc(STK_QUEUE_ELEMENT_SIZE * capacity);
    if (!new_data) {
        STK_LOG_ERROR("Queue reserve: malloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    /* 重新排列元素 */
    for (size_t i = 0; i < q->size; i++) {
        new_data[i] = q->data[(q->head + i) % q->capacity];
    }
    
    free(q->data);
    q->data = new_data;
    q->head = 0;
    q->tail = q->size;
    q->capacity = capacity;
    
    STK_LOG_DEBUG("Queue reserve: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_queue_shrink(stk_queue* q) {
    STK_RETURN_IF(!q, STK_EINVAL, "Queue shrink: NULL queue pointer");
    
    if (q->size == q->capacity) {
        return STK_OK;
    }
    
    if (q->size == 0) {
        /* 完全释放内存 */
        free(q->data);
        q->data = NULL;
        q->head = 0;
        q->tail = 0;
        q->capacity = 0;
    } else {
        /* 收缩到刚好容纳当前元素 */
        void** new_data = (void**)malloc(STK_QUEUE_ELEMENT_SIZE * q->size);
        if (!new_data) {
            STK_LOG_WARN("Queue shrink: malloc failed, keeping current capacity");
            return STK_OK;
        }
        
        for (size_t i = 0; i < q->size; i++) {
            new_data[i] = q->data[(q->head + i) % q->capacity];
        }
        
        free(q->data);
        q->data = new_data;
        q->head = 0;
        q->tail = q->size;
        q->capacity = q->size;
    }
    
    STK_LOG_DEBUG("Queue shrink: new capacity=%zu", q->capacity);
    return STK_OK;
}

STK_STATUS stk_queue_swap(stk_queue* a, stk_queue* b) {
    STK_RETURN_IF(!a, STK_EINVAL, "Queue swap: NULL first queue pointer");
    STK_RETURN_IF(!b, STK_EINVAL, "Queue swap: NULL second queue pointer");
    
    stk_queue tmp = *a;
    *a = *b;
    *b = tmp;
    
    STK_LOG_DEBUG("Queues swapped");
    return STK_OK;
}