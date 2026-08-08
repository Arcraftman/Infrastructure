#ifndef STK_UTILS_PRESET_H
#define STK_UTILS_PRESET_H

#include "stk/def.h"
#include "stk/utils/status.h"

#ifdef __cplusplus
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
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
/* Windows specific */
#else
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* STK_UTILS_PRESET_H */