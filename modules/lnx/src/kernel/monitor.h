#ifndef LNX_KERNEL_MONITOR_H
#define LNX_KERNEL_MONITOR_H

#include <linux/ioctl.h>
#include <linux/types.h>

/* ============================================================================
 * 模块配置
 * ============================================================================
 */
#define MONITOR_NAME        "system_monitor"
#define MONITOR_VERSION     "1.0.0"
#define DEVICE_NAME         "monitor"
#define DEVICE_CLASS        "monitor_class"

/* 缓冲区配置 */
#define MAX_LOG_ENTRIES     2048        /* 最大日志条目 */
#define MAX_MSG_SIZE        256         /* 消息最大长度 */
#define MAX_PID_LIST        8192        /* PID列表最大数量 */
#define MAX_CPU_CORES       256         /* 最大CPU核心数 */

/* ============================================================================
 * IOCTL 命令定义
 * ============================================================================
 */
#define MONITOR_MAGIC       'm'

/* 系统信息命令 */
#define MONITOR_GET_CPU_INFO    _IOR(MONITOR_MAGIC, 1, struct cpu_info)
#define MONITOR_GET_MEM_INFO    _IOR(MONITOR_MAGIC, 2, struct mem_info)
#define MONITOR_GET_SYS_LOAD    _IOR(MONITOR_MAGIC, 3, struct sys_load)
#define MONITOR_GET_DISK_INFO   _IOR(MONITOR_MAGIC, 4, struct disk_info)

/* 进程管理命令 */
#define MONITOR_GET_PROC_LIST   _IOR(MONITOR_MAGIC, 10, struct proc_list)
#define MONITOR_GET_PROC_INFO   _IOWR(MONITOR_MAGIC, 11, struct proc_info)
#define MONITOR_KILL_PROC       _IOW(MONITOR_MAGIC, 12, int)
#define MONITOR_GET_PROC_STATS  _IOR(MONITOR_MAGIC, 13, struct proc_stats)

/* 模块控制命令 */
#define MONITOR_SET_LOG_LEVEL   _IOW(MONITOR_MAGIC, 20, int)
#define MONITOR_GET_LOG_LEVEL   _IOR(MONITOR_MAGIC, 21, int)
#define MONITOR_GET_LOG         _IOR(MONITOR_MAGIC, 22, struct log_entry)
#define MONITOR_CLEAR_LOG       _IO(MONITOR_MAGIC, 23)
#define MONITOR_GET_STATS       _IOR(MONITOR_MAGIC, 24, struct monitor_stats)

/* ============================================================================
 * 数据结构定义
 * ============================================================================
 */

/**
 * CPU 信息结构
 */
struct cpu_info {
    int cpu_count;                  /* CPU核心数 */
    unsigned long user;             /* 用户态时间 */
    unsigned long nice;             /* Nice时间 */
    unsigned long system;           /* 系统态时间 */
    unsigned long idle;             /* 空闲时间 */
    unsigned long iowait;           /* IO等待时间 */
    unsigned long irq;              /* 硬中断时间 */
    unsigned long softirq;          /* 软中断时间 */
    unsigned long steal;            /* 虚拟化偷取时间 */
    unsigned long guest;            /* Guest系统时间 */
    unsigned long total;            /* 总时间 */
    unsigned long usage_percent;    /* 总体使用率 (0-100) */
    unsigned long per_cpu_usage[MAX_CPU_CORES]; /* 每个CPU的使用率 */
};

/**
 * 内存信息结构
 */
struct mem_info {
    unsigned long total_kb;         /* 总内存 (KB) */
    unsigned long free_kb;          /* 空闲内存 (KB) */
    unsigned long available_kb;     /* 可用内存 (KB) */
    unsigned long buffers_kb;       /* 缓冲区内存 (KB) */
    unsigned long cached_kb;        /* 缓存内存 (KB) */
    unsigned long shmem_kb;         /* 共享内存 (KB) */
    unsigned long slab_kb;          /* Slab内存 (KB) */
    
    unsigned long swap_total_kb;    /* Swap总大小 (KB) */
    unsigned long swap_free_kb;     /* Swap空闲 (KB) */
    unsigned long swap_cached_kb;   /* Swap缓存 (KB) */
    
