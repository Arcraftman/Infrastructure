#ifndef STK_CORE_HASHMAP_H
#define STK_CORE_HASHMAP_H

#include "stk/def.h"
#include "stk/utils/status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hash map with a selectable backend strategy.
 *
 * Both keys and values are stored as void*.  The map does NOT take
 * ownership of keys or values and does NOT copy them.
 *
 * The backend strategy MUST be chosen explicitly at creation time via
 * stk_hashmap_create() and a HASHMAP_MODE value.  There is no implicit
 * default and no compatibility shim: callers must state which backend
 * they want.
 *
 *   HASHMAP_MODE_DEFAULT      - separate chaining.  Each bucket is the head
 *                               of a linked list; collisions are resolved by
 *                               appending nodes.  Deletion simply unlinks and
 *                               frees the node (no tombstones).  Best when the
 *                               key/value types or workload favor simple,
 *                               allocation-based chaining.
 *
 *   HASHMAP_MODE_OPEN_ADDRESS - open addressing with linear probing and
 *                               tombstone markers.  All entries live in one
 *                               contiguous array; deletion marks a slot as a
 *                               tombstone so the probe chain stays intact.
 *                               Cache-friendly, handles frequent deletes well.
 *
 * Basic usage:
 * @code
 *   stk_hashmap m;
 *   stk_hashmap_create(&m, HASHMAP_MODE_OPEN_ADDRESS, 0, NULL, NULL);
 *   stk_hashmap_set(&m, "key1", val1);
 *   void *v = stk_hashmap_get(&m, "key1");
 *   stk_hashmap_remove(&m, "key1");
 *   stk_hashmap_free(&m);
 * @endcode
 */

typedef uint64_t (*stk_hashmap_hash_fn)(const void* key);
typedef bool (*stk_hashmap_eq_fn)(const void* a, const void* b);

/* --------------------------------------------------------------------------
 * Allocator injection (P0)
 *
 * Every dynamic allocation made by the hashmap goes through an injected
 * allocator.  This makes the map testable (inject a counting allocator to
 * assert exact malloc counts / zero leaks without valgrind) and embeddable
 * (inject a pool / arena allocator to avoid fragmentation).
 *
 *   alloc(ctx, size) -> allocate `size` bytes, return NULL on failure
 *   free(ctx, ptr)   -> release a block previously returned by alloc
 *
 * `ctx` is passed through untouched; set it to NULL when not needed.
 * Use STK_ALLOCATOR_DEFAULT for the libc malloc/free pair.
 */
typedef struct stk_allocator {
    void* (*alloc)(void* ctx, size_t size);
    void (*free)(void* ctx, void* ptr);
    void* ctx;
} stk_allocator;

/* The default allocator: libc malloc/free, ctx unused (NULL). */
STK_API extern const stk_allocator STK_ALLOCATOR_DEFAULT;

/* Backend strategy, chosen at creation time. */
typedef enum {
    HASHMAP_MODE_DEFAULT = 0,   /* separate chaining          */
    HASHMAP_MODE_OPEN_ADDRESS    /* open addressing + tombstone */
} HASHMAP_MODE;

typedef struct stk_hashmap_entry {
    void* key;
    void* value;
    bool occupied;
    bool tombstone;
    struct stk_hashmap_entry* next; /* used only by HASHMAP_MODE_DEFAULT (chaining) */
} stk_hashmap_entry;

typedef struct {
    stk_hashmap_entry* entries;
    size_t capacity; /* total slots (always a power of two) */
    size_t count;    /* number of live entries */
    size_t tombstones;
    HASHMAP_MODE mode; /* backend strategy (DEFAULT | OPEN_ADDRESS) */
    stk_hashmap_hash_fn hash_fn;
    stk_hashmap_eq_fn eq_fn;
    const stk_allocator* alloc; /* allocator used for all dynamic memory */
} stk_hashmap;

/* Default hash / equality functions (for C-string keys) ---------------- */

STK_API uint64_t stk_hashmap_str_hash(const void* key);
STK_API bool stk_hashmap_str_eq(const void* a, const void* b);

/* Default hash / equality functions (for raw pointer keys) -------------
 * Keys are compared / hashed by their address value, not what they point
 * to. Useful when the map is keyed on object identity.
 */
STK_API uint64_t stk_hashmap_ptr_hash(const void* key);
STK_API bool stk_hashmap_ptr_eq(const void* a, const void* b);

/* Default hash / equality functions (for integer keys) -----------------
 * The integer is stored as the key pointer itself (no allocation, no
 * copy): pass (void*)(uintptr_t)your_int as the key. Lookups must use the
 * exact same integer value.
 */
STK_API uint64_t stk_hashmap_int_hash(const void* key);
STK_API bool stk_hashmap_int_eq(const void* a, const void* b);

/* Lifetime ------------------------------------------------------------- */

