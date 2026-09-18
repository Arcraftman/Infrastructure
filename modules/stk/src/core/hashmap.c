#include "stk/core/hashmap.h"
#include "stk/core/allocator.h"
#include "internal/hashmap_private.h"
#include "internal/hashmap_chainmap.h"
#include "internal/hashmap_oamap.h"

static const size_t STK_DEFAULT_CAPACITY = 16;

/* ---------------- 创建 / 销毁 ---------------- */

stk_hashmap *stk_hashmap_create(stk_hashmap_mode mode, size_t capacity,
                                stk_hash_fn hash, stk_eq_fn eq,
                                const stk_allocator *alloc) {
    return stk_hashmap_create_full(mode, capacity, hash, eq, NULL, NULL, alloc);
}

stk_hashmap *stk_hashmap_create_full(stk_hashmap_mode mode, size_t capacity,
                                     stk_hash_fn hash, stk_eq_fn eq,
                                     stk_destroy_fn key_free,
                                     stk_destroy_fn val_free,
                                     const stk_allocator *alloc) {
    if (!hash || !eq) return NULL;
    stk_hashmap *m = (stk_hashmap *)stk_alloc(alloc, sizeof(stk_hashmap));
    if (!m) return NULL;
    m->mode = mode;
    m->version = 0;
    m->alloc = alloc;
    m->key_free = key_free;
    m->val_free = val_free;
    size_t cap = capacity ? capacity : STK_DEFAULT_CAPACITY;
    if (mode == STK_HASHMAP_CHAIN)
        stk_i_chainmap_init(&m->u.chain, cap, hash, eq, alloc);
    else
        stk_i_oamap_init(&m->u.oa, cap, hash, eq, alloc);
    return m;
}

void stk_hashmap_destroy(stk_hashmap *m) {
    if (!m) return;
    if (m->mode == STK_HASHMAP_CHAIN) {
        stk_chainmap *c = &m->u.chain;
        if (m->key_free || m->val_free) {
            for (size_t i = 0; i < c->bucket_count; i++)
                for (stk_chain_node *n = c->buckets[i]; n; n = n->next) {
                    if (m->key_free) m->key_free(n->key);
                    if (m->val_free) m->val_free(n->value);
                }
        }
        stk_i_chainmap_free(c);
    } else {
        stk_oamap *o = &m->u.oa;
        if (m->key_free || m->val_free) {
            for (size_t i = 0; i < o->slot_count; i++)
                if (o->slots[i].state == STK_OA_OCCUPIED) {
                    if (m->key_free) m->key_free(o->slots[i].key);
                    if (m->val_free) m->val_free(o->slots[i].value);
                }
        }
        stk_i_oamap_free(o);
    }
    stk_dealloc(m->alloc, m);
}

/* ---------------- 核心操作 ---------------- */

int stk_hashmap_set(stk_hashmap *m, void *key, void *value) {
    if (!m) return -1;
    int r = (m->mode == STK_HASHMAP_CHAIN)
        ? stk_i_chainmap_set(&m->u.chain, key, value, NULL)
        : stk_i_oamap_set(&m->u.oa, key, value, NULL);
    if (r >= 0) m->version++; /* 写操作：version 递增，使进行中的迭代失效 */
    return r;
}

void *stk_hashmap_get(const stk_hashmap *m, const void *key) {
    if (!m) return NULL;
    return (m->mode == STK_HASHMAP_CHAIN)
        ? stk_i_chainmap_get(&m->u.chain, key)
        : stk_i_oamap_get(&m->u.oa, key);
}

int stk_hashmap_remove(stk_hashmap *m, const void *key) {
    if (!m) return 0;
    void *old_key = NULL, *old_val = NULL;
    int r = (m->mode == STK_HASHMAP_CHAIN)
        ? stk_i_chainmap_remove(&m->u.chain, key, &old_key, &old_val)
        : stk_i_oamap_remove(&m->u.oa, key, &old_key, &old_val);
    if (r) {
        if (m->key_free) m->key_free(old_key);
        if (m->val_free) m->val_free(old_val);
        m->version++;
    }
    return r;
}

int stk_hashmap_contains(const stk_hashmap *m, const void *key) {
    /* 局限：若允许存储 NULL 作为 value，contains 与"未找到"无法区分，
     * 此时请用迭代器或自行判断。典型用法（value 非 NULL）下无此问题。 */
    return stk_hashmap_get(m, key) != NULL;
}

size_t stk_hashmap_size(const stk_hashmap *m) {
    if (!m) return 0;
    return (m->mode == STK_HASHMAP_CHAIN)
        ? stk_i_chainmap_size(&m->u.chain)
        : stk_i_oamap_size(&m->u.oa);
}