    int usage_percent;              /* 内存使用率百分比 */
    int swap_usage_percent;         /* Swap使用率百分比 */
};

/**
 * 系统负载结构
 */
struct sys_load {
    unsigned long load_avg_1;       /* 1分钟平均负载 (乘以100) */
    unsigned long load_avg_5;       /* 5分钟平均负载 (乘以100) */
    unsigned long load_avg_15;      /* 15分钟平均负载 (乘以100) */
    unsigned long running_tasks;    /* 正在运行的任务数 */
    unsigned long total_tasks;      /* 总任务数 */
    unsigned int latest_pid;        /* 最新分配的PID */
    unsigned long context_switches; /* 上下文切换次数 */
    unsigned long interrupts;       /* 中断次数 */
};

/**
 * 磁盘信息结构
 */
struct disk_info {
    char device_name[32];           /* 设备名称 */
    unsigned long total_kb;         /* 总容量 (KB) */
    unsigned long used_kb;          /* 已用容量 (KB) */
    unsigned long free_kb;          /* 空闲容量 (KB) */
    unsigned long read_kb;          /* 读取量 (KB) */
    unsigned long write_kb;         /* 写入量 (KB) */
    unsigned long read_ios;         /* 读取次数 */
    unsigned long write_ios;        /* 写入次数 */
    int usage_percent;              /* 使用率百分比 */
};

/**
 * 进程信息结构
 */
struct proc_info {
    int pid;                        /* 进程ID */
    int ppid;                       /* 父进程ID */
    int pgid;                       /* 进程组ID */
    int sid;                        /* 会话ID */
    int uid;                        /* 用户ID */
    int gid;                        /* 组ID */
    int euid;                       /* 有效用户ID */
    int egid;                       /* 有效组ID */
    
    char name[64];                  /* 进程名称 */
    char state;                     /* 进程状态: R=运行, S=睡眠, D=不可中断, Z=僵尸, T=停止 */
    char cmdline[256];              /* 命令行 */
    
    unsigned long vm_rss_kb;        /* 物理内存 (KB) */
    unsigned long vm_size_kb;       /* 虚拟内存 (KB) */
    unsigned long vm_swap_kb;       /* Swap内存 (KB) */
    
    unsigned long cpu_usage;        /* CPU使用率 (百分比 * 100) */
    unsigned long cpu_time_user;    /* 用户态CPU时间 */
    unsigned long cpu_time_sys;     /* 系统态CPU时间 */
    
    unsigned long uptime_sec;       /* 运行时间 (秒) */
    unsigned long long start_time;  /* 启动时间戳 */
    
    long nice;                      /* Nice值 */
    int priority;                   /* 动态优先级 */
    int rt_priority;                /* 实时优先级 */
    
    unsigned long voluntary_ctxt_switches;   /* 主动上下文切换 */
    unsigned long nonvoluntary_ctxt_switches; /* 被动上下文切换 */
    
    int threads;                    /* 线程数 */
    unsigned long open_fds;         /* 打开的文件描述符数 */
};

/**
 * 进程统计信息
 */
struct proc_stats {
    int total_processes;            /* 总进程数 */
    int running_processes;          /* 运行中的进程数 */
    int sleeping_processes;         /* 睡眠中的进程数 */
    int zombie_processes;           /* 僵尸进程数 */
    int stopped_processes;          /* 停止的进程数 */
    int threaded_processes;         /* 多线程进程数 */
};

/**
 * 进程列表结构
 */
struct proc_list {
    int count;                      /* 进程数量 */
    int pids[MAX_PID_LIST];         /* PID数组 */
};

/**
 * 日志条目结构
 */
struct log_entry {
    unsigned long timestamp;        /* 时间戳 (秒) */
    int level;                      /* 日志级别 */
    char message[MAX_MSG_SIZE];     /* 日志消息 */
};

/**
 * 模块统计信息
 */
