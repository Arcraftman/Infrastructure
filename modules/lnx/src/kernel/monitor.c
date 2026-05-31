/**
 * @file monitor.c
 * @brief Userspace system monitoring — reads /proc and /sys.
 *
 * Provides CPU, memory, load, and process information
 * via standard Linux `/proc` filesystem interfaces.
 */

#include "monitor.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* ============================================================================
 * Internal helpers
 * ============================================================================
 */

/** Read a whole /proc file into a fixed buffer. */
static int proc_read(const char *path, char *buf, size_t bufsz)
{
    int fd;
    ssize_t n;
    fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    n = read(fd, buf, bufsz - 1);
    close(fd);
    if (n <= 0) return -1;
    buf[n] = '\0';
    return (int)n;
}

/** Parse a "/proc/stat" line "cpu  ..." into a lnx_cpu_time_t. */
static int parse_cpu_line(const char *line, lnx_cpu_time_t *ct)
{
    unsigned long vals[10];
    int n;
    /* skip "cpu" prefix to handle both "cpu " and "cpu0" */
    if (strncmp(line, "cpu", 3) != 0) return -1;
    n = sscanf(line + 3, " %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
               &vals[0], &vals[1], &vals[2], &vals[3],
               &vals[4], &vals[5], &vals[6], &vals[7],
               &vals[8], &vals[9]);
    if (n < 4) return -1;
    memset(ct, 0, sizeof(*ct));
    ct->user    = vals[0];
    ct->nice    = vals[1];
    ct->system  = vals[2];
    ct->idle    = vals[3];
    if (n > 4) ct->iowait  = vals[4];
    if (n > 5) ct->irq     = vals[5];
    if (n > 6) ct->softirq = vals[6];
    if (n > 7) ct->steal   = vals[7];
    if (n > 8) ct->guest   = vals[8];
    ct->total = ct->user + ct->nice + ct->system + ct->idle +
                ct->iowait + ct->irq + ct->softirq + ct->steal + ct->guest;
    return 0;
}

/** Parse /proc/loadavg */
static int parse_loadavg(const char *line, double *l1, double *l5, double *l15,
                         unsigned long *running, unsigned long *total,
                         unsigned int *last_pid)
{
    return sscanf(line, "%lf %lf %lf %lu/%lu %u",
                  l1, l5, l15, running, total, last_pid);
}

/** Parse a /proc/[pid]/stat file. */
static int parse_proc_stat(const char *content, int *ppid, char *state,
                           unsigned long *utime, unsigned long *stime,
                           long *nice, long *num_threads,
                           unsigned long *vsize, unsigned long *rss)
{
    /* Fields: pid comm state ppid pgrp session tty_nr tpgid flags
     *         minflt cminflt majflt cmajflt utime stime ...
     *         cutime cstime priority nice num_threads itrealvalue starttime
     *         vsize rss ... */
    int n;
    /* We need to skip the comm field which is in parentheses.
       Start scanning after the closing ')' */
    const char *p = strrchr(content, ')');
    if (!p) return -1;
    p += 2; /* skip ") " */
    n = sscanf(p, "%c %d %*d %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %*d %*d %ld %ld %*u %lu %lu",
               state, ppid,
               utime, stime,
               &nice[0], &num_threads[0],
               vsize, rss);
    if (n < 8) return -1;
    return 0;
}

/* ============================================================================
 * CPU information
 * ============================================================================
 */

int lnx_sysmon_cpu_info(lnx_cpu_info_t *info)
{
    char buf[16384];
    char *line, *next;
    lnx_cpu_time_t total;
    int idx = 0;

    if (!info) { errno = EINVAL; return -1; }
    memset(info, 0, sizeof(*info));

    if (proc_read("/proc/stat", buf, sizeof(buf)) < 0)
        return -1;

    memset(&total, 0, sizeof(total));
    line = buf;
    while ((next = strchr(line, '\n')) != NULL) {
        *next = '\0';
        if (strncmp(line, "cpu", 3) == 0) {
            lnx_cpu_time_t ct;
            if (parse_cpu_line(line, &ct) == 0) {
                if (line[3] == ' ') {
                    /* aggregate "cpu " line */
                    total = ct;
                } else if (isdigit((unsigned char)line[3])) {
                    /* per-core "cpuN" */
                    if (idx < LNX_SYSMON_MAX_CPUS)
                        info->per_cpu[idx++] = ct;
                }
            }
        }
        line = next + 1;
    }

    info->cpu_count = idx;
    info->total      = total;
    info->total.user = total.user; /* copy the aggregate times */
    info->total.nice = total.nice;
    info->total.system = total.system;
    info->total.idle = total.idle;
    info->total.iowait = total.iowait;
    info->total.irq = total.irq;
    info->total.softirq = total.softirq;
    info->total.steal = total.steal;
    info->total.guest = total.guest;

    /* usage_percent */
    if (total.total > 0) {
        unsigned long idle_total = total.idle + total.iowait;
        info->usage_percent = (total.total - idle_total) * 100 / total.total;
    }

    return 0;
}

