#include "logger.h"


/* 日志级别名称 */
static const char* level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

/* 日志级别颜色 */
static const char* level_colors[] = {
    COLOR_TRACE, COLOR_DEBUG, COLOR_INFO, COLOR_WARN, COLOR_ERROR, COLOR_FATAL
};

/* 日志文件信息 */
typedef struct {
    FILE* fp;
    char* file_path;
    int current_size;
} log_file_t;

/* 全局日志状态 */
typedef struct {
    log_config_t config;
    log_file_t log_file;
    pthread_mutex_t mutex;
    volatile int initialized;
    volatile int enabled;
    time_t last_flush_time;
} log_state_t;

/* 线程本地缓冲区 */
static __thread char g_thread_buffer[THREAD_BUFFER_SIZE];

/* 全局状态 */
static log_state_t g_state = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .initialized = 0,
    .enabled = 1,
    .last_flush_time = 0,
    .log_file = {NULL, NULL, 0}
};

/* 获取当前时间字符串 */
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

/* 获取文件名（去掉路径） */
static const char* get_filename(const char* path) {
    const char* filename = strrchr(path, '/');
#ifdef _MSC_VER
    if (!filename) filename = strrchr(path, '\\');
#endif
    return filename ? filename + 1 : path;
}

/* 检查并执行日志轮转 */
static void check_log_rotation(void) {
    if (!g_state.config.max_file_size || !g_state.config.max_file_count) {
        return;
    }
    
    int max_size_bytes = g_state.config.max_file_size * 1024 * 1024;
    if (g_state.log_file.current_size < max_size_bytes) {
        return;
    }
    
    /* 关闭当前文件 */
    if (g_state.log_file.fp) {
        fclose(g_state.log_file.fp);
        g_state.log_file.fp = NULL;
    }
    
    /* 轮转旧文件 */
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
    
    /* 重命名当前文件为 .1 */
    snprintf(new_path, sizeof(new_path), "%s.1", g_state.log_file.file_path);
    rename(g_state.log_file.file_path, new_path);
    
    /* 重新打开日志文件 */
    g_state.log_file.fp = fopen(g_state.log_file.file_path, "a");
    if (g_state.log_file.fp) {
        setvbuf(g_state.log_file.fp, NULL, _IOLBF, 0);
        g_state.log_file.current_size = 0;
    }
}

/* 写入日志到文件 */
static void write_to_file(const char* buffer, size_t len) {
    if (!g_state.log_file.fp) return;
    
    fwrite(buffer, 1, len, g_state.log_file.fp);
    g_state.log_file.current_size += len;
    
    /* 检查是否需要轮转 */
    if (g_state.config.max_file_size > 0 && 
        g_state.config.max_file_count > 0 &&
        g_state.log_file.current_size >= g_state.config.max_file_size * 1024 * 1024) {
        check_log_rotation();
    }
}

/* 写入日志到控制台 */
static void write_to_console(const char* buffer, size_t len) {
    fwrite(buffer, 1, len, stdout);
}

/* 刷新缓冲区 */
static void flush_buffer(void) {
    if (g_state.log_file.fp) {
        fflush(g_state.log_file.fp);
    }
    if (g_state.config.output_option & LOG_OUTPUT_TO_CONSOLE) {
        fflush(stdout);
    }
    g_state.last_flush_time = time(NULL);
}

