#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/bitset.h"

#define BITS_PER_WORD (sizeof(size_t) * 8)
#define WORD_IDX(b)   ((b) / BITS_PER_WORD)
#define BIT_MASK(b)   ((size_t)1 << ((b) % BITS_PER_WORD))

STK_STATUS stk_bitset_init(stk_bitset *b, size_t nbits) {
    STK_RETURN_IF(!b, STK_EINVAL, "Bitset init: NULL bitset pointer");
    
    b->nwords = (nbits + BITS_PER_WORD - 1) / BITS_PER_WORD;
    if (b->nwords < 1) b->nwords = 1;
    b->words = (size_t *)calloc(b->nwords, sizeof(size_t));
    if (!b->words) {
        STK_LOG_ERROR("Bitset init: failed to allocate %zu words", b->nwords);
        return STK_ENOMEM;
    }
    b->nbits = nbits;
    
    STK_LOG_DEBUG("Bitset init: nbits=%zu, nwords=%zu", nbits, b->nwords);
    return STK_OK;
}

STK_STATUS stk_bitset_free(stk_bitset *b) {
    if (!b) {
        STK_LOG_WARN("Bitset free: NULL bitset pointer");
        return STK_EINVAL;
    }
    free(b->words);
    b->words = NULL;
    b->nwords = 0;
    b->nbits = 0;
    STK_LOG_DEBUG("Bitset freed");
    return STK_OK;
}

STK_STATUS stk_bitset_set(stk_bitset *b, size_t idx) {
    if (!b) {
        STK_LOG_ERROR("Bitset set: NULL bitset pointer");
        return STK_EINVAL;
    }
    if (idx >= b->nbits) {
        STK_LOG_ERROR("Bitset set: index %zu out of range (max=%zu)", idx, b->nbits - 1);
        return STK_ERANGE;
    }
    b->words[WORD_IDX(idx)] |= BIT_MASK(idx);
    STK_LOG_DEBUG("Bitset set: bit %zu", idx);
    return STK_OK;
}

STK_STATUS stk_bitset_clear(stk_bitset *b, size_t idx) {
    if (!b) {
        STK_LOG_ERROR("Bitset clear: NULL bitset pointer");
        return STK_EINVAL;
    }
    if (idx >= b->nbits) {
        STK_LOG_ERROR("Bitset clear: index %zu out of range (max=%zu)", idx, b->nbits - 1);
        return STK_ERANGE;
    }
    b->words[WORD_IDX(idx)] &= ~BIT_MASK(idx);
    STK_LOG_DEBUG("Bitset clear: bit %zu", idx);
    return STK_OK;
}

STK_STATUS stk_bitset_toggle(stk_bitset *b, size_t idx) {
    if (!b) {
        STK_LOG_ERROR("Bitset toggle: NULL bitset pointer");
        return STK_EINVAL;
    }
    if (idx >= b->nbits) {
        STK_LOG_ERROR("Bitset toggle: index %zu out of range (max=%zu)", idx, b->nbits - 1);
        return STK_ERANGE;
    }
    b->words[WORD_IDX(idx)] ^= BIT_MASK(idx);
    STK_LOG_DEBUG("Bitset toggle: bit %zu", idx);
    return STK_OK;
}

bool stk_bitset_get(const stk_bitset *b, size_t idx) {
    if (!b || idx >= b->nbits) {
        if (b) STK_LOG_WARN("Bitset get: index %zu out of range", idx);
        return false;
    }
    return (b->words[WORD_IDX(idx)] & BIT_MASK(idx)) != 0;
}

STK_STATUS stk_bitset_set_all(stk_bitset *b) {
    if (!b) {
        STK_LOG_ERROR("Bitset set_all: NULL bitset pointer");
        return STK_EINVAL;
    }
    memset(b->words, 0xff, b->nwords * sizeof(size_t));
    STK_LOG_DEBUG("Bitset set_all: %zu words", b->nwords);
    return STK_OK;
}

STK_STATUS stk_bitset_clear_all(stk_bitset *b) {
    if (!b) {
        STK_LOG_ERROR("Bitset clear_all: NULL bitset pointer");
        return STK_EINVAL;
    }
    memset(b->words, 0, b->nwords * sizeof(size_t));
    STK_LOG_DEBUG("Bitset clear_all: %zu words", b->nwords);
    return STK_OK;
}

STK_STATUS stk_bitset_negate(stk_bitset *b) {
    if (!b) {
        STK_LOG_ERROR("Bitset negate: NULL bitset pointer");
        return STK_EINVAL;
    }
    for (size_t i = 0; i < b->nwords; i++)
        b->words[i] = ~b->words[i];
    STK_LOG_DEBUG("Bitset negate: %zu words", b->nwords);
    return STK_OK;
}

static size_t popcount_word(size_t w) {
#if defined(__GNUC__) || defined(__clang__)
    return (size_t)__builtin_popcountll(w);
#else
    size_t cnt = 0;
    while (w) { cnt++; w &= w - 1; }
    return cnt;
#endif
}

size_t stk_bitset_count(const stk_bitset *b) {
    if (!b) {
        STK_LOG_WARN("Bitset count: NULL bitset pointer");
        return 0;
    }
    size_t cnt = 0;
    for (size_t i = 0; i < b->nwords; i++)
        cnt += popcount_word(b->words[i]);
    return cnt;
}

bool stk_bitset_any(const stk_bitset *b) {
    if (!b) return false;
    for (size_t i = 0; i < b->nwords; i++)
        if (b->words[i]) return true;
    return false;
}

bool stk_bitset_none(const stk_bitset *b) {
    return !stk_bitset_any(b);
}

bool stk_bitset_all(const stk_bitset *b) {
    if (!b) return false;
    for (size_t i = 0; i < b->nwords; i++) {
        if (b->words[i] != (size_t)-1) {
            if (i == b->nwords - 1) {
                size_t used = b->nbits % BITS_PER_WORD;
                size_t mask = used ? ((size_t)1 << used) - 1 : (size_t)-1;
                if (b->words[i] != mask) return false;
            } else {
                return false;
            }
        }
    }
    return true;
}

size_t stk_bitset_size(const stk_bitset *b) { 
    return b ? b->nbits : 0; 
}

size_t stk_bitset_next_set(const stk_bitset *b, size_t start) {
    if (!b) return (size_t)-1;
    for (size_t i = start; i < b->nbits; i++)
        if (stk_bitset_get(b, i)) return i;
    return (size_t)-1;
}

size_t stk_bitset_next_clear(const stk_bitset *b, size_t start) {
    if (!b) return (size_t)-1;
    for (size_t i = start; i < b->nbits; i++)
        if (!stk_bitset_get(b, i)) return i;
    return (size_t)-1;
}