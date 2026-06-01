#ifndef STK_UTILS_LOGGER_H
#define STK_UTILS_LOGGER_H

#include "preset.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Thread-local storage buffer size */
#define STK_THREAD_BUFFER_SIZE 8192
#define STK_DEFAULT_BUFFER_SIZE 4096
#define STK_DEFAULT_FLUSH_INTERVAL 3
#define STK_TIME_STRING_SIZE 64

/* Color definitions (only for ANSI terminals) */
#ifdef _WIN32
    #define STK_COLOR_RESET   ""
    #define STK_COLOR_TRACE   ""
    #define STK_COLOR_DEBUG   ""
    #define STK_COLOR_INFO    ""
    #define STK_COLOR_WARN    ""
    #define STK_COLOR_ERROR   ""
    #define STK_COLOR_FATAL   ""
#else
    #define STK_COLOR_RESET   "\033[0m"
    #define STK_COLOR_TRACE   "\033[90m"
    #define STK_COLOR_DEBUG   "\033[36m"
    #define STK_COLOR_INFO    "\033[32m"
    #define STK_COLOR_WARN    "\033[33m"
    #define STK_COLOR_ERROR   "\033[31m"
    #define STK_COLOR_FATAL   "\033[35m"
#endif

/* Log level definitions */
typedef enum {
    STK_LOG_LEVEL_TRACE = 0,
    STK_LOG_LEVEL_DEBUG = 1,
    STK_LOG_LEVEL_INFO  = 2,
    STK_LOG_LEVEL_WARN  = 3,
    STK_LOG_LEVEL_ERROR = 4,
    STK_LOG_LEVEL_FATAL = 5,
    STK_LOG_LEVEL_NONE  = 6
} stk_log_level_t;

/* Log output options */
typedef enum {
    STK_LOG_OUTPUT_CONSOLE = 0x01,  /* Output to console */
    STK_LOG_OUTPUT_FILE    = 0x02,  /* Output to file */
    STK_LOG_OUTPUT_BOTH    = 0x03   /* Output to both console and file */
} stk_log_output_t;

/* Log configuration structure */
typedef struct {
    stk_log_level_t log_level;          /* Log level */
    stk_log_output_t output_option;     /* Output options */
    const char *log_file;               /* Log file path */
    int enable_color;                   /* Enable color */
    int enable_timestamp;               /* Enable timestamp */
    int enable_fileline;                /* Enable filename and line number */
    int enable_function;                /* Enable function name */
    int enable_thread_id;               /* Enable thread ID */
    int enable_ms;                      /* Enable milliseconds */
    int buffer_size;                    /* Buffer size */
    int flush_interval;                 /* Flush interval (seconds, 0 = immediate) */
    int max_file_size;                  /* Max file size (MB) */
    int max_file_count;                 /* Max file count */
} stk_log_config_t;

/* API function declarations */
STK_API int stk_log_init(stk_log_config_t *config);
STK_API void stk_log_close(void);
STK_API int stk_log_reload(stk_log_config_t *config);
STK_API void stk_log_set_level(stk_log_level_t level);
STK_API stk_log_level_t stk_log_get_level(void);
STK_API void stk_log_flush(void);
STK_API void stk_log_write(stk_log_level_t level, const char *filename,
                           int line, const char *func, const char *format, ...);
STK_API int stk_log_is_enabled(stk_log_level_t level);

#ifdef __cplusplus
}
#endif

/* Log macro definitions */
#define STK_LOG_TRACE(format, ...) \
    stk_log_write(STK_LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define STK_LOG_DEBUG(format, ...) \
    stk_log_write(STK_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define STK_LOG_INFO(format, ...) \
    stk_log_write(STK_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define STK_LOG_WARN(format, ...) \
    stk_log_write(STK_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define STK_LOG_ERROR(format, ...) \
    stk_log_write(STK_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define STK_LOG_FATAL(format, ...) \
    stk_log_write(STK_LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#endif /* STK_UTILS_LOGGER_H */