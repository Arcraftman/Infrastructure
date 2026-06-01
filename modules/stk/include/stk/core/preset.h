#ifndef STK_CORE_PRESET_H
#define STK_CORE_PRESET_H

#include "stk/def.h"

#ifdef __cplusplus
#include "cpp/rbtree.hpp"
#include "cpp/string.hpp"
#include "cpp/vector.hpp"
extern "C" {
#else
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "arena.h"
#include "bitset.h"
#include "buffer.h"
#include "hashmap.h"
#include "heap.h"
#include "list.h"
#include "pool.h"
#include "rbtree.h"
#include "vector.h"


#endif

#ifdef __cplusplus
}
#endif

#endif