#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/slist.h"
#include "stk/core/preset.h"

/* =========================================================================
 * 内部辅助函数
 * ========================================================================= */

/* 创建新节点 */
static stk_snode* slist_create_node(void* value) {
    stk_snode* node = (stk_snode*)malloc(sizeof(stk_snode));
    if (!node) {
        STK_LOG_ERROR("Slist create_node: malloc failed");
        return NULL;
    }
    node->data = value;
    node->next = NULL;
    return node;
}

/* 获取前驱节点（用于删除和插入操作） */
static stk_snode* slist_get_prev(const stk_slist* list, size_t index) {
    if (index == 0 || index >= list->size) {
        return NULL;
    }
    stk_snode* curr = list->head;
    for (size_t i = 0; i < index - 1; i++) {
        curr = curr->next;
    }
    return curr;
}

/* =========================================================================
 * 生命周期管理
 * ========================================================================= */

STK_STATUS stk_slist_init(stk_slist* list) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist init: NULL list pointer");
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    
    STK_LOG_DEBUG("Slist initialized");
    return STK_OK;
}

STK_STATUS stk_slist_free(stk_slist* list) {
    if (!list) {
        STK_LOG_WARN("Slist free: NULL list pointer");
        return STK_EINVAL;
    }
    
    stk_snode* curr = list->head;
    size_t count = 0;
    while (curr) {
        stk_snode* next = curr->next;
        free(curr);
        curr = next;
        count++;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    
    STK_LOG_DEBUG("Slist freed: %zu nodes released", count);
    return STK_OK;
}

STK_STATUS stk_slist_free_custom(stk_slist* list, void (*free_data)(void*)) {
    if (!list) {
        STK_LOG_WARN("Slist free_custom: NULL list pointer");
        return STK_EINVAL;
    }
    
    stk_snode* curr = list->head;
    size_t count = 0;
    while (curr) {
        stk_snode* next = curr->next;
        if (free_data && curr->data) {
            free_data(curr->data);
        }
        free(curr);
        curr = next;
        count++;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    
    STK_LOG_DEBUG("Slist freed: %zu nodes released (with custom free)", count);
    return STK_OK;
}

/* =========================================================================
 * 头部操作
 * ========================================================================= */

STK_STATUS stk_slist_push_front(stk_slist* list, void* value) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist push_front: NULL list pointer");
    
    stk_snode* node = slist_create_node(value);
    if (!node) {
        STK_LOG_ERROR("Slist push_front: failed to create node");
        return STK_ENOMEM;
    }
    
    node->next = list->head;
    list->head = node;
    
    if (list->size == 0) {
        list->tail = node;  /* 第一个节点也是尾节点 */
    }
    
    list->size++;
    
    STK_LOG_DEBUG("Slist push_front: size=%zu", list->size);
    return STK_OK;
}

STK_STATUS stk_slist_pop_front(stk_slist* list) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist pop_front: NULL list pointer");
    STK_RETURN_IF(list->size == 0, STK_EMPTY, "Slist pop_front: list is empty");
    
    stk_snode* old_head = list->head;
    list->head = list->head->next;
    
    if (list->size == 1) {
        list->tail = NULL;
    }
    
    free(old_head);
    list->size--;
    
    STK_LOG_DEBUG("Slist pop_front: size=%zu", list->size);
    return STK_OK;
}

void* stk_slist_front(const stk_slist* list) {
    if (!list) {
        STK_LOG_WARN("Slist front: NULL list pointer");
        return NULL;
    }
    if (list->size == 0) {
        STK_LOG_WARN("Slist front: list is empty");
        return NULL;
    }
    return list->head->data;
}

void* stk_slist_pop_front_value(stk_slist* list) {
    if (!list) {
        STK_LOG_WARN("Slist pop_front_value: NULL list pointer");
        return NULL;
    }
    if (list->size == 0) {
        STK_LOG_WARN("Slist pop_front_value: list is empty");
        return NULL;
    }
    
    stk_snode* old_head = list->head;
    void* value = old_head->data;
    list->head = list->head->next;
    
    if (list->size == 1) {
        list->tail = NULL;
    }
    
    free(old_head);
    list->size--;
    
    STK_LOG_DEBUG("Slist pop_front_value: size=%zu", list->size);
    return value;
}

