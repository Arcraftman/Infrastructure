/*
 * stk_set — a hash set built on top of stk_hashmap.
 *
 * Engineering notes:
 *   - The set never reaches into hashmap's private fields; it uses only the
 *     public map API plus the map iterator.  This keeps set.c insulated from
 *     backend changes (chaining vs. open addressing).
 *   - Set elements are the map keys; values are always NULL.  The set does
 *     not own element memory — the caller owns the pointers it inserts.
 *   - Allocation is injected through stk_hashmap, so a set can use a custom
 *     allocator via stk_set_init_with_alloc.
 *   - Set algebra (union / intersection / difference / symmetric difference)
 *     is implemented with a single in-place filter helper, which is safe
 *     because hashmap_foreach snapshots entries before invoking callbacks.
 */

#include "stk/core/set.h"

#include "stk/core/hashmap.h"
#include "stk/core/preset.h"
#include "stk/def.h"
#include "stk/utils/logger.h"
#include "stk/utils/status.h"

/* ------------------------------------------------------------------ */
/* Construction / destruction                                         */
/* ------------------------------------------------------------------ */

STK_STATUS stk_set_init_with_alloc(stk_set* set,
                                   stk_hashmap_hash_fn hash_fn,
                                   stk_hashmap_eq_fn eq_fn,
                                   const stk_allocator* alloc)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: init NULL self");
    /* Open addressing is the better default for set workloads (no node
     * overhead, cache-friendly, frequent membership tests). */
    return stk_hashmap_create_with_alloc(&set->map, HASHMAP_MODE_OPEN_ADDRESS,
                                          0, hash_fn, eq_fn, alloc);
}

STK_STATUS stk_set_init(stk_set* set, stk_hashmap_hash_fn hash_fn, stk_hashmap_eq_fn eq_fn)
{
    return stk_set_init_with_alloc(set, hash_fn, eq_fn, NULL);
}

STK_STATUS stk_set_init_with_capacity(stk_set* set,
                                      size_t initial_capacity,
                                      stk_hashmap_hash_fn hash_fn,
                                      stk_hashmap_eq_fn eq_fn)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: init_with_capacity NULL self");
    return stk_hashmap_create(&set->map, HASHMAP_MODE_OPEN_ADDRESS,
                              initial_capacity, hash_fn, eq_fn);
}

STK_STATUS stk_set_free(stk_set* set)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: free NULL self");
    return stk_hashmap_free(&set->map);
}

/* ------------------------------------------------------------------ */
/* Core operations                                                    */
/* ------------------------------------------------------------------ */

STK_STATUS stk_set_insert(stk_set* set, void* value)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: insert NULL self");
    STK_RETURN_IF(!value, STK_EINVAL, "set: insert NULL value");
    return stk_hashmap_set(&set->map, value, NULL);
}

STK_STATUS stk_set_remove(stk_set* set, const void* value)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: remove NULL self");
    STK_RETURN_IF(!value, STK_EINVAL, "set: remove NULL value");
    stk_hashmap_remove(&set->map, value);
    return STK_OK;
}

bool stk_set_contains(const stk_set* set, const void* value)
{
    if (!set || !value)
        return false;
    return stk_hashmap_has(&set->map, value);
}

STK_STATUS stk_set_clear(stk_set* set)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: clear NULL self");
    return stk_hashmap_clear(&set->map);
}

/* ------------------------------------------------------------------ */
/* Set algebra                                                        */
/* ------------------------------------------------------------------ */

/* In-place filter: walk `dst`, and for each element decide whether to
 * keep it.  `keep` receives the element and the peer set, and returns
 * true if the element should remain.  Because hashmap_foreach snapshots
 * all pairs before invoking the callback, removing from `dst` mid-walk
 * is safe. */
typedef bool (*set_keep_fn)(const void* value, const stk_set* peer);

static void set_filter(stk_set* dst, const stk_set* peer, set_keep_fn keep)
{
    /* Snapshot the keys first (iterating + removing simultaneously is fine
     * with the pull iterator, but snapshotting keeps the logic obviously
     * correct regardless of backend). */
    size_t n = stk_set_size(dst);
    void** keys = n ? (void**)malloc(n * sizeof(void*)) : NULL;
    if (!keys && n) {
        STK_LOG_ERROR("set: filter snapshot alloc failed");
        return;
    }
    size_t k = 0;
    stk_hashmap_iter it;
    stk_hashmap_iter_init(&it, &dst->map);
    void* key;
    void* val;
    while (stk_hashmap_iter_next(&it, &key, &val) && k < n) {
        (void)val;
        keys[k++] = key;
    }
    for (size_t i = 0; i < k; i++) {
        if (!keep(keys[i], peer))
            stk_set_remove(dst, keys[i]);
    }
    free(keys);
}