struct monitor_stats {
    unsigned long start_time;       /* 模块启动时间 */
    unsigned long open_count;       /* 设备打开次数 */
    unsigned long read_count;       /* 读取次数 */
    unsigned long write_count;      /* 写入次数 */
    unsigned long ioctl_count;      /* IOCTL调用次数 */
    unsigned long error_count;      /* 错误次数 */
    unsigned long log_count;        /* 日志记录数 */
    int log_level;                  /* 当前日志级别 */
    bool is_active;                 /* 是否激活 */
};

/* ============================================================================
 * 日志级别定义
 * ============================================================================
 */
#define LOG_LEVEL_NONE      -1      /* 无日志 */
#define LOG_LEVEL_ERROR     0       /* 错误 */
#define LOG_LEVEL_WARN      1       /* 警告 */
#define LOG_LEVEL_INFO      2       /* 信息 */
#define LOG_LEVEL_DEBUG     3       /* 调试 */
#define LOG_LEVEL_TRACE     4       /* 追踪 */

/* ============================================================================
 * 设备数据结构
 * ============================================================================
 */
struct monitor_device {
    struct cdev      cdev;           /* 字符设备 */
    struct class    *class;          /* 设备类 */
    struct device   *device;         /* 设备对象 */
    
    struct mutex     lock;           /* 主互斥锁 */
    spinlock_t       log_lock;       /* 日志自旋锁 */
    
    /* 日志系统 */
    struct log_entry log_buffer[MAX_LOG_ENTRIES];
    int log_head;
    int log_tail;
    int log_count;
    int log_level;
    
    /* 统计信息 */
    atomic_t         open_count;
    atomic_t         read_count;
    atomic_t         write_count;
    atomic_t         ioctl_count;
    atomic_t         error_count;
    
    /* CPU时间追踪 (用于计算使用率) */
    struct cpu_info  prev_cpu_info;
    struct cpu_info  curr_cpu_info;
    unsigned long    last_update_time;
    
    /* 运行状态 */
    unsigned long    start_time;
    bool             is_running;
    
    /* 设备信息 */
    dev_t            device_number;
    int              major_number;
    int              minor_number;
};

/* ============================================================================
 * 日志宏定义
 * ============================================================================
 */
#define monitor_error(dev, fmt, ...) \
    monitor_log(dev, LOG_LEVEL_ERROR, "[ERROR] " fmt, ##__VA_ARGS__)

#define monitor_warn(dev, fmt, ...) \
    monitor_log(dev, LOG_LEVEL_WARN, "[WARN] " fmt, ##__VA_ARGS__)

#define monitor_info(dev, fmt, ...) \
    monitor_log(dev, LOG_LEVEL_INFO, "[INFO] " fmt, ##__VA_ARGS__)

#define monitor_debug(dev, fmt, ...) \
    monitor_log(dev, LOG_LEVEL_DEBUG, "[DEBUG] " fmt, ##__VA_ARGS__)

#define monitor_trace(dev, fmt, ...) \
    monitor_log(dev, LOG_LEVEL_TRACE, "[TRACE] " fmt, ##__VA_ARGS__)

/* ============================================================================
 * 函数声明
 * ============================================================================
 */

/* 初始化和清理 */
int  monitor_device_init(struct monitor_device *dev);
void monitor_device_exit(struct monitor_device *dev);

/* 日志函数 */
void monitor_log(struct monitor_device *dev, int level, const char *fmt, ...);
int  monitor_get_log(struct monitor_device *dev, struct log_entry *entry);
void monitor_clear_log(struct monitor_device *dev);

/* 系统信息获取 */
int  monitor_get_cpu_info(struct cpu_info *info);
int  monitor_get_memory_info(struct mem_info *info);
int  monitor_get_system_load(struct sys_load *load);
int  monitor_get_disk_info(struct disk_info *info, const char *device);

/* 进程管理 */
int  monitor_get_process_list(struct proc_list *list);
int  monitor_get_process_info(int pid, struct proc_info *info);
int  monitor_get_process_stats(struct proc_stats *stats);
int  monitor_kill_process(int pid);

/* 统计信息 */
void monitor_update_stats(struct monitor_device *dev);
int  monitor_get_stats(struct monitor_device *dev, struct monitor_stats *stats);

#endif /* KERNEL_MONITOR_H */