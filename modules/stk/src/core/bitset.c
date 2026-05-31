#include "stk/core/bitset.h"
#include <stdlib.h>
#include <string.h>

#define BITS_PER_WORD (sizeof(size_t) * 8)
#define WORD_IDX(b)   ((b) / BITS_PER_WORD)
#define BIT_MASK(b)   ((size_t)1 << ((b) % BITS_PER_WORD))

void bitset_init(bitset *b, size_t nbits) {
    b->nwords = (nbits + BITS_PER_WORD - 1) / BITS_PER_WORD;
    if (b->nwords < 1) b->nwords = 1;
    b->words  = (size_t *)calloc(b->nwords, sizeof(size_t));
    b->nbits  = b->words ? nbits : 0;
}

void bitset_free(bitset *b) {
    free(b->words);
    b->words  = NULL;
    b->nwords = 0;
    b->nbits  = 0;
}

void bitset_set(bitset *b, size_t idx) {
    if (idx >= b->nbits) return;
    b->words[WORD_IDX(idx)] |= BIT_MASK(idx);
}

void bitset_clear(bitset *b, size_t idx) {
    if (idx >= b->nbits) return;
    b->words[WORD_IDX(idx)] &= ~BIT_MASK(idx);
}

void bitset_toggle(bitset *b, size_t idx) {
    if (idx >= b->nbits) return;
    b->words[WORD_IDX(idx)] ^= BIT_MASK(idx);
}

bool bitset_get(const bitset *b, size_t idx) {
    if (idx >= b->nbits) return false;
    return (b->words[WORD_IDX(idx)] & BIT_MASK(idx)) != 0;
}

void bitset_set_all(bitset *b) {
    memset(b->words, 0xff, b->nwords * sizeof(size_t));
}

void bitset_clear_all(bitset *b) {
    memset(b->words, 0, b->nwords * sizeof(size_t));
}

void bitset_negate(bitset *b) {
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

size_t bitset_count(const bitset *b) {
    size_t cnt = 0;
    for (size_t i = 0; i < b->nwords; i++)
        cnt += popcount_word(b->words[i]);
    return cnt;
}

bool bitset_any(const bitset *b) {
    for (size_t i = 0; i < b->nwords; i++)
        if (b->words[i]) return true;
    return false;
}

bool bitset_none(const bitset *b) {
    return !bitset_any(b);
}

bool bitset_all(const bitset *b) {
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

size_t bitset_size(const bitset *b) { return b->nbits; }

size_t bitset_next_set(const bitset *b, size_t start) {
    for (size_t i = start; i < b->nbits; i++)
        if (bitset_get(b, i)) return i;
    return (size_t)-1;
}

size_t bitset_next_clear(const bitset *b, size_t start) {
    for (size_t i = start; i < b->nbits; i++)
        if (!bitset_get(b, i)) return i;
    return (size_t)-1;
}
