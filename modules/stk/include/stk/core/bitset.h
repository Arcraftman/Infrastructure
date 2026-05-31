#ifndef STK_BITSET_H
#define STK_BITSET_H

#include "stk/def.h"
#include <stddef.h>

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
 *   bitset b;
 *   bitset_init(&b, 128);
 *   bitset_set(&b, 42);
 *   bool v = bitset_get(&b, 42);   // true
 *   bitset_clear(&b, 42);
 *   bitset_free(&b);
 * @endcode
 */

typedef struct {
    size_t *words;     /* array of size_t bit-words */
    size_t  nwords;    /* number of words */
    size_t  nbits;     /* total number of bits */
} bitset;

/* Lifetime ------------------------------------------------------------- */

STK_API void   bitset_init(bitset *b, size_t nbits);
STK_API void   bitset_free(bitset *b);

/* Single-bit operations ------------------------------------------------ */

STK_API void   bitset_set(bitset *b, size_t idx);
STK_API void   bitset_clear(bitset *b, size_t idx);
STK_API void   bitset_toggle(bitset *b, size_t idx);
STK_API bool   bitset_get(const bitset *b, size_t idx);

/* Bulk operations ------------------------------------------------------ */

STK_API void   bitset_set_all(bitset *b);
STK_API void   bitset_clear_all(bitset *b);
STK_API void   bitset_negate(bitset *b);

/* Queries -------------------------------------------------------------- */

STK_API size_t bitset_count(const bitset *b);
STK_API bool   bitset_any(const bitset *b);
STK_API bool   bitset_none(const bitset *b);
STK_API bool   bitset_all(const bitset *b);
STK_API size_t bitset_size(const bitset *b);

/* Search --------------------------------------------------------------- */

STK_API size_t bitset_next_set(const bitset *b, size_t start);
STK_API size_t bitset_next_clear(const bitset *b, size_t start);

#ifdef __cplusplus
}
#endif

#endif /* STK_BITSET_H */
