#ifndef STK_SRC_CORE_CONFIG_H
#define STK_SRC_CORE_CONFIG_H

#include "def.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#define RB_RED   0
#define RB_BLACK 1

#define RB_DEFAULT_CAPACITY 16
#define RB_GROW_FACTOR 2

#define STRING_DEFAULT_CAPACITY 16
#define STRING_GROW_FACTOR 2

#define STRING_COMPARE_CASE_SENSITIVE 1
#define STRING_COMPARE_CASE_INSENSITIVE 0

#define VECTOR_DEFAULT_CAPACITY 16
#define VECTOR_GROW_FACTOR 2
#define VECTOR_ELEMENT_SIZE sizeof(void*)

#define VECTOR_USE_MEMSET 1
#define VECTOR_USE_REALLOC 1


#endif