/* =========================================================================
 * 尾部操作
 * ========================================================================= */

STK_STATUS stk_slist_push_back(stk_slist* list, void* value) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist push_back: NULL list pointer");
    
    stk_snode* node = slist_create_node(value);
    if (!node) {
        STK_LOG_ERROR("Slist push_back: failed to create node");
        return STK_ENOMEM;
    }
    
    if (list->size == 0) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    
    list->size++;
    
    STK_LOG_DEBUG("Slist push_back: size=%zu", list->size);
    return STK_OK;
}

STK_STATUS stk_slist_pop_back(stk_slist* list) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist pop_back: NULL list pointer");
    STK_RETURN_IF(list->size == 0, STK_EMPTY, "Slist pop_back: list is empty");
    
    if (list->size == 1) {
        free(list->head);
        list->head = NULL;
        list->tail = NULL;
    } else {
        /* 需要找到倒数第二个节点 */
        stk_snode* curr = list->head;
        while (curr->next != list->tail) {
            curr = curr->next;
        }
        free(list->tail);
        curr->next = NULL;
        list->tail = curr;
    }
    
    list->size--;
    
    STK_LOG_DEBUG("Slist pop_back: size=%zu", list->size);
    return STK_OK;
}

void* stk_slist_back(const stk_slist* list) {
    if (!list) {
        STK_LOG_WARN("Slist back: NULL list pointer");
        return NULL;
    }
    if (list->size == 0) {
        STK_LOG_WARN("Slist back: list is empty");
        return NULL;
    }
    return list->tail->data;
}

void* stk_slist_pop_back_value(stk_slist* list) {
    if (!list) {
        STK_LOG_WARN("Slist pop_back_value: NULL list pointer");
        return NULL;
    }
    if (list->size == 0) {
        STK_LOG_WARN("Slist pop_back_value: list is empty");
        return NULL;
    }
    
    void* value;
    
    if (list->size == 1) {
        value = list->head->data;
        free(list->head);
        list->head = NULL;
        list->tail = NULL;
    } else {
        /* 找到倒数第二个节点 */
        stk_snode* curr = list->head;
        while (curr->next != list->tail) {
            curr = curr->next;
        }
        value = list->tail->data;
        free(list->tail);
        curr->next = NULL;
        list->tail = curr;
    }
    
    list->size--;
    
    STK_LOG_DEBUG("Slist pop_back_value: size=%zu", list->size);
    return value;
}

/* =========================================================================
 * 随机访问
 * ========================================================================= */

void* stk_slist_get(const stk_slist* list, size_t index) {
    if (!list) {
        STK_LOG_WARN("Slist get: NULL list pointer");
        return NULL;
    }
    if (index >= list->size) {
        STK_LOG_WARN("Slist get: index %zu out of range (size=%zu)", index, list->size);
        return NULL;
    }
    
    stk_snode* curr = list->head;
    for (size_t i = 0; i < index; i++) {
        curr = curr->next;
    }
    return curr->data;
}

STK_STATUS stk_slist_set(stk_slist* list, size_t index, void* value) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist set: NULL list pointer");
    
    if (index >= list->size) {
        STK_LOG_ERROR("Slist set: index %zu out of range (size=%zu)", index, list->size);
        return STK_ERANGE;
    }
    
    stk_snode* curr = list->head;
    for (size_t i = 0; i < index; i++) {
        curr = curr->next;
    }
    curr->data = value;
    
    return STK_OK;
}

