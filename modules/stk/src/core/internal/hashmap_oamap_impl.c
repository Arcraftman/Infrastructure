#include "stk/core/internal/hashmap_oamap.h"

#include <stdlib.h>
#include <string.h>

/* 探测规则（线性探测）：
 * - OA_EMPTY: 查找可终止（此 key 不存在）；插入可用。
 * - OA_TOMB : 查找必须跳过（墓碑，后面可能还有真值）；插入可复用（记录 first_tomb）。
 * - OA_OCCUPIED: eq 命中即目标；否则继续。
 */

static size_t oa_idx(const stk_oamap *m, const void *key) {
    return m->hash(key) % m->slot_count;
}

void stk_i_oamap_init(stk_oamap *m, size_t slot_count,
                      stk_hash_fn hash, stk_eq_fn eq,
                      const stk_allocator *alloc) {
    if (m == NULL) return;
    if (slot_count == 0) slot_count = 16;
    if (hash == NULL) hash = stk_i_hash_ptr;
    if (eq == NULL)   eq = stk_i_eq_ptr;
    if (alloc == NULL) alloc = &STK_ALLOCATOR_DEFAULT;

    m->slot_count = slot_count;
    m->count = 0;
    m->tombstones = 0;
    m->hash = hash;
    m->eq = eq;
    m->alloc = alloc;
    /* calloc 保证所有 state = OA_EMPTY(0)、key/value = NULL */
    m->slots = (stk_oa_slot *)stk_i_alloc(alloc,
                 slot_count * sizeof(stk_oa_slot));
    if (m->slots) memset(m->slots, 0, slot_count * sizeof(stk_oa_slot));
}

void stk_i_oamap_free(stk_oamap *m) {
    if (m == NULL || m->slots == NULL) return;
    stk_i_oamap_clear(m);
    stk_i_free(m->alloc, m->slots);
    m->slots = NULL;
    m->slot_count = 0;
    m->count = 0;
    m->tombstones = 0;
}

int stk_i_oamap_set(stk_oamap *m, void *key, void *value) {
    if (m == NULL || m->slots == NULL) return 0;
    size_t i = oa_idx(m, key);
    size_t first_tomb = m->slot_count; /* 无效哨兵 */
    int found_existing = 0;
    size_t existing_idx = 0;

    for (size_t probe = 0; probe < m->slot_count; probe++) {
        size_t idx = (i + probe) % m->slot_count;
        stk_oa_slot *s = &m->slots[idx];
        if (s->state == OA_EMPTY) {
            break; /* 到此为止：key 不存在 */
        } else if (s->state == OA_TOMB) {
            if (first_tomb == m->slot_count) first_tomb = idx; /* 记录首个墓碑 */
        } else { /* OCCUPIED */
            if (m->eq(s->key, key)) {
                found_existing = 1;
                existing_idx = idx;
                break;
            }
        }
    }

    if (found_existing) {
        m->slots[existing_idx].value = value; /* 覆盖，key 不换 */
        return 1;
    }

    /* 没找到：优先复用墓碑，否则用探测途中遇到的第一个 EMPTY。
     * 注意：上面循环 break 时 idx 停在 EMPTY 处；若没遇到 EMPTY（表满）则不插。 */
    size_t target = first_tomb;
    if (target == m->slot_count) {
        /* 重新找第一个 EMPTY（上面 break 前最后访问的 idx 即 EMPTY，但已丢失，重扫一次更稳） */
        size_t j = oa_idx(m, key);
        for (size_t probe = 0; probe < m->slot_count; probe++) {
            size_t idx = (j + probe) % m->slot_count;
            if (m->slots[idx].state == OA_EMPTY) { target = idx; break; }
        }
    }
    if (target == m->slot_count) return 0; /* 表满且无墓碑，插入失败 */

    stk_oa_slot *s = &m->slots[target];
    if (s->state == OA_TOMB) m->tombstones--; /* 复用墓碑：墓碑数减一 */
    s->key = key;
    s->value = value;
    s->state = OA_OCCUPIED;
    m->count++;
    return 0;
}

void *stk_i_oamap_get(const stk_oamap *m, const void *key) {
    if (m == NULL || m->slots == NULL) return NULL;
    size_t i = oa_idx(m, key);
    for (size_t probe = 0; probe < m->slot_count; probe++) {
        size_t idx = (i + probe) % m->slot_count;
        const stk_oa_slot *s = &m->slots[idx];
        if (s->state == OA_EMPTY) return NULL;       /* 探测链断，必不存在 */
        if (s->state == OA_OCCUPIED && m->eq(s->key, key))
            return s->value;
        /* OA_TOMB 跳过 */
    }
    return NULL; /* 全表扫完（理论上不会到，除非表满且无 EMPTY） */
}

