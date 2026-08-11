#include "stk/core/hashmap.h"

#include <stdlib.h>
#include <string.h>

/* 公开句柄：内部持有一个 union（chainmap | oamap），对外不透明。
 * 只存 mode 做分派；capacity 仅创建时使用，不保留。
 * key/value 所有权归调用方：destroy 时不 free key/value。 */
struct stk_hashmap {
    stk_hashmap_mode mode;
    stk_hash_fn hash;
    stk_eq_fn   eq;
    const stk_allocator *alloc;
    union {
        stk_chainmap chain;
        stk_oamap    oa;
    } u;
};

stk_hashmap *stk_hashmap_create(stk_hashmap_mode mode, size_t capacity,
                                stk_hash_fn hash, stk_eq_fn eq,
                                const stk_allocator *alloc) {
    if (alloc == NULL) alloc = &STK_ALLOCATOR_DEFAULT;

    stk_hashmap *m = (stk_hashmap *)stk_i_alloc(alloc, sizeof(stk_hashmap));
    if (m == NULL) return NULL;

    m->mode = mode;
    m->hash = hash;
    m->eq = eq;
    m->alloc = alloc;
    memset(&m->u, 0, sizeof(m->u));

    if (mode == STK_HASHMAP_CHAIN) {
        stk_i_chainmap_init(&m->u.chain, capacity, hash, eq, alloc);
    } else if (mode == STK_HASHMAP_OA) {
        stk_i_oamap_init(&m->u.oa, capacity, hash, eq, alloc);
    } else {
        /* 未知 mode：释放并返回 NULL，调用方应视为失败 */
        stk_i_free(alloc, m);
        return NULL;
    }
    return m;
}

void stk_hashmap_destroy(stk_hashmap *m) {
    if (m == NULL) return;
    if (m->mode == STK_HASHMAP_CHAIN) {
        stk_i_chainmap_free(&m->u.chain);
    } else if (m->mode == STK_HASHMAP_OA) {
        stk_i_oamap_free(&m->u.oa);
    }
    /* 不 free key/value（所有权归调用方） */
    stk_i_free(m->alloc, m);
}

int stk_hashmap_set(stk_hashmap *m, void *key, void *value) {
    if (m == NULL) return 0;
    if (m->mode == STK_HASHMAP_CHAIN)
        return stk_i_chainmap_set(&m->u.chain, key, value);
    return stk_i_oamap_set(&m->u.oa, key, value);
}

void *stk_hashmap_get(const stk_hashmap *m, const void *key) {
    if (m == NULL) return NULL;
    if (m->mode == STK_HASHMAP_CHAIN)
        return stk_i_chainmap_get(&m->u.chain, key);
    return stk_i_oamap_get(&m->u.oa, key);
}

int stk_hashmap_remove(stk_hashmap *m, const void *key) {
    if (m == NULL) return 0;
    if (m->mode == STK_HASHMAP_CHAIN)
        return stk_i_chainmap_remove(&m->u.chain, key);
    return stk_i_oamap_remove(&m->u.oa, key);
}

void stk_hashmap_clear(stk_hashmap *m) {
    if (m == NULL) return;
    if (m->mode == STK_HASHMAP_CHAIN)
        stk_i_chainmap_clear(&m->u.chain);
    else
        stk_i_oamap_clear(&m->u.oa);
}

size_t stk_hashmap_size(const stk_hashmap *m) {
    if (m == NULL) return 0;
    if (m->mode == STK_HASHMAP_CHAIN)
        return m->u.chain.count;
    return m->u.oa.count;
}

int stk_hashmap_reserve(stk_hashmap *m, size_t new_capacity) {
    if (m == NULL) return 0;
    if (m->mode == STK_HASHMAP_CHAIN)
        return stk_i_chainmap_reserve(&m->u.chain, new_capacity);
    return stk_i_oamap_reserve(&m->u.oa, new_capacity);
}

void stk_hashmap_iter_begin(stk_hashmap_iter *it, const stk_hashmap *m) {
    if (it == NULL || m == NULL) return;
    it->mode = m->mode;
    if (m->mode == STK_HASHMAP_CHAIN)
        stk_i_chain_iter_begin(&it->u.chain, &m->u.chain);
    else
        stk_i_oa_iter_begin(&it->u.oa, &m->u.oa);
}

int stk_hashmap_iter_next(stk_hashmap_iter *it) {
    if (it == NULL) return 0;
    if (it->mode == STK_HASHMAP_CHAIN)
        return stk_i_chain_iter_next(&it->u.chain);
    return stk_i_oa_iter_next(&it->u.oa);
}

void *stk_hashmap_iter_key(const stk_hashmap_iter *it) {
    if (it == NULL) return NULL;
    if (it->mode == STK_HASHMAP_CHAIN)
        return stk_i_chain_iter_key(&it->u.chain);
    return stk_i_oa_iter_key(&it->u.oa);
}

void *stk_hashmap_iter_value(const stk_hashmap_iter *it) {
    if (it == NULL) return NULL;
    if (it->mode == STK_HASHMAP_CHAIN)
        return stk_i_chain_iter_value(&it->u.chain);
    return stk_i_oa_iter_value(&it->u.oa);
}

int stk_hashmap_iter_remove(stk_hashmap_iter *it) {
    if (it == NULL) return 0;
    if (it->mode == STK_HASHMAP_CHAIN)
        return stk_i_chain_iter_remove(&it->u.chain);
    return stk_i_oa_iter_remove(&it->u.oa);
}
