#ifndef STK_DETAIL_LIST_H
#define STK_DETAIL_LIST_H

#define STK_LIST_DECLARE_BASIC(type, prefix)                                                 \
    typedef struct stk_list_##prefix##_node {                                            \
        type data;                                                                       \
        struct stk_list_##prefix##_node* prev;                                           \
        struct stk_list_##prefix##_node* next;                                           \
    } stk_list_##prefix##_node;                                                          \
                                                                                         \
    typedef struct stk_list_##prefix {                                                   \
        stk_list_##prefix##_node* head;                                                  \
        stk_list_##prefix##_node* tail;                                                  \
        size_t size;                                                                     \
    } stk_list_##prefix;                                                                 \
                                                                                         \
    STK_API void stk_list_##prefix##_init(stk_list_##prefix* l);                         \
    STK_API void stk_list_##prefix##_free(stk_list_##prefix* l);                         \
    STK_API void stk_list_##prefix##_push_front(stk_list_##prefix* l, type val);         \
    STK_API void stk_list_##prefix##_push_back(stk_list_##prefix* l, type val);          \
    STK_API void stk_list_##prefix##_pop_front(stk_list_##prefix* l);                    \
    STK_API void stk_list_##prefix##_pop_back(stk_list_##prefix* l);                     \
    STK_API type stk_list_##prefix##_front(stk_list_##prefix* l);                        \
    STK_API type stk_list_##prefix##_back(stk_list_##prefix* l);                         \
    STK_API type stk_list_##prefix##_get(stk_list_##prefix* l, size_t idx);              \
    STK_API void stk_list_##prefix##_set(stk_list_##prefix* l, size_t idx, type val);    \
    STK_API void stk_list_##prefix##_insert(stk_list_##prefix* l, size_t idx, type val); \
    STK_API void stk_list_##prefix##_erase(stk_list_##prefix* l, size_t idx);            \
    STK_API size_t stk_list_##prefix##_len(stk_list_##prefix* l);                        \
    STK_API bool stk_list_##prefix##_empty(stk_list_##prefix* l);                        \
    STK_API void stk_list_##prefix##_clear(stk_list_##prefix* l);                        \
    STK_API void stk_list_##prefix##_reverse(stk_list_##prefix* l);                      \
    STK_API void stk_list_##prefix##_sort(stk_list_##prefix* l);

#define STK_LIST_DECLARE_STRUCT(type, prefix)                                                \
    typedef struct stk_list_##prefix##_node {                                            \
        type data;                                                                       \
        struct stk_list_##prefix##_node* prev;                                           \
        struct stk_list_##prefix##_node* next;                                           \
    } stk_list_##prefix##_node;                                                          \
                                                                                         \
    typedef struct stk_list_##prefix {                                                   \
        stk_list_##prefix##_node* head;                                                  \
        stk_list_##prefix##_node* tail;                                                  \
        size_t size;                                                                     \
    } stk_list_##prefix;                                                                 \
                                                                                         \
    STK_API void stk_list_##prefix##_init(stk_list_##prefix* l);                         \
    STK_API void stk_list_##prefix##_free(stk_list_##prefix* l);                         \
    STK_API void stk_list_##prefix##_push_front(stk_list_##prefix* l, type val);         \
    STK_API void stk_list_##prefix##_push_back(stk_list_##prefix* l, type val);          \
    STK_API void stk_list_##prefix##_pop_front(stk_list_##prefix* l);                    \
    STK_API void stk_list_##prefix##_pop_back(stk_list_##prefix* l);                     \
    STK_API type stk_list_##prefix##_front(stk_list_##prefix* l);                        \
    STK_API type stk_list_##prefix##_back(stk_list_##prefix* l);                         \
    STK_API type stk_list_##prefix##_get(stk_list_##prefix* l, size_t idx);              \
    STK_API void stk_list_##prefix##_set(stk_list_##prefix* l, size_t idx, type val);    \
    STK_API void stk_list_##prefix##_insert(stk_list_##prefix* l, size_t idx, type val); \
    STK_API void stk_list_##prefix##_erase(stk_list_##prefix* l, size_t idx);            \
    STK_API size_t stk_list_##prefix##_len(stk_list_##prefix* l);                        \
    STK_API bool stk_list_##prefix##_empty(stk_list_##prefix* l);                        \
    STK_API void stk_list_##prefix##_clear(stk_list_##prefix* l);                        \
    STK_API void stk_list_##prefix##_reverse(stk_list_##prefix* l);                      \
    STK_API void stk_list_##prefix##_sort(stk_list_##prefix* l);

