#ifndef STK_CORE_HASH_H
#define STK_CORE_HASH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 哈希函数类型：输入 key 指针，返回散列值。 */
typedef size_t (*stk_hash_fn)(const void *key);

/* 相等比较类型：相等返回非 0（注意与 STL 相反，0 表示不等）。 */
typedef int (*stk_eq_fn)(const void *a, const void *b);

/* 字符串键哈希（FNV-1a 64 → size_t 截断）。可直接作为 stk_hash_fn 传入。 */
size_t stk_hash_cstr(const void *key);

/* 整数 / 指针键哈希（key 指向 size_t / int 等整数）。可直接作为 stk_hash_fn 传入。 */
size_t stk_hash_int(const void *key);

/* 通用字节序列哈希（key + 长度）。签名不匹配 stk_hash_fn，
 * 供你自行封装的 hash_fn 内部调用。 */
size_t stk_hash_bytes(const void *data, size_t len);

/* 默认相等比较：字符串内容比较 / 整数内容比较 / 指针相等。
 * 可直接作为 stk_eq_fn 传入；整数比较要求 a、b 均指向 size_t / int 等整数。 */
int stk_eq_cstr(const void *a, const void *b);
int stk_eq_int(const void *a, const void *b);
int stk_eq_ptr(const void *a, const void *b);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_HASH_H */
