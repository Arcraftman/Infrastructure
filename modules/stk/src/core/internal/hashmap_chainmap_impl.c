#include "stk/core/internal/hashmap_chainmap.h"

#include <stdlib.h>
#include <string.h>

/* chain_find_slot: 返回「指向匹配节点的指针的指针」。
 * - 命中时：返回该节点前驱的 next 字段地址（桶头时即 &buckets[idx]）。
 * - 未命中时：返回该 key 应插入位置的地址（即 &buckets[idx]，链尾）。
 * 统一让 set/remove 对桶头与链中节点做 O(1) 操作。 */
static stk_chain_node **chain_find_slot(stk_chainmap *m, const void *key) {
    size_t idx = m->hash(key) % m->bucket_count;
    stk_chain_node **pp = &m->buckets[idx];
    while (*pp) {
        if (m->eq((*pp)->key, key)) return pp;
        pp = &(*pp)->next;
    }
    return pp; /* 空桶或链尾，作为插入点 */
}

void stk_i_chainmap_init(stk_chainmap *m, size_t bucket_count,
                         stk_hash_fn hash, stk_eq_fn eq,
                         const stk_allocator *alloc) {
    if (m == NULL) return;
    if (bucket_count == 0) bucket_count = 16; /* 最小桶数兜底 */
    if (hash == NULL) hash = stk_i_hash_ptr;
    if (eq == NULL)   eq = stk_i_eq_ptr;
    if (alloc == NULL) alloc = &STK_ALLOCATOR_DEFAULT;

    m->bucket_count = bucket_count;
    m->count = 0;
    m->hash = hash;
    m->eq = eq;
    m->alloc = alloc;
    m->buckets = (stk_chain_node **)stk_i_alloc(alloc,
                    bucket_count * sizeof(stk_chain_node *));
    /* 全部置 NULL（空桶） */
    for (size_t i = 0; i < bucket_count; i++) m->buckets[i] = NULL;
}

void stk_i_chainmap_free(stk_chainmap *m) {
    if (m == NULL || m->buckets == NULL) return;
    stk_i_chainmap_clear(m);
    stk_i_free(m->alloc, m->buckets);
    m->buckets = NULL;
    m->bucket_count = 0;
    m->count = 0;
}

int stk_i_chainmap_set(stk_chainmap *m, void *key, void *value) {
    if (m == NULL) return 0;
    stk_chain_node **pp = chain_find_slot(m, key);
    if (*pp) {
        (*pp)->value = value; /* 覆盖，不换 key（key 仍归调用方） */
        return 1;
    }
    stk_chain_node *node = (stk_chain_node *)stk_i_alloc(m->alloc,
                                                         sizeof(stk_chain_node));
    if (node == NULL) return 0; /* 分配失败，静默失败 */
    node->key = key;
    node->value = value;
    node->next = NULL;
    *pp = node; /* 头插（*pp 指向桶头或链尾的 next） */
    m->count++;
    return 0;
}

void *stk_i_chainmap_get(const stk_chainmap *m, const void *key) {
    if (m == NULL || m->buckets == NULL) return NULL;
    size_t idx = m->hash(key) % m->bucket_count;
    stk_chain_node *cur = m->buckets[idx];
    while (cur) {
        if (m->eq(cur->key, key)) return cur->value;
        cur = cur->next;
    }
    return NULL;
}

int stk_i_chainmap_remove(stk_chainmap *m, const void *key) {
    if (m == NULL) return 0;
    stk_chain_node **pp = chain_find_slot(m, key);
    if (*pp == NULL) return 0; /* 没找到 */
    stk_chain_node *dead = *pp;
    *pp = dead->next;          /* 解链（桶头或链中统一处理） */
    stk_i_free(m->alloc, dead);
    m->count--;
    return 1;
}

void stk_i_chainmap_clear(stk_chainmap *m) {
    if (m == NULL || m->buckets == NULL) return;
    for (size_t i = 0; i < m->bucket_count; i++) {
        stk_chain_node *cur = m->buckets[i];
        while (cur) {
            stk_chain_node *next = cur->next;
            stk_i_free(m->alloc, cur);
            cur = next;
        }
        m->buckets[i] = NULL;
    }
    m->count = 0;
}

int stk_i_chainmap_reserve(stk_chainmap *m, size_t new_bucket_count) {
    if (m == NULL || m->buckets == NULL) return 0;
    if (new_bucket_count <= m->bucket_count) return 0; /* 不缩容、不重复扩容 */

    stk_chain_node **new_buckets = (stk_chain_node **)stk_i_alloc(
        m->alloc, new_bucket_count * sizeof(stk_chain_node *));
    if (new_buckets == NULL) return 0;
    for (size_t i = 0; i < new_bucket_count; i++) new_buckets[i] = NULL;

    /* 原地复用旧节点：把每个旧节点重新挂到新桶（不 free 节点本身） */
    for (size_t i = 0; i < m->bucket_count; i++) {
        stk_chain_node *cur = m->buckets[i];
        while (cur) {
            stk_chain_node *next = cur->next;
            size_t idx = m->hash(cur->key) % new_bucket_count;
            cur->next = new_buckets[idx]; /* 头插到新桶 */
            new_buckets[idx] = cur;
            cur = next;
        }
    }

    stk_i_free(m->alloc, m->buckets);
    m->buckets = new_buckets;
    m->bucket_count = new_bucket_count;
    return 1;
}

/* ---- 迭代器 ---- */

void stk_i_chain_iter_begin(stk_chain_iter *it, const stk_chainmap *m) {
    it->map = m;
    it->bucket = 0;
    it->cur = NULL;
    it->next = NULL;
    if (m && m->buckets) {
        /* 找到第一个非空桶的头节点作为 next */
        while (it->bucket < m->bucket_count) {
            if (m->buckets[it->bucket]) { it->next = m->buckets[it->bucket]; break; }
            it->bucket++;
        }
    }
}

int stk_i_chain_iter_next(stk_chain_iter *it) {
    if (it->next == NULL) return 0; /* 已结束 */
    it->cur = it->next;
    /* 预取下一个：先沿着当前桶链表走，空了再跳下一非空桶 */
    if (it->cur->next) {
        it->next = it->cur->next;
    } else {
        it->bucket++;
        it->next = NULL;
        while (it->bucket < it->map->bucket_count) {
            if (it->map->buckets[it->bucket]) { it->next = it->map->buckets[it->bucket]; break; }
            it->bucket++;
        }
    }
    return 1;
}

void *stk_i_chain_iter_key(const stk_chain_iter *it) {
    return it->cur ? it->cur->key : NULL;
}
void *stk_i_chain_iter_value(const stk_chain_iter *it) {
    return it->cur ? it->cur->value : NULL;
}

int stk_i_chain_iter_remove(stk_chain_iter *it) {
    if (it->cur == NULL) return 0;
    /* 委托删除：根据 key 从表中摘除当前节点（unlink 前驱，free 节点）。
     * 因为 next 已 pre-fetch，删除 cur 不影响后续遍历。 */
    int r = stk_i_chainmap_remove((stk_chainmap *)it->map, it->cur->key);
    it->cur = NULL; /* 当前项已无效，next 继续有效 */
    return r;
}