void stk_hashmap_clear(stk_hashmap *m) {
    if (!m) return;
    if (m->mode == STK_HASHMAP_CHAIN) {
        stk_chainmap *c = &m->u.chain;
        if (m->key_free || m->val_free)
            for (size_t i = 0; i < c->bucket_count; i++)
                for (stk_chain_node *n = c->buckets[i]; n; n = n->next) {
                    if (m->key_free) m->key_free(n->key);
                    if (m->val_free) m->val_free(n->value);
                }
        stk_i_chainmap_clear(c);
    } else {
        stk_oamap *o = &m->u.oa;
        if (m->key_free || m->val_free)
            for (size_t i = 0; i < o->slot_count; i++)
                if (o->slots[i].state == STK_OA_OCCUPIED) {
                    if (m->key_free) m->key_free(o->slots[i].key);
                    if (m->val_free) m->val_free(o->slots[i].value);
                }
        stk_i_oamap_clear(o);
    }
    m->version++;
}

int stk_hashmap_reserve(stk_hashmap *m, size_t new_capacity) {
    if (!m) return 0;
    int r = (m->mode == STK_HASHMAP_CHAIN)
        ? stk_i_chainmap_reserve(&m->u.chain, new_capacity)
        : stk_i_oamap_reserve(&m->u.oa, new_capacity);
    if (r) m->version++;
    return r;
}

/* ---------------- 迭代器 ---------------- */

void stk_hashmap_iter_begin(stk_hashmap_iter *it, const stk_hashmap *m) {
    it->map = m;
    it->idx = (size_t)-1; /* 第一次 next 会 +1 落到 0 */
    it->node = NULL;
    it->prev = NULL;
    it->key = NULL;
    it->value = NULL;
    it->version = m->version;
}

int stk_hashmap_iter_next(stk_hashmap_iter *it) {
    /* 失败快检：迭代中途被修改 → 立即停止，避免静默损坏。 */
    if (it->version != it->map->version) return 0;
    const stk_hashmap *m = it->map;

    if (m->mode == STK_HASHMAP_OA) {
        const stk_oamap *o = &m->u.oa;
        size_t idx = it->idx + 1;
        while (idx < o->slot_count) {
            const stk_oa_slot *s = &o->slots[idx];
            if (s->state == STK_OA_OCCUPIED) {
                it->idx = idx;
                it->key = s->key;
                it->value = s->value;
                return 1;
            }
            idx++;
        }
        it->idx = idx; /* 越界，结束 */
        return 0;
    }

    /* CHAIN：idx 为当前桶号；node 为当前节点（NULL 表示需找下一桶）。 */
    const stk_chainmap *c = &m->u.chain;
    stk_chain_node *n = (stk_chain_node *)it->node;
    if (n) {
        it->prev = n;
        n = n->next;
    }
    while (n == NULL) {
        it->idx++;
        it->prev = NULL;
        if (it->idx >= c->bucket_count) return 0;
        n = c->buckets[it->idx];
    }
    it->node = n;
    it->key = n->key;
    it->value = n->value;
    return 1;
}

void *stk_hashmap_iter_key(const stk_hashmap_iter *it) {
    return it->key;
}

void *stk_hashmap_iter_value(const stk_hashmap_iter *it) {
    return it->value;
}

int stk_hashmap_iter_remove(stk_hashmap_iter *it) {
    if (it->version != it->map->version) return 0;
    stk_hashmap *m = (stk_hashmap *)it->map;
    void *k = it->key, *v = it->value;

    if (m->mode == STK_HASHMAP_OA) {
        stk_oamap *o = &m->u.oa;
        size_t slot = it->idx; /* next() 已先把 idx 前进到当前槽 */
        if (slot < o->slot_count && o->slots[slot].state == STK_OA_OCCUPIED) {
            o->slots[slot].state = STK_OA_TOMB;
            o->slots[slot].key = NULL;
            o->slots[slot].value = NULL;
            o->size--;
        }
    } else {
        stk_chainmap *c = &m->u.chain;
        stk_chain_node *n = (stk_chain_node *)it->node;
        stk_chain_node *p = (stk_chain_node *)it->prev;
        size_t b = it->idx;
        if (p) p->next = n->next;
        else    c->buckets[b] = n->next;
        it->node = n->next; /* 继续从下一个元素开始 */
        stk_dealloc(c->alloc, n);
        c->size--;
    }

    if (m->key_free) m->key_free(k);
    if (m->val_free) m->val_free(v);
    m->version++;
    it->version = m->version; /* 同步快照，使后续 next 不被自己这一改打断 */
    return 1;
}
