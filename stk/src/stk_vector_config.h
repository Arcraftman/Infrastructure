#ifndef STK_VECTOR_CONFIG_H
#define STK_VECTOR_CONFIG_H

#include <stddef.h>
#include <stdbool.h>
#include "stk_export.h"

// 向量默认容量
#define VECTOR_DEFAULT_CAPACITY 16
#define VECTOR_GROW_FACTOR 2

// 内存操作选项
#define VECTOR_USE_MEMSET 1
#define VECTOR_USE_REALLOC 1

#endif