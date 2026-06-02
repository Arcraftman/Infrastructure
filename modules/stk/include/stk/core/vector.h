// stk_vector.h

#ifndef STK_CORE_VECTOR_H
#define STK_CORE_VECTOR_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} stk_vector;

/* Lifetime ------------------------------------------------------------- */

STK_API STK_STATUS stk_vector_init(stk_vector* v);
STK_API STK_STATUS stk_vector_init_with_capacity(stk_vector* v, size_t cap);
STK_API STK_STATUS stk_vector_free(stk_vector* v);

/* Core operations ------------------------------------------------------ */

STK_API STK_STATUS stk_vector_push(stk_vector* v, void* val);
STK_API STK_STATUS stk_vector_pop(stk_vector* v);
STK_API void*      stk_vector_get(stk_vector* v, size_t idx);
STK_API void**     stk_vector_at(stk_vector* v, size_t idx);
STK_API STK_STATUS stk_vector_set(stk_vector* v, size_t idx, void* val);

/* Introspection -------------------------------------------------------- */

STK_API size_t stk_vector_len(const stk_vector* v);
STK_API bool   stk_vector_empty(const stk_vector* v);
STK_API size_t stk_vector_cap(const stk_vector* v);

/* Management ----------------------------------------------------------- */

STK_API STK_STATUS stk_vector_reserve(stk_vector* v, size_t cap);
STK_API STK_STATUS stk_vector_resize(stk_vector* v, size_t len);
STK_API STK_STATUS stk_vector_shrink(stk_vector* v);
STK_API STK_STATUS stk_vector_clear(stk_vector* v);
STK_API STK_STATUS stk_vector_fill(stk_vector* v, void* val);
STK_API STK_STATUS stk_vector_insert(stk_vector* v, size_t idx, void* val);
STK_API STK_STATUS stk_vector_erase(stk_vector* v, size_t idx);
STK_API STK_STATUS stk_vector_swap(stk_vector* a, stk_vector* b);

/* Element access ------------------------------------------------------- */

STK_API void* stk_vector_front(stk_vector* v);
STK_API void* stk_vector_back(stk_vector* v);

/* Algorithms ----------------------------------------------------------- */

STK_API STK_STATUS stk_vector_sort(stk_vector* v);
STK_API STK_STATUS stk_vector_reverse(stk_vector* v);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_VECTOR_H */