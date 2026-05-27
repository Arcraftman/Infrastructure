#ifndef STK_SRC_CORE_DETAIL_LIST_H
#define STK_SRC_CORE_DETAIL_LIST_H

#define LIST_DECLARE_BASIC(type, prefix)                                         \
    typedef struct list_##prefix##_node                                          \
    {                                                                            \
        type data;                                                               \
        struct list_##prefix##_node *prev;                                       \
        struct list_##prefix##_node *next;                                       \
    } list_##prefix##_node;                                                      \
                                                                                 \
    typedef struct list_##prefix                                                 \
    {                                                                            \
        list_##prefix##_node *head;                                              \
        list_##prefix##_node *tail;                                              \
        size_t size;                                                             \
    } list_##prefix;                                                             \
                                                                                 \
    STK_API void list_##prefix##_init(list_##prefix *l);                         \
    STK_API void list_##prefix##_free(list_##prefix *l);                         \
    STK_API void list_##prefix##_push_front(list_##prefix *l, type val);         \
    STK_API void list_##prefix##_push_back(list_##prefix *l, type val);          \
    STK_API void list_##prefix##_pop_front(list_##prefix *l);                    \
    STK_API void list_##prefix##_pop_back(list_##prefix *l);                     \
    STK_API type list_##prefix##_front(list_##prefix *l);                        \
    STK_API type list_##prefix##_back(list_##prefix *l);                         \
    STK_API type list_##prefix##_get(list_##prefix *l, size_t idx);              \
    STK_API void list_##prefix##_set(list_##prefix *l, size_t idx, type val);    \
    STK_API void list_##prefix##_insert(list_##prefix *l, size_t idx, type val); \
    STK_API void list_##prefix##_erase(list_##prefix *l, size_t idx);            \
    STK_API size_t list_##prefix##_len(list_##prefix *l);                        \
    STK_API bool list_##prefix##_empty(list_##prefix *l);                        \
    STK_API void list_##prefix##_clear(list_##prefix *l);                        \
    STK_API void list_##prefix##_reverse(list_##prefix *l);                      \
    STK_API void list_##prefix##_sort(list_##prefix *l);

#define LIST_DECLARE_STRUCT(type, prefix)                                        \
    typedef struct list_##prefix##_node                                          \
    {                                                                            \
        type data;                                                               \
        struct list_##prefix##_node *prev;                                       \
        struct list_##prefix##_node *next;                                       \
    } list_##prefix##_node;                                                      \
                                                                                 \
    typedef struct list_##prefix                                                 \
    {                                                                            \
        list_##prefix##_node *head;                                              \
        list_##prefix##_node *tail;                                              \
        size_t size;                                                             \
    } list_##prefix;                                                             \
                                                                                 \
    STK_API void list_##prefix##_init(list_##prefix *l);                         \
    STK_API void list_##prefix##_free(list_##prefix *l);                         \
    STK_API void list_##prefix##_push_front(list_##prefix *l, type val);         \
    STK_API void list_##prefix##_push_back(list_##prefix *l, type val);          \
    STK_API void list_##prefix##_pop_front(list_##prefix *l);                    \
    STK_API void list_##prefix##_pop_back(list_##prefix *l);                     \
    STK_API type list_##prefix##_front(list_##prefix *l);                        \
    STK_API type list_##prefix##_back(list_##prefix *l);                         \
    STK_API type list_##prefix##_get(list_##prefix *l, size_t idx);              \
    STK_API void list_##prefix##_set(list_##prefix *l, size_t idx, type val);    \
    STK_API void list_##prefix##_insert(list_##prefix *l, size_t idx, type val); \
    STK_API void list_##prefix##_erase(list_##prefix *l, size_t idx);            \
    STK_API size_t list_##prefix##_len(list_##prefix *l);                        \
    STK_API bool list_##prefix##_empty(list_##prefix *l);                        \
    STK_API void list_##prefix##_clear(list_##prefix *l);                        \
    STK_API void list_##prefix##_reverse(list_##prefix *l);                      \
    STK_API void list_##prefix##_sort(list_##prefix *l);

