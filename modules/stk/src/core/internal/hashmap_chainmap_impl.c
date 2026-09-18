#include "hashmap_chainmap.h"
#include <string.h>

static size_t stk_i_chainmap_bucket_for(const stk_chainmap *m, const void *key) {
    return m->hash(key) & (m->bucket_count - 1);
}

static size_t stk_i_next_pow2(size_t n) {
    size_t p = 16;
    while (p < n) p <<= 1;
    return p;
}

/* 零初始化分配：用自定义分配器时也能拿到全 0 内存（桶数组依赖 NULL 初值）。 */
static void *stk_i_chain_alloc_zeroed(const stk_chainmap *m, size_t count, size_t esize) {
    size_t total = count * esize;
    void *p = stk_alloc(m->alloc, total);
    if (p) memset(p, 0, total);
    return p;
}

void stk_i_chainmap_init(stk_chainmap *m, size_t capacity, stk_hash_fn hash, stk_eq_fn eq, const stk_allocator *alloc) {
    m->alloc = alloc;
    m->bucket_count = stk_i_next_pow2(capacity ? capacity : 1);
    m->buckets = (stk_chain_node **)stk_i_chain_alloc_zeroed(m, m->bucket_count, sizeof(stk_chain_node *));
    m->size = 0;
    m->hash = hash;
    m->eq = eq;
}

void stk_i_chainmap_free(stk_chainmap *m) {
    stk_i_chainmap_clear(m);
    stk_dealloc(m->alloc, m->buckets);
    m->buckets = NULL;
    m->bucket_count = 0;
}

int stk_i_chainmap_set(stk_chainmap *m, void *key, void *value, void **out_old) {
    size_t b = stk_i_chainmap_bucket_for(m, key);
    for (stk_chain_node *n = m->buckets[b]; n; n = n->next) {
        if (m->eq(n->key, key)) {
            if (out_old) *out_old = n->value;
            n->value = value;
            return 1; /* 覆盖 */
        }
    }
    stk_chain_node *n = (stk_chain_node *)stk_alloc(m->alloc, sizeof(stk_chain_node));
    if (!n) return -1; /* OOM */
    n->key = key;
    n->value = value;
    n->next = m->buckets[b];
    m->buckets[b] = n;
    m->size++;
    /* 负载因子 0.75 触发扩容（链式桶本身可无限增长，这里只为控制链长） */
    if (m->size > (m->bucket_count * 3) / 4) {
        stk_i_chainmap_reserve(m, m->bucket_count * 2);
    }
    return 0; /* 新插 */
}

void *stk_i_chainmap_get(const stk_chainmap *m, const void *key) {
    size_t b = stk_i_chainmap_bucket_for(m, key);
    for (stk_chain_node *n = m->buckets[b]; n; n = n->next) {
        if (m->eq(n->key, key)) return n->value;
    }
    return NULL;
}

int stk_i_chainmap_remove(stk_chainmap *m, const void *key, void **out_key, void **out_value) {
    size_t b = stk_i_chainmap_bucket_for(m, key);
    stk_chain_node *prev = NULL;
    for (stk_chain_node *n = m->buckets[b]; n; prev = n, n = n->next) {
        if (m->eq(n->key, key)) {
            if (prev) prev->next = n->next;
            else      m->buckets[b] = n->next;
            if (out_key)   *out_key = n->key;
            if (out_value) *out_value = n->value;
            stk_dealloc(m->alloc, n);
            m->size--;
            return 1;
        }
    }
    return 0;
}

void stk_i_chainmap_clear(stk_chainmap *m) {
    for (size_t i = 0; i < m->bucket_count; i++) {
        stk_chain_node *n = m->buckets[i];
        while (n) {
            stk_chain_node *nx = n->next;
            stk_dealloc(m->alloc, n);
            n = nx;
        }
        m->buckets[i] = NULL;
    }
    m->size = 0;
}

size_t stk_i_chainmap_size(const stk_chainmap *m) {
    return m->size;
}

int stk_i_chainmap_reserve(stk_chainmap *m, size_t new_bucket_count) {
    size_t p = stk_i_next_pow2(new_bucket_count);
    if (p <= m->bucket_count) return 1;
    stk_chain_node **nb = (stk_chain_node **)stk_i_chain_alloc_zeroed(m, p, sizeof(stk_chain_node *));
    if (!nb) return 0;
    for (size_t i = 0; i < m->bucket_count; i++) {
        stk_chain_node *n = m->buckets[i];
        while (n) {
            stk_chain_node *nx = n->next;
            size_t b = m->hash(n->key) & (p - 1);
            n->next = nb[b];
            nb[b] = n;
            n = nx;
        }
    }
    stk_dealloc(m->alloc, m->buckets);
    m->buckets = nb;
    m->bucket_count = p;
    return 1;
}
