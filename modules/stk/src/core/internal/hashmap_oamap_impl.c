#include "hashmap_oamap.h"
#include <string.h>

/* 负载因子上限：used(含墓碑) 达到此比例即扩容 / 清理墓碑。 */
#define STK_OA_LOAD_UPPER 0.77

static size_t stk_i_next_pow2(size_t n) {
    size_t p = 16;
    while (p < n) p <<= 1;
    return p;
}

/* 零初始化分配（OA 依赖 EMPTY=0 的初值）。 */
static void *stk_i_oa_alloc_zeroed(const stk_oamap *m, size_t count, size_t esize) {
    size_t total = count * esize;
    void *p = stk_alloc(m->alloc, total);
    if (p) memset(p, 0, total);
    return p;
}

void stk_i_oamap_init(stk_oamap *m, size_t capacity, stk_hash_fn hash, stk_eq_fn eq, const stk_allocator *alloc) {
    m->alloc = alloc;
    m->slot_count = stk_i_next_pow2(capacity ? capacity : 1);
    m->slots = (stk_oa_slot *)stk_i_oa_alloc_zeroed(m, m->slot_count, sizeof(stk_oa_slot));
    m->size = 0;
    m->used = 0;
    m->hash = hash;
    m->eq = eq;
}

void stk_i_oamap_free(stk_oamap *m) {
    stk_dealloc(m->alloc, m->slots);
    m->slots = NULL;
    m->slot_count = 0;
    m->size = 0;
    m->used = 0;
}

void stk_i_oamap_clear(stk_oamap *m) {
    for (size_t i = 0; i < m->slot_count; i++) {
        m->slots[i].state = STK_OA_EMPTY;
        m->slots[i].key = NULL;
        m->slots[i].value = NULL;
    }
    m->size = 0;
    m->used = 0;
}

int stk_i_oamap_set(stk_oamap *m, void *key, void *value, void **out_old) {
    size_t mask = m->slot_count - 1;
    size_t idx = m->hash(key) & mask;
    size_t first_tomb = (size_t)-1;

    for (;;) {
        stk_oa_slot *s = &m->slots[idx];
        if (s->state == STK_OA_EMPTY) break;
        if (s->state == STK_OA_TOMB) {
            if (first_tomb == (size_t)-1) first_tomb = idx;
        } else if (m->eq(s->key, key)) {
            if (out_old) *out_old = s->value;
            s->value = value;
            return 1; /* 覆盖 */
        }
        idx = (idx + 1) & mask;
    }

    /* 优先复用墓碑槽，否则填空槽。 */
    size_t target = (first_tomb != (size_t)-1) ? first_tomb : idx;
    stk_oa_slot *s = &m->slots[target];
    s->key = key;
    s->value = value;
    s->state = STK_OA_OCCUPIED;
    if (first_tomb == (size_t)-1) m->used++; /* 填墓碑不增加 used */
    m->size++;

    if (m->used >= (size_t)(m->slot_count * STK_OA_LOAD_UPPER)) {
        stk_i_oamap_reserve(m, m->slot_count * 2);
    }
    return 0; /* 新插 */
}

void *stk_i_oamap_get(const stk_oamap *m, const void *key) {
    size_t mask = m->slot_count - 1;
    size_t idx = m->hash(key) & mask;
    for (;;) {
        const stk_oa_slot *s = &m->slots[idx];
        if (s->state == STK_OA_EMPTY) return NULL; /* 探测序列遇空，必不存在 */
        if (s->state == STK_OA_OCCUPIED && m->eq(s->key, key)) return s->value;
        idx = (idx + 1) & mask;
    }
}

int stk_i_oamap_remove(stk_oamap *m, const void *key, void **out_key, void **out_value) {
    size_t mask = m->slot_count - 1;
    size_t idx = m->hash(key) & mask;
    for (;;) {
        stk_oa_slot *s = &m->slots[idx];
        if (s->state == STK_OA_EMPTY) return 0;
        if (s->state == STK_OA_OCCUPIED && m->eq(s->key, key)) {
            if (out_key)   *out_key = s->key;
            if (out_value) *out_value = s->value;
            s->state = STK_OA_TOMB;
            s->key = NULL;
            s->value = NULL;
            m->size--;
            return 1;
        }
        idx = (idx + 1) & mask;
    }
}

size_t stk_i_oamap_size(const stk_oamap *m) {
    return m->size;
}

int stk_i_oamap_reserve(stk_oamap *m, size_t new_slot_count) {
    size_t p = stk_i_next_pow2(new_slot_count);
    if (p <= m->slot_count) return 1;
    stk_oa_slot *ns = (stk_oa_slot *)stk_i_oa_alloc_zeroed(m, p, sizeof(stk_oa_slot));
    if (!ns) return 0;
    size_t old_count = m->slot_count;
    stk_oa_slot *os = m->slots;
    m->slots = ns;
    m->slot_count = p;
    m->size = 0;
    m->used = 0;
    /* 重新线性探测插入（墓碑不搬，借此顺带清理）。 */
    for (size_t i = 0; i < old_count; i++) {
        if (os[i].state == STK_OA_OCCUPIED) {
            size_t mask = p - 1;
            size_t idx = m->hash(os[i].key) & mask;
            while (m->slots[idx].state != STK_OA_EMPTY) idx = (idx + 1) & mask;
            m->slots[idx].key = os[i].key;
            m->slots[idx].value = os[i].value;
            m->slots[idx].state = STK_OA_OCCUPIED;
            m->size++;
            m->used++;
        }
    }
    stk_dealloc(m->alloc, os);
    return 1;
}
