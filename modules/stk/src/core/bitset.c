#include "stk/core/bitset.h"
#include <stdlib.h>
#include <string.h>

#define BITS_PER_WORD (sizeof(size_t) * 8)
#define WORD_IDX(b)   ((b) / BITS_PER_WORD)
#define BIT_MASK(b)   ((size_t)1 << ((b) % BITS_PER_WORD))

void stk_bitset_init(stk_bitset *b, size_t nbits) {
    b->nwords = (nbits + BITS_PER_WORD - 1) / BITS_PER_WORD;
    if (b->nwords < 1) b->nwords = 1;
    b->words  = (size_t *)calloc(b->nwords, sizeof(size_t));
    b->nbits  = b->words ? nbits : 0;
}

void stk_bitset_free(stk_bitset *b) {
    free(b->words);
    b->words  = NULL;
    b->nwords = 0;
    b->nbits  = 0;
}

void stk_bitset_set(stk_bitset *b, size_t idx) {
    if (idx >= b->nbits) return;
    b->words[WORD_IDX(idx)] |= BIT_MASK(idx);
}

void stk_bitset_clear(stk_bitset *b, size_t idx) {
    if (idx >= b->nbits) return;
    b->words[WORD_IDX(idx)] &= ~BIT_MASK(idx);
}

void stk_bitset_toggle(stk_bitset *b, size_t idx) {
    if (idx >= b->nbits) return;
    b->words[WORD_IDX(idx)] ^= BIT_MASK(idx);
}

bool stk_bitset_get(const stk_bitset *b, size_t idx) {
    if (idx >= b->nbits) return false;
    return (b->words[WORD_IDX(idx)] & BIT_MASK(idx)) != 0;
}

void stk_bitset_set_all(stk_bitset *b) {
    memset(b->words, 0xff, b->nwords * sizeof(size_t));
}

void stk_bitset_clear_all(stk_bitset *b) {
    memset(b->words, 0, b->nwords * sizeof(size_t));
}

void stk_bitset_negate(stk_bitset *b) {
    for (size_t i = 0; i < b->nwords; i++)
        b->words[i] = ~b->words[i];
}

static size_t popcount_word(size_t w) {
#if defined(__GNUC__) || defined(__clang__)
    return (size_t)__builtin_popcountll(w);
#else
    /* Kernighan's algorithm */
    size_t cnt = 0;
    while (w) { cnt++; w &= w - 1; }
    return cnt;
#endif
}

size_t stk_bitset_count(const stk_bitset *b) {
    size_t cnt = 0;
    for (size_t i = 0; i < b->nwords; i++)
        cnt += popcount_word(b->words[i]);
    return cnt;
}

bool stk_bitset_any(const stk_bitset *b) {
    for (size_t i = 0; i < b->nwords; i++)
        if (b->words[i]) return true;
    return false;
}

bool stk_bitset_none(const stk_bitset *b) {
    return !stk_bitset_any(b);
}

bool stk_bitset_all(const stk_bitset *b) {
    for (size_t i = 0; i < b->nwords; i++) {
        if (b->words[i] != (size_t)-1) {
            /* Check the last word might have padding bits */
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

size_t stk_bitset_size(const stk_bitset *b) { return b->nbits; }

size_t stk_bitset_next_set(const stk_bitset *b, size_t start) {
    for (size_t i = start; i < b->nbits; i++)
        if (stk_bitset_get(b, i)) return i;
    return (size_t)-1;
}

size_t stk_bitset_next_clear(const stk_bitset *b, size_t start) {
    for (size_t i = start; i < b->nbits; i++)
        if (!stk_bitset_get(b, i)) return i;
    return (size_t)-1;
}