int stk_i_oamap_remove(stk_oamap *m, const void *key) {
    if (m == NULL || m->slots == NULL) return 0;
    size_t i = oa_idx(m, key);
    for (size_t probe = 0; probe < m->slot_count; probe++) {
        size_t idx = (i + probe) % m->slot_count;
        stk_oa_slot *s = &m->slots[idx];
        if (s->state == OA_EMPTY) return 0;          /* 链断，没找到 */
        if (s->state == OA_OCCUPIED && m->eq(s->key, key)) {
            s->state = OA_TOMB;                      /* 关键：置墓碑，不置 EMPTY */
            s->key = NULL;
            s->value = NULL;
            m->count--;
            m->tombstones++;
            return 1;
        }
    }
    return 0;
}

void stk_i_oamap_clear(stk_oamap *m) {
    if (m == NULL || m->slots == NULL) return;
    memset(m->slots, 0, m->slot_count * sizeof(stk_oa_slot)); /* 全回 EMPTY(0) */
    m->count = 0;
    m->tombstones = 0;
}

int stk_i_oamap_reserve(stk_oamap *m, size_t new_slot_count) {
    if (m == NULL || m->slots == NULL) return 0;
    if (new_slot_count <= m->slot_count) return 0;
    /* 新表必须能容纳当前所有元素（否则插入会失败） */
    if (new_slot_count < m->count) return 0;

    stk_oa_slot *new_slots = (stk_oa_slot *)stk_i_alloc(
        m->alloc, new_slot_count * sizeof(stk_oa_slot));
    if (new_slots == NULL) return 0;
    memset(new_slots, 0, new_slot_count * sizeof(stk_oa_slot)); /* 全 EMPTY */

    /* 重放所有 OCCUPIED 到新表（线性探测，墓碑不搬） */
    for (size_t i = 0; i < m->slot_count; i++) {
        if (m->slots[i].state != OA_OCCUPIED) continue;
        size_t j = m->hash(m->slots[i].key) % new_slot_count;
        size_t placed = 0;
        for (size_t probe = 0; probe < new_slot_count; probe++) {
            size_t idx = (j + probe) % new_slot_count;
            if (new_slots[idx].state == OA_EMPTY) {
                new_slots[idx].key = m->slots[i].key;
                new_slots[idx].value = m->slots[i].value;
                new_slots[idx].state = OA_OCCUPIED;
                placed = 1;
                break;
            }
        }
        if (!placed) {
            /* 极端哈希退化导致放不下：回滚 */
            stk_i_free(m->alloc, new_slots);
            return 0;
        }
    }

    stk_i_free(m->alloc, m->slots);
    m->slots = new_slots;
    m->slot_count = new_slot_count;
    m->tombstones = 0; /* 墓碑已在重放中丢弃 */
    return 1;
}

/* ---- 迭代器 ---- */

void stk_i_oa_iter_begin(stk_oa_iter *it, const stk_oamap *m) {
    it->map = m;
    it->slot = 0;
    it->current = (size_t)-1;
    /* 不预找第一个 occupied：交给 next 从 0 完整扫描，避免漏掉
     * index 比首个 occupied 更小的元素（线性探测会使元素乱序分布）。 */
}

int stk_i_oa_iter_next(stk_oa_iter *it) {
    if (it->map == NULL || it->map->slots == NULL) return 0;
    /* 从 current+1（首次从 0）往后找下一个 OCCUPIED，扫到 slot_count 末尾即止。
     * 不设回绕，保证每个槽最多访问一次、全表覆盖。 */
    size_t start = (it->current == (size_t)-1) ? 0 : it->current + 1;
    for (size_t i = start; i < it->map->slot_count; i++) {
        if (it->map->slots[i].state == OA_OCCUPIED) {
            it->current = i;
            it->slot = i;
            return 1;
        }
    }
    it->current = (size_t)-1;
    return 0;
}

void *stk_i_oa_iter_key(const stk_oa_iter *it) {
    if (it->current == (size_t)-1) return NULL;
    return it->map->slots[it->current].key;
}
void *stk_i_oa_iter_value(const stk_oa_iter *it) {
    if (it->current == (size_t)-1) return NULL;
    return it->map->slots[it->current].value;
}

int stk_i_oa_iter_remove(stk_oa_iter *it) {
    if (it->current == (size_t)-1) return 0;
    stk_oa_slot *s = &((stk_oamap *)it->map)->slots[it->current];
    if (s->state != OA_OCCUPIED) return 0;
    s->state = OA_TOMB;
    s->key = NULL;
    s->value = NULL;
    ((stk_oamap *)it->map)->count--;
    ((stk_oamap *)it->map)->tombstones++;
    it->current = (size_t)-1; /* 当前项已无效，slot 游标已前进 */
    return 1;
}
