#include "stk/utils/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #define stk_gettid() GetCurrentThreadId()
    #define stk_localtime_r(t, tm) localtime_s(tm, t)
#else
    #include <unistd.h>
    #include <pthread.h>
    #include <sys/syscall.h>
    #define stk_gettid() syscall(SYS_gettid)
    #define stk_localtime_r(t, tm) localtime_r(t, tm)
#endif

static const char *level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
    STK_COLOR_TRACE, STK_COLOR_DEBUG, STK_COLOR_INFO,
    STK_COLOR_WARN, STK_COLOR_ERROR, STK_COLOR_FATAL
};

typedef struct {
    FILE *fp;
    char *file_path;
    int current_size;
} stk_log_file_t;

typedef struct {
    stk_log_config_t config;
    stk_log_file_t log_file;
    pthread_mutex_t mutex;
    volatile int initialized;
    volatile int enabled;
    time_t last_flush_time;
} stk_log_state_t;

static __thread char g_thread_buffer[STK_THREAD_BUFFER_SIZE];
static stk_log_state_t g_state = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .initialized = 0,
    .enabled = 1,
    .last_flush_time = 0,
    .log_file = {NULL, NULL, 0}
};

static void get_timestamp(char *buffer, size_t size, int with_ms) {
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    stk_localtime_r(&tv.tv_sec, &tm);

    if (with_ms) {
        snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec,
                 (int)(tv.tv_usec / 1000));
    } else {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm);
    }
}

static const char *get_filename(const char *path) {
    const char *filename = strrchr(path, '/');
#ifdef _WIN32
    if (!filename) filename = strrchr(path, '\\');
#endif
    return filename ? filename + 1 : path;
}

static void check_log_rotation(void) {
    if (!g_state.config.max_file_size || !g_state.config.max_file_count) {
        return;
    }

    int max_size_bytes = g_state.config.max_file_size * 1024 * 1024;
    if (g_state.log_file.current_size < max_size_bytes) {
        return;
    }

    if (g_state.log_file.fp) {
        fclose(g_state.log_file.fp);
        g_state.log_file.fp = NULL;
    }

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

    snprintf(new_path, sizeof(new_path), "%s.1", g_state.log_file.file_path);
    rename(g_state.log_file.file_path, new_path);

    g_state.log_file.fp = fopen(g_state.log_file.file_path, "a");
    if (g_state.log_file.fp) {
        setvbuf(g_state.log_file.fp, NULL, _IOLBF, 0);
        g_state.log_file.current_size = 0;
    }
}

static void write_to_file(const char *buffer, size_t len) {
    if (!g_state.log_file.fp) return;

    fwrite(buffer, 1, len, g_state.log_file.fp);
    g_state.log_file.current_size += len;

    if (g_state.config.max_file_size > 0 &&
        g_state.config.max_file_count > 0 &&
        g_state.log_file.current_size >= g_state.config.max_file_size * 1024 * 1024) {
        check_log_rotation();
    }
}

static void write_to_console(const char *buffer, size_t len) {
    fwrite(buffer, 1, len, stdout);
}

static void flush_buffer(void) {
    if (g_state.log_file.fp) {
        fflush(g_state.log_file.fp);
    }
    if (g_state.config.output_option & STK_LOG_OUTPUT_CONSOLE) {
        fflush(stdout);
    }
    g_state.last_flush_time = time(NULL);
}

