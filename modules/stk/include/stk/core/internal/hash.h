#ifndef STK_CORE_INTERNAL_HASH_H
#define STK_CORE_INTERNAL_HASH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 哈希函数：只吃 key 指针，长度解释权交给具体实现。
 * 默认提供 cstr / ptr 两种；二进制 key 让用户自己写 hash_fn。 */
typedef size_t (*stk_hash_fn)(const void *key);

/* 相等函数：返回非 0 表示相等。 */
typedef int (*stk_eq_fn)(const void *a, const void *b);

/* 通用字节哈希（FNV-1a 64bit 截断 size_t），按 (key, len) 处理。
 * 不直接用作 stk_hash_fn（签名不匹配），供自定义 hash_fn 内部调用。 */
size_t stk_i_hash_bytes(const void *key, size_t len);

/* 默认哈希：以 NUL 结尾的字符串哈希 */
size_t stk_i_hash_cstr(const void *key);
/* 默认哈希：指针值本身 */
size_t stk_i_hash_ptr(const void *key);

/* 默认相等 */
int stk_i_eq_cstr(const void *a, const void *b);
int stk_i_eq_ptr(const void *a, const void *b);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_INTERNAL_HASH_H */
