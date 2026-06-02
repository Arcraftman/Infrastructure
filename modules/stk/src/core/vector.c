#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/internal/vector.h"
#include "stk/core/vector.h"

#define STK_VECTOR_DEFAULT_CAPACITY 16
#define STK_VECTOR_GROW_FACTOR 2
#define STK_VECTOR_ELEMENT_SIZE sizeof(void*)

static bool grow(stk_vector* v) {
    size_t new_cap = v->capacity == 0 ? STK_VECTOR_DEFAULT_CAPACITY : v->capacity * STK_VECTOR_GROW_FACTOR;
    void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * new_cap);
    if (!new_data) {
        STK_LOG_ERROR("Vector grow: realloc failed (size=%zu)", new_cap);
        return false;
    }
    v->data = new_data;
    v->capacity = new_cap;
    STK_LOG_DEBUG("Vector grew: capacity=%zu", new_cap);
    return true;
}

STK_STATUS stk_vector_init(stk_vector* v) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector init: NULL vector pointer");
    
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
    
    STK_LOG_DEBUG("Vector initialized");
    return STK_OK;
}

STK_STATUS stk_vector_init_with_capacity(stk_vector* v, size_t cap) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector init_with_capacity: NULL vector pointer");
    
    if (cap == 0) {
        v->data = NULL;
        v->size = 0;
        v->capacity = 0;
        return STK_OK;
    }
    
    v->data = (void**)malloc(STK_VECTOR_ELEMENT_SIZE * cap);
    if (!v->data) {
        STK_LOG_ERROR("Vector init_with_capacity: malloc failed (size=%zu)", cap);
        return STK_ENOMEM;
    }
    v->size = 0;
    v->capacity = cap;
    
    STK_LOG_DEBUG("Vector init_with_capacity: capacity=%zu", cap);
    return STK_OK;
}

STK_STATUS stk_vector_free(stk_vector* v) {
    if (!v) {
        STK_LOG_WARN("Vector free: NULL vector pointer");
        return STK_EINVAL;
    }
    
    if (v->data) free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
    
    STK_LOG_DEBUG("Vector freed");
    return STK_OK;
}

STK_STATUS stk_vector_push(stk_vector* v, void* val) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector push: NULL vector pointer");
    
    if (v->size >= v->capacity) {
        if (!grow(v)) {
            STK_LOG_ERROR("Vector push: failed to grow vector");
            return STK_ENOMEM;
        }
    }
    ((void**)v->data)[v->size++] = val;
    
    STK_LOG_DEBUG("Vector push: size=%zu", v->size);
    return STK_OK;
}

STK_STATUS stk_vector_pop(stk_vector* v) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector pop: NULL vector pointer");
    STK_RETURN_IF(v->size == 0, STK_EMPTY, "Vector pop: vector is empty");
    
    v->size--;
    
    STK_LOG_DEBUG("Vector pop: size=%zu", v->size);
    return STK_OK;
}

void* stk_vector_get(stk_vector* v, size_t idx) {
    if (!v) {
        STK_LOG_WARN("Vector get: NULL vector pointer");
        return NULL;
    }
    if (idx >= v->size) {
        STK_LOG_ERROR("Vector get: index %zu out of range (size=%zu)", idx, v->size);
        return NULL;
    }
    return ((void**)v->data)[idx];
}

void** stk_vector_at(stk_vector* v, size_t idx) {
    if (!v || idx >= v->size) {
        if (v) STK_LOG_WARN("Vector at: index %zu out of range", idx);
        return NULL;
    }
    return &((void**)v->data)[idx];
}

STK_STATUS stk_vector_set(stk_vector* v, size_t idx, void* val) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector set: NULL vector pointer");
    
    if (idx >= v->size) {
        STK_LOG_ERROR("Vector set: index %zu out of range (size=%zu)", idx, v->size);
        return STK_ERANGE;
    }
    ((void**)v->data)[idx] = val;
    return STK_OK;
}

size_t stk_vector_len(const stk_vector* v) {
    return v ? v->size : 0;
}

bool stk_vector_empty(const stk_vector* v) {
    return v ? v->size == 0 : true;
}

size_t stk_vector_cap(const stk_vector* v) {
    return v ? v->capacity : 0;
}

STK_STATUS stk_vector_reserve(stk_vector* v, size_t cap) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector reserve: NULL vector pointer");
    
    if (cap <= v->capacity) return STK_OK;
    
    void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * cap);
    if (!new_data) {
        STK_LOG_ERROR("Vector reserve: realloc failed (size=%zu)", cap);
        return STK_ENOMEM;
    }
    v->data = new_data;
    v->capacity = cap;
    
    STK_LOG_DEBUG("Vector reserve: capacity=%zu", cap);
    return STK_OK;
}

