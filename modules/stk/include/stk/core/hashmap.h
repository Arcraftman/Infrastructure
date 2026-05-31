#ifndef STK_HASHMAP_H
#define STK_HASHMAP_H

#include "stk/def.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Open-addressing hash map with linear probing.
 *
 * Both keys and values are stored as void*.  The map does NOT take
 * ownership of keys or values and does NOT copy them.
 *
 * Basic usage:
 * @code
 *   hashmap m;
 *   hashmap_init(&m, 0, NULL, NULL);
 *   hashmap_set(&m, "key1", val1);
 *   void *v = hashmap_get(&m, "key1");
 *   hashmap_remove(&m, "key1");
 *   hashmap_free(&m);
 * @endcode
 */

typedef uint64_t (*hashmap_hash_fn)(const void *key);
typedef bool     (*hashmap_eq_fn)(const void *a, const void *b);

typedef struct {
    void   *key;
    void   *value;
    bool    occupied;
    bool    tombstone;
} hashmap_entry;

typedef struct {
    hashmap_entry *entries;
    size_t         capacity;     /* total slots (always a power of two) */
    size_t         count;        /* number of live entries */
    size_t         tombstones;
    hashmap_hash_fn hash_fn;
    hashmap_eq_fn   eq_fn;
} hashmap;

/* Default hash / equality functions (for C-string keys) ---------------- */

STK_API uint64_t hashmap_str_hash(const void *key);
STK_API bool     hashmap_str_eq(const void *a, const void *b);

/* Lifetime ------------------------------------------------------------- */

STK_API void     hashmap_init(hashmap *m, size_t initial_capacity,
                              hashmap_hash_fn hash_fn, hashmap_eq_fn eq_fn);
STK_API void     hashmap_free(hashmap *m);

/* Core operations ------------------------------------------------------ */

STK_API void     hashmap_set(hashmap *m, void *key, void *value);
STK_API void    *hashmap_get(const hashmap *m, const void *key);
STK_API bool     hashmap_has(const hashmap *m, const void *key);
STK_API void    *hashmap_remove(hashmap *m, const void *key);
STK_API void     hashmap_clear(hashmap *m);

/* Iteration ------------------------------------------------------------ */

STK_API void     hashmap_foreach(const hashmap *m,
                                 bool (*fn)(void *key, void *value, void *ud),
                                 void *ud);

/* Introspection -------------------------------------------------------- */

STK_API size_t   hashmap_count(const hashmap *m);
STK_API size_t   hashmap_capacity(const hashmap *m);
STK_API double   hashmap_load_factor(const hashmap *m);

#ifdef __cplusplus
}
#endif

#endif /* STK_HASHMAP_H */
