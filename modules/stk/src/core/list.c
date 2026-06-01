#include "stk/core/list.h"


static stk_list_node *create_node(void *val) {
    stk_list_node *node = malloc(sizeof(stk_list_node));
    if (!node) return NULL;
    node->data = val;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void stk_list_init(stk_list *l) {
    if (!l) return;
    l->head = l->tail = NULL;
    l->size = 0;
}

void stk_list_free(stk_list *l) {
    if (!l) return;
    stk_list_node *cur = l->head;
    while (cur) {
        stk_list_node *next = cur->next;
        free(cur);
        cur = next;
    }
    l->head = l->tail = NULL;
    l->size = 0;
}

void stk_list_push_front(stk_list *l, void *val) {
    if (!l) return;
    stk_list_node *node = create_node(val);
    if (!node) return;
    
    if (!l->head) {
        l->head = l->tail = node;
    } else {
        l->head->prev = node;
        node->next = l->head;
        l->head = node;
    }
    l->size++;
}

void stk_list_push_back(stk_list *l, void *val) {
    if (!l) return;
    stk_list_node *node = create_node(val);
    if (!node) return;
    
    if (!l->tail) {
        l->head = l->tail = node;
    } else {
        l->tail->next = node;
        node->prev = l->tail;
        l->tail = node;
    }
    l->size++;
}

void stk_list_pop_front(stk_list *l) {
    if (!l || !l->head) return;
    
    stk_list_node *old_head = l->head;
    l->head = l->head->next;
    
    if (l->head) {
        l->head->prev = NULL;
    } else {
        l->tail = NULL;
    }
    
    free(old_head);
    l->size--;
}

void stk_list_pop_back(stk_list *l) {
    if (!l || !l->tail) return;
    
    stk_list_node *old_tail = l->tail;
    l->tail = l->tail->prev;
    
    if (l->tail) {
        l->tail->next = NULL;
    } else {
        l->head = NULL;
    }
    
    free(old_tail);
    l->size--;
}

void *stk_list_front(stk_list *l) {
    return (l && l->head) ? l->head->data : NULL;
}

void *stk_list_back(stk_list *l) {
    return (l && l->tail) ? l->tail->data : NULL;
}

static stk_list_node *node_at(stk_list *l, size_t idx) {
    if (!l || idx >= l->size) return NULL;
    
    stk_list_node *node;
    if (idx < l->size / 2) {
        node = l->head;
        for (size_t i = 0; i < idx; i++)
            node = node->next;
    } else {
        node = l->tail;
        for (size_t i = l->size - 1; i > idx; i--)
            node = node->prev;
    }
    return node;
}

void *stk_list_get(stk_list *l, size_t idx) {
    stk_list_node *node = node_at(l, idx);
    return node ? node->data : NULL;
}

void stk_list_set(stk_list *l, size_t idx, void *val) {
    stk_list_node *node = node_at(l, idx);
    if (node) node->data = val;
}

void stk_list_insert(stk_list *l, size_t idx, void *val) {
    if (!l || idx > l->size) return;
    
    if (idx == 0) {
        stk_list_push_front(l, val);
        return;
    }
    if (idx == l->size) {
        stk_list_push_back(l, val);
        return;
    }
    
    stk_list_node *right = node_at(l, idx);
    stk_list_node *left = right->prev;
    stk_list_node *node = create_node(val);
    if (!node) return;
    
    node->prev = left;
    node->next = right;
    left->next = node;
    right->prev = node;
    l->size++;
}

void stk_list_erase(stk_list *l, size_t idx) {
    if (!l || idx >= l->size) return;
    
    if (idx == 0) {
        stk_list_pop_front(l);
        return;
    }
    if (idx == l->size - 1) {
        stk_list_pop_back(l);
        return;
    }
    
    stk_list_node *target = node_at(l, idx);
    if (!target) return;
    
    target->prev->next = target->next;
    target->next->prev = target->prev;
    free(target);
    l->size--;
}

size_t stk_list_len(stk_list *l) {
    return l ? l->size : 0;
}

bool stk_list_empty(stk_list *l) {
    return l ? l->size == 0 : true;
}

void stk_list_clear(stk_list *l) {
    if (!l) return;
    stk_list_free(l);
}
