#include "lnx/def.h"
#include "lnx/sched.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>

/* =========================================================================
 * CPU count — header-declared wrappers
 * ========================================================================= */

static int cached_page_size = 0;

LNX_API int
lnx_cpu_count(void)
{
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (int)n;
}

LNX_API int
lnx_cpu_available(void)
{
    /* Try cgroup-aware CPU quota first */
    FILE *fp = fopen("/sys/fs/cgroup/cpu/cpu.cfs_quota_us", "r");
    if (!fp)
        fp = fopen("/sys/fs/cgroup/cpu.max", "r");
    if (fp) {
        long quota = -1, period = -1;
        int ret = fscanf(fp, "%ld %ld", &quota, &period);
        fclose(fp);
        if (ret == 2 && quota > 0 && period > 0) {
            int cpus = (int)((quota + period / 2) / period);
            if (cpus > 0 && cpus < 1024)
                return cpus;
        }
    }
    /* Fall back to online count */
    return lnx_cpu_count();
}

LNX_API int
lnx_cpu_current(void)
{
    int cpu = sched_getcpu();
    return (cpu >= 0) ? cpu : 0;
}

/* =========================================================================
 * CPU affinity
 * ========================================================================= */

LNX_API int
lnx_thread_pin_cpu(int cpu)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
}

LNX_API int
lnx_thread_get_affinity(unsigned long *mask, size_t size)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    pid_t pid = getpid();
    if (sched_getaffinity(pid, sizeof(cpuset), &cpuset) != 0)
        return -1;
    if (mask) {
        size_t copy = size < sizeof(cpuset.__bits) ? size : sizeof(cpuset.__bits);
        memcpy(mask, cpuset.__bits, copy);
    }
    return 0;
}

/* =========================================================================
 * Scheduling policy
 * ========================================================================= */

static const char *const SCHED_NAMES[] = {
    [0] = "SCHED_OTHER",
    [1] = "SCHED_FIFO",
    [2] = "SCHED_RR",
    [3] = "SCHED_BATCH",
    [5] = "SCHED_IDLE",
};

static const int SCHED_POLICY_MAP[] = {
    [0] = SCHED_OTHER,
    [1] = SCHED_FIFO,
    [2] = SCHED_RR,
    [3] = SCHED_BATCH,
    [5] = SCHED_IDLE,
};

LNX_API const char *
lnx_sched_policy_name(lnx_sched_policy_t policy)
{
    if ((int)policy >= 0 && (int)policy < 6 && SCHED_NAMES[policy])
        return SCHED_NAMES[policy];
    return "SCHED_UNKNOWN";
}

LNX_API int
lnx_sched_set(lnx_sched_policy_t policy, int priority)
{
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = priority;
    int pol = SCHED_POLICY_MAP[policy];
    return sched_setscheduler(0, pol, &param);
}

LNX_API int
lnx_sched_get(pid_t pid)
{
    int pol = sched_getscheduler(pid);
    if (pol < 0)
        return -1;
    /* Map kernel policy values back to our enum */
    switch (pol) {
        case SCHED_OTHER: return 0;
        case SCHED_FIFO:  return 1;
        case SCHED_RR:    return 2;
        case SCHED_BATCH: return 3;
        case SCHED_IDLE:  return 5;
        default:          return -1;
    }
}

/* =========================================================================
 * Memory / Page info
 * ========================================================================= */

LNX_API long
lnx_page_size(void)
{
    long sz = sysconf(_SC_PAGESIZE);
    if (sz > 0) cached_page_size = (int)sz;
    return sz;
}

LNX_API long
lnx_page_size_cached(void)
{
    if (cached_page_size == 0)
        cached_page_size = (int)lnx_page_size();
    return (long)cached_page_size;
}

LNX_API int
lnx_huge_page_sizes(long *sizes, int max_count)
{
    FILE *fp = fopen("/sys/kernel/mm/hugepages", "r");
    if (!fp) {
        /* Fallback: read /proc/meminfo for Hugepagesize */
        fp = fopen("/proc/meminfo", "r");
        if (!fp) return 0;

        char line[256];
        int count = 0;
        while (fgets(line, sizeof(line), fp) && count < max_count) {
            if (strncmp(line, "Hugepagesize:", 13) == 0) {
                unsigned long val = 0;
                if (sscanf(line + 13, "%lu", &val) == 1)
                    sizes[count++] = val * 1024; /* kB → bytes */
                break;
            }
        }
        fclose(fp);
        return count;
    }
    fclose(fp);

    /* Read /sys/kernel/mm/hugepages/ directory entries */
    int count = 0;
    char path[256];
    for (int i = 0; i < max_count && i < 32; i++) {
        snprintf(path, sizeof(path),
                 "/sys/kernel/mm/hugepages/hugepages-%dkB", (1 << (i + 8)));
        if (access(path, F_OK) == 0)
            sizes[count++] = (1L << (i + 8)) * 1024; /* kB → bytes */
    }
    return count;
}

LNX_API unsigned long long
lnx_total_ram(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long psize = sysconf(_SC_PAGESIZE);
    if (pages < 0 || psize < 0)
        return 0;
    return (unsigned long long)pages * (unsigned long long)psize;
}

/* =========================================================================
 * Extended API — additional helpers beyond the minimal header
 * ========================================================================= */

LNX_API int
lnx_sched_online_cpus(void)
{
    return lnx_cpu_count();
}

LNX_API int
lnx_sched_configured_cpus(void)
{
    long n = sysconf(_SC_NPROCESSORS_CONF);
    return (int)n;
}

LNX_API int
lnx_sched_uptime(double *uptime_sec, double *idle_sec)
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp)
        return -1;
    double up = 0.0, idle = 0.0;
    int ret = fscanf(fp, "%lf %lf", &up, &idle);
    fclose(fp);
    if (ret != 2)
        return -1;
    if (uptime_sec) *uptime_sec = up;
    if (idle_sec)   *idle_sec   = idle;
    return 0;
}

LNX_API unsigned long
lnx_sched_avail_ram(void)
{
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long psize = sysconf(_SC_PAGESIZE);
    if (pages < 0 || psize < 0)
        return 0;
    return (unsigned long)(pages * psize);
}

LNX_API int
lnx_sched_ticks_per_sec(void)
{
    long tps = sysconf(_SC_CLK_TCK);
    return (int)tps;
}

LNX_API int
lnx_sched_loadavg(double *load1, double *load5, double *load15)
{
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp)
        return -1;
    int ret = fscanf(fp, "%lf %lf %lf", load1, load5, load15);
    fclose(fp);
    return (ret == 3) ? 0 : -1;
}