STK_STATUS stk_slist_insert(stk_slist* list, size_t index, void* value) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist insert: NULL list pointer");
    
    if (index > list->size) {
        STK_LOG_ERROR("Slist insert: index %zu out of range (size=%zu)", index, list->size);
        return STK_ERANGE;
    }
    
    /* 头部插入 */
    if (index == 0) {
        return stk_slist_push_front(list, value);
    }
    
    /* 尾部插入 */
    if (index == list->size) {
        return stk_slist_push_back(list, value);
    }
    
    /* 中间插入 */
    stk_snode* prev = slist_get_prev(list, index);
    if (!prev) {
        STK_LOG_ERROR("Slist insert: failed to find predecessor");
        return STK_ERROR;
    }
    
    stk_snode* node = slist_create_node(value);
    if (!node) {
        STK_LOG_ERROR("Slist insert: failed to create node");
        return STK_ENOMEM;
    }
    
    node->next = prev->next;
    prev->next = node;
    list->size++;
    
    STK_LOG_DEBUG("Slist insert: inserted at index %zu, size=%zu", index, list->size);
    return STK_OK;
}

STK_STATUS stk_slist_erase(stk_slist* list, size_t index) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist erase: NULL list pointer");
    
    if (index >= list->size) {
        STK_LOG_ERROR("Slist erase: index %zu out of range (size=%zu)", index, list->size);
        return STK_ERANGE;
    }
    
    /* 头部删除 */
    if (index == 0) {
        return stk_slist_pop_front(list);
    }
    
    /* 中间或尾部删除 */
    stk_snode* prev = slist_get_prev(list, index);
    if (!prev) {
        STK_LOG_ERROR("Slist erase: failed to find predecessor");
        return STK_ERROR;
    }
    
    stk_snode* target = prev->next;
    prev->next = target->next;
    
    /* 如果删除的是尾节点，更新 tail */
    if (target == list->tail) {
        list->tail = prev;
    }
    
    free(target);
    list->size--;
    
    STK_LOG_DEBUG("Slist erase: erased index %zu, size=%zu", index, list->size);
    return STK_OK;
}

/* =========================================================================
 * 查找操作
 * ========================================================================= */

size_t stk_slist_find(const stk_slist* list, const void* value,
                       bool (*equal)(const void* a, const void* b)) {
    if (!list || !equal) {
        if (list) STK_LOG_WARN("Slist find: NULL %s", !equal ? "equal function" : "list");
        return (size_t)-1;
    }
    
    stk_snode* curr = list->head;
    for (size_t i = 0; curr; i++) {
        if (equal(curr->data, value)) {
            return i;
        }
        curr = curr->next;
    }
    return (size_t)-1;
}

bool stk_slist_contains(const stk_slist* list, const void* value,
                         bool (*equal)(const void* a, const void* b)) {
    size_t idx = stk_slist_find(list, value, equal);
    return idx != (size_t)-1;
}

/* =========================================================================
 * 查询接口
 * ========================================================================= */

size_t stk_slist_size(const stk_slist* list) {
    return list ? list->size : 0;
}

bool stk_slist_empty(const stk_slist* list) {
    return list ? list->size == 0 : true;
}

/* =========================================================================
 * 修改操作
 * ========================================================================= */

STK_STATUS stk_slist_clear(stk_slist* list) {
    return stk_slist_free(list);
}

STK_STATUS stk_slist_clear_custom(stk_slist* list, void (*free_data)(void*)) {
    return stk_slist_free_custom(list, free_data);
}

STK_STATUS stk_slist_reverse(stk_slist* list) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist reverse: NULL list pointer");
    
    if (list->size < 2) {
        return STK_OK;  /* 不需要反转 */
    }
    
    stk_snode* prev = NULL;
    stk_snode* curr = list->head;
    stk_snode* next = NULL;
    
    /* 更新 tail 为原来的 head */
    list->tail = list->head;
    
    while (curr) {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    
    list->head = prev;
    
    STK_LOG_DEBUG("Slist reversed");
    return STK_OK;
}