/* Constructor with an explicit allocator (P0).
 *
 * All dynamic memory (bucket array, chained nodes, internal scratch) is
 * obtained through `alloc`.  Pass &STK_ALLOCATOR_DEFAULT (or NULL) to use
 * the libc malloc/free pair.  The allocator pointer is stored by value; the
 * struct it points to must outlive the hashmap.
 *
 * @param mode          HASHMAP_MODE_DEFAULT or HASHMAP_MODE_OPEN_ADDRESS.
 * @param hash_fn/eq_fn Pass NULL to use the default string hash/eq pair.
 * @param alloc         Inject an allocator, or NULL for STK_ALLOCATOR_DEFAULT.
 */
STK_API STK_STATUS stk_hashmap_create_with_alloc(stk_hashmap* m,
                                                 HASHMAP_MODE mode,
                                                 size_t initial_capacity,
                                                 stk_hashmap_hash_fn hash_fn,
                                                 stk_hashmap_eq_fn eq_fn,
                                                 const stk_allocator* alloc);

/* Backward-compatible constructor.  Equivalent to
 * stk_hashmap_create_with_alloc(..., NULL) — uses STK_ALLOCATOR_DEFAULT.
 * The backend is chosen by `mode`; there is no implicit default.
 */
STK_API STK_STATUS stk_hashmap_create(stk_hashmap* m,
                                      HASHMAP_MODE mode,
                                      size_t initial_capacity,
                                      stk_hashmap_hash_fn hash_fn,
                                      stk_hashmap_eq_fn eq_fn);
STK_API STK_STATUS stk_hashmap_free(stk_hashmap* m);

/* Core operations ------------------------------------------------------ */

STK_API STK_STATUS stk_hashmap_set(stk_hashmap* m, void* key, void* value);
STK_API void* stk_hashmap_get(const stk_hashmap* m, const void* key);
STK_API bool stk_hashmap_has(const stk_hashmap* m, const void* key);
STK_API void* stk_hashmap_remove(stk_hashmap* m, const void* key);
STK_API STK_STATUS stk_hashmap_clear(stk_hashmap* m);

/* Reserve capacity for at least `capacity` slots (rounded up to a power of
 * two).  Does nothing if already large enough.  Useful to avoid repeated
 * rehashing when the final size is known up front. */
STK_API STK_STATUS stk_hashmap_reserve(stk_hashmap* m, size_t capacity);

/* Insert only if `key` is absent.  If the key already exists, `*existing`
 * (when non-NULL) receives the current value and nothing is inserted.
 * Returns STK_OK in both cases. */
STK_API STK_STATUS stk_hashmap_put_if_absent(stk_hashmap* m, void* key, void* value,
                                             void** existing);

/* Iteration ------------------------------------------------------------ */

/* Callback-style iteration (push model).  Safe against removals performed
 * inside the callback because it snapshots all pairs up front. */
STK_API STK_STATUS stk_hashmap_foreach(const stk_hashmap* m,
                                       bool (*fn)(void* key, void* value, void* ud),
                                       void* ud);

/* Explicit iterator (P1, pull model).
 *
 * Unlike foreach, the iterator walks the map on demand.  This lets the
 * caller break early, nest traversals, and remove the *current* entry
 * without corrupting the walk: after iter_remove() the cursor has already
 * advanced, so the deleted slot/node is never touched again.
 *
 *   stk_hashmap_iter it;
 *   stk_hashmap_iter_init(&it, &m);
 *   void *k, *v;
 *   while (stk_hashmap_iter_next(&it, &k, &v)) {
 *       if (should_drop(k)) stk_hashmap_iter_remove(&it); // removes k
 *   }
 *
 * iter_remove() removes the entry most recently returned by iter_next().
 * Calling it twice without an intervening iter_next() is a no-op (returns
 * NULL).  iter_next() after iter_remove() continues correctly.
 */
typedef struct stk_hashmap_iter {
    const stk_hashmap* m;
    size_t bucket; /* current bucket index (chaining scan cursor)    */
    size_t slot;   /* open-address probe cursor                      */
    stk_hashmap_entry* node; /* next chained node to return (DEFAULT only)  */
    stk_hashmap_entry* curr; /* entry most recently returned by iter_next   */
    bool valid;    /* whether `curr` points at a live entry          */
} stk_hashmap_iter;

STK_API void stk_hashmap_iter_init(stk_hashmap_iter* it, const stk_hashmap* m);
STK_API bool stk_hashmap_iter_next(stk_hashmap_iter* it, void** key, void** value);
STK_API void* stk_hashmap_iter_remove(stk_hashmap_iter* it);

/* Introspection -------------------------------------------------------- */

STK_API size_t stk_hashmap_count(const stk_hashmap* m);
STK_API size_t stk_hashmap_capacity(const stk_hashmap* m);
STK_API double stk_hashmap_load_factor(const stk_hashmap* m);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_HASHMAP_H */
