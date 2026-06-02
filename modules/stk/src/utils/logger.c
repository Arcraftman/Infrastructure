#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/utils/preset.h"


#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
#else
    #include <unistd.h>
    #include <pthread.h>
    #include <sys/syscall.h>
#endif

/* ============================================================
 * Logger implementation
 * ============================================================ */

#if STK_LOGGING_ENABLED

/* Global logger state */
static struct {
    stk_log_config_t config;
    FILE *log_fp;
    int initialized;
    time_t last_flush;
    int file_sequence;
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} g_logger = {
    .config = {
        .log_level = STK_LOG_LEVEL_DEBUG,
        .output_option = STK_LOG_OUTPUT_CONSOLE,
        .log_file = NULL,
        .enable_color = 1,
        .enable_timestamp = 1,
        .enable_fileline = 1,
        .enable_function = 1,
        .enable_thread_id = 1,
        .enable_ms = 0,
        .buffer_size = STK_DEFAULT_BUFFER_SIZE,
        .flush_interval = STK_DEFAULT_FLUSH_INTERVAL,
        .max_file_size = 10,
        .max_file_count = 3,
    },
    .log_fp = NULL,
    .initialized = 0,
    .last_flush = 0,
    .file_sequence = 0,
};

/* Initialize mutex */
static void logger_lock_init(void) {
#ifdef _WIN32
    InitializeCriticalSection(&g_logger.lock);
#else
    pthread_mutex_init(&g_logger.lock, NULL);
#endif
}

static void logger_lock(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_logger.lock);
#else
    pthread_mutex_lock(&g_logger.lock);
#endif
}

static void logger_unlock(void) {
#ifdef _WIN32
    LeaveCriticalSection(&g_logger.lock);
#else
    pthread_mutex_unlock(&g_logger.lock);
#endif
}

/* Get current time as string */
static void get_time_string(char *buf, size_t size, int with_ms) {
    struct timeval tv;
    struct tm tm;
    
    gettimeofday(&tv, NULL);
    
#ifdef _WIN32
    localtime_s(&tm, &tv.tv_sec);
#else
    localtime_r(&tv.tv_sec, &tm);
#endif
    
    if (with_ms) {
        strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
        size_t len = strlen(buf);
        snprintf(buf + len, size - len, ".%03ld", tv.tv_usec / 1000);
    } else {
        strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
    }
}

/* Get thread ID */
static unsigned long get_thread_id(void) {
#ifdef _WIN32
    return (unsigned long)GetCurrentThreadId();
#else
    return (unsigned long)syscall(SYS_gettid);
#endif
}

