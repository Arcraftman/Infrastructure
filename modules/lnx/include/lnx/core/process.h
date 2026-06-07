#ifndef LNX_PROCESS_H
#define LNX_PROCESS_H

#include "lnx/def.h"

#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Process Information
 * ========================================================================= */

/** Returns the current process ID. */
LNX_API pid_t lnx_proc_pid(void);

/** Returns the parent process ID. */
LNX_API pid_t lnx_proc_ppid(void);

/** Returns the process name from /proc/self/status (Linux) or equivalent. */
LNX_API int lnx_proc_name(char *buf, size_t size);

/** Returns the executable path of the current process. */
LNX_API int lnx_proc_exe_path(char *buf, size_t size);

/** Returns the command-line arguments as a single NUL-joined string. */
LNX_API int lnx_proc_cmdline(char *buf, size_t size);

/* =========================================================================
 * Process State
 * ========================================================================= */

/** Process state returned by lnx_proc_state(). */
typedef enum {
    LNX_PROC_STATE_UNKNOWN,
    LNX_PROC_STATE_RUNNING,   /** R */
    LNX_PROC_STATE_SLEEPING,  /** S */
    LNX_PROC_STATE_DISK_SLEEP,/** D */
    LNX_PROC_STATE_ZOMBIE,    /** Z */
    LNX_PROC_STATE_STOPPED,   /** T */
    LNX_PROC_STATE_TRACING,   /** t */
    LNX_PROC_STATE_DEAD,      /** X */
    LNX_PROC_STATE_IDLE,      /** I */
} lnx_proc_state_t;

/** Returns the state of the given process from /proc/[pid]/status. */
LNX_API lnx_proc_state_t lnx_proc_state(pid_t pid);

/** Returns true if the process with the given PID is alive. */
LNX_API bool lnx_proc_is_alive(pid_t pid);

/* =========================================================================
 * Process Resource Usage
 * ========================================================================= */

/** Returns the number of file descriptors used by the current process. */
LNX_API int lnx_proc_fd_count(void);

/** Returns virtual memory size in bytes, or 0 on error. */
LNX_API unsigned long lnx_proc_vm_size(void);

/** Returns resident memory size in bytes, or 0 on error. */
LNX_API unsigned long lnx_proc_rss(void);

/** Returns the number of threads in the current process. */
LNX_API int lnx_proc_thread_count(void);

/** Returns the uptime of the current process in seconds. */
LNX_API double lnx_proc_uptime(void);

/* =========================================================================
 * Process Execution
 * ========================================================================= */

/** Result of a executed command. */
typedef struct {
    int   exit_code;
    char *stdout_data;
    char *stderr_data;
} lnx_proc_result_t;

/**
 * Execute a command and capture its output.
 * Returns a result struct — caller must free stdout_data and stderr_data.
 * Returns NULL set on failure.
 */
LNX_API lnx_proc_result_t lnx_proc_exec(const char *cmd);

/**
 * Execute a command with arguments (no shell expansion) and capture output.
 */
LNX_API lnx_proc_result_t lnx_proc_execv(const char **argv);

#ifdef __cplusplus
}
#endif

#endif /* LNX_PROCESS_H */
