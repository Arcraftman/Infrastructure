#ifndef LNX_H
#define LNX_H

/**
 * @file lnx.h
 * @brief Umbrella header for the Infra lnx (Linux/Unix system) module.
 *
 * Include this single header to get all public lnx module APIs.
 *
 * ## Module Overview
 *
 * The lnx module provides portable wrappers for Linux/Unix system
 * interfaces, including:
 *
 *   - **process** – PID, name, executable path, cmdline, execution
 *   - **signal** – Signal guard (save/restore), block/unblock/ignore
 *   - **scheduling** – CPU count, thread pinning, scheduler policy,
 *     memory/page info
 *   - **filesystem** – File size, metadata, read/write, locking,
 *     temporary files, path helpers
 *   - **filesystem notification** – inotify-based directory/file
 *     change monitoring
 *
 * All functions use the `lnx_` prefix and follow POSIX conventions
 * with consistent error handling (return -1 / NULL on error, set errno).
 *
 * @defgroup lnx_module lnx (Linux/Unix System)
 */

#include "lnx/def.h"
#include "lnx/process.h"
#include "lnx/signal.h"
#include "lnx/sched.h"
#include "lnx/file.h"
#include "lnx/fsnotify.h"

#endif /* LNX_H */
