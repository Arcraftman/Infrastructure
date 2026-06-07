#include "lnx/def.h"
#include "lnx/process.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

LNX_API pid_t
lnx_proc_pid(void)
{
    return getpid();
}

LNX_API pid_t
lnx_proc_ppid(void)
{
    return getppid();
}

LNX_API int
lnx_proc_name(char *buf, size_t size)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp)
        return -1;
    char line[256];
    int ret = -1;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Name:", 5) == 0) {
            const char *p = line + 5;
            while (*p == ' ' || *p == '\t') p++;
            size_t len = strlen(p);
            if (len > 0 && p[len - 1] == '\n')
                len--;
            if (len < size) {
                memcpy(buf, p, len);
                buf[len] = '\0';
                ret = (int)len;
            }
            break;
        }
    }
    fclose(fp);
    return ret;
}

LNX_API int
lnx_proc_exe_path(char *buf, size_t size)
{
    ssize_t n = readlink("/proc/self/exe", buf, size - 1);
    if (n < 0)
        return -1;
    buf[n] = '\0';
    return (int)n;
}

LNX_API int
lnx_proc_cmdline(char *buf, size_t size)
{
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0)
        return -1;
    ssize_t n = read(fd, buf, size - 1);
    close(fd);
    if (n < 0)
        return -1;
    buf[n] = '\0';
    /* Replace embedded NULs with spaces for readability */
    for (ssize_t i = 0; i < n - 1; i++) {
        if (buf[i] == '\0')
            buf[i] = ' ';
    }
    return (int)n;
}

/* Helper: parse /proc/[pid]/stat for state information */
static int
parse_proc_stat(pid_t pid, char *state_out, unsigned long *utime,
                unsigned long *stime, long *priority, long *nice)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp)
        return -1;

    /* Format: pid (comm) state ppid pgrp session tty_nr tpgid flags
     *           minflt cminflt majflt cmajflt utime stime ...
     * We need fields 3 (state), 14 (utime), 15 (stime), 18 (priority), 19 (nice)
     */
    char buf[1024];
    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    /* Skip PID */
    char *p = buf;
    while (*p && *p != ' ') p++;
    if (!*p) return -1;
    p++;
    /* Skip (comm) — find matching close paren */
    if (*p == '(') {
        p++;
        int depth = 1;
        while (*p && depth > 0) {
            if (*p == '(') depth++;
            else if (*p == ')') depth--;
            if (depth > 0) p++;
        }
        if (*p == ')') p++;
    }
    if (!*p) return -1;
    p++; /* skip space after ) */

    /* Now we're at field 3 (state) */
    if (state_out)
        *state_out = *p;
    p++;
    /* Skip fields 4-13 (10 fields) */
    for (int i = 0; i < 10 && *p; i++) {
        while (*p && *p != ' ') p++;
        if (*p) p++;
    }
    /* Now at field 14 (utime) */
    if (utime) *utime = strtoul(p, &p, 10);
    p++;
    /* Field 15 (stime) */
    if (stime) *stime = strtoul(p, &p, 10);
    /* Skip fields 16-17 */
    for (int i = 0; i < 2 && *p; i++) {
        while (*p && *p != ' ') p++;
        if (*p) p++;
    }
    /* Field 18 (priority) */
    if (priority) *priority = strtol(p, &p, 10);
    p++;
    /* Field 19 (nice) */
    if (nice) *nice = strtol(p, &p, 10);

    return 0;
}

LNX_API lnx_proc_state_t
lnx_proc_state(pid_t pid)
{
    char state = 0;
    if (parse_proc_stat(pid, &state, NULL, NULL, NULL, NULL) != 0)
        return LNX_PROC_STATE_UNKNOWN;

    switch (state) {
        case 'R': return LNX_PROC_STATE_RUNNING;
        case 'S': return LNX_PROC_STATE_SLEEPING;
        case 'D': return LNX_PROC_STATE_DISK_SLEEP;
        case 'Z': return LNX_PROC_STATE_ZOMBIE;
        case 'T': return LNX_PROC_STATE_STOPPED;
        case 't': return LNX_PROC_STATE_TRACING;
        case 'X': return LNX_PROC_STATE_DEAD;
        case 'I': return LNX_PROC_STATE_IDLE;
        default:  return LNX_PROC_STATE_UNKNOWN;
    }
}

LNX_API bool
lnx_proc_is_alive(pid_t pid)
{
    return kill(pid, 0) == 0;
}

LNX_API int
lnx_proc_fd_count(void)
{
    int count = 0;
    DIR *dir = opendir("/proc/self/fd");
    if (!dir)
        return -1;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        count++;
    }
    closedir(dir);
    /* Subtract . and .. */
    return count > 2 ? count - 2 : 0;
}