/* ============================================================================
 * Memory information
 * ============================================================================
 */

int lnx_sysmon_mem_info(lnx_mem_info_t *info)
{
    char buf[8192];
    char *line, *next;
    unsigned long val;

    if (!info) { errno = EINVAL; return -1; }
    memset(info, 0, sizeof(*info));

    if (proc_read("/proc/meminfo", buf, sizeof(buf)) < 0)
        return -1;

    line = buf;
    while ((next = strchr(line, '\n')) != NULL) {
        *next = '\0';
        if (sscanf(line, "MemTotal: %lu kB", &val) == 1)
            info->total_kb = val;
        else if (sscanf(line, "MemFree: %lu kB", &val) == 1)
            info->free_kb = val;
        else if (sscanf(line, "MemAvailable: %lu kB", &val) == 1)
            info->available_kb = val;
        else if (sscanf(line, "Buffers: %lu kB", &val) == 1)
            info->buffers_kb = val;
        else if (sscanf(line, "Cached: %lu kB", &val) == 1)
            info->cached_kb = val;
        else if (sscanf(line, "Shmem: %lu kB", &val) == 1)
            info->shmem_kb = val;
        else if (sscanf(line, "SReclaimable: %lu kB", &val) == 1)
            info->slab_kb += val;
        else if (sscanf(line, "SUnreclaim: %lu kB", &val) == 1)
            info->slab_kb += val;
        else if (sscanf(line, "SwapTotal: %lu kB", &val) == 1)
            info->swap_total_kb = val;
        else if (sscanf(line, "SwapFree: %lu kB", &val) == 1)
            info->swap_free_kb = val;
        line = next + 1;
    }

    if (info->total_kb > 0) {
        unsigned long used = info->total_kb - info->free_kb;
        info->usage_percent = (int)(used * 100 / info->total_kb);
    }
    if (info->swap_total_kb > 0) {
        unsigned long used = info->swap_total_kb - info->swap_free_kb;
        info->swap_usage_percent = (int)(used * 100 / info->swap_total_kb);
    }

    return 0;
}

/* ============================================================================
 * System load
 * ============================================================================
 */

int lnx_sysmon_load(lnx_sys_load_t *load)
{
    char buf[1024];

    if (!load) { errno = EINVAL; return -1; }
    memset(load, 0, sizeof(*load));

    if (proc_read("/proc/loadavg", buf, sizeof(buf)) < 0)
        return -1;

    parse_loadavg(buf,
                  &load->load_avg_1,
                  &load->load_avg_5,
                  &load->load_avg_15,
                  &load->running_tasks,
                  &load->total_tasks,
                  &load->latest_pid);
    return 0;
}

/* ============================================================================
 * Process list
 * ============================================================================
 */

int lnx_sysmon_proc_list(lnx_proc_list_t *list)
{
    DIR *d;
    struct dirent *de;
    int count = 0;

    if (!list) { errno = EINVAL; return -1; }
    memset(list, 0, sizeof(*list));

    d = opendir("/proc");
    if (!d) return -1;

    while ((de = readdir(d)) != NULL) {
        if (de->d_type == DT_DIR && isdigit((unsigned char)de->d_name[0])) {
            if (count < LNX_SYSMON_MAX_PIDS)
                list->pids[count++] = atoi(de->d_name);
        }
    }
    closedir(d);
    list->count = count;
    return 0;
}

/* ============================================================================
 * Process information
 * ============================================================================
 */

