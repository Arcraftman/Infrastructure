#include "vector.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void grow(vector* v) {
    size_t new_cap = v->capacity == 0 ? VECTOR_DEFAULT_CAPACITY : v->capacity * VECTOR_GROW_FACTOR;
    void** new_data = realloc(v->data, VECTOR_ELEMENT_SIZE * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

void vector_init(vector* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vector_free(vector* v) {
    if (v->data) free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vector_push(vector* v, void* val) {  /* store as void* */
    if (!v) return;
    if (v->size >= v->capacity) grow(v);
    ((void**)v->data)[v->size++] = val;
}

void vector_pop(vector* v) {
    if (!v || v->size == 0) return;
    v->size--;
}

void* vector_get(vector* v, size_t idx) {  /* store as void* */
    if (!v || idx >= v->size) {
        fprintf(stderr, "stk_vector_get: index %zu out of range\n", idx);
        exit(1);
    }
    return ((void**)v->data)[idx];
}

void** vector_at(vector* v, size_t idx) {  /* store as void** */
    if (!v || idx >= v->size) return NULL;
    return &((void**)v->data)[idx];
}

void vector_set(vector* v, size_t idx, void* val) {  /* store as void* */
    if (!v || idx >= v->size) return;
    ((void**)v->data)[idx] = val;
}

size_t vector_len(vector* v) {
    return v ? v->size : 0;
}

bool vector_empty(vector* v) {
    return v ? v->size == 0 : true;
}

size_t vector_cap(vector* v) {
    return v ? v->capacity : 0;
}

void vector_reserve(vector* v, size_t cap) {
    if (!v || cap <= v->capacity) return;
    void** new_data = realloc(v->data, VECTOR_ELEMENT_SIZE * cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = cap;
}

void vector_resize(vector* v, size_t len) {
    if (!v) return;
    if (len > v->capacity) {
        void** new_data = realloc(v->data, VECTOR_ELEMENT_SIZE * len);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = len;
    }
    if (len > v->size) {
        memset((void**)v->data + v->size, 0, VECTOR_ELEMENT_SIZE * (len - v->size));
    }
    v->size = len;
}

void vector_shrink(vector* v) {
    if (!v || v->size == v->capacity) return;
    if (v->size == 0) {
        free(v->data);
        v->data = NULL;
        v->capacity = 0;
    } else {
        void** new_data = realloc(v->data, VECTOR_ELEMENT_SIZE * v->size);
        if (new_data) {
            v->data = new_data;
            v->capacity = v->size;
        }
    }
}

void vector_clear(vector* v) {
    if (!v) return;
    v->size = 0;
}

void vector_fill(vector* v, void* val) {  /* store as void* */
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        ((void**)v->data)[i] = val;
    }
}

void vector_insert(vector* v, size_t idx, void* val) {  // changed to void*
    if (!v || idx > v->size) return;
    if (v->size >= v->capacity) grow(v);
    if (idx < v->size) {
        memmove(&((void**)v->data)[idx + 1], &((void**)v->data)[idx], 
                VECTOR_ELEMENT_SIZE * (v->size - idx));
    }
    ((void**)v->data)[idx] = val;
    v->size++;
}

void vector_erase(vector* v, size_t idx) {
    if (!v || idx >= v->size) return;
    if (idx < v->size - 1) {
        memmove(&((void**)v->data)[idx], &((void**)v->data)[idx + 1], 
                VECTOR_ELEMENT_SIZE * (v->size - idx - 1));
    }
    v->size--;
}

void vector_swap(vector* a, vector* b) {
    if (!a || !b) return;
    vector tmp = *a;
    *a = *b;
    *b = tmp;
}

void* vector_front(vector* v) {
    if (!v || v->size == 0) return NULL;
    return &((void**)v->data)[0];
}

void* vector_back(vector* v) {
    if (!v || v->size == 0) return NULL;
    return &((void**)v->data)[v->size - 1];
}

static int cmp_voidptr(const void* a, const void* b) {
    const void* pa = *(const void* const*)a;
    const void* pb = *(const void* const*)b;
    return (pa > pb) - (pa < pb);
}

void vector_sort(vector* v) {
    if (!v || v->size < 2) return;
    qsort(v->data, v->size, VECTOR_ELEMENT_SIZE, cmp_voidptr);
}

void vector_reverse(vector* v) {
    if (!v || v->size < 2) return;
    for (size_t i = 0; i < v->size / 2; i++) {
        void* tmp = ((void**)v->data)[i];
        ((void**)v->data)[i] = ((void**)v->data)[v->size - 1 - i];
        ((void**)v->data)[v->size - 1 - i] = tmp;
    }
}


// int vector
VECTOR_IMPLEMENT_BASIC(int, int)

// float vector
VECTOR_IMPLEMENT_BASIC(float, flt)

// double vector
VECTOR_IMPLEMENT_BASIC(double, dub)

// str vector - requires correct comparison function signature
static int str_compare(const void* a, const void* b) {
    const str* sa = (str*)a;
    const str* sb = (str*)b;
    return str_cmp(sa, sb);
}

VECTOR_IMPLEMENT_STRUCT(str, str, str_compare)