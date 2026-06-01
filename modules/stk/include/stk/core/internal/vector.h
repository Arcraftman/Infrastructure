#ifndef STK_DETAIL_VECTOR_H
#define STK_DETAIL_VECTOR_H

// Basic-type vector declarations (int, double, float, etc.) — stores values
#define STK_VECTOR_DECLARE_BASIC(type, name)                                             \
    typedef struct {                                                                     \
        type* data;                                                                      \
        size_t size;                                                                     \
        size_t capacity;                                                                 \
    } stk_vector_##name;                                                                 \
                                                                                         \
    STK_API void stk_vector_##name##_init(stk_vector_##name* v);                         \
    STK_API void stk_vector_##name##_free(stk_vector_##name* v);                         \
    STK_API void stk_vector_##name##_push(stk_vector_##name* v, type val);               \
    STK_API void stk_vector_##name##_pop(stk_vector_##name* v);                          \
    STK_API type stk_vector_##name##_get(stk_vector_##name* v, size_t idx);              \
    STK_API type* stk_vector_##name##_at(stk_vector_##name* v, size_t idx);              \
    STK_API void stk_vector_##name##_set(stk_vector_##name* v, size_t idx, type val);    \
    STK_API size_t stk_vector_##name##_len(stk_vector_##name* v);                        \
    STK_API bool stk_vector_##name##_empty(stk_vector_##name* v);                        \
    STK_API size_t stk_vector_##name##_cap(stk_vector_##name* v);                        \
    STK_API void stk_vector_##name##_reserve(stk_vector_##name* v, size_t cap);          \
    STK_API void stk_vector_##name##_resize(stk_vector_##name* v, size_t len);           \
    STK_API void stk_vector_##name##_shrink(stk_vector_##name* v);                       \
    STK_API void stk_vector_##name##_clear(stk_vector_##name* v);                        \
    STK_API void stk_vector_##name##_fill(stk_vector_##name* v, type val);               \
    STK_API void stk_vector_##name##_insert(stk_vector_##name* v, size_t idx, type val); \
    STK_API void stk_vector_##name##_erase(stk_vector_##name* v, size_t idx);            \
    STK_API void stk_vector_##name##_swap(stk_vector_##name* a, stk_vector_##name* b);   \
    STK_API type* stk_vector_##name##_front(stk_vector_##name* v);                       \
    STK_API type* stk_vector_##name##_back(stk_vector_##name* v);                        \
    STK_API void stk_vector_##name##_sort(stk_vector_##name* v);                         \
    STK_API void stk_vector_##name##_reverse(stk_vector_##name* v);

// ============================================================
// Struct-type vector declarations (str, custom types, etc.) — stores pointers
// ============================================================
#define STK_VECTOR_DECLARE_STRUCT(type, name)                                             \
    typedef struct {                                                                      \
        type** data; /* stores pointers, type is type** */                                \
        size_t size;                                                                      \
        size_t capacity;                                                                  \
    } stk_vector_##name;                                                                  \
                                                                                          \
    STK_API void stk_vector_##name##_init(stk_vector_##name* v);                          \
    STK_API void stk_vector_##name##_free(stk_vector_##name* v);                          \
    STK_API void stk_vector_##name##_push(stk_vector_##name* v, type* val);               \
    STK_API void stk_vector_##name##_pop(stk_vector_##name* v);                           \
    STK_API type* stk_vector_##name##_get(stk_vector_##name* v, size_t idx);              \
    STK_API type** stk_vector_##name##_at(stk_vector_##name* v, size_t idx);              \
    STK_API void stk_vector_##name##_set(stk_vector_##name* v, size_t idx, type* val);    \
    STK_API size_t stk_vector_##name##_len(stk_vector_##name* v);                         \
    STK_API bool stk_vector_##name##_empty(stk_vector_##name* v);                         \
    STK_API size_t stk_vector_##name##_cap(stk_vector_##name* v);                         \
    STK_API void stk_vector_##name##_reserve(stk_vector_##name* v, size_t cap);           \
    STK_API void stk_vector_##name##_resize(stk_vector_##name* v, size_t len);            \
    STK_API void stk_vector_##name##_shrink(stk_vector_##name* v);                        \
    STK_API void stk_vector_##name##_clear(stk_vector_##name* v);                         \
    STK_API void stk_vector_##name##_fill(stk_vector_##name* v, type* val);               \
    STK_API void stk_vector_##name##_insert(stk_vector_##name* v, size_t idx, type* val); \
    STK_API void stk_vector_##name##_erase(stk_vector_##name* v, size_t idx);             \
    STK_API void stk_vector_##name##_swap(stk_vector_##name* a, stk_vector_##name* b);    \
    STK_API type* stk_vector_##name##_front(stk_vector_##name* v);                        \
    STK_API type* stk_vector_##name##_back(stk_vector_##name* v);                         \
    STK_API void stk_vector_##name##_sort(stk_vector_##name* v);                          \
    STK_API void stk_vector_##name##_reverse(stk_vector_##name* v);

