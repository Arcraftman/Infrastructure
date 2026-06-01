#ifndef STK_UTILS_LOGGER_H
#define STK_UTILS_LOGGER_H


#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Logging is only enabled in Debug builds.
 * For Release builds, all logging macros become no-ops.
 * ============================================================ */

#if defined(STK_ENABLE_LOGGING) || defined(DEBUG) || defined(_DEBUG)
    #define STK_LOGGING_ENABLED 1
#else
    #define STK_LOGGING_ENABLED 0
#endif

/* Thread-local storage buffer size */
#define STK_THREAD_BUFFER_SIZE 8192
#define STK_DEFAULT_BUFFER_SIZE 4096
#define STK_DEFAULT_FLUSH_INTERVAL 3
#define STK_TIME_STRING_SIZE 64

/* Color definitions */
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
    STK_LOG_OUTPUT_CONSOLE = 0x01,
    STK_LOG_OUTPUT_FILE    = 0x02,
    STK_LOG_OUTPUT_BOTH    = 0x03
} stk_log_output_t;

/* Log configuration structure */
typedef struct {
    stk_log_level_t log_level;
    stk_log_output_t output_option;
    const char *log_file;
    int enable_color;
    int enable_timestamp;
    int enable_fileline;
    int enable_function;
    int enable_thread_id;
    int enable_ms;
    int buffer_size;
    int flush_interval;
    int max_file_size;
    int max_file_count;
} stk_log_config_t;

/* API function declarations - all return STK_STATUS */
STK_API STK_STATUS stk_log_init(stk_log_config_t *config);
STK_API STK_STATUS stk_log_close(void);
STK_API STK_STATUS stk_log_reload(stk_log_config_t *config);
STK_API STK_STATUS stk_log_set_level(stk_log_level_t level);
STK_API STK_STATUS stk_log_flush(void);
STK_API STK_STATUS stk_log_write(stk_log_level_t level, const char *filename,
                                  int line, const char *func, const char *format, ...);
STK_API int        stk_log_is_enabled(stk_log_level_t level);
STK_API stk_log_level_t stk_log_get_level(void);

#ifdef __cplusplus
}
#endif

/* ============================================================
 * Log macros - conditionally compiled
 * ============================================================ */

#if STK_LOGGING_ENABLED

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

    /* Assertion macro that logs before aborting */
    #define STK_ASSERT(expr, format, ...) \
        do { \
            if (!(expr)) { \
                stk_log_write(STK_LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, \
                              "Assertion failed: " #expr ". " format, ##__VA_ARGS__); \
                abort(); \
            } \
        } while(0)

    /* Return error with logging */
    #define STK_RETURN_IF_ERROR(expr, format, ...) \
        do { \
            STK_STATUS _stk_err = (expr); \
            if (_stk_err != STK_OK) { \
                STK_LOG_ERROR(format, ##__VA_ARGS__); \
                return _stk_err; \
            } \
        } while(0)

    /* Return error if condition fails */
    #define STK_RETURN_IF(cond, err, format, ...) \
        do { \
            if (cond) { \
                STK_LOG_ERROR(format, ##__VA_ARGS__); \
                return (err); \
            } \
        } while(0)

    /* Set error and goto cleanup */
    #define STK_GOTO_IF_ERROR(expr, label, format, ...) \
        do { \
            STK_STATUS _stk_rc = (expr); \
            if (_stk_rc != STK_OK) { \
                STK_LOG_ERROR(format, ##__VA_ARGS__); \
                goto label; \
            } \
        } while(0)

    /* Check pointer and return error if NULL */
    #define STK_CHECK_PTR(ptr, format, ...) \
        STK_RETURN_IF(!(ptr), STK_EINVAL, format, ##__VA_ARGS__)

    /* Check index range */
    #define STK_CHECK_RANGE(idx, max, format, ...) \
        STK_RETURN_IF((idx) >= (max), STK_ERANGE, format, ##__VA_ARGS__)

    /* Log and return error directly */
    #define STK_ERROR_RETURN(err, format, ...) \
        do { \
            STK_LOG_ERROR(format, ##__VA_ARGS__); \
            return (err); \
        } while(0)

    /* Log and goto label */
    #define STK_ERROR_GOTO(label, format, ...) \
        do { \
            STK_LOG_ERROR(format, ##__VA_ARGS__); \
            goto label; \
        } while(0)

#else /* Release build - all macros become no-ops */

    #define STK_LOG_TRACE(format, ...) ((void)0)
    #define STK_LOG_DEBUG(format, ...) ((void)0)
    #define STK_LOG_INFO(format, ...)  ((void)0)
    #define STK_LOG_WARN(format, ...)  ((void)0)
    #define STK_LOG_ERROR(format, ...) ((void)0)
    #define STK_LOG_FATAL(format, ...) ((void)0)

    #define STK_ASSERT(expr, format, ...) \
        do { \
            if (!(expr)) { \
                abort(); \
            } \
        } while(0)

    #define STK_RETURN_IF_ERROR(expr, format, ...) \
        do { \
            STK_STATUS _stk_err = (expr); \
            if (_stk_err != STK_OK) return _stk_err; \
        } while(0)

    #define STK_RETURN_IF(cond, err, format, ...) \
        do { \
            if (cond) return (err); \
        } while(0)

    #define STK_GOTO_IF_ERROR(expr, label, format, ...) \
        do { \
            STK_STATUS _stk_rc = (expr); \
            if (_stk_rc != STK_OK) goto label; \
        } while(0)

    #define STK_CHECK_PTR(ptr, format, ...) \
        do { if (!(ptr)) return STK_EINVAL; } while(0)

    #define STK_CHECK_RANGE(idx, max, format, ...) \
        do { if ((idx) >= (max)) return STK_ERANGE; } while(0)

    #define STK_ERROR_RETURN(err, format, ...) \
        return (err)

    #define STK_ERROR_GOTO(label, format, ...) \
        goto label

#endif /* STK_LOGGING_ENABLED */

#endif /* STK_UTILS_LOGGER_H */