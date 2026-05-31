#ifndef STK_SRC_CORE_H
#define STK_SRC_CORE_H

#include "stk/def.h"
#include "core/config.h"

#ifdef __cplusplus
#include "core/cpp/vector.hpp"
#include "core/cpp/rbtree.hpp"
#include "core/cpp/string.hpp"
extern "C" {
#else
#include "core/vector.h"
#include "core/rbtree.h"
#include "core/str.h"
#include "core/list.h"
#include "core/arena.h"
#include "core/buffer.h"
#include "core/hashmap.h"
#include "core/pool.h"
#include "core/heap.h"
#include "core/bitset.h"
#endif

#ifdef __cplusplus
}
#endif

#endif