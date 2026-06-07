#ifndef LNX_FSNOTIFY_H
#define LNX_FSNOTIFY_H

#include "lnx/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * File System Notification (inotify wrapper)
 *
 * This API wraps Linux inotify(7) into a simple event-watch interface.
 * ========================================================================= */

/** Opaque handle for a filesystem notification instance. */
typedef struct lnx_fsnotify lnx_fsnotify_t;

/** Event types for lnx_fsnotify. */
typedef enum {
    LNX_FSNOTIFY_ACCESS    = 0x00000001, /** File was accessed. */
    LNX_FSNOTIFY_MODIFY    = 0x00000002, /** File was modified. */
    LNX_FSNOTIFY_ATTRIB    = 0x00000004, /** Metadata changed. */
    LNX_FSNOTIFY_CLOSE     = 0x00000008, /** File was closed. */
    LNX_FSNOTIFY_OPEN      = 0x00000010, /** File was opened. */
    LNX_FSNOTIFY_MOVE      = 0x00000020, /** File was moved (in/out). */
    LNX_FSNOTIFY_CREATE    = 0x00000040, /** File was created. */
    LNX_FSNOTIFY_DELETE    = 0x00000080, /** File was deleted. */
    LNX_FSNOTIFY_DELETE_SELF = 0x00000100, /** Watched file was deleted. */
    LNX_FSNOTIFY_MOVE_SELF = 0x00000200, /** Watched file was moved. */
    LNX_FSNOTIFY_ALL       = 0x000003FF, /** All event types. */
} lnx_fsnotify_mask_t;

/** A single notification event. */
typedef struct {
    int          wd;         /** Watch descriptor. */
    uint32_t     mask;       /** Event mask. */
    uint32_t     cookie;     /** Cookie for rename matching. */
    const char  *name;       /** File name (heap-allocated). */
    size_t       name_len;   /** Length of name. */
} lnx_fsnotify_event_t;

/**
 * Create a new inotify instance.
 * Returns NULL on error.
 */
LNX_API lnx_fsnotify_t *lnx_fsnotify_create(void);

/**
 * Destroy an inotify instance and all its watches.
 */
LNX_API void lnx_fsnotify_destroy(lnx_fsnotify_t *nf);

/**
 * Add a watch on a path for the given event mask.
 * Returns the watch descriptor, or -1 on error.
 */
LNX_API int lnx_fsnotify_add_watch(lnx_fsnotify_t *nf, const char *path, uint32_t mask);

/**
 * Remove a watch. Returns 0 on success, -1 on error.
 */
LNX_API int lnx_fsnotify_rm_watch(lnx_fsnotify_t *nf, int wd);

/**
 * Read pending events.
 * Returns the number of events read, 0 if none available, or -1 on error.
 * Each event's `name` must be freed by the caller.
 */
LNX_API int lnx_fsnotify_read(lnx_fsnotify_t *nf, lnx_fsnotify_event_t *events, int count);

/**
 * Get the underlying file descriptor (for use with poll/select/epoll).
 */
LNX_API int lnx_fsnotify_fd(lnx_fsnotify_t *nf);

#ifdef __cplusplus
}
#endif

#endif /* LNX_FSNOTIFY_H */