/* 核心日志写入 */
static void write_log(const char* buffer, size_t len) {
    if (!buffer || len == 0 || !g_state.initialized) return;
    
    pthread_mutex_lock(&g_state.mutex);
    
    if (g_state.config.output_option & LOG_OUTPUT_TO_FILE) {
        write_to_file(buffer, len);
    }
    
    if (g_state.config.output_option & LOG_OUTPUT_TO_CONSOLE) {
        write_to_console(buffer, len);
    }
    
    /* 定期刷新 */
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

/* 内部日志函数（避免在初始化时使用宏） */
static void internal_log(log_level_t level, const char* format, ...) {
    if (!g_state.initialized || !g_state.enabled || level < g_state.config.log_level) {
        return;
    }
    
    char* buffer = g_thread_buffer;
    char* ptr = buffer;
    int remaining = THREAD_BUFFER_SIZE - 1;
    int written = 0;
    
    /* 时间戳 */
    if (g_state.config.enable_timestamp) {
        char timestamp[TIME_STRING_SIZE];
        get_timestamp(timestamp, sizeof(timestamp), g_state.config.enable_ms);
        written = snprintf(ptr, remaining, "[%s] ", timestamp);
        ptr += written;
        remaining -= written;
    }
    
    /* 日志级别 */
    written = snprintf(ptr, remaining, "[%-5s] ", level_names[level]);
    ptr += written;
    remaining -= written;
    
    /* 用户消息 */
    va_list args;
    va_start(args, format);
    written = vsnprintf(ptr, remaining, format, args);
    va_end(args);
    ptr += (written < remaining) ? written : remaining;
    
    /* 换行 */
    if (remaining > 0) {
        *ptr++ = '\n';
        *ptr = '\0';
    }
    
    /* 写入日志 */
    write_log(buffer, strlen(buffer));
}

/* 公共日志写入函数 */
void log_write(log_level_t level, const char* filename, 
                   int line, const char* func, const char* format, ...) {
    if (!g_state.initialized || !g_state.enabled || level < g_state.config.log_level) {
        return;
    }
    
    char* buffer = g_thread_buffer;
    char* ptr = buffer;
    int remaining = THREAD_BUFFER_SIZE - 1;
    int written = 0;
    
    /* 颜色开始 */
    int use_color = g_state.config.enable_color && 
                    (g_state.config.output_option & LOG_OUTPUT_TO_CONSOLE);
    if (use_color) {
        written = snprintf(ptr, remaining, "%s", level_colors[level]);
        ptr += written;
        remaining -= written;
    }
    
    /* 时间戳 */
    if (g_state.config.enable_timestamp) {
        char timestamp[TIME_STRING_SIZE];
        get_timestamp(timestamp, sizeof(timestamp), g_state.config.enable_ms);
        written = snprintf(ptr, remaining, "[%s] ", timestamp);
        ptr += written;
        remaining -= written;
    }
    
    /* 线程ID */
    if (g_state.config.enable_thread_id) {
        written = snprintf(ptr, remaining, "[TID:%ld] ", gettid());
        ptr += written;
        remaining -= written;
    }
    
    /* 日志级别 */
    written = snprintf(ptr, remaining, "[%-5s] ", level_names[level]);
    ptr += written;
    remaining -= written;
    
    /* 文件名和行号 */
    if (g_state.config.enable_fileline && filename) {
        written = snprintf(ptr, remaining, "[%s:%d] ", 
                          get_filename(filename), line);
        ptr += written;
        remaining -= written;
    }
    
    /* 函数名 */
    if (g_state.config.enable_function && func && level >= LOG_LEVEL_WARN) {
        written = snprintf(ptr, remaining, "[%s] ", func);
        ptr += written;
        remaining -= written;
    }
    
    /* 用户消息 */
    va_list args;
    va_start(args, format);
    written = vsnprintf(ptr, remaining, format, args);
    va_end(args);
    ptr += (written < remaining) ? written : remaining;
    
    /* 换行 */
    if (remaining > 0) {
        *ptr++ = '\n';
        *ptr = '\0';
    }
    
    /* 颜色结束 */
    if (use_color) {
        char* end = buffer + strlen(buffer);
        if (end < buffer + THREAD_BUFFER_SIZE - sizeof(COLOR_RESET)) {
            snprintf(end, remaining, "%s", COLOR_RESET);
        }
    }
    
    /* 写入日志 */
    write_log(buffer, strlen(buffer));
    
    /* 致命错误立即刷新 */
    if (level == LOG_LEVEL_FATAL) {
        flush_buffer();
    }
}

/* 打开日志文件 */
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
    
    /* 设置行缓冲模式 */
    setvbuf(g_state.log_file.fp, NULL, _IOLBF, 0);
    
    /* 获取当前文件大小 */
    fseek(g_state.log_file.fp, 0, SEEK_END);
    g_state.log_file.current_size = ftell(g_state.log_file.fp);
    
    return 0;
}

/* 关闭日志文件 */
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

/* 初始化日志系统 */
int log_init(log_config_t* config) {
    if (!config) return -1;
    
    pthread_mutex_lock(&g_state.mutex);
    
    /* 复制配置 */
    memcpy(&g_state.config, config, sizeof(log_config_t));
    
    /* 设置默认值 */
    if (g_state.config.buffer_size <= 0) {
        g_state.config.buffer_size = DEFAULT_BUFFER_SIZE;
    }
    if (g_state.config.flush_interval < 0) {
        g_state.config.flush_interval = DEFAULT_FLUSH_INTERVAL;
    }
    if (g_state.config.output_option == 0) {
        g_state.config.output_option = LOG_OUTPUT_TO_CONSOLE;
    }
    
    /* 初始化日志文件 */
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
    
    /* 使用内部函数记录初始化成功（避免宏展开问题） */
    internal_log(LOG_LEVEL_INFO, "Logger initialized successfully");
    
    return 0;
}

/* 关闭日志系统 */
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

/* 重新加载配置 */
int log_reload(log_config_t* config) {
    if (!config || !g_state.initialized) return -1;
    
    pthread_mutex_lock(&g_state.mutex);
    
    /* 更新配置 */
    log_level_t old_level = g_state.config.log_level;
    memcpy(&g_state.config, config, sizeof(log_config_t));
    
    /* 保持原来的日志级别如果没有指定 */
    if (config->log_level == 0 && old_level != 0) {
        g_state.config.log_level = old_level;
    }
    
    /* 重新打开日志文件 */
    if (g_state.config.output_option & LOG_OUTPUT_TO_FILE) {
        close_log_file();
        open_log_file();
    }
    
    pthread_mutex_unlock(&g_state.mutex);
    
    internal_log(LOG_LEVEL_INFO, "Logger reloaded");
    
    return 0;
}

/* 设置日志级别 */
void log_set_level(log_level_t level) {
    g_state.config.log_level = level;
}

/* 获取当前日志级别 */
log_level_t log_get_level(void) {
    return g_state.config.log_level;
}

/* 刷新缓冲区 */
void log_flush(void) {
    flush_buffer();
}

/* 检查级别是否启用 */
int log_is_enabled(log_level_t level) {
    return g_state.initialized && g_state.enabled && level >= g_state.config.log_level;
}