static bool keep_in_both(const void* v, const stk_set* peer) { return stk_set_contains(peer, v); }
static bool keep_not_in(const void* v, const stk_set* peer)  { return !stk_set_contains(peer, v); }

STK_STATUS stk_set_union(stk_set* dst, const stk_set* src)
{
    STK_RETURN_IF(!dst, STK_EINVAL, "set: union NULL dst");
    STK_RETURN_IF(!src, STK_EINVAL, "set: union NULL src");

    stk_hashmap_iter it;
    stk_hashmap_iter_init(&it, &src->map);
    void* key;
    void* val;
    while (stk_hashmap_iter_next(&it, &key, &val)) {
        (void)val;
        if (!stk_set_contains(dst, key))
            stk_set_insert(dst, key);
    }
    return STK_OK;
}

STK_STATUS stk_set_intersection(stk_set* dst, const stk_set* src)
{
    STK_RETURN_IF(!dst, STK_EINVAL, "set: intersection NULL dst");
    STK_RETURN_IF(!src, STK_EINVAL, "set: intersection NULL src");
    set_filter(dst, src, keep_in_both);   /* keep only elements also in src */
    return STK_OK;
}

STK_STATUS stk_set_difference(stk_set* dst, const stk_set* src)
{
    STK_RETURN_IF(!dst, STK_EINVAL, "set: difference NULL dst");
    STK_RETURN_IF(!src, STK_EINVAL, "set: difference NULL src");
    set_filter(dst, src, keep_not_in);     /* drop elements that are in src  */
    return STK_OK;
}

STK_STATUS stk_set_symmetric_difference(stk_set* dst, const stk_set* src)
{
    STK_RETURN_IF(!dst, STK_EINVAL, "set: symdiff NULL dst");
    STK_RETURN_IF(!src, STK_EINVAL, "set: symdiff NULL src");

    /* (dst \ src) ∪ (src \ dst) == (dst ∪ src) \ (dst ∩ src). */
    stk_set inter;
    stk_set_init_with_alloc(&inter, dst->map.hash_fn, dst->map.eq_fn, dst->map.alloc);

    /* inter = dst ∩ src */
    stk_hashmap_iter it;
    stk_hashmap_iter_init(&it, &src->map);
    void* k;
    void* v;
    while (stk_hashmap_iter_next(&it, &k, &v)) {
        (void)v;
        if (stk_set_contains(dst, k))
            stk_set_insert(&inter, k);
    }

    stk_set_union(dst, src);                 /* dst = dst ∪ src */
    set_filter(dst, &inter, keep_not_in);    /* drop dst∩src -> symmetric diff */

    stk_set_free(&inter);
    return STK_OK;
}

bool stk_set_is_subset(const stk_set* a, const stk_set* b)
{
    if (!a || !b)
        return false;
    if (stk_set_size(a) > stk_set_size(b))
        return false;

    stk_hashmap_iter it;
    stk_hashmap_iter_init(&it, &a->map);
    void* key;
    void* val;
    while (stk_hashmap_iter_next(&it, &key, &val)) {
        (void)val;
        if (!stk_set_contains(b, key))
            return false;
    }
    return true;
}

bool stk_set_is_equal(const stk_set* a, const stk_set* b)
{
    if (!a || !b)
        return false;
    if (stk_set_size(a) != stk_set_size(b))
        return false;
    return stk_set_is_subset(a, b);
}

/* ------------------------------------------------------------------ */
/* Query / traversal                                                  */
/* ------------------------------------------------------------------ */

size_t stk_set_size(const stk_set* set)  { return set ? stk_hashmap_count(&set->map) : 0; }
bool   stk_set_empty(const stk_set* set) { return stk_set_size(set) == 0; }

STK_STATUS stk_set_foreach(const stk_set* set,
                           bool (*fn)(void* value, void* user_data),
                           void* user_data)
{
    STK_RETURN_IF(!set, STK_EINVAL, "set: foreach NULL self");
    STK_RETURN_IF(!fn,  STK_EINVAL, "set: foreach NULL fn");

    stk_hashmap_iter it;
    stk_hashmap_iter_init(&it, &set->map);
    void* key;
    void* val;
    while (stk_hashmap_iter_next(&it, &key, &val)) {
        (void)val;
        if (!fn(key, user_data))
            break;
    }
    return STK_OK;
}

void** stk_set_to_array(const stk_set* set)
{
    if (!set)
        return NULL;
    size_t n = stk_set_size(set);
    if (n == 0)
        return NULL;

    void** array = (void**)malloc(n * sizeof(void*));
    if (!array)
        return NULL;

    size_t idx = 0;
    stk_hashmap_iter it;
    stk_hashmap_iter_init(&it, &set->map);
    void* key;
    void* val;
    while (stk_hashmap_iter_next(&it, &key, &val) && idx < n) {
        (void)val;
        array[idx++] = key;
    }
    return array;
}

void stk_set_free_array(void** array)
{
    free(array);
}
