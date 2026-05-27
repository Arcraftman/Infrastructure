#include "list.h"

static list_node *create_node(void *val) {
    list_node *node = malloc(sizeof(list_node));
    if (!node) return NULL;
    node->data = val;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void list_init(list *l) {
    if (!l) return;
    l->head = l->tail = NULL;
    l->size = 0;
}

void list_free(list *l) {
    if (!l) return;
    list_node *cur = l->head;
    while (cur) {
        list_node *next = cur->next;
        free(cur);
        cur = next;
    }
    l->head = l->tail = NULL;
    l->size = 0;
}

void list_push_front(list *l, void *val) {
    if (!l) return;
    list_node *node = create_node(val);
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

void list_push_back(list *l, void *val) {
    if (!l) return;
    list_node *node = create_node(val);
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

void list_pop_front(list *l) {
    if (!l || !l->head) return;
    
    list_node *old_head = l->head;
    l->head = l->head->next;
    
    if (l->head) {
        l->head->prev = NULL;
    } else {
        l->tail = NULL;
    }
    
    free(old_head);
    l->size--;
}

void list_pop_back(list *l) {
    if (!l || !l->tail) return;
    
    list_node *old_tail = l->tail;
    l->tail = l->tail->prev;
    
    if (l->tail) {
        l->tail->next = NULL;
    } else {
        l->head = NULL;
    }
    
    free(old_tail);
    l->size--;
}

void *list_front(list *l) {
    return (l && l->head) ? l->head->data : NULL;
}

void *list_back(list *l) {
    return (l && l->tail) ? l->tail->data : NULL;
}

static list_node *node_at(list *l, size_t idx) {
    if (!l || idx >= l->size) return NULL;
    
    list_node *node;
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

void *list_get(list *l, size_t idx) {
    list_node *node = node_at(l, idx);
    return node ? node->data : NULL;
}

void list_set(list *l, size_t idx, void *val) {
    list_node *node = node_at(l, idx);
    if (node) node->data = val;
}

void list_insert(list *l, size_t idx, void *val) {
    if (!l || idx > l->size) return;
    
    if (idx == 0) {
        list_push_front(l, val);
        return;
    }
    if (idx == l->size) {
        list_push_back(l, val);
        return;
    }
    
    list_node *right = node_at(l, idx);
    list_node *left = right->prev;
    list_node *node = create_node(val);
    if (!node) return;
    
    node->prev = left;
    node->next = right;
    left->next = node;
    right->prev = node;
    l->size++;
}

void list_erase(list *l, size_t idx) {
    if (!l || idx >= l->size) return;
    
    if (idx == 0) {
        list_pop_front(l);
        return;
    }
    if (idx == l->size - 1) {
        list_pop_back(l);
        return;
    }
    
    list_node *target = node_at(l, idx);
    if (!target) return;
    
    target->prev->next = target->next;
    target->next->prev = target->prev;
    free(target);
    l->size--;
}

size_t list_len(list *l) {
    return l ? l->size : 0;
}

bool list_empty(list *l) {
    return l ? l->size == 0 : true;
}

void list_clear(list *l) {
    if (!l) return;
    list_free(l);
}

LIST_IMPLEMENT_STRUCT(str,str,str_cmp)