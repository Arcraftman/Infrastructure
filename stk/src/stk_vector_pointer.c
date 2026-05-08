// _Vector_Pointer.c
#include "stk_vector_pointer.h"


static void VecPointer_Grow(VecPointer* v) {
    size_t new_cap = v->capacity == 0 ? VECTOR_DEFAULT_CAPACITY : v->capacity * VECTOR_GROW_FACTOR;
    void** new_data = (void**)realloc(v->data, sizeof(void*) * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

STK_API void VecPointer_Init(VecPointer* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecPointer_Destroy(VecPointer* v) {
    if (v->data) {
        free(v->data);
        v->data = NULL;
    }
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecPointer_PushBack(VecPointer* v, void* value) {
    if (!v) return;
    if (v->size >= v->capacity) {
        VecPointer_Grow(v);
    }
    v->data[v->size++] = value;
}

STK_API void VecPointer_PopBack(VecPointer* v) {
    if (!v || v->size == 0) return;
    v->size--;
}

STK_API void* VecPointer_At(VecPointer* v, size_t index) {
    if (!v || index >= v->size) {
        fprintf(stderr, "VecPointer_At: index %zu out of range (size=%zu)\n", index, v ? v->size : 0);
        exit(1);
    }
    return v->data[index];
}

STK_API size_t VecPointer_Size(VecPointer* v) {
    return v ? v->size : 0;
}

STK_API int VecPointer_Empty(VecPointer* v) {
    return v ? v->size == 0 : 1;
}

STK_API void VecPointer_Resize(VecPointer* v, size_t new_size) {
    if (!v) return;
    if (new_size > v->capacity) {
        void** new_data = (void**)realloc(v->data, sizeof(void*) * new_size);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = new_size;
    }
    if (new_size > v->size) {
        memset(v->data + v->size, 0, sizeof(void*) * (new_size - v->size));
    }
    v->size = new_size;
}

STK_API void VecPointer_Fill(VecPointer* v, void* value) {
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        v->data[i] = value;
    }
}