LNX_API unsigned long
lnx_proc_vm_size(void)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp)
        return 0;
    char line[256];
    unsigned long val = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "VmSize: %lu kB", &val) == 1)
            break;
        if (sscanf(line, "VmRSS: %lu kB", &val) == 1) {
            val *= 1024; /* kB → bytes */
            break;
        }
    }
    fclose(fp);
    return val;
}

LNX_API unsigned long
lnx_proc_rss(void)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp)
        return 0;
    char line[256];
    unsigned long val = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "VmRSS: %lu kB", &val) == 1) {
            val *= 1024;
            break;
        }
    }
    fclose(fp);
    return val;
}

LNX_API int
lnx_proc_thread_count(void)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp)
        return -1;
    char line[256];
    int count = -1;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "Threads: %d", &count) == 1)
            break;
    }
    fclose(fp);
    return count;
}

LNX_API double
lnx_proc_uptime(void)
{
    /* /proc/self/stat field 22 = starttime in ticks */
    pid_t pid = getpid();
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp)
        return -1;

    char buf[1024];
    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    /* Skip to field 22 */
    char *p = buf;
    while (*p && *p != ' ') {
        p++;
    }
    p++;    /* skip pid */
    if (*p == '(') {
        p++;
        int depth = 1;
        while (*p && depth > 0) {
            if (*p == '(') depth++;
            else if (*p == ')') depth--;
            if (depth > 0) p++;
        }
        if (*p == ')') p++;
    }
    p++; /* skip space */

    /* Now at field 3, skip through field 21 (19 more fields) */
    for (int i = 0; i < 19 && *p; i++) {
        while (*p && *p != ' ') p++;
        if (*p) p++;
    }
    /* Field 22 = starttime */
    unsigned long long start_ticks = strtoull(p, NULL, 10);

    double uptime_sec = 0;
    FILE *fp2 = fopen("/proc/uptime", "r");
    if (fp2) {
        fscanf(fp2, "%lf", &uptime_sec);
        fclose(fp2);
    }

    long ticks = sysconf(_SC_CLK_TCK);
    if (ticks <= 0) ticks = 100;

    double proc_start = (double)start_ticks / ticks;
    return uptime_sec - proc_start;
}

LNX_API lnx_proc_result_t
lnx_proc_exec(const char *cmd)
{
    lnx_proc_result_t result = { -1, NULL, NULL };

    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0)
        return result;

    pid_t child = fork();
    if (child < 0) {
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);
        return result;
    }

    if (child == 0) {
        /* Child */
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }

    /* Parent */
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    /* Read output */
    char stdout_buf[65536], stderr_buf[65536];
    ssize_t stdout_n = 0, stderr_n = 0;

    /* Read stdout */
    ssize_t n;
    while ((n = read(stdout_pipe[0],
              stdout_buf + stdout_n,
              sizeof(stdout_buf) - stdout_n - 1)) > 0)
        stdout_n += n;
    close(stdout_pipe[0]);

    while ((n = read(stderr_pipe[0],
              stderr_buf + stderr_n,
              sizeof(stderr_buf) - stderr_n - 1)) > 0)
        stderr_n += n;
    close(stderr_pipe[0]);

    stdout_buf[stdout_n] = '\0';
    stderr_buf[stderr_n] = '\0';

    /* Wait for child */
    int status;
    waitpid(child, &status, 0);
    result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    /* Copy output */
    if (stdout_n > 0) {
        result.stdout_data = strdup(stdout_buf);
    }
    if (stderr_n > 0) {
        result.stderr_data = strdup(stderr_buf);
    }

    return result;
}

LNX_API lnx_proc_result_t
lnx_proc_execv(const char **argv)
{
    lnx_proc_result_t result = { -1, NULL, NULL };

    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0)
        return result;

    pid_t child = fork();
    if (child < 0) {
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);
        return result;
    }

    if (child == 0) {
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        execvp(argv[0], (char *const *)argv);
        _exit(127);
    }

    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    char stdout_buf[65536], stderr_buf[65536];
    ssize_t stdout_n = 0, stderr_n = 0;
    ssize_t n;

    while ((n = read(stdout_pipe[0],
              stdout_buf + stdout_n,
              sizeof(stdout_buf) - stdout_n - 1)) > 0)
        stdout_n += n;
    close(stdout_pipe[0]);

    while ((n = read(stderr_pipe[0],
              stderr_buf + stderr_n,
              sizeof(stderr_buf) - stderr_n - 1)) > 0)
        stderr_n += n;
    close(stderr_pipe[0]);

    stdout_buf[stdout_n] = '\0';
    stderr_buf[stderr_n] = '\0';

    int status;
    waitpid(child, &status, 0);
    result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    if (stdout_n > 0) result.stdout_data = strdup(stdout_buf);
    if (stderr_n > 0) result.stderr_data = strdup(stderr_buf);

    return result;
}
