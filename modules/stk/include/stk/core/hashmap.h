#ifndef STK_CORE_HASHMAP_H
#define STK_CORE_HASHMAP_H

#include "stk/core/internal/hashmap_allocator.h"
#include "stk/core/internal/hash.h"
#include "stk/core/internal/hashmap_chainmap.h"
#include "stk/core/internal/hashmap_oamap.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 两种后端：仅作为创建参数，结构体内部各自独立，不做运行时分派。 */
typedef enum stk_hashmap_mode {
    STK_HASHMAP_CHAIN = 0,
    STK_HASHMAP_OA    = 1
} stk_hashmap_mode;

/* 公开句柄。内部持有一个 union（chainmap / oamap），对外不透明。 */
typedef struct stk_hashmap stk_hashmap;

stk_hashmap *stk_hashmap_create(stk_hashmap_mode mode, size_t capacity,
                                stk_hash_fn hash, stk_eq_fn eq,
                                const stk_allocator *alloc);
void stk_hashmap_destroy(stk_hashmap *m);

int  stk_hashmap_set(stk_hashmap *m, void *key, void *value); /* 0=新 1=覆盖 */
void *stk_hashmap_get(const stk_hashmap *m, const void *key); /* NULL=未找到 */
int  stk_hashmap_remove(stk_hashmap *m, const void *key);     /* 0/1 */
void stk_hashmap_clear(stk_hashmap *m);
size_t stk_hashmap_size(const stk_hashmap *m);

/* 扩容：手动提升容量（不自动触发）。成功返回 1，参数非法或失败返回 0。 */
int stk_hashmap_reserve(stk_hashmap *m, size_t new_capacity);

/* 公开迭代器：封装两套后端的内部迭代器，对外不暴露具体类型。 */
typedef struct stk_hashmap_iter {
    stk_hashmap_mode mode;
    union {
        stk_chain_iter chain;
        stk_oa_iter    oa;
    } u;
} stk_hashmap_iter;

void  stk_hashmap_iter_begin(stk_hashmap_iter *it, const stk_hashmap *m);
int   stk_hashmap_iter_next(stk_hashmap_iter *it);   /* 0=结束 1=有元素 */
void *stk_hashmap_iter_key(const stk_hashmap_iter *it);
void *stk_hashmap_iter_value(const stk_hashmap_iter *it);
int   stk_hashmap_iter_remove(stk_hashmap_iter *it); /* 删当前项，仍可继续 next */

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_HASHMAP_H */
