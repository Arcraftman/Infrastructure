#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/stat.h>
static const char* level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

/* Log level colors */
static const char* level_colors[] = {
    COLOR_TRACE, COLOR_DEBUG, COLOR_INFO, COLOR_WARN, COLOR_ERROR, COLOR_FATAL
};

/* Log file information */
typedef struct {
    FILE* fp;
    char* file_path;
    int current_size;
} log_file_t;

/* Global log state */
typedef struct {
    log_config_t config;
    log_file_t log_file;
    pthread_mutex_t mutex;
    volatile int initialized;
    volatile int enabled;
    time_t last_flush_time;
} log_state_t;

/* Thread-local buffer */
static __thread char g_thread_buffer[THREAD_BUFFER_SIZE];

/* Global state */
static log_state_t g_state = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .initialized = 0,
    .enabled = 1,
    .last_flush_time = 0,
    .log_file = {NULL, NULL, 0}
};

/* Get timestamp string */
static void get_timestamp(char* buffer, size_t size, int with_ms) {
    struct timeval tv;
    struct tm tm;
    
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    
    if (with_ms) {
        snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec,
                 (int)(tv.tv_usec / 1000));
    } else {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm);
    }
}

/* Extract filename from path */
static const char* get_filename(const char* path) {
    const char* filename = strrchr(path, '/');
#ifdef _MSC_VER
    if (!filename) filename = strrchr(path, '\\');
#endif
    return filename ? filename + 1 : path;
}

/* Check and perform log rotation */
static void check_log_rotation(void) {
    if (!g_state.config.max_file_size || !g_state.config.max_file_count) {
        return;
    }
    
    int max_size_bytes = g_state.config.max_file_size * 1024 * 1024;
    if (g_state.log_file.current_size < max_size_bytes) {
        return;
    }
    
    /* Close current file */
    if (g_state.log_file.fp) {
        fclose(g_state.log_file.fp);
        g_state.log_file.fp = NULL;
    }
    
    /* Rotate old files */
    char old_path[512];
    char new_path[512];
    
    for (int i = g_state.config.max_file_count - 1; i > 0; i--) {
        snprintf(old_path, sizeof(old_path), "%s.%d", g_state.log_file.file_path, i);
        snprintf(new_path, sizeof(new_path), "%s.%d", g_state.log_file.file_path, i - 1);
        
        if (i == g_state.config.max_file_count - 1) {
            remove(old_path);
        } else if (access(old_path, F_OK) == 0) {
            rename(old_path, new_path);
        }
    }
    
    /* Rename current file to .1 */
    snprintf(new_path, sizeof(new_path), "%s.1", g_state.log_file.file_path);
    rename(g_state.log_file.file_path, new_path);
    
    /* Reopen log file */
    g_state.log_file.fp = fopen(g_state.log_file.file_path, "a");
    if (g_state.log_file.fp) {
        setvbuf(g_state.log_file.fp, NULL, _IOLBF, 0);
        g_state.log_file.current_size = 0;
    }
}

/* Write log to file */
static void write_to_file(const char* buffer, size_t len) {
    if (!g_state.log_file.fp) return;
    
    fwrite(buffer, 1, len, g_state.log_file.fp);
    g_state.log_file.current_size += len;
    
    /* Check if rotation needed */
    if (g_state.config.max_file_size > 0 && 
        g_state.config.max_file_count > 0 &&
        g_state.log_file.current_size >= g_state.config.max_file_size * 1024 * 1024) {
        check_log_rotation();
    }
}

/* Write log to console */
static void write_to_console(const char* buffer, size_t len) {
    fwrite(buffer, 1, len, stdout);
}

/* Flush buffer */
static void flush_buffer(void) {
    if (g_state.log_file.fp) {
        fflush(g_state.log_file.fp);
    }
    if (g_state.config.output_option & LOG_OUTPUT_TO_CONSOLE) {
        fflush(stdout);
    }
    g_state.last_flush_time = time(NULL);
}

