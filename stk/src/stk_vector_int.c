// _Vector_Int.c
#include "stk_vector_int.h"

static void VecInt_Grow(VecInt* v) {
    size_t new_cap = v->capacity == 0 ? VECTOR_DEFAULT_CAPACITY : v->capacity * VECTOR_GROW_FACTOR;
    int* new_data = (int*)realloc(v->data, sizeof(int) * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

STK_API void VecInt_Init(VecInt* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecInt_Destroy(VecInt* v) {
    if (v->data) {
        free(v->data);
        v->data = NULL;
    }
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecInt_PushBack(VecInt* v, int value) {
    if (!v) return;
    if (v->size >= v->capacity) {
        VecInt_Grow(v);
    }
    v->data[v->size++] = value;
}

STK_API void VecInt_PopBack(VecInt* v) {
    if (!v || v->size == 0) return;
    v->size--;
}

STK_API int VecInt_At(VecInt* v, size_t index) {
    if (!v || index >= v->size) {
        fprintf(stderr, "VecInt_At: index %zu out of range (size=%zu)\n", index, v ? v->size : 0);
        exit(1);
    }
    return v->data[index];
}

STK_API size_t VecInt_Size(VecInt* v) {
    return v ? v->size : 0;
}

STK_API int VecInt_Empty(VecInt* v) {
    return v ? v->size == 0 : 1;
}

STK_API void VecInt_Resize(VecInt* v, size_t new_size) {
    if (!v) return;
    if (new_size > v->capacity) {
        int* new_data = (int*)realloc(v->data, sizeof(int) * new_size);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = new_size;
    }
    if (new_size > v->size) {
        memset(v->data + v->size, 0, sizeof(int) * (new_size - v->size));
    }
    v->size = new_size;
}

STK_API void VecInt_Fill(VecInt* v, int value) {
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        v->data[i] = value;
    }
}
