#ifndef STK_SRC_CORE_VECTOR_H
#define STK_SRC_CORE_VECTOR_H

#include "config.h"
#include "str.h"
#include "detail/vector.h"

typedef struct vector
{
    void *data;
    size_t size;
    size_t capacity;
} vector;

STK_API void vector_init(vector *v);
STK_API void vector_free(vector *v);
STK_API void vector_push(vector *v, void* val);           
STK_API void vector_pop(vector *v);
STK_API void* vector_get(vector *v, size_t idx);         
STK_API void** vector_at(vector *v, size_t idx);         
STK_API void vector_set(vector *v, size_t idx, void* val); 
STK_API size_t vector_len(vector *v);
STK_API bool vector_empty(vector *v);
STK_API size_t vector_cap(vector *v);
STK_API void vector_reserve(vector *v, size_t cap);
STK_API void vector_resize(vector *v, size_t len);
STK_API void vector_shrink(vector *v);
STK_API void vector_clear(vector *v);
STK_API void vector_fill(vector *v, void* val);          
STK_API void vector_insert(vector *v, size_t idx, void* val); 
STK_API void vector_erase(vector *v, size_t idx);
STK_API void vector_swap(vector *a, vector *b);
STK_API void* vector_front(vector *v);
STK_API void* vector_back(vector *v);
STK_API void vector_sort(vector *v);
STK_API void vector_reverse(vector *v);

// 声明类型化的向量
VECTOR_DECLARE_BASIC(int, int)
VECTOR_DECLARE_BASIC(float, flt)
VECTOR_DECLARE_BASIC(double, dub)
VECTOR_DECLARE_STRUCT(str, str)

#endif