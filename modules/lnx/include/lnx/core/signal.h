#ifndef LNX_SIGNAL_H
#define LNX_SIGNAL_H

#include "lnx/def.h"
#include <signal.h>
#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Signal Guard — save / restore signal dispositions
 * ========================================================================= */

/** Opaque signal guard type. */
typedef struct lnx_signal_guard lnx_signal_guard_t;

/** Allocate and initialize a signal guard. */
LNX_API lnx_signal_guard_t *lnx_signal_guard_init(void);

/** Free a signal guard (does not restore handlers). */
LNX_API void lnx_signal_guard_free(lnx_signal_guard_t *g);

/** Capture the current disposition for a signal. */
LNX_API int lnx_signal_guard_capture(lnx_signal_guard_t *g, int signum);

/** Restore the captured disposition for one signal. */
LNX_API int lnx_signal_guard_restore(lnx_signal_guard_t *g, int signum);

/** Restore all captured dispositions. */
LNX_API int lnx_signal_guard_restore_all(lnx_signal_guard_t *g);

/* =========================================================================
 * Signal Blocking (per-signal-number convenience)
 * ========================================================================= */

/**
 * Block delivery of a single signal to the calling thread.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_signal_block(int signum);

/**
 * Unblock delivery of a single signal to the calling thread.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_signal_unblock(int signum);

/**
 * Ignore a signal (SIG_IGN).
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_signal_ignore(int signum);

/**
 * Restore the default disposition for a signal (SIG_DFL).
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_signal_default(int signum);

/* =========================================================================
 * Signal Name / Number Conversion
 * ========================================================================= */

/**
 * Write the canonical signal name into a caller-supplied buffer.
 * Returns the number of characters written (excluding NUL).
 */
LNX_API int lnx_signal_name(int signum, char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* LNX_SIGNAL_H */
