#include "stk/core/hashmap.h"
#include <stdint.h>
#include <string.h>

/* 规范 1：hash 函数签名严格匹配 stk_hashmap_hash_fn
   —— 入参 const void*，返回 uint64_t */
static uint64_t hash_str(const void* key)
{
    const char* s = (const char*)key;
    uint64_t h = 1469598103934665603ULL; /* FNV-1a offset */
    for (; *s; ++s) {
        h ^= (uint64_t)(unsigned char)*s;
        h *= 1099511628211ULL;
    }
    return h;
}

int main(void)
{
    stk_hashmap hashmap;
    int v1 = 42;   /* 规范 2：value 是 void*，用真实变量的地址，别传字面量 */
    int v2 = 7;

    /* 规范 3：init 第3参严格传 stk_hashmap_hash_fn */
    if (stk_hashmap_init(&hashmap, 32, hash_str, NULL) != STK_OK) {
        return 1;
    }

    /* 规范 4：key/value 都是 void*，传地址 */
    stk_hashmap_set(&hashmap, "k1", &v1);
    stk_hashmap_set(&hashmap, "k2", &v2);

    int* got = (int*)stk_hashmap_get(&hashmap, "k1");
    if (got == NULL || *got != 42) {
        stk_hashmap_clear(&hashmap);
        stk_hashmap_free(&hashmap);
        return 1;
    }

    stk_hashmap_free(&hashmap);
    return 0;
}
