#include "stk/def.h"
#include "stk/core/preset.h"
#include "stk/core/vector.h"

#define STK_VECTOR_DEFAULT_CAPACITY 16
#define STK_VECTOR_GROW_FACTOR 2
#define STK_VECTOR_ELEMENT_SIZE sizeof(void*)

#define STK_VECTOR_USE_MEMSET 1
#define STK_VECTOR_USE_REALLOC 1

static void grow(stk_vector* v) {
    size_t new_cap = v->capacity == 0 ? STK_VECTOR_DEFAULT_CAPACITY : v->capacity * STK_VECTOR_GROW_FACTOR;
    void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

void stk_vector_init(stk_vector* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void stk_vector_free(stk_vector* v) {
    if (v->data) free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void stk_vector_push(stk_vector* v, void* val) {  /* store as void* */
    if (!v) return;
    if (v->size >= v->capacity) grow(v);
    ((void**)v->data)[v->size++] = val;
}

void stk_vector_pop(stk_vector* v) {
    if (!v || v->size == 0) return;
    v->size--;
}

void* stk_vector_get(stk_vector* v, size_t idx) {  /* store as void* */
    if (!v || idx >= v->size) {
        fprintf(stderr, "stk_vector_get: index %zu out of range\n", idx);
        exit(1);
    }
    return ((void**)v->data)[idx];
}

void** stk_vector_at(stk_vector* v, size_t idx) {  /* store as void** */
    if (!v || idx >= v->size) return NULL;
    return &((void**)v->data)[idx];
}

void stk_vector_set(stk_vector* v, size_t idx, void* val) {  /* store as void* */
    if (!v || idx >= v->size) return;
    ((void**)v->data)[idx] = val;
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

void stk_vector_reserve(stk_vector* v, size_t cap) {
    if (!v || cap <= v->capacity) return;
    void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = cap;
}

void stk_vector_resize(stk_vector* v, size_t len) {
    if (!v) return;
    if (len > v->capacity) {
        void** new_data = realloc(v->data, STK_VECTOR_ELEMENT_SIZE * len);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = len;
    }
    if (len > v->size) {
        memset((void**)v->data + v->size, 0, STK_VECTOR_ELEMENT_SIZE * (len - v->size));
    }
    v->size = len;
}

void stk_vector_shrink(stk_vector* v) {
    if (!v || v->size == v->capacity) return;
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
}

void stk_vector_clear(stk_vector* v) {
    if (!v) return;
    v->size = 0;
}

void stk_vector_fill(stk_vector* v, void* val) {  /* store as void* */
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        ((void**)v->data)[i] = val;
    }
}

void stk_vector_insert(stk_vector* v, size_t idx, void* val) {  // changed to void*
    if (!v || idx > v->size) return;
    if (v->size >= v->capacity) grow(v);
    if (idx < v->size) {
        memmove(&((void**)v->data)[idx + 1], &((void**)v->data)[idx], 
                STK_VECTOR_ELEMENT_SIZE * (v->size - idx));
    }
    ((void**)v->data)[idx] = val;
    v->size++;
}

void stk_vector_erase(stk_vector* v, size_t idx) {
    if (!v || idx >= v->size) return;
    if (idx < v->size - 1) {
        memmove(&((void**)v->data)[idx], &((void**)v->data)[idx + 1], 
                STK_VECTOR_ELEMENT_SIZE * (v->size - idx - 1));
    }
    v->size--;
}

void stk_vector_swap(stk_vector* a, stk_vector* b) {
    if (!a || !b) return;
    stk_vector tmp = *a;
    *a = *b;
    *b = tmp;
}

void* stk_vector_front(stk_vector* v) {
    if (!v || v->size == 0) return NULL;
    return &((void**)v->data)[0];
}

void* stk_vector_back(stk_vector* v) {
    if (!v || v->size == 0) return NULL;
    return &((void**)v->data)[v->size - 1];
}

static int cmp_voidptr(const void* a, const void* b) {
    const void* pa = *(const void* const*)a;
    const void* pb = *(const void* const*)b;
    return (pa > pb) - (pa < pb);
}

void stk_vector_sort(stk_vector* v) {
    if (!v || v->size < 2) return;
    qsort(v->data, v->size, STK_VECTOR_ELEMENT_SIZE, cmp_voidptr);
}

void stk_vector_reverse(stk_vector* v) {
    if (!v || v->size < 2) return;
    for (size_t i = 0; i < v->size / 2; i++) {
        void* tmp = ((void**)v->data)[i];
        ((void**)v->data)[i] = ((void**)v->data)[v->size - 1 - i];
        ((void**)v->data)[v->size - 1 - i] = tmp;
    }
}


// int vector
STK_VECTOR_IMPLEMENT_BASIC(int, int)

// float vector
STK_VECTOR_IMPLEMENT_BASIC(float, flt)

// double vector
STK_VECTOR_IMPLEMENT_BASIC(double, dub)


