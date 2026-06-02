#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/list.h"

static stk_list_node *create_node(void *val) {
    stk_list_node *node = malloc(sizeof(stk_list_node));
    if (!node) {
        STK_LOG_ERROR("List create_node: malloc failed");
        return NULL;
    }
    node->data = val;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

STK_STATUS stk_list_init(stk_list *l) {
    STK_RETURN_IF(!l, STK_EINVAL, "List init: NULL list pointer");
    
    l->head = l->tail = NULL;
    l->size = 0;
    
    STK_LOG_DEBUG("List initialized");
    return STK_OK;
}

STK_STATUS stk_list_free(stk_list *l) {
    if (!l) {
        STK_LOG_WARN("List free: NULL list pointer");
        return STK_EINVAL;
    }
    
    stk_list_node *cur = l->head;
    size_t count = 0;
    while (cur) {
        stk_list_node *next = cur->next;
        free(cur);
        cur = next;
        count++;
    }
    l->head = l->tail = NULL;
    l->size = 0;
    
    STK_LOG_DEBUG("List freed: %zu nodes released", count);
    return STK_OK;
}

STK_STATUS stk_list_push_front(stk_list *l, void *val) {
    STK_RETURN_IF(!l, STK_EINVAL, "List push_front: NULL list pointer");
    
    stk_list_node *node = create_node(val);
    if (!node) {
        STK_LOG_ERROR("List push_front: failed to create node");
        return STK_ENOMEM;
    }
    
    if (!l->head) {
        l->head = l->tail = node;
    } else {
        l->head->prev = node;
        node->next = l->head;
        l->head = node;
    }
    l->size++;
    
    STK_LOG_DEBUG("List push_front: size=%zu", l->size);
    return STK_OK;
}

STK_STATUS stk_list_push_back(stk_list *l, void *val) {
    STK_RETURN_IF(!l, STK_EINVAL, "List push_back: NULL list pointer");
    
    stk_list_node *node = create_node(val);
    if (!node) {
        STK_LOG_ERROR("List push_back: failed to create node");
        return STK_ENOMEM;
    }
    
    if (!l->tail) {
        l->head = l->tail = node;
    } else {
        l->tail->next = node;
        node->prev = l->tail;
        l->tail = node;
    }
    l->size++;
    
    STK_LOG_DEBUG("List push_back: size=%zu", l->size);
    return STK_OK;
}

STK_STATUS stk_list_pop_front(stk_list *l) {
    STK_RETURN_IF(!l, STK_EINVAL, "List pop_front: NULL list pointer");
    STK_RETURN_IF(!l->head, STK_EMPTY, "List pop_front: list is empty");
    
    stk_list_node *old_head = l->head;
    l->head = l->head->next;
    
    if (l->head) {
        l->head->prev = NULL;
    } else {
        l->tail = NULL;
    }
    
    free(old_head);
    l->size--;
    
    STK_LOG_DEBUG("List pop_front: size=%zu", l->size);
    return STK_OK;
}

STK_STATUS stk_list_pop_back(stk_list *l) {
    STK_RETURN_IF(!l, STK_EINVAL, "List pop_back: NULL list pointer");
    STK_RETURN_IF(!l->tail, STK_EMPTY, "List pop_back: list is empty");
    
    stk_list_node *old_tail = l->tail;
    l->tail = l->tail->prev;
    
    if (l->tail) {
        l->tail->next = NULL;
    } else {
        l->head = NULL;
    }
    
    free(old_tail);
    l->size--;
    
    STK_LOG_DEBUG("List pop_back: size=%zu", l->size);
    return STK_OK;
}

void *stk_list_front(stk_list *l) {
    if (!l) {
        STK_LOG_WARN("List front: NULL list pointer");
        return NULL;
    }
    return l->head ? l->head->data : NULL;
}

void *stk_list_back(stk_list *l) {
    if (!l) {
        STK_LOG_WARN("List back: NULL list pointer");
        return NULL;
    }
    return l->tail ? l->tail->data : NULL;
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
    if (!l) {
        STK_LOG_WARN("List get: NULL list pointer");
        return NULL;
    }
    stk_list_node *node = node_at(l, idx);
    if (!node && idx < l->size) {
        STK_LOG_WARN("List get: index %zu out of range", idx);
    }
    return node ? node->data : NULL;
}

STK_STATUS stk_list_set(stk_list *l, size_t idx, void *val) {
    STK_RETURN_IF(!l, STK_EINVAL, "List set: NULL list pointer");
    
    stk_list_node *node = node_at(l, idx);
    if (!node) {
        STK_LOG_ERROR("List set: index %zu out of range (size=%zu)", idx, l->size);
        return STK_ERANGE;
    }
    node->data = val;
    return STK_OK;
}

STK_STATUS stk_list_insert(stk_list *l, size_t idx, void *val) {
    STK_RETURN_IF(!l, STK_EINVAL, "List insert: NULL list pointer");
    
    if (idx > l->size) {
        STK_LOG_ERROR("List insert: index %zu out of range (size=%zu)", idx, l->size);
        return STK_ERANGE;
    }
    
    if (idx == 0) return stk_list_push_front(l, val);
    if (idx == l->size) return stk_list_push_back(l, val);
    
    stk_list_node *right = node_at(l, idx);
    if (!right) {
        STK_LOG_ERROR("List insert: failed to find node at index %zu", idx);
        return STK_ERROR;
    }
    
    stk_list_node *left = right->prev;
    stk_list_node *node = create_node(val);
    if (!node) {
        STK_LOG_ERROR("List insert: failed to create node");
        return STK_ENOMEM;
    }
    
    node->prev = left;
    node->next = right;
    left->next = node;
    right->prev = node;
    l->size++;
    
    STK_LOG_DEBUG("List insert: inserted at index %zu, size=%zu", idx, l->size);
    return STK_OK;
}

STK_STATUS stk_list_erase(stk_list *l, size_t idx) {
    STK_RETURN_IF(!l, STK_EINVAL, "List erase: NULL list pointer");
    
    if (idx >= l->size) {
        STK_LOG_ERROR("List erase: index %zu out of range (size=%zu)", idx, l->size);
        return STK_ERANGE;
    }
    
    if (idx == 0) return stk_list_pop_front(l);
    if (idx == l->size - 1) return stk_list_pop_back(l);
    
    stk_list_node *target = node_at(l, idx);
    if (!target) {
        STK_LOG_ERROR("List erase: failed to find node at index %zu", idx);
        return STK_ERROR;
    }
    
    target->prev->next = target->next;
    target->next->prev = target->prev;
    free(target);
    l->size--;
    
    STK_LOG_DEBUG("List erase: erased index %zu, size=%zu", idx, l->size);
    return STK_OK;
}

size_t stk_list_len(stk_list *l) {
    return l ? l->size : 0;
}

bool stk_list_empty(stk_list *l) {
    return l ? l->size == 0 : true;
}

STK_STATUS stk_list_clear(stk_list *l) {
    return stk_list_free(l);
}