// ============================================================
// Basic-type vector implementation
// ============================================================
#define STK_VECTOR_IMPLEMENT_BASIC(type, name)                                             \
    static void stk_vector_##name##_grow(stk_vector_##name* v)                             \
    {                                                                                      \
        size_t new_cap =                                                                   \
            v->capacity == 0 ? STK_VECTOR_DEFAULT_CAPACITY : v->capacity * STK_VECTOR_GROW_FACTOR; \
        type* new_data = realloc(v->data, sizeof(type) * new_cap);                         \
        if (!new_data)                                                                     \
            return;                                                                        \
        v->data = new_data;                                                                \
        v->capacity = new_cap;                                                             \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_init(stk_vector_##name* v)                                    \
    {                                                                                      \
        v->data = NULL;                                                                    \
        v->size = 0;                                                                       \
        v->capacity = 0;                                                                   \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_free(stk_vector_##name* v)                                    \
    {                                                                                      \
        if (v->data)                                                                       \
            free(v->data);                                                                 \
        v->data = NULL;                                                                    \
        v->size = 0;                                                                       \
        v->capacity = 0;                                                                   \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_push(stk_vector_##name* v, type val)                          \
    {                                                                                      \
        if (!v)                                                                            \
            return;                                                                        \
        if (v->size >= v->capacity)                                                        \
            stk_vector_##name##_grow(v);                                                   \
        v->data[v->size++] = val;                                                          \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_pop(stk_vector_##name* v)                                     \
    {                                                                                      \
        if (!v || v->size == 0)                                                            \
            return;                                                                        \
        v->size--;                                                                         \
    }                                                                                      \
                                                                                           \
    type stk_vector_##name##_get(stk_vector_##name* v, size_t idx)                         \
    {                                                                                      \
        if (!v || idx >= v->size) {                                                        \
            fprintf(stderr, "stk_vector_" #name "_get: index %zu out of range\n", idx);    \
            exit(1);                                                                       \
        }                                                                                  \
        return v->data[idx];                                                               \
    }                                                                                      \
                                                                                           \
    type* stk_vector_##name##_at(stk_vector_##name* v, size_t idx)                         \
    {                                                                                      \
        if (!v || idx >= v->size)                                                          \
            return NULL;                                                                   \
        return &v->data[idx];                                                              \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_set(stk_vector_##name* v, size_t idx, type val)               \
    {                                                                                      \
        if (!v || idx >= v->size)                                                          \
            return;                                                                        \
        v->data[idx] = val;                                                                \
    }                                                                                      \
                                                                                           \
    size_t stk_vector_##name##_len(stk_vector_##name* v)                                   \
    {                                                                                      \
        return v ? v->size : 0;                                                            \
    }                                                                                      \
                                                                                           \
    bool stk_vector_##name##_empty(stk_vector_##name* v)                                   \
    {                                                                                      \
        return v ? v->size == 0 : true;                                                    \
    }                                                                                      \
                                                                                           \
    size_t stk_vector_##name##_cap(stk_vector_##name* v)                                   \
    {                                                                                      \
        return v ? v->capacity : 0;                                                        \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_reserve(stk_vector_##name* v, size_t cap)                     \
    {                                                                                      \
        if (!v || cap <= v->capacity)                                                      \
            return;                                                                        \
        type* new_data = realloc(v->data, sizeof(type) * cap);                             \
        if (!new_data)                                                                     \
            return;                                                                        \
        v->data = new_data;                                                                \
        v->capacity = cap;                                                                 \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_resize(stk_vector_##name* v, size_t len)                      \
    {                                                                                      \
        if (!v)                                                                            \
            return;                                                                        \
        if (len > v->capacity) {                                                           \
            type* new_data = realloc(v->data, sizeof(type) * len);                         \
            if (!new_data)                                                                 \
                return;                                                                    \
            v->data = new_data;                                                            \
            v->capacity = len;                                                             \
        }                                                                                  \
        if (len > v->size) {                                                               \
            memset(v->data + v->size, 0, sizeof(type) * (len - v->size));                  \
        }                                                                                  \
        v->size = len;                                                                     \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_shrink(stk_vector_##name* v)                                  \
    {                                                                                      \
        if (!v || v->size == v->capacity)                                                  \
            return;                                                                        \
        if (v->size == 0) {                                                                \
            free(v->data);                                                                 \
            v->data = NULL;                                                                \
            v->capacity = 0;                                                               \
        } else {                                                                           \
            type* new_data = realloc(v->data, sizeof(type) * v->size);                     \
            if (new_data) {                                                                \
                v->data = new_data;                                                        \
                v->capacity = v->size;                                                     \
            }                                                                              \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_clear(stk_vector_##name* v)                                   \
    {                                                                                      \
        if (!v)                                                                            \
            return;                                                                        \
        v->size = 0;                                                                       \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_fill(stk_vector_##name* v, type val)                          \
    {                                                                                      \
        if (!v)                                                                            \
            return;                                                                        \
        for (size_t i = 0; i < v->size; i++)                                               \
            v->data[i] = val;                                                              \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_insert(stk_vector_##name* v, size_t idx, type val)            \
    {                                                                                      \
        if (!v || idx > v->size)                                                           \
            return;                                                                        \
        if (v->size >= v->capacity)                                                        \
            stk_vector_##name##_grow(v);                                                   \
        if (idx < v->size) {                                                               \
            memmove(&v->data[idx + 1], &v->data[idx], sizeof(type) * (v->size - idx));     \
        }                                                                                  \
        v->data[idx] = val;                                                                \
        v->size++;                                                                         \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_erase(stk_vector_##name* v, size_t idx)                       \
    {                                                                                      \
        if (!v || idx >= v->size)                                                          \
            return;                                                                        \
        if (idx < v->size - 1) {                                                           \
            memmove(&v->data[idx], &v->data[idx + 1], sizeof(type) * (v->size - idx - 1)); \
        }                                                                                  \
        v->size--;                                                                         \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_swap(stk_vector_##name* a, stk_vector_##name* b)              \
    {                                                                                      \
        if (!a || !b)                                                                      \
            return;                                                                        \
        stk_vector_##name tmp = *a;                                                        \
        *a = *b;                                                                           \
        *b = tmp;                                                                          \
    }                                                                                      \
                                                                                           \
    type* stk_vector_##name##_front(stk_vector_##name* v)                                  \
    {                                                                                      \
        if (!v || v->size == 0)                                                            \
            return NULL;                                                                   \
        return &v->data[0];                                                                \
    }                                                                                      \
                                                                                           \
    type* stk_vector_##name##_back(stk_vector_##name* v)                                   \
    {                                                                                      \
        if (!v || v->size == 0)                                                            \
            return NULL;                                                                   \
        return &v->data[v->size - 1];                                                      \
    }                                                                                      \
                                                                                           \
    static int stk_vector_##name##_cmp(const void* a, const void* b)                       \
    {                                                                                      \
        type va = *(const type*)a;                                                         \
        type vb = *(const type*)b;                                                         \
        return (va > vb) - (va < vb);                                                      \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_sort(stk_vector_##name* v)                                    \
    {                                                                                      \
        if (!v || v->size < 2)                                                             \
            return;                                                                        \
        qsort(v->data, v->size, sizeof(type), stk_vector_##name##_cmp);                    \
    }                                                                                      \
                                                                                           \
    void stk_vector_##name##_reverse(stk_vector_##name* v)                                 \
    {                                                                                      \
        if (!v || v->size < 2)                                                             \
            return;                                                                        \
        for (size_t i = 0; i < v->size / 2; i++) {                                         \
            type tmp = v->data[i];                                                         \
            v->data[i] = v->data[v->size - 1 - i];                                         \
            v->data[v->size - 1 - i] = tmp;                                                \
        }                                                                                  \
    }

// ============================================================
// Struct-type vector implementation (stores pointers)
// ============================================================
#define STK_VECTOR_IMPLEMENT_STRUCT(type, name, cmp_func)                                   \
    static void stk_vector_##name##_grow(stk_vector_##name* v)                              \
    {                                                                                       \
        size_t new_cap =                                                                    \
            v->capacity == 0 ? STK_VECTOR_DEFAULT_CAPACITY : v->capacity * STK_VECTOR_GROW_FACTOR;  \
        type** new_data = realloc(v->data, sizeof(type*) * new_cap);                        \
        if (!new_data)                                                                      \
            return;                                                                         \
        v->data = new_data;                                                                 \
        v->capacity = new_cap;                                                              \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_init(stk_vector_##name* v)                                     \
    {                                                                                       \
        v->data = NULL;                                                                     \
        v->size = 0;                                                                        \
        v->capacity = 0;                                                                    \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_free(stk_vector_##name* v)                                     \
    {                                                                                       \
        if (v->data)                                                                        \
            free(v->data);                                                                  \
        v->data = NULL;                                                                     \
        v->size = 0;                                                                        \
        v->capacity = 0;                                                                    \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_push(stk_vector_##name* v, type* val)                          \
    {                                                                                       \
        if (!v)                                                                             \
            return;                                                                         \
        if (v->size >= v->capacity)                                                         \
            stk_vector_##name##_grow(v);                                                    \
        v->data[v->size++] = val;                                                           \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_pop(stk_vector_##name* v)                                      \
    {                                                                                       \
        if (!v || v->size == 0)                                                             \
            return;                                                                         \
        v->size--;                                                                          \
    }                                                                                       \
                                                                                            \
    type* stk_vector_##name##_get(stk_vector_##name* v, size_t idx)                         \
    {                                                                                       \
        if (!v || idx >= v->size) {                                                         \
            fprintf(stderr, "stk_vector_" #name "_get: index %zu out of range\n", idx);     \
            exit(1);                                                                        \
        }                                                                                   \
        return v->data[idx];                                                                \
    }                                                                                       \
                                                                                            \
    type** stk_vector_##name##_at(stk_vector_##name* v, size_t idx)                         \
    {                                                                                       \
        if (!v || idx >= v->size)                                                           \
            return NULL;                                                                    \
        return &v->data[idx];                                                               \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_set(stk_vector_##name* v, size_t idx, type* val)               \
    {                                                                                       \
        if (!v || idx >= v->size)                                                           \
            return;                                                                         \
        v->data[idx] = val;                                                                 \
    }                                                                                       \
                                                                                            \
    size_t stk_vector_##name##_len(stk_vector_##name* v)                                    \
    {                                                                                       \
        return v ? v->size : 0;                                                             \
    }                                                                                       \
                                                                                            \
    bool stk_vector_##name##_empty(stk_vector_##name* v)                                    \
    {                                                                                       \
        return v ? v->size == 0 : true;                                                     \
    }                                                                                       \
                                                                                            \
    size_t stk_vector_##name##_cap(stk_vector_##name* v)                                    \
    {                                                                                       \
        return v ? v->capacity : 0;                                                         \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_reserve(stk_vector_##name* v, size_t cap)                      \
    {                                                                                       \
        if (!v || cap <= v->capacity)                                                       \
            return;                                                                         \
        type** new_data = realloc(v->data, sizeof(type*) * cap);                            \
        if (!new_data)                                                                      \
            return;                                                                         \
        v->data = new_data;                                                                 \
        v->capacity = cap;                                                                  \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_resize(stk_vector_##name* v, size_t len)                       \
    {                                                                                       \
        if (!v)                                                                             \
            return;                                                                         \
        if (len > v->capacity) {                                                            \
            type** new_data = realloc(v->data, sizeof(type*) * len);                        \
            if (!new_data)                                                                  \
                return;                                                                     \
            v->data = new_data;                                                             \
            v->capacity = len;                                                              \
        }                                                                                   \
        if (len > v->size) {                                                                \
            memset(v->data + v->size, 0, sizeof(type*) * (len - v->size));                  \
        }                                                                                   \
        v->size = len;                                                                      \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_shrink(stk_vector_##name* v)                                   \
    {                                                                                       \
        if (!v || v->size == v->capacity)                                                   \
            return;                                                                         \
        if (v->size == 0) {                                                                 \
            free(v->data);                                                                  \
            v->data = NULL;                                                                 \
            v->capacity = 0;                                                                \
        } else {                                                                            \
            type** new_data = realloc(v->data, sizeof(type*) * v->size);                    \
            if (new_data) {                                                                 \
                v->data = new_data;                                                         \
                v->capacity = v->size;                                                      \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_clear(stk_vector_##name* v)                                    \
    {                                                                                       \
        if (!v)                                                                             \
            return;                                                                         \
        v->size = 0;                                                                        \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_fill(stk_vector_##name* v, type* val)                          \
    {                                                                                       \
        if (!v)                                                                             \
            return;                                                                         \
        for (size_t i = 0; i < v->size; i++)                                                \
            v->data[i] = val;                                                               \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_insert(stk_vector_##name* v, size_t idx, type* val)            \
    {                                                                                       \
        if (!v || idx > v->size)                                                            \
            return;                                                                         \
        if (v->size >= v->capacity)                                                         \
            stk_vector_##name##_grow(v);                                                    \
        if (idx < v->size) {                                                                \
            memmove(&v->data[idx + 1], &v->data[idx], sizeof(type*) * (v->size - idx));     \
        }                                                                                   \
        v->data[idx] = val;                                                                 \
        v->size++;                                                                          \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_erase(stk_vector_##name* v, size_t idx)                        \
    {                                                                                       \
        if (!v || idx >= v->size)                                                           \
            return;                                                                         \
        if (idx < v->size - 1) {                                                            \
            memmove(&v->data[idx], &v->data[idx + 1], sizeof(type*) * (v->size - idx - 1)); \
        }                                                                                   \
        v->size--;                                                                          \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_swap(stk_vector_##name* a, stk_vector_##name* b)               \
    {                                                                                       \
        if (!a || !b)                                                                       \
            return;                                                                         \
        stk_vector_##name tmp = *a;                                                         \
        *a = *b;                                                                            \
        *b = tmp;                                                                           \
    }                                                                                       \
                                                                                            \
    type* stk_vector_##name##_front(stk_vector_##name* v)                                   \
    {                                                                                       \
        if (!v || v->size == 0)                                                             \
            return NULL;                                                                    \
        return v->data[0];                                                                  \
    }                                                                                       \
                                                                                            \
    type* stk_vector_##name##_back(stk_vector_##name* v)                                    \
    {                                                                                       \
        if (!v || v->size == 0)                                                             \
            return NULL;                                                                    \
        return v->data[v->size - 1];                                                        \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_sort(stk_vector_##name* v)                                     \
    {                                                                                       \
        if (!v || v->size < 2)                                                              \
            return;                                                                         \
        qsort(v->data, v->size, sizeof(type*), cmp_func);                                   \
    }                                                                                       \
                                                                                            \
    void stk_vector_##name##_reverse(stk_vector_##name* v)                                  \
    {                                                                                       \
        if (!v || v->size < 2)                                                              \
            return;                                                                         \
        for (size_t i = 0; i < v->size / 2; i++) {                                          \
            type* tmp = v->data[i];                                                         \
            v->data[i] = v->data[v->size - 1 - i];                                          \
            v->data[v->size - 1 - i] = tmp;                                                 \
        }                                                                                   \
    }

#endif // STK_DETAIL_VECTOR_H