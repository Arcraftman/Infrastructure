#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/stack.h"

/* =========================================================================
 * 内部常量定义
 * ========================================================================= */

#define STK_STACK_DEFAULT_CAPACITY 16
#define STK_STACK_GROW_FACTOR 2
#define STK_STACK_SHRINK_FACTOR 2
#define STK_STACK_ELEMENT_SIZE sizeof(void*)

/* =========================================================================
 * 内部辅助函数
 * ========================================================================= */

/* 扩容 - 容量翻倍 */
static bool stack_grow(stk_stack* s) {
    size_t new_capacity;
    
    if (s->capacity == 0) {
        new_capacity = STK_STACK_DEFAULT_CAPACITY;
    } else if (s->capacity < (SIZE_MAX / STK_STACK_GROW_FACTOR)) {
        new_capacity = s->capacity * STK_STACK_GROW_FACTOR;
    } else {
        /* 已达到最大值，无法扩容 */
        STK_LOG_ERROR("Stack grow: cannot grow beyond SIZE_MAX");
        return false;
    }
    
    void** new_data = realloc(s->data, STK_STACK_ELEMENT_SIZE * new_capacity);
    if (!new_data) {
        STK_LOG_ERROR("Stack grow: realloc failed (size=%zu)", new_capacity);
        return false;
    }
    
    s->data = new_data;
    s->capacity = new_capacity;
    
    STK_LOG_DEBUG("Stack grew: capacity=%zu -> %zu", 
                  s->capacity / STK_STACK_GROW_FACTOR, s->capacity);
    return true;
}

/* 缩容 - 当使用率低于 1/4 时减半容量 */
static bool stack_shrink_if_needed(stk_stack* s) {
    if (s->capacity <= STK_STACK_DEFAULT_CAPACITY) {
        return true;  /* 不低于默认容量 */
    }
    
    if (s->size > s->capacity / STK_STACK_SHRINK_FACTOR) {
        return true;  /* 使用率足够高，不需要缩容 */
    }
    
    size_t new_capacity = s->capacity / STK_STACK_GROW_FACTOR;
    if (new_capacity < STK_STACK_DEFAULT_CAPACITY) {
        new_capacity = STK_STACK_DEFAULT_CAPACITY;
    }
    
    void** new_data = realloc(s->data, STK_STACK_ELEMENT_SIZE * new_capacity);
    if (new_data) {
        s->data = new_data;
        s->capacity = new_capacity;
        STK_LOG_DEBUG("Stack shrunk: capacity=%zu -> %zu", 
                      s->capacity * STK_STACK_GROW_FACTOR, s->capacity);
    }
    /* 缩容失败不影响栈的正常使用 */
    
    return true;
}

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_stack_init(stk_stack* s) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack init: NULL stack pointer");
    
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
    
    STK_LOG_DEBUG("Stack initialized");
    return STK_OK;
}

STK_STATUS stk_stack_init_with_capacity(stk_stack* s, size_t capacity) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack init_with_capacity: NULL stack pointer");
    
    if (capacity == 0) {
        s->data = NULL;
        s->size = 0;
        s->capacity = 0;
        return STK_OK;
    }
    
    s->data = (void**)malloc(STK_STACK_ELEMENT_SIZE * capacity);
    if (!s->data) {
        STK_LOG_ERROR("Stack init_with_capacity: malloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    s->size = 0;
    s->capacity = capacity;
    
    STK_LOG_DEBUG("Stack init_with_capacity: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_stack_free(stk_stack* s) {
    if (!s) {
        STK_LOG_WARN("Stack free: NULL stack pointer");
        return STK_EINVAL;
    }
    
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
    s->size = 0;
    s->capacity = 0;
    
    STK_LOG_DEBUG("Stack freed");
    return STK_OK;
}

/* =========================================================================
 * 核心操作
 * ========================================================================= */

STK_STATUS stk_stack_push(stk_stack* s, void* value) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack push: NULL stack pointer");
    
    /* 检查是否需要扩容 */
    if (s->size >= s->capacity) {
        if (!stack_grow(s)) {
            STK_LOG_ERROR("Stack push: failed to grow stack");
            return STK_ENOMEM;
        }
    }
    
    s->data[s->size++] = value;
    
    STK_LOG_DEBUG("Stack push: size=%zu, capacity=%zu", s->size, s->capacity);
    return STK_OK;
}

STK_STATUS stk_stack_pop(stk_stack* s) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack pop: NULL stack pointer");
    STK_RETURN_IF(s->size == 0, STK_EMPTY, "Stack pop: stack is empty");
    
    s->size--;
    
    /* 尝试缩容 */
    stack_shrink_if_needed(s);
    
    STK_LOG_DEBUG("Stack pop: size=%zu, capacity=%zu", s->size, s->capacity);
    return STK_OK;
}

