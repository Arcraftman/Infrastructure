#ifndef STK_CORE_LIST_H
#define STK_CORE_LIST_H

#include "stk/core/preset.h"
#include "detail/list.h"


// Linked list node
typedef struct stk_list_node {
    void* data;
    struct stk_list_node* prev;
    struct stk_list_node* next;
} stk_list_node;

// Linked list structure
typedef struct stk_list {
    stk_list_node* head;
    stk_list_node* tail;
    size_t size;
} stk_list;

// Core operations
STK_API void stk_list_init(stk_list* l);
STK_API void stk_list_free(stk_list* l);
STK_API void stk_list_push_front(stk_list* l, void* val);
STK_API void stk_list_push_back(stk_list* l, void* val);
STK_API void stk_list_pop_front(stk_list* l);
STK_API void stk_list_pop_back(stk_list* l);
STK_API void* stk_list_front(stk_list* l);
STK_API void* stk_list_back(stk_list* l);
STK_API void* stk_list_get(stk_list* l, size_t idx);
STK_API void stk_list_set(stk_list* l, size_t idx, void* val);
STK_API void stk_list_insert(stk_list* l, size_t idx, void* val);
STK_API void stk_list_erase(stk_list* l, size_t idx);
STK_API size_t stk_list_len(stk_list* l);
STK_API bool stk_list_empty(stk_list* l);
STK_API void stk_list_clear(stk_list* l);


#endif