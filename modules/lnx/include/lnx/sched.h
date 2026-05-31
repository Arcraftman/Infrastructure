#ifndef LNX_SCHED_H
#define LNX_SCHED_H

#include "lnx/def.h"
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * CPU Information
 * ========================================================================= */

/** Returns the number of online CPUs / hardware threads. */
LNX_API int lnx_cpu_count(void);

/** Returns the number of available CPUs for the current process (cgroup-aware). */
LNX_API int lnx_cpu_available(void);

/** Returns the CPU number the calling thread is currently running on. */
LNX_API int lnx_cpu_current(void);

/* =========================================================================
 * CPU Affinity
 * ========================================================================= */

/**
 * Pin the calling thread to a specific CPU.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_thread_pin_cpu(int cpu);

/**
 * Get the CPU affinity mask for the current process.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_thread_get_affinity(unsigned long *mask, size_t size);

/* =========================================================================
 * Scheduling
 * ========================================================================= */

/** Scheduling policy names. */
typedef enum {
    LNX_SCHED_NORMAL  = 0, /** SCHED_OTHER */
    LNX_SCHED_FIFO    = 1, /** SCHED_FIFO */
    LNX_SCHED_RR      = 2, /** SCHED_RR */
    LNX_SCHED_BATCH   = 3, /** SCHED_BATCH */
    LNX_SCHED_IDLE    = 5, /** SCHED_IDLE */
} lnx_sched_policy_t;

/** Returns the scheduling policy name as a string (e.g., "SCHED_OTHER"). */
LNX_API const char *lnx_sched_policy_name(lnx_sched_policy_t policy);

/**
 * Set the scheduling policy and priority for the current process.
 * Requires appropriate privileges for real-time policies.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_sched_set(lnx_sched_policy_t policy, int priority);

/**
 * Get the current scheduling policy for a process.
 * Returns the policy on success, -1 on error.
 */
LNX_API int lnx_sched_get(pid_t pid);

/* =========================================================================
 * Memory / Page Info
 * ========================================================================= */

/** Returns the system page size in bytes. */
LNX_API long lnx_page_size(void);

/** Returns the system page size in bytes, cached after first call. */
LNX_API long lnx_page_size_cached(void);

/** Returns the number of huge page size options available. */
LNX_API int lnx_huge_page_sizes(long *sizes, int max_count);

/** Returns total physical RAM in bytes, or 0 on error. */
LNX_API unsigned long long lnx_total_ram(void);

/* =========================================================================
 * Extended API — additional system info helpers
 * ========================================================================= */

/** Returns the number of online CPUs (alias for lnx_cpu_count). */
LNX_API int lnx_sched_online_cpus(void);

/** Returns the number of configured CPUs. */
LNX_API int lnx_sched_configured_cpus(void);

/** Returns available RAM in bytes. */
LNX_API unsigned long lnx_sched_avail_ram(void);

/** Returns the number of clock ticks per second. */
LNX_API int lnx_sched_ticks_per_sec(void);

/**
 * Read system uptime.
 * @param uptime_sec  [out] Seconds since boot (may be NULL).
 * @param idle_sec    [out] Idle seconds (may be NULL).
 * @return 0 on success, -1 on error.
 */
LNX_API int lnx_sched_uptime(double *uptime_sec, double *idle_sec);

/**
 * Read system load average.
 * @param load1  [out] 1-minute load (may be NULL).
 * @param load5  [out] 5-minute load (may be NULL).
 * @param load15 [out] 15-minute load (may be NULL).
 * @return 0 on success, -1 on error.
 */
LNX_API int lnx_sched_loadavg(double *load1, double *load5, double *load15);

#ifdef __cplusplus
}
#endif

#endif /* LNX_SCHED_H */
