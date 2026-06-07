#ifndef LNX_SYSMON_H
#define LNX_SYSMON_H

/**
 * @file monitor.h
 * @brief Userspace system monitoring interface
 *
 * Provides system-wide CPU, memory, process, and load monitoring
 * by reading /proc and /sys filesystem entries.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Configuration constants
 * ============================================================================
 */
#define LNX_SYSMON_MAX_CPUS        256
#define LNX_SYSMON_MAX_PIDS        8192
#define LNX_SYSMON_PROC_NAME_LEN   64
#define LNX_SYSMON_CMDLINE_LEN     256
#define LNX_SYSMON_DEVICE_NAME_LEN 32

/* ============================================================================
 * Data structures
 * ============================================================================
 */

/** Per-CPU time snapshot (from /proc/stat). */
typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
    unsigned long steal;
    unsigned long guest;
    unsigned long total;
} lnx_cpu_time_t;

/** CPU information. */
typedef struct {
    int                cpu_count;
    unsigned long      usage_percent; /** 0-100 */
    lnx_cpu_time_t     total;
    lnx_cpu_time_t     per_cpu[LNX_SYSMON_MAX_CPUS];
} lnx_cpu_info_t;

/** Memory information (from /proc/meminfo). */
typedef struct {
    unsigned long total_kb;
    unsigned long free_kb;
    unsigned long available_kb;
    unsigned long buffers_kb;
    unsigned long cached_kb;
    unsigned long shmem_kb;
    unsigned long slab_kb;
    unsigned long swap_total_kb;
    unsigned long swap_free_kb;
    int usage_percent;      /** 0-100 */
    int swap_usage_percent; /** 0-100 */
} lnx_mem_info_t;

/** System load averages. */
typedef struct {
    double      load_avg_1;
    double      load_avg_5;
    double      load_avg_15;
    unsigned long running_tasks;
    unsigned long total_tasks;
    unsigned int  latest_pid;
    unsigned long context_switches;
    unsigned long interrupts;
} lnx_sys_load_t;

/** Per-process information. */
typedef struct {
    int  pid;
    int  ppid;
    int  pgid;
    int  sid;
    int  uid;
    int  gid;
    int  euid;
    int  egid;

    char name[LNX_SYSMON_PROC_NAME_LEN];
    char state;                  /** R=running, S=sleeping, D=disk sleep, Z=zombie, T=stopped */
    char cmdline[LNX_SYSMON_CMDLINE_LEN];

    unsigned long vm_rss_kb;
    unsigned long vm_size_kb;

    unsigned long cpu_usage;      /** Percentage × 100 */
    unsigned long cpu_time_user;
    unsigned long cpu_time_sys;

    unsigned long uptime_sec;
    long          nice;
    int           priority;
    int           rt_priority;

    unsigned long voluntary_ctxt_switches;
    unsigned long nonvoluntary_ctxt_switches;

    int           threads;
    unsigned long open_fds;
} lnx_proc_info_t;

/** Process statistics summary. */
typedef struct {
    int total_processes;
    int running_processes;
    int sleeping_processes;
    int zombie_processes;
    int stopped_processes;
    int threaded_processes;
} lnx_proc_stats_t;

/** Process ID list. */
typedef struct {
    int  count;
    int  pids[LNX_SYSMON_MAX_PIDS];
} lnx_proc_list_t;

/* ============================================================================
 * System info functions
 * ============================================================================
 */

/**
 * Get CPU time and usage information.
 * Returns 0 on success, -1 on error.
 */
int lnx_sysmon_cpu_info(lnx_cpu_info_t *info);

/**
 * Get memory usage information.
 * Returns 0 on success, -1 on error.
 */
int lnx_sysmon_mem_info(lnx_mem_info_t *info);

/**
 * Get system load averages and task counts.
 * Returns 0 on success, -1 on error.
 */
int lnx_sysmon_load(lnx_sys_load_t *load);

/* ============================================================================
 * Process functions
 * ============================================================================
 */

/**
 * Get a list of all PIDs in the system.
 * Returns 0 on success, -1 on error.
 */
int lnx_sysmon_proc_list(lnx_proc_list_t *list);

/**
 * Get detailed information about a single process.
 * Returns 0 on success, -1 on error (errno set).
 */
int lnx_sysmon_proc_info(int pid, lnx_proc_info_t *info);

/**
 * Get aggregate process state statistics.
 * Returns 0 on success, -1 on error.
 */
int lnx_sysmon_proc_stats(lnx_proc_stats_t *stats);

/**
 * Send a signal to a process.
 * Returns 0 on success, -1 on error (errno set).
 */
int lnx_sysmon_kill(int pid, int sig);

#ifdef __cplusplus
}
#endif

#endif /* LNX_SYSMON_H */