/* Core log writer */
static void write_log(const char* buffer, size_t len) {
    if (!buffer || len == 0 || !g_state.initialized) return;
    
    pthread_mutex_lock(&g_state.mutex);
    
    if (g_state.config.output_option & LOG_OUTPUT_TO_FILE) {
        write_to_file(buffer, len);
    }
    
    if (g_state.config.output_option & LOG_OUTPUT_TO_CONSOLE) {
        write_to_console(buffer, len);
    }
    
    /* Periodic flush */
    if (g_state.config.flush_interval > 0) {
        time_t now = time(NULL);
        if (now - g_state.last_flush_time >= g_state.config.flush_interval) {
            flush_buffer();
        }
    } else if (g_state.config.flush_interval == 0) {
        flush_buffer();
    }
    
    pthread_mutex_unlock(&g_state.mutex);
}

/* Internal logger (avoid macros before init) */
static void internal_log(log_level_t level, const char* format, ...) {
    if (!g_state.initialized || !g_state.enabled || level < g_state.config.log_level) {
        return;
    }
    
    char* buffer = g_thread_buffer;
    char* ptr = buffer;
    int remaining = THREAD_BUFFER_SIZE - 1;
    int written = 0;
    
    /* Timestamp */
    if (g_state.config.enable_timestamp) {
        char timestamp[TIME_STRING_SIZE];
        get_timestamp(timestamp, sizeof(timestamp), g_state.config.enable_ms);
        written = snprintf(ptr, remaining, "[%s] ", timestamp);
        ptr += written;
        remaining -= written;
    }
    
    /* Log level */
    written = snprintf(ptr, remaining, "[%-5s] ", level_names[level]);
    ptr += written;
    remaining -= written;
    
    /* User message */
    va_list args;
    va_start(args, format);
    written = vsnprintf(ptr, remaining, format, args);
    va_end(args);
    ptr += (written < remaining) ? written : remaining;
    
    /* Newline */
    if (remaining > 0) {
        *ptr++ = '\n';
        *ptr = '\0';
    }
    
    /* Write log */
    write_log(buffer, strlen(buffer));
}

/* Public log write function */
void log_write(log_level_t level, const char* filename, 
                   int line, const char* func, const char* format, ...) {
    if (!g_state.initialized || !g_state.enabled || level < g_state.config.log_level) {
        return;
    }
    
    char* buffer = g_thread_buffer;
    char* ptr = buffer;
    int remaining = THREAD_BUFFER_SIZE - 1;
    int written = 0;
    
    /* Color start */
    int use_color = g_state.config.enable_color && 
                    (g_state.config.output_option & LOG_OUTPUT_TO_CONSOLE);
    if (use_color) {
        written = snprintf(ptr, remaining, "%s", level_colors[level]);
        ptr += written;
        remaining -= written;
    }
    
    /* Timestamp */
    if (g_state.config.enable_timestamp) {
        char timestamp[TIME_STRING_SIZE];
        get_timestamp(timestamp, sizeof(timestamp), g_state.config.enable_ms);
        written = snprintf(ptr, remaining, "[%s] ", timestamp);
        ptr += written;
        remaining -= written;
    }
    
    /* Thread ID */
    if (g_state.config.enable_thread_id) {
        written = snprintf(ptr, remaining, "[TID:%ld] ", gettid());
        ptr += written;
        remaining -= written;
    }
    
    /* Log level */
    written = snprintf(ptr, remaining, "[%-5s] ", level_names[level]);
    ptr += written;
    remaining -= written;
    
    /* Filename and line number */
    if (g_state.config.enable_fileline && filename) {
        written = snprintf(ptr, remaining, "[%s:%d] ", 
                          get_filename(filename), line);
        ptr += written;
        remaining -= written;
    }
    
    /* Function name */
    if (g_state.config.enable_function && func && level >= LOG_LEVEL_WARN) {
        written = snprintf(ptr, remaining, "[%s] ", func);
        ptr += written;
        remaining -= written;
    }

    /* User message */
    va_list args;
    va_start(args, format);
    written = vsnprintf(ptr, remaining, format, args);
    va_end(args);
    ptr += (written < remaining) ? written : remaining;

    /* Newline */
    if (remaining > 0) {
        *ptr++ = '\n';
        *ptr = '\0';
    }
    
    /* Color reset */
    if (use_color) {
        char* end = buffer + strlen(buffer);
        if (end < buffer + THREAD_BUFFER_SIZE - sizeof(COLOR_RESET)) {
            snprintf(end, remaining, "%s", COLOR_RESET);
        }
    }
    
    /* Write log */
    write_log(buffer, strlen(buffer));
    
    /* Fatal: flush immediately */
    if (level == LOG_LEVEL_FATAL) {
        flush_buffer();
    }
}

