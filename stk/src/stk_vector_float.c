// _Vector_Float.c
#include "stk_vector_float.h"



static void VecFloat_Grow(VecFloat* v) {
    size_t new_cap = v->capacity == 0 ? VECTOR_DEFAULT_CAPACITY : v->capacity * VECTOR_GROW_FACTOR;
    float* new_data = (float*)realloc(v->data, sizeof(float) * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

STK_API void VecFloat_Init(VecFloat* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecFloat_Destroy(VecFloat* v) {
    if (v->data) {
        free(v->data);
        v->data = NULL;
    }
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecFloat_PushBack(VecFloat* v, float value) {
    if (!v) return;
    if (v->size >= v->capacity) {
        VecFloat_Grow(v);
    }
    v->data[v->size++] = value;
}

STK_API void VecFloat_PopBack(VecFloat* v) {
    if (!v || v->size == 0) return;
    v->size--;
}

STK_API float VecFloat_At(VecFloat* v, size_t index) {
    if (!v || index >= v->size) {
        fprintf(stderr, "VecFloat_At: index %zu out of range (size=%zu)\n", index, v ? v->size : 0);
        exit(1);
    }
    return v->data[index];
}

STK_API size_t VecFloat_Size(VecFloat* v) {
    return v ? v->size : 0;
}

STK_API int VecFloat_Empty(VecFloat* v) {
    return v ? v->size == 0 : 1;
}

STK_API void VecFloat_Resize(VecFloat* v, size_t new_size) {
    if (!v) return;
    if (new_size > v->capacity) {
        float* new_data = (float*)realloc(v->data, sizeof(float) * new_size);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = new_size;
    }
    if (new_size > v->size) {
        memset(v->data + v->size, 0, sizeof(float) * (new_size - v->size));
    }
    v->size = new_size;
}

STK_API void VecFloat_Fill(VecFloat* v, float value) {
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        v->data[i] = value;
    }
}