/* Get log level string */
static const char* level_to_string(stk_log_level_t level, int color) {
    if (color) {
        switch (level) {
            case STK_LOG_LEVEL_TRACE: return STK_COLOR_TRACE "TRACE" STK_COLOR_RESET;
            case STK_LOG_LEVEL_DEBUG: return STK_COLOR_DEBUG "DEBUG" STK_COLOR_RESET;
            case STK_LOG_LEVEL_INFO:  return STK_COLOR_INFO "INFO " STK_COLOR_RESET;
            case STK_LOG_LEVEL_WARN:  return STK_COLOR_WARN "WARN " STK_COLOR_RESET;
            case STK_LOG_LEVEL_ERROR: return STK_COLOR_ERROR "ERROR" STK_COLOR_RESET;
            case STK_LOG_LEVEL_FATAL: return STK_COLOR_FATAL "FATAL" STK_COLOR_RESET;
            default: return "UNKNOWN";
        }
    } else {
        switch (level) {
            case STK_LOG_LEVEL_TRACE: return "TRACE";
            case STK_LOG_LEVEL_DEBUG: return "DEBUG";
            case STK_LOG_LEVEL_INFO:  return "INFO ";
            case STK_LOG_LEVEL_WARN:  return "WARN ";
            case STK_LOG_LEVEL_ERROR: return "ERROR";
            case STK_LOG_LEVEL_FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
}

/* Rotate log file if needed */
static void rotate_log_file(void) {
    if (!g_logger.config.log_file || g_logger.config.max_file_size <= 0) {
        return;
    }
    
    if (g_logger.log_fp) {
        long pos = ftell(g_logger.log_fp);
        if (pos >= (long)(g_logger.config.max_file_size * 1024 * 1024)) {
            fclose(g_logger.log_fp);
            g_logger.log_fp = NULL;
            
            char old_path[512];
            char new_path[512];
            for (int i = g_logger.config.max_file_count - 1; i > 0; i--) {
                snprintf(old_path, sizeof(old_path), "%s.%d", g_logger.config.log_file, i);
                snprintf(new_path, sizeof(new_path), "%s.%d", g_logger.config.log_file, i + 1);
                rename(old_path, new_path);
            }
            snprintf(old_path, sizeof(old_path), "%s.1", g_logger.config.log_file);
            rename(g_logger.config.log_file, old_path);
        }
    }
}

/* Write to file */
static void write_to_file(const char *msg) {
    if (!g_logger.log_fp && g_logger.config.log_file) {
        g_logger.log_fp = fopen(g_logger.config.log_file, "a");
        if (!g_logger.log_fp) return;
        setvbuf(g_logger.log_fp, NULL, _IOLBF, g_logger.config.buffer_size);
    }
    
    if (g_logger.log_fp) {
        rotate_log_file();
        fprintf(g_logger.log_fp, "%s", msg);
        fflush(g_logger.log_fp);
    }
}

/* Write to console */
static void write_to_console(const char *msg) {
    printf("%s", msg);
    fflush(stdout);
}

/* Core log writing function */
static void log_write_impl(stk_log_level_t level, const char *filename,
                           int line, const char *func, const char *format, va_list ap) {
    if (!g_logger.initialized || level < g_logger.config.log_level) {
        return;
    }
    
    char time_str[STK_TIME_STRING_SIZE] = "";
    char tid_str[32] = "";
    char fileline_str[256] = "";
    char func_str[128] = "";
    char msg_buffer[STK_THREAD_BUFFER_SIZE];
    char final_buffer[STK_THREAD_BUFFER_SIZE + 512];
    
    if (g_logger.config.enable_timestamp) {
        get_time_string(time_str, sizeof(time_str), g_logger.config.enable_ms);
    }
    
    if (g_logger.config.enable_thread_id) {
        snprintf(tid_str, sizeof(tid_str), "[%lu] ", get_thread_id());
    }
    
    if (g_logger.config.enable_fileline && filename && line > 0) {
        const char *fname = strrchr(filename, '/');
        if (!fname) fname = strrchr(filename, '\\');
        if (!fname) fname = filename;
        else fname++;
        snprintf(fileline_str, sizeof(fileline_str), "[%s:%d] ", fname, line);
    }
    
    if (g_logger.config.enable_function && func) {
        snprintf(func_str, sizeof(func_str), "[%s] ", func);
    }
    
    vsnprintf(msg_buffer, sizeof(msg_buffer), format, ap);
    
    const char *level_str = level_to_string(level, g_logger.config.enable_color);
    int color = g_logger.config.enable_color;
    
    if (color) {
        snprintf(final_buffer, sizeof(final_buffer),
                 "%s%s%s%s%s%s %s %s\n",
                 time_str, (time_str[0] ? " " : ""),
                 tid_str, fileline_str, func_str,
                 level_str, msg_buffer, STK_COLOR_RESET);
    } else {
        snprintf(final_buffer, sizeof(final_buffer),
                 "%s%s%s%s%s%s %s\n",
                 time_str, (time_str[0] ? " " : ""),
                 tid_str, fileline_str, func_str,
                 level_str, msg_buffer);
    }
    
    logger_lock();
    
    if (g_logger.config.output_option & STK_LOG_OUTPUT_CONSOLE) {
        write_to_console(final_buffer);
    }
    if (g_logger.config.output_option & STK_LOG_OUTPUT_FILE) {
        if (color) {
            char plain_buffer[STK_THREAD_BUFFER_SIZE + 512];
            char *p = plain_buffer;
            for (const char *src = final_buffer; *src; src++) {
                if (*src == '\033') {
                    while (*src && *src != 'm') src++;
                    if (*src) src++;
                }
                if (*src) *p++ = *src;
            }
            *p = '\0';
            write_to_file(plain_buffer);
        } else {
            write_to_file(final_buffer);
        }
    }
    
    time_t now = time(NULL);
    if (g_logger.config.flush_interval > 0 && 
        now - g_logger.last_flush >= g_logger.config.flush_interval) {
        if (g_logger.log_fp) fflush(g_logger.log_fp);
        g_logger.last_flush = now;
    }
    
    logger_unlock();
}

/* Public API implementations - all return STK_STATUS */
STK_STATUS stk_log_init(stk_log_config_t *config) {
    logger_lock_init();
    logger_lock();
    
    if (config) {
        g_logger.config = *config;
    }
    
    if (g_logger.config.log_file) {
        g_logger.log_fp = fopen(g_logger.config.log_file, "a");
        if (!g_logger.log_fp) {
            logger_unlock();
            return STK_EFILE;
        }
        setvbuf(g_logger.log_fp, NULL, _IOLBF, g_logger.config.buffer_size);
    }
    
    g_logger.initialized = 1;
    g_logger.last_flush = time(NULL);
    
    stk_log_write(STK_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, 
                  "Logger initialized (level=%d)", g_logger.config.log_level);
    
    logger_unlock();
    return STK_OK;
}

STK_STATUS stk_log_close(void) {
    logger_lock();
    if (g_logger.log_fp) {
        fclose(g_logger.log_fp);
        g_logger.log_fp = NULL;
    }
    g_logger.initialized = 0;
    logger_unlock();
    return STK_OK;
}

STK_STATUS stk_log_reload(stk_log_config_t *config) {
    if (!config) return STK_EINVAL;
    
    logger_lock();
    g_logger.config = *config;
    
    if (g_logger.log_fp) {
        fclose(g_logger.log_fp);
        g_logger.log_fp = NULL;
    }
    
    if (g_logger.config.log_file) {
        g_logger.log_fp = fopen(g_logger.config.log_file, "a");
        if (!g_logger.log_fp) {
            logger_unlock();
            return STK_EFILE;
        }
        setvbuf(g_logger.log_fp, NULL, _IOLBF, g_logger.config.buffer_size);
    }
    
    logger_unlock();
    return STK_OK;
}

STK_STATUS stk_log_set_level(stk_log_level_t level) {
    logger_lock();
    g_logger.config.log_level = level;
    logger_unlock();
    return STK_OK;
}

stk_log_level_t stk_log_get_level(void) {
    return g_logger.config.log_level;
}

STK_STATUS stk_log_flush(void) {
    logger_lock();
    if (g_logger.log_fp) {
        fflush(g_logger.log_fp);
    }
    fflush(stdout);
    logger_unlock();
    return STK_OK;
}

STK_STATUS stk_log_write(stk_log_level_t level, const char *filename,
                          int line, const char *func, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    log_write_impl(level, filename, line, func, format, ap);
    va_end(ap);
    return STK_OK;
}

int stk_log_is_enabled(stk_log_level_t level) {
    return g_logger.initialized && level >= g_logger.config.log_level;
}

#else /* STK_LOGGING_ENABLED not defined - stub implementations */

STK_STATUS stk_log_init(stk_log_config_t *config) { (void)config; return STK_OK; }
STK_STATUS stk_log_close(void) { return STK_OK; }
STK_STATUS stk_log_reload(stk_log_config_t *config) { (void)config; return STK_OK; }
STK_STATUS stk_log_set_level(stk_log_level_t level) { (void)level; return STK_OK; }
stk_log_level_t stk_log_get_level(void) { return STK_LOG_LEVEL_NONE; }
STK_STATUS stk_log_flush(void) { return STK_OK; }
STK_STATUS stk_log_write(stk_log_level_t level, const char *filename, int line, 
                          const char *func, const char *format, ...) { 
    (void)level; (void)filename; (void)line; (void)func; (void)format;
    return STK_OK;
}
int stk_log_is_enabled(stk_log_level_t level) { (void)level; return 0; }

#endif /* STK_LOGGING_ENABLED */