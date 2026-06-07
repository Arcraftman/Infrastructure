#ifndef LNX_FILE_H
#define LNX_FILE_H

#include "lnx/def.h"
#include <stdio.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * File Path Utilities (Linux-specific /proc/self/fd helpers)
 * ========================================================================= */

/** Resolves a file descriptor to its absolute path via /proc/self/fd/N. */
LNX_API int lnx_fd_path(int fd, char *buf, size_t size);

/** Returns true if the given path exists. */
LNX_API bool lnx_path_exists(const char *path);

/** Returns true if path is a regular file. */
LNX_API bool lnx_path_is_file(const char *path);

/** Returns true if path is a directory. */
LNX_API bool lnx_path_is_dir(const char *path);

/** Returns true if path is a symbolic link. */
LNX_API bool lnx_path_is_symlink(const char *path);

/* =========================================================================
 * Extended File API — additional helpers
 * ========================================================================= */

/** Legacy: returns 1 if path exists, 0 if not, -1 on error (use lnx_path_exists). */
LNX_API int lnx_file_exists(const char *path);

/** Legacy: returns 1 if dir, 0 if not (use lnx_path_is_dir). */
LNX_API int lnx_file_is_dir(const char *path);

/** Legacy: returns 1 if regular file, 0 if not (use lnx_path_is_file). */
LNX_API int lnx_file_is_reg(const char *path);

/** Legacy: returns 1 if symlink, 0 if not (use lnx_path_is_symlink). */
LNX_API int lnx_file_is_symlink(const char *path);

/**
 * Read the target of a symbolic link.
 * @param path  The symlink path.
 * @param buf   Output buffer.
 * @param size  Buffer size.
 * @return Number of bytes written on success, -1 on error.
 */
LNX_API int lnx_file_read_link(const char *path, char *buf, size_t size);

/**
 * Resolve a path to its canonical absolute path.
 * @param path  The path to resolve.
 * @param buf   Output buffer.
 * @param size  Buffer size.
 * @return Number of bytes written on success, -1 on error.
 */
LNX_API int lnx_file_real_path(const char *path, char *buf, size_t size);

/**
 * Create a file if it does not exist, or update its timestamp.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_file_touch(const char *path);

/**
 * Copy permission bits from one file to another.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_file_copy_permissions(const char *src, const char *dst);

/**
 * Check file accessibility (wrapper around access(2)).
 * Returns 0 if allowed, -1 on error.
 */
LNX_API int lnx_file_access(const char *path, int mode);

/** Returns the file size in bytes, or -1 on error. */
LNX_API off_t lnx_file_size(const char *path);

/** Returns the file modification time as time_t, or -1 on error. */
LNX_API time_t lnx_file_mtime(const char *path);

/* =========================================================================
 * File Permissions
 * ========================================================================= */

/** File permission bits for lnx_file_chmod. */
typedef enum {
    LNX_PERM_OWNER_R   = 0400,
    LNX_PERM_OWNER_W   = 0200,
    LNX_PERM_OWNER_X   = 0100,
    LNX_PERM_GROUP_R   = 0040,
    LNX_PERM_GROUP_W   = 0020,
    LNX_PERM_GROUP_X   = 0010,
    LNX_PERM_OTHER_R   = 0004,
    LNX_PERM_OTHER_W   = 0002,
    LNX_PERM_OTHER_X   = 0001,
    LNX_PERM_SETUID    = 04000,
    LNX_PERM_SETGID    = 02000,
    LNX_PERM_STICKY    = 01000,
    LNX_PERM_DEFAULT   = 0644,
    LNX_PERM_SCRIPT    = 0755,
} lnx_perm_t;

/** Changes permissions of a file. Returns 0 on success, -1 on error. */
LNX_API int lnx_file_chmod(const char *path, unsigned int mode);

/** Returns the permissions of a file, or -1 on error. */
LNX_API int lnx_file_mode(const char *path);

/* =========================================================================
 * File I/O (Simple wrappers)
 * ========================================================================= */

/**
 * Read entire file into a heap-allocated buffer.
 * Returns NULL on error. Caller must free() the result.
 */
LNX_API char *lnx_file_read(const char *path);

/**
 * Read entire file into buffer of given size.
 * Returns the number of bytes read, or -1 on error.
 */
LNX_API ssize_t lnx_file_read_buf(const char *path, void *buf, size_t size);

/**
 * Write buffer to file, atomically replacing the destination.
 * Uses an atomic rename on Linux. Returns 0 on success, -1 on error.
 */
LNX_API int lnx_file_write(const char *path, const void *data, size_t len);

/**
 * Append data to a file, creating it if necessary.
 * Returns 0 on success, -1 on error.
 */
LNX_API int lnx_file_append(const char *path, const void *data, size_t len);

/* =========================================================================
 * File Locking
 * ========================================================================= */

/** File lock type for lnx_file_lock / lnx_file_unlock. */
typedef enum {
    LNX_LOCK_SHARED,  /** Read lock */
    LNX_LOCK_EXCLUSIVE, /** Write lock */
} lnx_lock_type_t;

/**
 * Acquire a file lock (advisory) on an open file descriptor.
 * Blocks until the lock is acquired. Returns 0 on success, -1 on error.
 */
LNX_API int lnx_file_lock(int fd, lnx_lock_type_t type);

/**
 * Try to acquire a file lock without blocking.
 * Returns 0 on success, -1 if lock is held by another process.
 */
LNX_API int lnx_file_try_lock(int fd, lnx_lock_type_t type);

/**
 * Release a file lock. Returns 0 on success, -1 on error.
 */
LNX_API int lnx_file_unlock(int fd);

/* =========================================================================
 * Temporary Files
 * ========================================================================= */

/**
 * Create a temporary file in the default tmp directory.
 * Returns a FILE*, or NULL on error.
 * The file is opened in "w+b" mode.
 */
LNX_API FILE *lnx_tmpfile(void);

/**
 * Create a temporary directory and return its path.
 * The path is heap-allocated — caller must free() it.
 * Returns NULL on error.
 */
LNX_API char *lnx_tmpdir(void);

#ifdef __cplusplus
}
#endif

#endif /* LNX_FILE_H */
