// _Vector_String.c
#include "stk_vector_string.h"



static void VecString_Grow(VecString* v) {
    size_t new_cap = v->capacity == 0 ? VECTOR_DEFAULT_CAPACITY : v->capacity * VECTOR_GROW_FACTOR;
    String* new_data = (String*)realloc(v->data, sizeof(String) * new_cap);
    if (!new_data) return;
    v->data = new_data;
    v->capacity = new_cap;
}

STK_API void VecString_Init(VecString* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecString_Destroy(VecString* v) {
    if (v->data) {
        // 先释放每个 String 的内部数据
        for (size_t i = 0; i < v->size; i++) {
            StrDestroy(&v->data[i]);
        }
        free(v->data);
        v->data = NULL;
    }
    v->size = 0;
    v->capacity = 0;
}

STK_API void VecString_PushBack(VecString* v, String value) {
    if (!v) return;
    if (v->size >= v->capacity) {
        VecString_Grow(v);
    }
    // 深拷贝 String
    StrInitFrom(&v->data[v->size], value.data);
    v->size++;
}

STK_API void VecString_PopBack(VecString* v) {
    if (!v || v->size == 0) return;
    v->size--;
    StrDestroy(&v->data[v->size]);
}

STK_API String VecString_At(VecString* v, size_t index) {
    static String empty = { NULL, 0 };
    if (!v || index >= v->size) {
        fprintf(stderr, "VecString_At: index %zu out of range (size=%zu)\n", index, v ? v->size : 0);
        return empty;
    }
    return v->data[index];
}

STK_API size_t VecString_Size(VecString* v) {
    return v ? v->size : 0;
}

STK_API int VecString_Empty(VecString* v) {
    return v ? v->size == 0 : 1;
}

STK_API void VecString_Resize(VecString* v, size_t new_size) {
    if (!v) return;
    if (new_size > v->capacity) {
        String* new_data = (String*)realloc(v->data, sizeof(String) * new_size);
        if (!new_data) return;
        v->data = new_data;
        v->capacity = new_size;
    }
    if (new_size > v->size) {
        for (size_t i = v->size; i < new_size; i++) {
            StrInit(&v->data[i]);
        }
    }
    else {
        for (size_t i = new_size; i < v->size; i++) {
            StrDestroy(&v->data[i]);
        }
    }
    v->size = new_size;
}

STK_API void VecString_Fill(VecString* v, String value) {
    if (!v) return;
    for (size_t i = 0; i < v->size; i++) {
        StrAssign(&v->data[i], value.data);
    }
}