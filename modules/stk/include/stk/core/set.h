#ifndef STK_CORE_SET_H
#define STK_CORE_SET_H

#include "stk/core/hashmap.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* set 基于 chainmap 实现（只存 key，value 恒为 NULL）。 */
typedef struct stk_set stk_set;

stk_set *stk_set_create(stk_hash_fn hash, stk_eq_fn eq,
                        const stk_allocator *alloc);
void stk_set_destroy(stk_set *s);

int stk_set_insert(stk_set *s, void *key);   /* 0=新 1=已存在 */
int stk_set_contains(const stk_set *s, const void *key);
int stk_set_remove(stk_set *s, const void *key);
size_t stk_set_size(const stk_set *s);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_SET_H */