STK_STATUS stk_slist_swap(stk_slist* a, stk_slist* b) {
    STK_RETURN_IF(!a, STK_EINVAL, "Slist swap: NULL first list pointer");
    STK_RETURN_IF(!b, STK_EINVAL, "Slist swap: NULL second list pointer");
    
    stk_slist tmp = *a;
    *a = *b;
    *b = tmp;
    
    STK_LOG_DEBUG("Slist swapped");
    return STK_OK;
}

/* =========================================================================
 * 遍历操作
 * ========================================================================= */

STK_STATUS stk_slist_foreach(const stk_slist* list, 
                              bool (*fn)(void* data, void* user_data),
                              void* user_data) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist foreach: NULL list pointer");
    STK_RETURN_IF(!fn, STK_EINVAL, "Slist foreach: NULL callback function");
    
    stk_snode* curr = list->head;
    while (curr) {
        if (!fn(curr->data, user_data)) {
            break;
        }
        curr = curr->next;
    }
    return STK_OK;
}

STK_STATUS stk_slist_foreach_mut(stk_slist* list,
                                  bool (*fn)(void** data, void* user_data),
                                  void* user_data) {
    STK_RETURN_IF(!list, STK_EINVAL, "Slist foreach_mut: NULL list pointer");
    STK_RETURN_IF(!fn, STK_EINVAL, "Slist foreach_mut: NULL callback function");
    
    stk_snode* curr = list->head;
    while (curr) {
        if (!fn(&curr->data, user_data)) {
            break;
        }
        curr = curr->next;
    }
    return STK_OK;
}

/* =========================================================================
 * 高级操作
 * ========================================================================= */

STK_STATUS stk_slist_merge(stk_slist* dst, stk_slist* src) {
    STK_RETURN_IF(!dst, STK_EINVAL, "Slist merge: NULL destination list");
    STK_RETURN_IF(!src, STK_EINVAL, "Slist merge: NULL source list");
    
    if (src->size == 0) {
        return STK_OK;  /* 源链表为空，无事可做 */
    }
    
    if (dst->size == 0) {
        /* 目标链表为空，直接接管 */
        dst->head = src->head;
        dst->tail = src->tail;
        dst->size = src->size;
    } else {
        /* 将源链表连接到目标链表尾部 */
        dst->tail->next = src->head;
        dst->tail = src->tail;
        dst->size += src->size;
    }
    
    /* 清空源链表 */
    src->head = NULL;
    src->tail = NULL;
    src->size = 0;
    
    STK_LOG_DEBUG("Slist merged: new size=%zu", dst->size);
    return STK_OK;
}

size_t stk_slist_remove_all(stk_slist* list, const void* value,
                             bool (*equal)(const void* a, const void* b)) {
    if (!list || !equal) {
        if (list) STK_LOG_WARN("Slist remove_all: NULL %s", !equal ? "equal function" : "list");
        return 0;
    }
    
    size_t removed = 0;
    
    /* 处理头部连续匹配的情况 */
    while (list->head && equal(list->head->data, value)) {
        stk_snode* old = list->head;
        list->head = list->head->next;
        free(old);
        list->size--;
        removed++;
    }
    
    if (list->size == 0) {
        list->tail = NULL;
        STK_LOG_DEBUG("Slist remove_all: removed %zu elements", removed);
        return removed;
    }
    
    /* 处理中间和尾部 */
    stk_snode* curr = list->head;
    while (curr && curr->next) {
        if (equal(curr->next->data, value)) {
            stk_snode* target = curr->next;
            curr->next = target->next;
            if (target == list->tail) {
                list->tail = curr;
            }
            free(target);
            list->size--;
            removed++;
        } else {
            curr = curr->next;
        }
    }
    
    STK_LOG_DEBUG("Slist remove_all: removed %zu elements", removed);
    return removed;
}

stk_snode* stk_slist_node_at(const stk_slist* list, size_t index) {
    if (!list || index >= list->size) {
        if (list) STK_LOG_WARN("Slist node_at: index %zu out of range", index);
        return NULL;
    }
    
    stk_snode* curr = list->head;
    for (size_t i = 0; i < index; i++) {
        curr = curr->next;
    }
    return curr;
}