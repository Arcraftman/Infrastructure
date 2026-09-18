#include "stk/core/set.h"
#include "internal/hashmap_chainmap.h"   /* 内部后端：与 hashmap.c 同一引用方式（靠 -I include/stk/core/internal） */
#include <stdlib.h>

/* set 基于 chainmap 实现：只存 key，value 存为 key 自身。
 * 内部直接内嵌一个 stk_chainmap（栈上，不额外堆分配 chainmap 本身）。
 * 只调用 chainmap 的 internal API，不访问其任何内部字段。 */
struct stk_set {
    stk_chainmap m;
};

stk_set *stk_set_create(stk_hash_fn hash, stk_eq_fn eq,
                        const stk_allocator *alloc) {
    stk_set *s = (stk_set *)stk_alloc(alloc, sizeof(stk_set));
    if (s == NULL) return NULL;
    /* bucket_count 传 0，由 chainmap_init 兜底到 16 */
    stk_i_chainmap_init(&s->m, 0, hash, eq, alloc);
    return s;
}

void stk_set_destroy(stk_set *s) {
    if (s == NULL) return;
    stk_i_chainmap_free(&s->m);
    stk_dealloc(s->m.alloc, s);
}

int stk_set_insert(stk_set *s, void *key) {
    if (s == NULL) return 0;
    /* value 存 key 自身：stk_i_chainmap_get 返回非 NULL 即代表「存在」，
     * 避免 value=NULL 与「未找到」冲突。out_old 不需要，传 NULL。
     * 返回约定：覆盖(已存在)→1，新插→0；OOM 当作 0（未存在）。 */
    int r = stk_i_chainmap_set(&s->m, key, key, NULL);
    return r == 1 ? 1 : 0;
}

int stk_set_contains(const stk_set *s, const void *key) {
    if (s == NULL) return 0;
    return stk_i_chainmap_get(&s->m, key) != NULL;
}

int stk_set_remove(stk_set *s, const void *key) {
    if (s == NULL) return 0;
    return stk_i_chainmap_remove(&s->m, key, NULL, NULL);
}

size_t stk_set_size(const stk_set *s) {
    if (s == NULL) return 0;
    return s->m.size;
}