STK_STATUS stk_vector_resize(stk_vector* v, size_t len) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector resize: NULL vector pointer");
    
    if (len > v->capacity) {
        void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * len);
        if (!new_data) {
            STK_LOG_ERROR("Vector resize: realloc failed (size=%zu)", len);
            return STK_ENOMEM;
        }
        v->data = new_data;
        v->capacity = len;
    }
    if (len > v->size) {
        memset((void**)v->data + v->size, 0, STK_VECTOR_ELEMENT_SIZE * (len - v->size));
    }
    v->size = len;
    
    STK_LOG_DEBUG("Vector resize: size=%zu, capacity=%zu", v->size, v->capacity);
    return STK_OK;
}

STK_STATUS stk_vector_shrink(stk_vector* v) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector shrink: NULL vector pointer");
    
    if (v->size == v->capacity) return STK_OK;
    
    if (v->size == 0) {
        free(v->data);
        v->data = NULL;
        v->capacity = 0;
    } else {
        void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * v->size);
        if (new_data) {
            v->data = new_data;
            v->capacity = v->size;
        }
    }
    
    STK_LOG_DEBUG("Vector shrink: new capacity=%zu", v->capacity);
    return STK_OK;
}

STK_STATUS stk_vector_clear(stk_vector* v) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector clear: NULL vector pointer");
    
    v->size = 0;
    STK_LOG_DEBUG("Vector cleared");
    return STK_OK;
}

STK_STATUS stk_vector_fill(stk_vector* v, void* val) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector fill: NULL vector pointer");
    
    for (size_t i = 0; i < v->size; i++) {
        ((void**)v->data)[i] = val;
    }
    return STK_OK;
}

STK_STATUS stk_vector_insert(stk_vector* v, size_t idx, void* val) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector insert: NULL vector pointer");
    
    if (idx > v->size) {
        STK_LOG_ERROR("Vector insert: index %zu out of range (size=%zu)", idx, v->size);
        return STK_ERANGE;
    }
    
    if (v->size >= v->capacity && !grow(v)) {
        STK_LOG_ERROR("Vector insert: failed to grow vector");
        return STK_ENOMEM;
    }
    
    if (idx < v->size) {
        memmove(&((void**)v->data)[idx + 1], &((void**)v->data)[idx], 
                STK_VECTOR_ELEMENT_SIZE * (v->size - idx));
    }
    ((void**)v->data)[idx] = val;
    v->size++;
    
    STK_LOG_DEBUG("Vector insert: inserted at %zu, size=%zu", idx, v->size);
    return STK_OK;
}

STK_STATUS stk_vector_erase(stk_vector* v, size_t idx) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector erase: NULL vector pointer");
    
    if (idx >= v->size) {
        STK_LOG_ERROR("Vector erase: index %zu out of range (size=%zu)", idx, v->size);
        return STK_ERANGE;
    }
    
    if (idx < v->size - 1) {
        memmove(&((void**)v->data)[idx], &((void**)v->data)[idx + 1], 
                STK_VECTOR_ELEMENT_SIZE * (v->size - idx - 1));
    }
    v->size--;
    
    STK_LOG_DEBUG("Vector erase: erased index %zu, new size=%zu", idx, v->size);
    return STK_OK;
}

STK_STATUS stk_vector_swap(stk_vector* a, stk_vector* b) {
    STK_RETURN_IF(!a, STK_EINVAL, "Vector swap: NULL first vector");
    STK_RETURN_IF(!b, STK_EINVAL, "Vector swap: NULL second vector");
    
    stk_vector tmp = *a;
    *a = *b;
    *b = tmp;
    
    STK_LOG_DEBUG("Vectors swapped");
    return STK_OK;
}

void* stk_vector_front(stk_vector* v) {
    if (!v || v->size == 0) {
        if (v) STK_LOG_WARN("Vector front: vector is empty");
        return NULL;
    }
    return &((void**)v->data)[0];
}

void* stk_vector_back(stk_vector* v) {
    if (!v || v->size == 0) {
        if (v) STK_LOG_WARN("Vector back: vector is empty");
        return NULL;
    }
    return &((void**)v->data)[v->size - 1];
}

static int cmp_voidptr(const void* a, const void* b) {
    const void* pa = *(const void* const*)a;
    const void* pb = *(const void* const*)b;
    return (pa > pb) - (pa < pb);
}

STK_STATUS stk_vector_sort(stk_vector* v) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector sort: NULL vector pointer");
    
    if (v->size < 2) return STK_OK;
    
    qsort(v->data, v->size, STK_VECTOR_ELEMENT_SIZE, cmp_voidptr);
    STK_LOG_DEBUG("Vector sorted: size=%zu", v->size);
    return STK_OK;
}

STK_STATUS stk_vector_reverse(stk_vector* v) {
    STK_RETURN_IF(!v, STK_EINVAL, "Vector reverse: NULL vector pointer");
    
    if (v->size < 2) return STK_OK;
    
    for (size_t i = 0; i < v->size / 2; i++) {
        void* tmp = ((void**)v->data)[i];
        ((void**)v->data)[i] = ((void**)v->data)[v->size - 1 - i];
        ((void**)v->data)[v->size - 1 - i] = tmp;
    }
    
    STK_LOG_DEBUG("Vector reversed");
    return STK_OK;
}