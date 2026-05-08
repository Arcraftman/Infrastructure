// _Vector_Double.c
#include "stk_vector_double.h"


static void VecDouble_Grow(VecDouble* v) {
    size_t new_cap = v->capacity == 0 ? VECTOR_DEFAULT_CAPACITY : v->capacity * VECTOR_GROW_FACTOR;
    double* new_data = (double*)realloc(v->data, sizeof(double) * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

STK_API void VecDouble_Init(VecDouble* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecDouble_Destroy(VecDouble* v) {
    if (v->data) {
        free(v->data);
        v->data = NULL;
    }
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecDouble_PushBack(VecDouble* v, double value) {
    if (!v) return;
    if (v->size >= v->capacity) {
        VecDouble_Grow(v);
    }
    v->data[v->size++] = value;
}

STK_API void VecDouble_PopBack(VecDouble* v) {
    if (!v || v->size == 0) return;
    v->size--;
}

STK_API double VecDouble_At(VecDouble* v, size_t index) {
    if (!v || index >= v->size) {
        fprintf(stderr, "VecDouble_At: index %zu out of range (size=%zu)\n", index, v ? v->size : 0);
        exit(1);
    }
    return v->data[index];
}

STK_API size_t VecDouble_Size(VecDouble* v) {
    return v ? v->size : 0;
}

STK_API int VecDouble_Empty(VecDouble* v) {
    return v ? v->size == 0 : 1;
}

STK_API void VecDouble_Resize(VecDouble* v, size_t new_size) {
    if (!v) return;
    if (new_size > v->capacity) {
        double* new_data = (double*)realloc(v->data, sizeof(double) * new_size);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = new_size;
    }
    if (new_size > v->size) {
        memset(v->data + v->size, 0, sizeof(double) * (new_size - v->size));
    }
    v->size = new_size;
}

STK_API void VecDouble_Fill(VecDouble* v, double value) {
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        v->data[i] = value;
    }
}