static void write_log(const char *buffer, size_t len) {
    if (!buffer || len == 0 || !g_state.initialized) return;

    pthread_mutex_lock(&g_state.mutex);

    if (g_state.config.output_option & STK_LOG_OUTPUT_FILE) {
        write_to_file(buffer, len);
    }

    if (g_state.config.output_option & STK_LOG_OUTPUT_CONSOLE) {
        write_to_console(buffer, len);
    }

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

static void internal_log(stk_log_level_t level, const char *format, ...) {
    if (!g_state.initialized || !g_state.enabled || level < g_state.config.log_level) {
        return;
    }

    char *buffer = g_thread_buffer;
    char *ptr = buffer;
    int remaining = STK_THREAD_BUFFER_SIZE - 1;
    int written = 0;

    if (g_state.config.enable_timestamp) {
        char timestamp[STK_TIME_STRING_SIZE];
        get_timestamp(timestamp, sizeof(timestamp), g_state.config.enable_ms);
        written = snprintf(ptr, remaining, "[%s] ", timestamp);
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, "[%-5s] ", level_names[level]);
    ptr += written;
    remaining -= written;

    va_list args;
    va_start(args, format);
    written = vsnprintf(ptr, remaining, format, args);
    va_end(args);
    ptr += (written < remaining) ? written : remaining;

    if (remaining > 0) {
        *ptr++ = '\n';
        *ptr = '\0';
    }

    write_log(buffer, strlen(buffer));
}

void stk_log_write(stk_log_level_t level, const char *filename,
                   int line, const char *func, const char *format, ...) {
    if (!g_state.initialized || !g_state.enabled || level < g_state.config.log_level) {
        return;
    }

    char *buffer = g_thread_buffer;
    char *ptr = buffer;
    int remaining = STK_THREAD_BUFFER_SIZE - 1;
    int written = 0;

    int use_color = g_state.config.enable_color &&
                    (g_state.config.output_option & STK_LOG_OUTPUT_CONSOLE);
    if (use_color) {
        written = snprintf(ptr, remaining, "%s", level_colors[level]);
        ptr += written;
        remaining -= written;
    }

    if (g_state.config.enable_timestamp) {
        char timestamp[STK_TIME_STRING_SIZE];
        get_timestamp(timestamp, sizeof(timestamp), g_state.config.enable_ms);
        written = snprintf(ptr, remaining, "[%s] ", timestamp);
        ptr += written;
        remaining -= written;
    }

    if (g_state.config.enable_thread_id) {
        written = snprintf(ptr, remaining, "[TID:%ld] ", (long)stk_gettid());
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, "[%-5s] ", level_names[level]);
    ptr += written;
    remaining -= written;

    if (g_state.config.enable_fileline && filename) {
        written = snprintf(ptr, remaining, "[%s:%d] ", get_filename(filename), line);
        ptr += written;
        remaining -= written;
    }

    if (g_state.config.enable_function && func && level >= STK_LOG_LEVEL_WARN) {
        written = snprintf(ptr, remaining, "[%s] ", func);
        ptr += written;
        remaining -= written;
    }

    va_list args;
    va_start(args, format);
    written = vsnprintf(ptr, remaining, format, args);
    va_end(args);
    ptr += (written < remaining) ? written : remaining;

    if (remaining > 0) {
        *ptr++ = '\n';
        *ptr = '\0';
    }

    if (use_color) {
        char *end = buffer + strlen(buffer);
        if (end < buffer + STK_THREAD_BUFFER_SIZE - sizeof(STK_COLOR_RESET)) {
            snprintf(end, remaining, "%s", STK_COLOR_RESET);
        }
    }

    write_log(buffer, strlen(buffer));

    if (level == STK_LOG_LEVEL_FATAL) {
        flush_buffer();
    }
}

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

    setvbuf(g_state.log_file.fp, NULL, _IOLBF, 0);

    fseek(g_state.log_file.fp, 0, SEEK_END);
    g_state.log_file.current_size = ftell(g_state.log_file.fp);

    return 0;
}

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

int stk_log_init(stk_log_config_t *config) {
    if (!config) return -1;

    pthread_mutex_lock(&g_state.mutex);

    memcpy(&g_state.config, config, sizeof(stk_log_config_t));

    if (g_state.config.buffer_size <= 0) {
        g_state.config.buffer_size = STK_DEFAULT_BUFFER_SIZE;
    }
    if (g_state.config.flush_interval < 0) {
        g_state.config.flush_interval = STK_DEFAULT_FLUSH_INTERVAL;
    }
    if (g_state.config.output_option == 0) {
        g_state.config.output_option = STK_LOG_OUTPUT_CONSOLE;
    }

    if ((g_state.config.output_option & STK_LOG_OUTPUT_FILE) &&
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

    internal_log(STK_LOG_LEVEL_INFO, "Logger initialized successfully");

    return 0;
}

void stk_log_close(void) {
    if (!g_state.initialized) return;

    internal_log(STK_LOG_LEVEL_INFO, "Logger shutting down");

    pthread_mutex_lock(&g_state.mutex);

    flush_buffer();
    close_log_file();
    g_state.initialized = 0;
    g_state.enabled = 0;

    pthread_mutex_unlock(&g_state.mutex);
}

int stk_log_reload(stk_log_config_t *config) {
    if (!config || !g_state.initialized) return -1;

    pthread_mutex_lock(&g_state.mutex);

    stk_log_level_t old_level = g_state.config.log_level;
    memcpy(&g_state.config, config, sizeof(stk_log_config_t));

    if (config->log_level == 0 && old_level != 0) {
        g_state.config.log_level = old_level;
    }

    if (g_state.config.output_option & STK_LOG_OUTPUT_FILE) {
        close_log_file();
        open_log_file();
    }

    pthread_mutex_unlock(&g_state.mutex);

    internal_log(STK_LOG_LEVEL_INFO, "Logger reloaded");

    return 0;
}

void stk_log_set_level(stk_log_level_t level) {
    g_state.config.log_level = level;
}

stk_log_level_t stk_log_get_level(void) {
    return g_state.config.log_level;
}

void stk_log_flush(void) {
    flush_buffer();
}

int stk_log_is_enabled(stk_log_level_t level) {
    return g_state.initialized && g_state.enabled && level >= g_state.config.log_level;
}