/* Open log file */
static int open_log_file(void) {
    if (!g_state.config.log_file || !g_state.config.log_file[0]) {
        return -1;
    }
    
    if (g_state.log_file.file_path) {
        free(g_state.log_file.file_path);
    }
    
    g_state.log_file.file_path = strdup(g_state.config.log_file);
    if (!g_state.log_file.file_path) {
        return -1;
    }
    
    g_state.log_file.fp = fopen(g_state.config.log_file, "a");
    if (!g_state.log_file.fp) {
        free(g_state.log_file.file_path);
        g_state.log_file.file_path = NULL;
        return -1;
    }
    
    /* Set line buffering mode */
    setvbuf(g_state.log_file.fp, NULL, _IOLBF, 0);
    
    /* Get current file size */
    fseek(g_state.log_file.fp, 0, SEEK_END);
    g_state.log_file.current_size = ftell(g_state.log_file.fp);
    
    return 0;
}

/* Close log file */
static void close_log_file(void) {
    if (g_state.log_file.fp) {
        fflush(g_state.log_file.fp);
        fclose(g_state.log_file.fp);
        g_state.log_file.fp = NULL;
    }
    
    if (g_state.log_file.file_path) {
        free(g_state.log_file.file_path);
        g_state.log_file.file_path = NULL;
    }
}

/* Initialize logger */
int log_init(log_config_t* config) {
    if (!config) return -1;
    
    pthread_mutex_lock(&g_state.mutex);
    
    /* Copy configuration */
    memcpy(&g_state.config, config, sizeof(log_config_t));
    
    /* Set defaults */
    if (g_state.config.buffer_size <= 0) {
        g_state.config.buffer_size = DEFAULT_BUFFER_SIZE;
    }
    if (g_state.config.flush_interval < 0) {
        g_state.config.flush_interval = DEFAULT_FLUSH_INTERVAL;
    }
    if (g_state.config.output_option == 0) {
        g_state.config.output_option = LOG_OUTPUT_TO_CONSOLE;
    }
    
    /* Initialize log file */
    if ((g_state.config.output_option & LOG_OUTPUT_TO_FILE) && 
        g_state.config.log_file) {
        if (open_log_file() != 0) {
            pthread_mutex_unlock(&g_state.mutex);
            return -1;
        }
    }
    
    g_state.last_flush_time = time(NULL);
    g_state.initialized = 1;
    g_state.enabled = 1;
    
    pthread_mutex_unlock(&g_state.mutex);
    
    /* Use internal function to avoid macro expansion issues */
    internal_log(LOG_LEVEL_INFO, "Logger initialized successfully");
    
    return 0;
}

/* Shutdown logger */
void log_close(void) {
    if (!g_state.initialized) return;
    
    internal_log(LOG_LEVEL_INFO, "Logger shutting down");
    
    pthread_mutex_lock(&g_state.mutex);
    
    flush_buffer();
    close_log_file();
    g_state.initialized = 0;
    g_state.enabled = 0;
    
    pthread_mutex_unlock(&g_state.mutex);
}

/* Reload configuration */
int log_reload(log_config_t* config) {
    if (!config || !g_state.initialized) return -1;
    
    pthread_mutex_lock(&g_state.mutex);
    
    /* Update configuration */
    log_level_t old_level = g_state.config.log_level;
    memcpy(&g_state.config, config, sizeof(log_config_t));
    
    /* Preserve old level if not specified */
    if (config->log_level == 0 && old_level != 0) {
        g_state.config.log_level = old_level;
    }
    
    /* Reopen log file */
    if (g_state.config.output_option & LOG_OUTPUT_TO_FILE) {
        close_log_file();
        open_log_file();
    }
    
    pthread_mutex_unlock(&g_state.mutex);
    
    internal_log(LOG_LEVEL_INFO, "Logger reloaded");
    
    return 0;
}

/* Set log level */
void log_set_level(log_level_t level) {
    g_state.config.log_level = level;
}

/* Get current log level */
log_level_t log_get_level(void) {
    return g_state.config.log_level;
}

/* Flush buffer */
void log_flush(void) {
    flush_buffer();
}

/* Check if level is enabled */
int log_is_enabled(log_level_t level) {
    return g_state.initialized && g_state.enabled && level >= g_state.config.log_level;
}