int lnx_sysmon_proc_info(int pid, lnx_proc_info_t *info)
{
    char path[64];
    char buf[4096];
    char cl_buf[LNX_SYSMON_CMDLINE_LEN];
    int ppid;
    char state;
    unsigned long utime, stime, vsize, rss;
    long nice, num_threads;
    int rc, fd;

    if (!info || pid <= 0) { errno = EINVAL; return -1; }
    memset(info, 0, sizeof(*info));

    /* /proc/[pid]/stat */
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    if (proc_read(path, buf, sizeof(buf)) < 0)
        return -1;

    /* Extract name from parentheses */
    char *p1 = strchr(buf, '(');
    char *p2 = strrchr(buf, ')');
    if (!p1 || !p2 || p2 <= p1) return -1;
    size_t nlen = (size_t)(p2 - p1 - 1);
    if (nlen >= LNX_SYSMON_PROC_NAME_LEN) nlen = LNX_SYSMON_PROC_NAME_LEN - 1;
    memcpy(info->name, p1 + 1, nlen);
    info->name[nlen] = '\0';

    rc = parse_proc_stat(buf, &ppid, &state, &utime, &stime,
                         &nice, &num_threads, &vsize, &rss);
    if (rc != 0) return -1;

    info->pid     = pid;
    info->ppid    = ppid;
    info->state   = state;
    info->nice    = nice;
    info->threads = (int)num_threads;

    info->cpu_time_user = utime;
    info->cpu_time_sys  = stime;

    /* /proc/[pid]/status for UID/GID and context switches */
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    if (proc_read(path, buf, sizeof(buf)) > 0) {
        char *ln, *nxt;
        ln = buf;
        while ((nxt = strchr(ln, '\n')) != NULL) {
            *nxt = '\0';
            if (sscanf(ln, "Uid: %d %*d %*d %*d", &info->uid) == 1) {}
            else if (sscanf(ln, "Gid: %d %*d %*d %*d", &info->gid) == 1) {}
            else if (sscanf(ln, " voluntary_ctxt_switches: %lu",
                            &info->voluntary_ctxt_switches) == 1) {}
            else if (sscanf(ln, " nonvoluntary_ctxt_switches: %lu",
                            &info->nonvoluntary_ctxt_switches) == 1) {}
            else if (sscanf(ln, "VmRSS: %lu kB", &info->vm_rss_kb) == 1) {}
            else if (sscanf(ln, "VmSize: %lu kB", &info->vm_size_kb) == 1) {}
            ln = nxt + 1;
        }
    }

    /* /proc/[pid]/cmdline */
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        ssize_t nr = read(fd, cl_buf, sizeof(cl_buf) - 1);
        close(fd);
        if (nr > 0) {
            /* replace embedded NULs with spaces */
            for (ssize_t i = 0; i < nr; i++)
                if (cl_buf[i] == '\0') cl_buf[i] = ' ';
            cl_buf[nr] = '\0';
            memcpy(info->cmdline, cl_buf, (size_t)nr < sizeof(info->cmdline) ? (size_t)nr : sizeof(info->cmdline) - 1);
        }
    }

    /* /proc/[pid]/fd for open file count */
    snprintf(path, sizeof(path), "/proc/%d/fd", pid);
    DIR *fdd = opendir(path);
    if (fdd) {
        unsigned long fdc = 0;
        while (readdir(fdd)) fdc++;
        closedir(fdd);
        info->open_fds = fdc > 2 ? fdc - 2 : 0; /* skip . and .. */
    }

    /* uptime from /proc/uptime - proc start time */
    info->uptime_sec = 0; /* simplified; real calc needs boot time */

    return 0;
}

/* ============================================================================
 * Process statistics
 * ============================================================================
 */

int lnx_sysmon_proc_stats(lnx_proc_stats_t *stats)
{
    DIR *d;
    struct dirent *de;
    char path[320];
    char buf[512];

    if (!stats) { errno = EINVAL; return -1; }
    memset(stats, 0, sizeof(*stats));

    d = opendir("/proc");
    if (!d) return -1;

    while ((de = readdir(d)) != NULL) {
        if (de->d_type != DT_DIR || !isdigit((unsigned char)de->d_name[0]))
            continue;

        stats->total_processes++;

        snprintf(path, sizeof(path), "/proc/%s/stat", de->d_name);
        if (proc_read(path, buf, sizeof(buf)) > 0) {
            /* state is the first field after the closing ')' */
            char *cp = strrchr(buf, ')');
            if (cp && cp[1] == ' ') {
                char st = cp[2];
                switch (st) {
                case 'R': stats->running_processes++;  break;
                case 'S': case 'D': stats->sleeping_processes++; break;
                case 'Z': stats->zombie_processes++;   break;
                case 'T': stats->stopped_processes++;  break;
                default: break;
                }
            }
            /* count threads */
            cp = strrchr(buf, ')');
            if (cp) {
                unsigned long th = 0;
                /* fields after state: ppid pgrp session tty flags minflt
                   cminflt majflt cmajflt utime stime cutime cstime priority
                   nice num_threads ... */
                /* Skip state + ppid + pgrp + session + tty + tpgid + flags
                   minflt cminflt majflt cmajflt utime stime cutime cstime */
                if (sscanf(cp + 2, "%*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu",
                           &th) >= 1 && th > 1)
                    stats->threaded_processes++;
            }
        }
    }
    closedir(d);
    return 0;
}

/* ============================================================================
 * Signal delivery
 * ============================================================================
 */

int lnx_sysmon_kill(int pid, int sig)
{
    return kill(pid, sig);
}