// 基础类型实现
#define LIST_IMPLEMENT_BASIC(type, prefix)                                             \
    static list_##prefix##_node *list_##prefix##_create_node(type val)                 \
    {                                                                                  \
        list_##prefix##_node *node = malloc(sizeof(list_##prefix##_node));             \
        if (!node)                                                                     \
            return NULL;                                                               \
        node->data = val;                                                              \
        node->prev = NULL;                                                             \
        node->next = NULL;                                                             \
        return node;                                                                   \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_init(list_##prefix *l)                                        \
    {                                                                                  \
        l->head = l->tail = NULL;                                                      \
        l->size = 0;                                                                   \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_free(list_##prefix *l)                                        \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_node *cur = l->head;                                           \
        while (cur)                                                                    \
        {                                                                              \
            list_##prefix##_node *next = cur->next;                                    \
            free(cur);                                                                 \
            cur = next;                                                                \
        }                                                                              \
        l->head = l->tail = NULL;                                                      \
        l->size = 0;                                                                   \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_push_front(list_##prefix *l, type val)                        \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_node *node = list_##prefix##_create_node(val);                 \
        if (!node)                                                                     \
            return;                                                                    \
        if (!l->head)                                                                  \
        {                                                                              \
            l->head = l->tail = node;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            node->next = l->head;                                                      \
            l->head->prev = node;                                                      \
            l->head = node;                                                            \
        }                                                                              \
        l->size++;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_push_back(list_##prefix *l, type val)                         \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_node *node = list_##prefix##_create_node(val);                 \
        if (!node)                                                                     \
            return;                                                                    \
        if (!l->tail)                                                                  \
        {                                                                              \
            l->head = l->tail = node;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            node->prev = l->tail;                                                      \
            l->tail->next = node;                                                      \
            l->tail = node;                                                            \
        }                                                                              \
        l->size++;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_pop_front(list_##prefix *l)                                   \
    {                                                                                  \
        if (!l || !l->head)                                                            \
            return;                                                                    \
        list_##prefix##_node *old = l->head;                                           \
        l->head = l->head->next;                                                       \
        if (l->head)                                                                   \
        {                                                                              \
            l->head->prev = NULL;                                                      \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            l->tail = NULL;                                                            \
        }                                                                              \
        free(old);                                                                     \
        l->size--;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_pop_back(list_##prefix *l)                                    \
    {                                                                                  \
        if (!l || !l->tail)                                                            \
            return;                                                                    \
        list_##prefix##_node *old = l->tail;                                           \
        l->tail = l->tail->prev;                                                       \
        if (l->tail)                                                                   \
        {                                                                              \
            l->tail->next = NULL;                                                      \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            l->head = NULL;                                                            \
        }                                                                              \
        free(old);                                                                     \
        l->size--;                                                                     \
    }                                                                                  \
                                                                                       \
    type list_##prefix##_front(list_##prefix *l)                                       \
    {                                                                                  \
        return (l && l->head) ? l->head->data : (type)0;                               \
    }                                                                                  \
                                                                                       \
    type list_##prefix##_back(list_##prefix *l)                                        \
    {                                                                                  \
        return (l && l->tail) ? l->tail->data : (type)0;                               \
    }                                                                                  \
                                                                                       \
    static list_##prefix##_node *list_##prefix##_node_at(list_##prefix *l, size_t idx) \
    {                                                                                  \
        if (!l || idx >= l->size)                                                      \
            return NULL;                                                               \
        list_##prefix##_node *node;                                                    \
        if (idx < l->size / 2)                                                         \
        {                                                                              \
            node = l->head;                                                            \
            for (size_t i = 0; i < idx; i++)                                           \
                node = node->next;                                                     \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            node = l->tail;                                                            \
            for (size_t i = l->size - 1; i > idx; i--)                                 \
                node = node->prev;                                                     \
        }                                                                              \
        return node;                                                                   \
    }                                                                                  \
                                                                                       \
    type list_##prefix##_get(list_##prefix *l, size_t idx)                             \
    {                                                                                  \
        list_##prefix##_node *node = list_##prefix##_node_at(l, idx);                  \
        return node ? node->data : (type)0;                                            \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_set(list_##prefix *l, size_t idx, type val)                   \
    {                                                                                  \
        list_##prefix##_node *node = list_##prefix##_node_at(l, idx);                  \
        if (node)                                                                      \
            node->data = val;                                                          \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_insert(list_##prefix *l, size_t idx, type val)                \
    {                                                                                  \
        if (!l || idx > l->size)                                                       \
            return;                                                                    \
        if (idx == 0)                                                                  \
        {                                                                              \
            list_##prefix##_push_front(l, val);                                        \
            return;                                                                    \
        }                                                                              \
        if (idx == l->size)                                                            \
        {                                                                              \
            list_##prefix##_push_back(l, val);                                         \
            return;                                                                    \
        }                                                                              \
        list_##prefix##_node *right = list_##prefix##_node_at(l, idx);                 \
        list_##prefix##_node *left = right->prev;                                      \
        list_##prefix##_node *node = list_##prefix##_create_node(val);                 \
        if (!node)                                                                     \
            return;                                                                    \
        node->prev = left;                                                             \
        node->next = right;                                                            \
        left->next = node;                                                             \
        right->prev = node;                                                            \
        l->size++;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_erase(list_##prefix *l, size_t idx)                           \
    {                                                                                  \
        if (!l || idx >= l->size)                                                      \
            return;                                                                    \
        if (idx == 0)                                                                  \
        {                                                                              \
            list_##prefix##_pop_front(l);                                              \
            return;                                                                    \
        }                                                                              \
        if (idx == l->size - 1)                                                        \
        {                                                                              \
            list_##prefix##_pop_back(l);                                               \
            return;                                                                    \
        }                                                                              \
        list_##prefix##_node *target = list_##prefix##_node_at(l, idx);                \
        if (!target)                                                                   \
            return;                                                                    \
        target->prev->next = target->next;                                             \
        target->next->prev = target->prev;                                             \
        free(target);                                                                  \
        l->size--;                                                                     \
    }                                                                                  \
                                                                                       \
    size_t list_##prefix##_len(list_##prefix *l)                                       \
    {                                                                                  \
        return l ? l->size : 0;                                                        \
    }                                                                                  \
                                                                                       \
    bool list_##prefix##_empty(list_##prefix *l)                                       \
    {                                                                                  \
        return l ? l->size == 0 : true;                                                \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_clear(list_##prefix *l)                                       \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_free(l);                                                       \
        list_##prefix##_init(l);                                                       \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_reverse(list_##prefix *l)                                     \
    {                                                                                  \
        if (!l || l->size < 2)                                                         \
            return;                                                                    \
        list_##prefix##_node *cur = l->head;                                           \
        while (cur)                                                                    \
        {                                                                              \
            list_##prefix##_node *next = cur->next;                                    \
            cur->next = cur->prev;                                                     \
            cur->prev = next;                                                          \
            cur = next;                                                                \
        }                                                                              \
        list_##prefix##_node *tmp = l->head;                                           \
        l->head = l->tail;                                                             \
        l->tail = tmp;                                                                 \
    }                                                                                  \
                                                                                       \
    static int list_##prefix##_cmp(const void *a, const void *b)                       \
    {                                                                                  \
        type va = *(const type *)a;                                                    \
        type vb = *(const type *)b;                                                    \
        return (va > vb) - (va < vb);                                                  \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_sort(list_##prefix *l)                                        \
    {                                                                                  \
        if (!l || l->size < 2)                                                         \
            return;                                                                    \
        type *arr = malloc(l->size * sizeof(type));                                    \
        if (!arr)                                                                      \
            return;                                                                    \
        size_t i = 0;                                                                  \
        for (list_##prefix##_node *cur = l->head; cur; cur = cur->next)                \
        {                                                                              \
            arr[i++] = cur->data;                                                      \
        }                                                                              \
        qsort(arr, l->size, sizeof(type), list_##prefix##_cmp);                        \
        i = 0;                                                                         \
        for (list_##prefix##_node *cur = l->head; cur; cur = cur->next)                \
        {                                                                              \
            cur->data = arr[i++];                                                      \
        }                                                                              \
        free(arr);                                                                     \
    }

// 结构体类型实现（需要自定义比较函数）
#define LIST_IMPLEMENT_STRUCT(type, prefix, cmp_func)                                  \
    static list_##prefix##_node *list_##prefix##_create_node(type val)                 \
    {                                                                                  \
        list_##prefix##_node *node = malloc(sizeof(list_##prefix##_node));             \
        if (!node)                                                                     \
            return NULL;                                                               \
        node->data = val;                                                              \
        node->prev = NULL;                                                             \
        node->next = NULL;                                                             \
        return node;                                                                   \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_init(list_##prefix *l)                                        \
    {                                                                                  \
        l->head = l->tail = NULL;                                                      \
        l->size = 0;                                                                   \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_free(list_##prefix *l)                                        \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_node *cur = l->head;                                           \
        while (cur)                                                                    \
        {                                                                              \
            list_##prefix##_node *next = cur->next;                                    \
            free(cur);                                                                 \
            cur = next;                                                                \
        }                                                                              \
        l->head = l->tail = NULL;                                                      \
        l->size = 0;                                                                   \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_push_front(list_##prefix *l, type val)                        \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_node *node = list_##prefix##_create_node(val);                 \
        if (!node)                                                                     \
            return;                                                                    \
        if (!l->head)                                                                  \
        {                                                                              \
            l->head = l->tail = node;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            node->next = l->head;                                                      \
            l->head->prev = node;                                                      \
            l->head = node;                                                            \
        }                                                                              \
        l->size++;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_push_back(list_##prefix *l, type val)                         \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_node *node = list_##prefix##_create_node(val);                 \
        if (!node)                                                                     \
            return;                                                                    \
        if (!l->tail)                                                                  \
        {                                                                              \
            l->head = l->tail = node;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            node->prev = l->tail;                                                      \
            l->tail->next = node;                                                      \
            l->tail = node;                                                            \
        }                                                                              \
        l->size++;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_pop_front(list_##prefix *l)                                   \
    {                                                                                  \
        if (!l || !l->head)                                                            \
            return;                                                                    \
        list_##prefix##_node *old = l->head;                                           \
        l->head = l->head->next;                                                       \
        if (l->head)                                                                   \
        {                                                                              \
            l->head->prev = NULL;                                                      \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            l->tail = NULL;                                                            \
        }                                                                              \
        free(old);                                                                     \
        l->size--;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_pop_back(list_##prefix *l)                                    \
    {                                                                                  \
        if (!l || !l->tail)                                                            \
            return;                                                                    \
        list_##prefix##_node *old = l->tail;                                           \
        l->tail = l->tail->prev;                                                       \
        if (l->tail)                                                                   \
        {                                                                              \
            l->tail->next = NULL;                                                      \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            l->head = NULL;                                                            \
        }                                                                              \
        free(old);                                                                     \
        l->size--;                                                                     \
    }                                                                                  \
                                                                                       \
    type list_##prefix##_front(list_##prefix *l)                                       \
    {                                                                                  \
        return (l && l->head) ? l->head->data : (type){0};                             \
    }                                                                                  \
                                                                                       \
    type list_##prefix##_back(list_##prefix *l)                                        \
    {                                                                                  \
        return (l && l->tail) ? l->tail->data : (type){0};                             \
    }                                                                                  \
                                                                                       \
    static list_##prefix##_node *list_##prefix##_node_at(list_##prefix *l, size_t idx) \
    {                                                                                  \
        if (!l || idx >= l->size)                                                      \
            return NULL;                                                               \
        list_##prefix##_node *node;                                                    \
        if (idx < l->size / 2)                                                         \
        {                                                                              \
            node = l->head;                                                            \
            for (size_t i = 0; i < idx; i++)                                           \
                node = node->next;                                                     \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            node = l->tail;                                                            \
            for (size_t i = l->size - 1; i > idx; i--)                                 \
                node = node->prev;                                                     \
        }                                                                              \
        return node;                                                                   \
    }                                                                                  \
                                                                                       \
    type list_##prefix##_get(list_##prefix *l, size_t idx)                             \
    {                                                                                  \
        list_##prefix##_node *node = list_##prefix##_node_at(l, idx);                  \
        return node ? node->data : (type){0};                                          \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_set(list_##prefix *l, size_t idx, type val)                   \
    {                                                                                  \
        list_##prefix##_node *node = list_##prefix##_node_at(l, idx);                  \
        if (node)                                                                      \
            node->data = val;                                                          \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_insert(list_##prefix *l, size_t idx, type val)                \
    {                                                                                  \
        if (!l || idx > l->size)                                                       \
            return;                                                                    \
        if (idx == 0)                                                                  \
        {                                                                              \
            list_##prefix##_push_front(l, val);                                        \
            return;                                                                    \
        }                                                                              \
        if (idx == l->size)                                                            \
        {                                                                              \
            list_##prefix##_push_back(l, val);                                         \
            return;                                                                    \
        }                                                                              \
        list_##prefix##_node *right = list_##prefix##_node_at(l, idx);                 \
        list_##prefix##_node *left = right->prev;                                      \
        list_##prefix##_node *node = list_##prefix##_create_node(val);                 \
        if (!node)                                                                     \
            return;                                                                    \
        node->prev = left;                                                             \
        node->next = right;                                                            \
        left->next = node;                                                             \
        right->prev = node;                                                            \
        l->size++;                                                                     \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_erase(list_##prefix *l, size_t idx)                           \
    {                                                                                  \
        if (!l || idx >= l->size)                                                      \
            return;                                                                    \
        if (idx == 0)                                                                  \
        {                                                                              \
            list_##prefix##_pop_front(l);                                              \
            return;                                                                    \
        }                                                                              \
        if (idx == l->size - 1)                                                        \
        {                                                                              \
            list_##prefix##_pop_back(l);                                               \
            return;                                                                    \
        }                                                                              \
        list_##prefix##_node *target = list_##prefix##_node_at(l, idx);                \
        if (!target)                                                                   \
            return;                                                                    \
        target->prev->next = target->next;                                             \
        target->next->prev = target->prev;                                             \
        free(target);                                                                  \
        l->size--;                                                                     \
    }                                                                                  \
                                                                                       \
    size_t list_##prefix##_len(list_##prefix *l)                                       \
    {                                                                                  \
        return l ? l->size : 0;                                                        \
    }                                                                                  \
                                                                                       \
    bool list_##prefix##_empty(list_##prefix *l)                                       \
    {                                                                                  \
        return l ? l->size == 0 : true;                                                \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_clear(list_##prefix *l)                                       \
    {                                                                                  \
        if (!l)                                                                        \
            return;                                                                    \
        list_##prefix##_free(l);                                                       \
        list_##prefix##_init(l);                                                       \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_reverse(list_##prefix *l)                                     \
    {                                                                                  \
        if (!l || l->size < 2)                                                         \
            return;                                                                    \
        list_##prefix##_node *cur = l->head;                                           \
        while (cur)                                                                    \
        {                                                                              \
            list_##prefix##_node *next = cur->next;                                    \
            cur->next = cur->prev;                                                     \
            cur->prev = next;                                                          \
            cur = next;                                                                \
        }                                                                              \
        list_##prefix##_node *tmp = l->head;                                           \
        l->head = l->tail;                                                             \
        l->tail = tmp;                                                                 \
    }                                                                                  \
                                                                                       \
    void list_##prefix##_sort(list_##prefix *l)                                        \
    {                                                                                  \
        if (!l || l->size < 2)                                                         \
            return;                                                                    \
        type *arr = malloc(l->size * sizeof(type));                                    \
        if (!arr)                                                                      \
            return;                                                                    \
        size_t i = 0;                                                                  \
        for (list_##prefix##_node *cur = l->head; cur; cur = cur->next)                \
        {                                                                              \
            arr[i++] = cur->data;                                                      \
        }                                                                              \
        qsort(arr, l->size, sizeof(type), cmp_func);                                   \
        i = 0;                                                                         \
        for (list_##prefix##_node *cur = l->head; cur; cur = cur->next)                \
        {                                                                              \
            cur->data = arr[i++];                                                      \
        }                                                                              \
        free(arr);                                                                     \
    }

#endif