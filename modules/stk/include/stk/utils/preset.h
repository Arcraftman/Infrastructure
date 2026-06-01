#ifndef STK_UTILS_PRESET_H
#define STK_UTILS_PRESET_H


#ifdef __cplusplus
extern "C" {
#else

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifdef _WIN32
    /* Windows specific */
#else
    #include <unistd.h>
    #include <pthread.h>
    #include <sys/syscall.h>
#endif


#endif

#ifdef __cplusplus
}
#endif

#endif /* STK_UTILS_PRESET_H */