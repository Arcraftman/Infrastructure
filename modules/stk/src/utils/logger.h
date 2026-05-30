#ifndef STK_LOGGER_H
#define LOGGER_H

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

/* 线程本地存储缓冲区大小 */
#define THREAD_BUFFER_SIZE 8192
#define DEFAULT_BUFFER_SIZE 4096
#define DEFAULT_FLUSH_INTERVAL 3
#define TIME_STRING_SIZE 64

/* 颜色定义 */
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

/* 日志级别定义 */
typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_WARN = 3,
    LOG_LEVEL_ERROR = 4,
    LOG_LEVEL_FATAL = 5,
    LOG_LEVEL_NONE = 6
} log_level_t;

/* 日志输出目标选项 */
typedef enum {
    LOG_OUTPUT_TO_CONSOLE = 0x01,  /* 输出到控制台 */
    LOG_OUTPUT_TO_FILE    = 0x02,  /* 输出到文件 */
    LOG_OUTPUT_TO_BOTH    = 0x03   /* 同时输出到控制台和文件 */
} log_output_option_t;

/* 日志配置结构 */
typedef struct {
    log_level_t log_level;              /* 日志级别 */
    log_output_option_t output_option;   /* 输出选项 */
    const char* log_file;                    /* 日志文件路径 */
    int enable_color;                        /* 是否启用颜色 */
    int enable_timestamp;                    /* 是否启用时间戳 */
    int enable_fileline;                     /* 是否启用文件名和行号 */
    int enable_function;                     /* 是否启用函数名 */
    int enable_thread_id;                    /* 是否启用线程ID */
    int enable_ms;                           /* 是否显示毫秒 */
    int buffer_size;                         /* 缓冲区大小 */
    int flush_interval;                      /* 刷新间隔（秒，0表示立即刷新） */
    int max_file_size;                       /* 最大文件大小（MB） */
    int max_file_count;                      /* 最大文件数量 */
} log_config_t;

/* API 函数声明 */
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

/* 日志宏定义 */
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

#endif /* LOGGER_H */