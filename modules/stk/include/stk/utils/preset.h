#ifndef STK_UTILS_PRESET_H
#define STK_UTILS_PRESET_H



#ifdef __cplusplus

extern "C" {
#else

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>   // 添加这一行
#include <sys/stat.h>

#ifdef _WIN32
    // Windows 特定代码
#else
    #include <unistd.h>
    #include <pthread.h>
    #include <sys/syscall.h>
#endif

#include "logger.h"
#include "other.h"

#endif

#ifdef __cplusplus
}
#endif

#endif