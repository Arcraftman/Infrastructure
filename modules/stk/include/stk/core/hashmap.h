#ifndef STK_CORE_HASHMAP_H
#define STK_CORE_HASHMAP_H


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
 *   stk_hashmap m;
 *   stk_hashmap_init(&m, 0, NULL, NULL);
 *   stk_hashmap_set(&m, "key1", val1);
 *   void *v = stk_hashmap_get(&m, "key1");
 *   stk_hashmap_remove(&m, "key1");
 *   stk_hashmap_free(&m);
 * @endcode
 */

typedef uint64_t (*stk_hashmap_hash_fn)(const void *key);
typedef bool     (*stk_hashmap_eq_fn)(const void *a, const void *b);

typedef struct {
    void   *key;
    void   *value;
    bool    occupied;
    bool    tombstone;
} stk_hashmap_entry;

typedef struct {
    stk_hashmap_entry *entries;
    size_t             capacity;     /* total slots (always a power of two) */
    size_t             count;        /* number of live entries */
    size_t             tombstones;
    stk_hashmap_hash_fn hash_fn;
    stk_hashmap_eq_fn   eq_fn;
} stk_hashmap;

/* Default hash / equality functions (for C-string keys) ---------------- */

STK_API uint64_t stk_hashmap_str_hash(const void *key);
STK_API bool     stk_hashmap_str_eq(const void *a, const void *b);

/* Lifetime ------------------------------------------------------------- */

STK_API STK_STATUS stk_hashmap_init(stk_hashmap *m, size_t initial_capacity,
                                    stk_hashmap_hash_fn hash_fn, stk_hashmap_eq_fn eq_fn);
STK_API STK_STATUS stk_hashmap_free(stk_hashmap *m);

/* Core operations ------------------------------------------------------ */

STK_API STK_STATUS stk_hashmap_set(stk_hashmap *m, void *key, void *value);
STK_API void      *stk_hashmap_get(const stk_hashmap *m, const void *key);
STK_API bool       stk_hashmap_has(const stk_hashmap *m, const void *key);
STK_API void      *stk_hashmap_remove(stk_hashmap *m, const void *key);
STK_API STK_STATUS stk_hashmap_clear(stk_hashmap *m);

/* Iteration ------------------------------------------------------------ */

STK_API STK_STATUS stk_hashmap_foreach(const stk_hashmap *m,
                                       bool (*fn)(void *key, void *value, void *ud),
                                       void *ud);

/* Introspection -------------------------------------------------------- */

STK_API size_t   stk_hashmap_count(const stk_hashmap *m);
STK_API size_t   stk_hashmap_capacity(const stk_hashmap *m);
STK_API double   stk_hashmap_load_factor(const stk_hashmap *m);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_HASHMAP_H */