// Basic type implementation
#define STK_LIST_IMPLEMENT_BASIC(type, prefix)                                                         \
    static stk_list_##prefix##_node* stk_list_##prefix##_create_node(type val)                     \
    {                                                                                              \
        stk_list_##prefix##_node* node = malloc(sizeof(stk_list_##prefix##_node));                 \
        if (!node)                                                                                 \
            return NULL;                                                                           \
        node->data = val;                                                                          \
        node->prev = NULL;                                                                         \
        node->next = NULL;                                                                         \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_init(stk_list_##prefix* l)                                            \
    {                                                                                              \
        l->head = l->tail = NULL;                                                                  \
        l->size = 0;                                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_free(stk_list_##prefix* l)                                            \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_node* cur = l->head;                                                   \
        while (cur) {                                                                              \
            stk_list_##prefix##_node* next = cur->next;                                            \
            free(cur);                                                                             \
            cur = next;                                                                            \
        }                                                                                          \
        l->head = l->tail = NULL;                                                                  \
        l->size = 0;                                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_push_front(stk_list_##prefix* l, type val)                            \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_node* node = stk_list_##prefix##_create_node(val);                     \
        if (!node)                                                                                 \
            return;                                                                                \
        if (!l->head) {                                                                            \
            l->head = l->tail = node;                                                              \
        } else {                                                                                   \
            node->next = l->head;                                                                  \
            l->head->prev = node;                                                                  \
            l->head = node;                                                                        \
        }                                                                                          \
        l->size++;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_push_back(stk_list_##prefix* l, type val)                             \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_node* node = stk_list_##prefix##_create_node(val);                     \
        if (!node)                                                                                 \
            return;                                                                                \
        if (!l->tail) {                                                                            \
            l->head = l->tail = node;                                                              \
        } else {                                                                                   \
            node->prev = l->tail;                                                                  \
            l->tail->next = node;                                                                  \
            l->tail = node;                                                                        \
        }                                                                                          \
        l->size++;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_pop_front(stk_list_##prefix* l)                                       \
    {                                                                                              \
        if (!l || !l->head)                                                                        \
            return;                                                                                \
        stk_list_##prefix##_node* old = l->head;                                                   \
        l->head = l->head->next;                                                                   \
        if (l->head) {                                                                             \
            l->head->prev = NULL;                                                                  \
        } else {                                                                                   \
            l->tail = NULL;                                                                        \
        }                                                                                          \
        free(old);                                                                                 \
        l->size--;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_pop_back(stk_list_##prefix* l)                                        \
    {                                                                                              \
        if (!l || !l->tail)                                                                        \
            return;                                                                                \
        stk_list_##prefix##_node* old = l->tail;                                                   \
        l->tail = l->tail->prev;                                                                   \
        if (l->tail) {                                                                             \
            l->tail->next = NULL;                                                                  \
        } else {                                                                                   \
            l->head = NULL;                                                                        \
        }                                                                                          \
        free(old);                                                                                 \
        l->size--;                                                                                 \
    }                                                                                              \
                                                                                                   \
    type stk_list_##prefix##_front(stk_list_##prefix* l)                                           \
    {                                                                                              \
        return (l && l->head) ? l->head->data : (type)0;                                           \
    }                                                                                              \
                                                                                                   \
    type stk_list_##prefix##_back(stk_list_##prefix* l)                                            \
    {                                                                                              \
        return (l && l->tail) ? l->tail->data : (type)0;                                           \
    }                                                                                              \
                                                                                                   \
    static stk_list_##prefix##_node* stk_list_##prefix##_node_at(stk_list_##prefix* l, size_t idx) \
    {                                                                                              \
        if (!l || idx >= l->size)                                                                  \
            return NULL;                                                                           \
        stk_list_##prefix##_node* node;                                                            \
        if (idx < l->size / 2) {                                                                   \
            node = l->head;                                                                        \
            for (size_t i = 0; i < idx; i++)                                                       \
                node = node->next;                                                                 \
        } else {                                                                                   \
            node = l->tail;                                                                        \
            for (size_t i = l->size - 1; i > idx; i--)                                             \
                node = node->prev;                                                                 \
        }                                                                                          \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    type stk_list_##prefix##_get(stk_list_##prefix* l, size_t idx)                                 \
    {                                                                                              \
        stk_list_##prefix##_node* node = stk_list_##prefix##_node_at(l, idx);                      \
        return node ? node->data : (type)0;                                                        \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_set(stk_list_##prefix* l, size_t idx, type val)                       \
    {                                                                                              \
        stk_list_##prefix##_node* node = stk_list_##prefix##_node_at(l, idx);                      \
        if (node)                                                                                  \
            node->data = val;                                                                      \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_insert(stk_list_##prefix* l, size_t idx, type val)                    \
    {                                                                                              \
        if (!l || idx > l->size)                                                                   \
            return;                                                                                \
        if (idx == 0) {                                                                            \
            stk_list_##prefix##_push_front(l, val);                                                \
            return;                                                                                \
        }                                                                                          \
        if (idx == l->size) {                                                                      \
            stk_list_##prefix##_push_back(l, val);                                                 \
            return;                                                                                \
        }                                                                                          \
        stk_list_##prefix##_node* right = stk_list_##prefix##_node_at(l, idx);                     \
        stk_list_##prefix##_node* left = right->prev;                                              \
        stk_list_##prefix##_node* node = stk_list_##prefix##_create_node(val);                     \
        if (!node)                                                                                 \
            return;                                                                                \
        node->prev = left;                                                                         \
        node->next = right;                                                                        \
        left->next = node;                                                                         \
        right->prev = node;                                                                        \
        l->size++;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_erase(stk_list_##prefix* l, size_t idx)                               \
    {                                                                                              \
        if (!l || idx >= l->size)                                                                  \
            return;                                                                                \
        if (idx == 0) {                                                                            \
            stk_list_##prefix##_pop_front(l);                                                      \
            return;                                                                                \
        }                                                                                          \
        if (idx == l->size - 1) {                                                                  \
            stk_list_##prefix##_pop_back(l);                                                       \
            return;                                                                                \
        }                                                                                          \
        stk_list_##prefix##_node* target = stk_list_##prefix##_node_at(l, idx);                    \
        if (!target)                                                                               \
            return;                                                                                \
        target->prev->next = target->next;                                                         \
        target->next->prev = target->prev;                                                         \
        free(target);                                                                              \
        l->size--;                                                                                 \
    }                                                                                              \
                                                                                                   \
    size_t stk_list_##prefix##_len(stk_list_##prefix* l)                                           \
    {                                                                                              \
        return l ? l->size : 0;                                                                    \
    }                                                                                              \
                                                                                                   \
    bool stk_list_##prefix##_empty(stk_list_##prefix* l)                                           \
    {                                                                                              \
        return l ? l->size == 0 : true;                                                            \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_clear(stk_list_##prefix* l)                                           \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_free(l);                                                               \
        stk_list_##prefix##_init(l);                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_reverse(stk_list_##prefix* l)                                         \
    {                                                                                              \
        if (!l || l->size < 2)                                                                     \
            return;                                                                                \
        stk_list_##prefix##_node* cur = l->head;                                                   \
        while (cur) {                                                                              \
            stk_list_##prefix##_node* next = cur->next;                                            \
            cur->next = cur->prev;                                                                 \
            cur->prev = next;                                                                      \
            cur = next;                                                                            \
        }                                                                                          \
        stk_list_##prefix##_node* tmp = l->head;                                                   \
        l->head = l->tail;                                                                         \
        l->tail = tmp;                                                                             \
    }                                                                                              \
                                                                                                   \
    static int stk_list_##prefix##_cmp(const void* a, const void* b)                               \
    {                                                                                              \
        type va = *(const type*)a;                                                                 \
        type vb = *(const type*)b;                                                                 \
        return (va > vb) - (va < vb);                                                              \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_sort(stk_list_##prefix* l)                                            \
    {                                                                                              \
        if (!l || l->size < 2)                                                                     \
            return;                                                                                \
        type* arr = malloc(l->size * sizeof(type));                                                \
        if (!arr)                                                                                  \
            return;                                                                                \
        size_t i = 0;                                                                              \
        for (stk_list_##prefix##_node* cur = l->head; cur; cur = cur->next) {                      \
            arr[i++] = cur->data;                                                                  \
        }                                                                                          \
        qsort(arr, l->size, sizeof(type), stk_list_##prefix##_cmp);                                \
        i = 0;                                                                                     \
        for (stk_list_##prefix##_node* cur = l->head; cur; cur = cur->next) {                      \
            cur->data = arr[i++];                                                                  \
        }                                                                                          \
        free(arr);                                                                                 \
    }

// Struct type implementation (requires custom comparison function)
#define STK_LIST_IMPLEMENT_STRUCT(type, prefix, cmp_func)                                              \
    static stk_list_##prefix##_node* stk_list_##prefix##_create_node(type val)                     \
    {                                                                                              \
        stk_list_##prefix##_node* node = malloc(sizeof(stk_list_##prefix##_node));                 \
        if (!node)                                                                                 \
            return NULL;                                                                           \
        node->data = val;                                                                          \
        node->prev = NULL;                                                                         \
        node->next = NULL;                                                                         \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_init(stk_list_##prefix* l)                                            \
    {                                                                                              \
        l->head = l->tail = NULL;                                                                  \
        l->size = 0;                                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_free(stk_list_##prefix* l)                                            \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_node* cur = l->head;                                                   \
        while (cur) {                                                                              \
            stk_list_##prefix##_node* next = cur->next;                                            \
            free(cur);                                                                             \
            cur = next;                                                                            \
        }                                                                                          \
        l->head = l->tail = NULL;                                                                  \
        l->size = 0;                                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_push_front(stk_list_##prefix* l, type val)                            \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_node* node = stk_list_##prefix##_create_node(val);                     \
        if (!node)                                                                                 \
            return;                                                                                \
        if (!l->head) {                                                                            \
            l->head = l->tail = node;                                                              \
        } else {                                                                                   \
            node->next = l->head;                                                                  \
            l->head->prev = node;                                                                  \
            l->head = node;                                                                        \
        }                                                                                          \
        l->size++;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_push_back(stk_list_##prefix* l, type val)                             \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_node* node = stk_list_##prefix##_create_node(val);                     \
        if (!node)                                                                                 \
            return;                                                                                \
        if (!l->tail) {                                                                            \
            l->head = l->tail = node;                                                              \
        } else {                                                                                   \
            node->prev = l->tail;                                                                  \
            l->tail->next = node;                                                                  \
            l->tail = node;                                                                        \
        }                                                                                          \
        l->size++;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_pop_front(stk_list_##prefix* l)                                       \
    {                                                                                              \
        if (!l || !l->head)                                                                        \
            return;                                                                                \
        stk_list_##prefix##_node* old = l->head;                                                   \
        l->head = l->head->next;                                                                   \
        if (l->head) {                                                                             \
            l->head->prev = NULL;                                                                  \
        } else {                                                                                   \
            l->tail = NULL;                                                                        \
        }                                                                                          \
        free(old);                                                                                 \
        l->size--;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_pop_back(stk_list_##prefix* l)                                        \
    {                                                                                              \
        if (!l || !l->tail)                                                                        \
            return;                                                                                \
        stk_list_##prefix##_node* old = l->tail;                                                   \
        l->tail = l->tail->prev;                                                                   \
        if (l->tail) {                                                                             \
            l->tail->next = NULL;                                                                  \
        } else {                                                                                   \
            l->head = NULL;                                                                        \
        }                                                                                          \
        free(old);                                                                                 \
        l->size--;                                                                                 \
    }                                                                                              \
                                                                                                   \
    type stk_list_##prefix##_front(stk_list_##prefix* l)                                           \
    {                                                                                              \
        return (l && l->head) ? l->head->data : (type){0};                                         \
    }                                                                                              \
                                                                                                   \
    type stk_list_##prefix##_back(stk_list_##prefix* l)                                            \
    {                                                                                              \
        return (l && l->tail) ? l->tail->data : (type){0};                                         \
    }                                                                                              \
                                                                                                   \
    static stk_list_##prefix##_node* stk_list_##prefix##_node_at(stk_list_##prefix* l, size_t idx) \
    {                                                                                              \
        if (!l || idx >= l->size)                                                                  \
            return NULL;                                                                           \
        stk_list_##prefix##_node* node;                                                            \
        if (idx < l->size / 2) {                                                                   \
            node = l->head;                                                                        \
            for (size_t i = 0; i < idx; i++)                                                       \
                node = node->next;                                                                 \
        } else {                                                                                   \
            node = l->tail;                                                                        \
            for (size_t i = l->size - 1; i > idx; i--)                                             \
                node = node->prev;                                                                 \
        }                                                                                          \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    type stk_list_##prefix##_get(stk_list_##prefix* l, size_t idx)                                 \
    {                                                                                              \
        stk_list_##prefix##_node* node = stk_list_##prefix##_node_at(l, idx);                      \
        return node ? node->data : (type){0};                                                      \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_set(stk_list_##prefix* l, size_t idx, type val)                       \
    {                                                                                              \
        stk_list_##prefix##_node* node = stk_list_##prefix##_node_at(l, idx);                      \
        if (node)                                                                                  \
            node->data = val;                                                                      \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_insert(stk_list_##prefix* l, size_t idx, type val)                    \
    {                                                                                              \
        if (!l || idx > l->size)                                                                   \
            return;                                                                                \
        if (idx == 0) {                                                                            \
            stk_list_##prefix##_push_front(l, val);                                                \
            return;                                                                                \
        }                                                                                          \
        if (idx == l->size) {                                                                      \
            stk_list_##prefix##_push_back(l, val);                                                 \
            return;                                                                                \
        }                                                                                          \
        stk_list_##prefix##_node* right = stk_list_##prefix##_node_at(l, idx);                     \
        stk_list_##prefix##_node* left = right->prev;                                              \
        stk_list_##prefix##_node* node = stk_list_##prefix##_create_node(val);                     \
        if (!node)                                                                                 \
            return;                                                                                \
        node->prev = left;                                                                         \
        node->next = right;                                                                        \
        left->next = node;                                                                         \
        right->prev = node;                                                                        \
        l->size++;                                                                                 \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_erase(stk_list_##prefix* l, size_t idx)                               \
    {                                                                                              \
        if (!l || idx >= l->size)                                                                  \
            return;                                                                                \
        if (idx == 0) {                                                                            \
            stk_list_##prefix##_pop_front(l);                                                      \
            return;                                                                                \
        }                                                                                          \
        if (idx == l->size - 1) {                                                                  \
            stk_list_##prefix##_pop_back(l);                                                       \
            return;                                                                                \
        }                                                                                          \
        stk_list_##prefix##_node* target = stk_list_##prefix##_node_at(l, idx);                    \
        if (!target)                                                                               \
            return;                                                                                \
        target->prev->next = target->next;                                                         \
        target->next->prev = target->prev;                                                         \
        free(target);                                                                              \
        l->size--;                                                                                 \
    }                                                                                              \
                                                                                                   \
    size_t stk_list_##prefix##_len(stk_list_##prefix* l)                                           \
    {                                                                                              \
        return l ? l->size : 0;                                                                    \
    }                                                                                              \
                                                                                                   \
    bool stk_list_##prefix##_empty(stk_list_##prefix* l)                                           \
    {                                                                                              \
        return l ? l->size == 0 : true;                                                            \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_clear(stk_list_##prefix* l)                                           \
    {                                                                                              \
        if (!l)                                                                                    \
            return;                                                                                \
        stk_list_##prefix##_free(l);                                                               \
        stk_list_##prefix##_init(l);                                                               \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_reverse(stk_list_##prefix* l)                                         \
    {                                                                                              \
        if (!l || l->size < 2)                                                                     \
            return;                                                                                \
        stk_list_##prefix##_node* cur = l->head;                                                   \
        while (cur) {                                                                              \
            stk_list_##prefix##_node* next = cur->next;                                            \
            cur->next = cur->prev;                                                                 \
            cur->prev = next;                                                                      \
            cur = next;                                                                            \
        }                                                                                          \
        stk_list_##prefix##_node* tmp = l->head;                                                   \
        l->head = l->tail;                                                                         \
        l->tail = tmp;                                                                             \
    }                                                                                              \
                                                                                                   \
    void stk_list_##prefix##_sort(stk_list_##prefix* l)                                            \
    {                                                                                              \
        if (!l || l->size < 2)                                                                     \
            return;                                                                                \
        type* arr = malloc(l->size * sizeof(type));                                                \
        if (!arr)                                                                                  \
            return;                                                                                \
        size_t i = 0;                                                                              \
        for (stk_list_##prefix##_node* cur = l->head; cur; cur = cur->next) {                      \
            arr[i++] = cur->data;                                                                  \
        }                                                                                          \
        qsort(arr, l->size, sizeof(type), cmp_func);                                               \
        i = 0;                                                                                     \
        for (stk_list_##prefix##_node* cur = l->head; cur; cur = cur->next) {                      \
            cur->data = arr[i++];                                                                  \
        }                                                                                          \
        free(arr);                                                                                 \
    }

#endif