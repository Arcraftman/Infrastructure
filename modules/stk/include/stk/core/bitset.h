#ifndef STK_CORE_BITSET_H
#define STK_CORE_BITSET_H



#ifdef __cplusplus
extern "C" {
#endif

/* Fixed-size bitset (bit-array).
 *
 * A space-efficient set of bits providing all typical bitwise operations
 * and population-count queries.  Useful for flags, state tracking, and
 * small-set membership tests.
 *
 * Basic usage:
 * @code
 *   stk_bitset b;
 *   stk_bitset_init(&b, 128);
 *   stk_bitset_set(&b, 42);
 *   bool v = stk_bitset_get(&b, 42);   // true
 *   stk_bitset_clear(&b, 42);
 *   stk_bitset_free(&b);
 * @endcode
 */

typedef struct {
    size_t *words;     /* array of size_t bit-words */
    size_t  nwords;    /* number of words */
    size_t  nbits;     /* total number of bits */
} stk_bitset;

/* Lifetime ------------------------------------------------------------- */

STK_API STK_STATUS stk_bitset_init(stk_bitset *b, size_t nbits);
STK_API STK_STATUS stk_bitset_free(stk_bitset *b);

/* Single-bit operations ------------------------------------------------ */

STK_API STK_STATUS stk_bitset_set(stk_bitset *b, size_t idx);
STK_API STK_STATUS stk_bitset_clear(stk_bitset *b, size_t idx);
STK_API STK_STATUS stk_bitset_toggle(stk_bitset *b, size_t idx);
STK_API bool       stk_bitset_get(const stk_bitset *b, size_t idx);

/* Bulk operations ------------------------------------------------------ */

STK_API STK_STATUS stk_bitset_set_all(stk_bitset *b);
STK_API STK_STATUS stk_bitset_clear_all(stk_bitset *b);
STK_API STK_STATUS stk_bitset_negate(stk_bitset *b);

/* Queries -------------------------------------------------------------- */

STK_API size_t stk_bitset_count(const stk_bitset *b);
STK_API bool   stk_bitset_any(const stk_bitset *b);
STK_API bool   stk_bitset_none(const stk_bitset *b);
STK_API bool   stk_bitset_all(const stk_bitset *b);
STK_API size_t stk_bitset_size(const stk_bitset *b);

/* Search --------------------------------------------------------------- */

STK_API size_t stk_bitset_next_set(const stk_bitset *b, size_t start);
STK_API size_t stk_bitset_next_clear(const stk_bitset *b, size_t start);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_BITSET_H */