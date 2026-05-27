#ifndef STK_SRC_CORE_LIST_H
#define STK_SRC_CORE_LIST_H

#include "config.h"
#include "detail/list.h"
#include "str.h"


// 链表节点
typedef struct list_node {
    void* data;
    struct list_node* prev;
    struct list_node* next;
} list_node;

// 链表结构
typedef struct list {
    list_node* head;
    list_node* tail;
    size_t size;
} list;

// 核心操作
STK_API void list_init(list* l);
STK_API void list_free(list* l);
STK_API void list_push_front(list* l, void* val);
STK_API void list_push_back(list* l, void* val);
STK_API void list_pop_front(list* l);
STK_API void list_pop_back(list* l);
STK_API void* list_front(list* l);
STK_API void* list_back(list* l);
STK_API void* list_get(list* l, size_t idx);
STK_API void list_set(list* l, size_t idx, void* val);
STK_API void list_insert(list* l, size_t idx, void* val);
STK_API void list_erase(list* l, size_t idx);
STK_API size_t list_len(list* l);
STK_API bool list_empty(list* l);
STK_API void list_clear(list* l);

LIST_DECLARE_STRUCT(str,str)


#endif