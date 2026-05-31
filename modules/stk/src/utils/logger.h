#ifndef STK_LOGGER_H
#define STK_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <windows.h>
#include <process.h>
#define gettid() GetCurrentThreadId()
#define localtime_r(t, tm) localtime_s(tm, t)
#else
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

/* Thread-local storage buffer size */
#define THREAD_BUFFER_SIZE 8192
#define DEFAULT_BUFFER_SIZE 4096
#define DEFAULT_FLUSH_INTERVAL 3
#define TIME_STRING_SIZE 64

/* Color definitions */
#define COLOR_RESET   "\033[0m"
#define COLOR_TRACE   "\033[90m"
#define COLOR_DEBUG   "\033[36m"
#define COLOR_INFO    "\033[32m"
#define COLOR_WARN    "\033[33m"
#define COLOR_ERROR   "\033[31m"
#define COLOR_FATAL   "\033[35m"

#ifdef __cplusplus
extern "C" {
#endif

/* Log level definitions */
typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_WARN = 3,
    LOG_LEVEL_ERROR = 4,
    LOG_LEVEL_FATAL = 5,
    LOG_LEVEL_NONE = 6
} log_level_t;

/* Log output options */
typedef enum {
    LOG_OUTPUT_TO_CONSOLE = 0x01,  /* Output to console */
    LOG_OUTPUT_TO_FILE    = 0x02,  /* Output to file */
    LOG_OUTPUT_TO_BOTH    = 0x03   /* Output to both console and file */
} log_output_option_t;

/* Log configuration structure */
typedef struct {
    log_level_t log_level;              /* Log level */
    log_output_option_t output_option;  /* Output options */
    const char* log_file;              /* Log file path */
    int enable_color;                  /* Enable color */
    int enable_timestamp;              /* Enable timestamp */
    int enable_fileline;               /* Enable filename and line number */
    int enable_function;               /* Enable function name */
    int enable_thread_id;              /* Enable thread ID */
    int enable_ms;                     /* Enable milliseconds */
    int buffer_size;                   /* Buffer size */
    int flush_interval;                /* Flush interval (seconds, 0 = immediate) */
    int max_file_size;                 /* Max file size (MB) */
    int max_file_count;                /* Max file count */
} log_config_t;

/* API function declarations */
int log_init(log_config_t* config);
void log_close(void);
int log_reload(log_config_t* config);
void log_set_level(log_level_t level);
log_level_t log_get_level(void);
void log_flush(void);
void log_write(log_level_t level, const char* filename, 
                   int line, const char* func, const char* format, ...);
int log_is_enabled(log_level_t level);

#ifdef __cplusplus
}
#endif

/* Log macro definitions */
#define LOG_TRACE(format, ...) \
    log_write(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    log_write(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    log_write(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_FATAL(format, ...) \
    log_write(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#endif /* STK_LOGGER_H */