void* stk_stack_top(const stk_stack* s) {
    if (!s) {
        STK_LOG_WARN("Stack top: NULL stack pointer");
        return NULL;
    }
    if (s->size == 0) {
        STK_LOG_WARN("Stack top: stack is empty");
        return NULL;
    }
    return s->data[s->size - 1];
}

void* stk_stack_pop_value(stk_stack* s) {
    if (!s) {
        STK_LOG_WARN("Stack pop_value: NULL stack pointer");
        return NULL;
    }
    if (s->size == 0) {
        STK_LOG_WARN("Stack pop_value: stack is empty");
        return NULL;
    }
    
    void* value = s->data[--s->size];
    
    /* 尝试缩容 */
    stack_shrink_if_needed(s);
    
    STK_LOG_DEBUG("Stack pop_value: size=%zu", s->size);
    return value;
}

STK_STATUS stk_stack_clear(stk_stack* s) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack clear: NULL stack pointer");
    
    s->size = 0;
    /* 注意：不清空内存，下次 push 会直接覆盖 */
    
    STK_LOG_DEBUG("Stack cleared: size=%zu, capacity=%zu", s->size, s->capacity);
    return STK_OK;
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_stack_size(const stk_stack* s) {
    return s ? s->size : 0;
}

bool stk_stack_empty(const stk_stack* s) {
    return s ? s->size == 0 : true;
}

size_t stk_stack_capacity(const stk_stack* s) {
    return s ? s->capacity : 0;
}

/* =========================================================================
 * 内存管理
 * ========================================================================= */

STK_STATUS stk_stack_reserve(stk_stack* s, size_t capacity) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack reserve: NULL stack pointer");
    
    if (capacity <= s->capacity) {
        return STK_OK;  /* 已有足够空间 */
    }
    
    void** new_data = realloc(s->data, STK_STACK_ELEMENT_SIZE * capacity);
    if (!new_data) {
        STK_LOG_ERROR("Stack reserve: realloc failed (size=%zu)", capacity);
        return STK_ENOMEM;
    }
    
    s->data = new_data;
    s->capacity = capacity;
    
    STK_LOG_DEBUG("Stack reserve: capacity=%zu", capacity);
    return STK_OK;
}

STK_STATUS stk_stack_shrink(stk_stack* s) {
    STK_RETURN_IF(!s, STK_EINVAL, "Stack shrink: NULL stack pointer");
    
    if (s->size == s->capacity) {
        return STK_OK;  /* 正好合适 */
    }
    
    if (s->size == 0) {
        /* 完全释放内存 */
        free(s->data);
        s->data = NULL;
        s->capacity = 0;
    } else {
        /* 收缩到刚好容纳当前元素 */
        void** new_data = realloc(s->data, STK_STACK_ELEMENT_SIZE * s->size);
        if (new_data) {
            s->data = new_data;
            s->capacity = s->size;
        }
    }
    
    STK_LOG_DEBUG("Stack shrink: new capacity=%zu", s->capacity);
    return STK_OK;
}

STK_STATUS stk_stack_swap(stk_stack* a, stk_stack* b) {
    STK_RETURN_IF(!a, STK_EINVAL, "Stack swap: NULL first stack pointer");
    STK_RETURN_IF(!b, STK_EINVAL, "Stack swap: NULL second stack pointer");
    
    stk_stack tmp = *a;
    *a = *b;
    *b = tmp;
    
    STK_LOG_DEBUG("Stacks swapped");
    return STK_OK;
}