#ifndef STK_SRC_CORE_CONFIG_H
#define STK_SRC_CORE_CONFIG_H

/* =========================================================================
 * config.h — Internal build configuration for the stk/core module.
 *
 * This header is included by all core source files and internal
 * (non-public) headers.  It must NOT be installed; it is private to
 * the library build.
 * ========================================================================= */

#include "stk/def.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * Red-Black Tree defaults
 * ----------------------------------------------------------------------- */
#define RB_RED   0
#define RB_BLACK 1

#define RB_DEFAULT_CAPACITY 16
#define RB_GROW_FACTOR      2

/* -----------------------------------------------------------------------
 * String defaults
 * ----------------------------------------------------------------------- */
#define STRING_DEFAULT_CAPACITY 16
#define STRING_GROW_FACTOR      2

#define STRING_COMPARE_CASE_SENSITIVE   1
#define STRING_COMPARE_CASE_INSENSITIVE 0

/* -----------------------------------------------------------------------
 * Vector defaults
 * ----------------------------------------------------------------------- */
#define VECTOR_DEFAULT_CAPACITY 16
#define VECTOR_GROW_FACTOR      2
#define VECTOR_ELEMENT_SIZE     sizeof(void *)

#define VECTOR_USE_MEMSET  1
#define VECTOR_USE_REALLOC 1

/* -----------------------------------------------------------------------
 * Arena defaults
 * ----------------------------------------------------------------------- */
#define ARENA_DEFAULT_BLOCK_SIZE 4096
#define ARENA_ALIGNMENT          sizeof(double)

/* -----------------------------------------------------------------------
 * Buffer defaults
 * ----------------------------------------------------------------------- */
#define BUFFER_DEFAULT_CAPACITY  64
#define BUFFER_GROW_FACTOR       2

/* -----------------------------------------------------------------------
 * Hashmap defaults
 * ----------------------------------------------------------------------- */
#define HASHMAP_DEFAULT_CAPACITY 32
#define HASHMAP_LOAD_FACTOR      0.75
#define HASHMAP_GROW_FACTOR      2

/* -----------------------------------------------------------------------
 * Pool defaults
 * ----------------------------------------------------------------------- */
#define POOL_DEFAULT_CHUNK_SIZE  4096
#define POOL_ALIGNMENT           sizeof(double)

/* -----------------------------------------------------------------------
 * Binary Heap defaults
 * ----------------------------------------------------------------------- */
#define HEAP_DEFAULT_CAPACITY    16
#define HEAP_GROW_FACTOR         2

/* -----------------------------------------------------------------------
 * Bitset defaults
 * ----------------------------------------------------------------------- */
#define BITSET_DEFAULT_BITS      64
#define BITSET_BITS_PER_WORD     (sizeof(uint64_t) * 8)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STK_SRC_CORE_CONFIG_H */