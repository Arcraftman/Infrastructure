#include "lnx/def.h"
#include "lnx/file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * File Path Utilities
 * ========================================================================= */

LNX_API int
lnx_fd_path(int fd, char *buf, size_t size)
{
    char proc[64];
    int n = snprintf(proc, sizeof(proc), "/proc/self/fd/%d", fd);
    if (n < 0 || (size_t)n >= sizeof(proc))
        return -1;
    ssize_t len = readlink(proc, buf, size - 1);
    if (len < 0)
        return -1;
    buf[len] = '\0';
    return (int)len;
}

LNX_API bool
lnx_path_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

LNX_API bool
lnx_path_is_file(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return false;
    return S_ISREG(st.st_mode) ? true : false;
}

LNX_API bool
lnx_path_is_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return false;
    return S_ISDIR(st.st_mode) ? true : false;
}

LNX_API bool
lnx_path_is_symlink(const char *path)
{
    struct stat st;
    if (lstat(path, &st) != 0)
        return false;
    return S_ISLNK(st.st_mode) ? true : false;
}

/* =========================================================================
 * File Metadata
 * ========================================================================= */

LNX_API off_t
lnx_file_size(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return -1;
    return st.st_size;
}

LNX_API time_t
lnx_file_mtime(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return -1;
    return st.st_mtime;
}

/* =========================================================================
 * File Permissions
 * ========================================================================= */

LNX_API int
lnx_file_chmod(const char *path, unsigned int mode)
{
    return chmod(path, mode);
}

LNX_API int
lnx_file_mode(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return -1;
    return st.st_mode & 07777;
}

/* =========================================================================
 * File I/O (Simple wrappers)
 * ========================================================================= */

LNX_API char *
lnx_file_read(const char *path)
{
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return NULL;

    off_t sz = lseek(fd, 0, SEEK_END);
    if (sz < 0) { close(fd); return NULL; }
    if (lseek(fd, 0, SEEK_SET) < 0) { close(fd); return NULL; }

    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { close(fd); return NULL; }

    size_t remain = (size_t)sz;
    size_t off = 0;
    while (remain > 0) {
        ssize_t r = read(fd, buf + off, remain);
        if (r < 0) {
            if (errno == EINTR) continue;
            free(buf); close(fd); return NULL;
        }
        if (r == 0) break; /* EOF before expected */
        off += (size_t)r;
        remain -= (size_t)r;
    }
    buf[off] = '\0';
    close(fd);
    return buf;
}

LNX_API ssize_t
lnx_file_read_buf(const char *path, void *buf, size_t size)
{
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return -1;

    size_t total = 0;
    while (total < size) {
        ssize_t r = read(fd, (char *)buf + total, size - total);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (r == 0) break;
        total += (size_t)r;
    }
    close(fd);
    return (ssize_t)total;
}

LNX_API int
lnx_file_write(const char *path, const void *data, size_t len)
{
    /* Atomic write: write to temp file, then rename */
    char tmp[4096];
    int n = snprintf(tmp, sizeof(tmp), "%s.tmp.XXXXXX", path);
    if (n < 0 || (size_t)n >= sizeof(tmp))
        return -1;

    int fd = mkstemp(tmp);
    if (fd < 0)
        return -1;

    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, (const char *)data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd); unlink(tmp);
            return -1;
        }
        off += (size_t)w;
    }
    close(fd);

    if (rename(tmp, path) != 0) {
        unlink(tmp);
        return -1;
    }
    return 0;
}

LNX_API int
lnx_file_append(const char *path, const void *data, size_t len)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (fd < 0)
        return -1;

    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, (const char *)data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        off += (size_t)w;
    }
    close(fd);
    return 0;
}

/* =========================================================================
 * File Locking
 * ========================================================================= */

static int
lock_type_to_flock_cmd(lnx_lock_type_t type, int block)
{
    switch (type) {
        case LNX_LOCK_SHARED:
            return block ? F_RDLCK : F_SETLKW;
        case LNX_LOCK_EXCLUSIVE:
            return block ? F_WRLCK : F_SETLKW;
    }
    return F_RDLCK;
}

LNX_API int
lnx_file_lock(int fd, lnx_lock_type_t type)
{
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = (type == LNX_LOCK_SHARED) ? F_RDLCK : F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return fcntl(fd, F_SETLKW, &fl);
}

LNX_API int
lnx_file_try_lock(int fd, lnx_lock_type_t type)
{
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = (type == LNX_LOCK_SHARED) ? F_RDLCK : F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}

LNX_API int
lnx_file_unlock(int fd)
{
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}

/* =========================================================================
 * Temporary Files
 * ========================================================================= */

LNX_API FILE *
lnx_tmpfile(void)
{
    int fd;
    FILE *fp;

    /* Try mkstemp in TMPDIR first, fallback to /tmp */
    const char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";

    char tmpl[4096];
    int n = snprintf(tmpl, sizeof(tmpl), "%s/lnx_tmp_XXXXXX", tmpdir);
    if (n < 0 || (size_t)n >= sizeof(tmpl))
        return NULL;

    fd = mkstemp(tmpl);
    if (fd < 0)
        return NULL;

    /* Immediately unlink so it's anonymous */
    unlink(tmpl);

    fp = fdopen(fd, "w+b");
    if (!fp) {
        close(fd);
        return NULL;
    }
    return fp;
}

LNX_API char *
lnx_tmpdir(void)
{
    const char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";

    char tmpl[4096];
    int n = snprintf(tmpl, sizeof(tmpl), "%s/lnx_dir_XXXXXX", tmpdir);
    if (n < 0 || (size_t)n >= sizeof(tmpl))
        return NULL;

    char *path = strdup(tmpl);
    if (!path)
        return NULL;

    if (!mkdtemp(path)) {
        free(path);
        return NULL;
    }
    return path;
}

/* =========================================================================
 * Extended API — additional helpers from original file.c
 * ========================================================================= */

LNX_API int
lnx_file_exists(const char *path)
{
    return lnx_path_exists(path) ? 1 : 0;
}

LNX_API int
lnx_file_is_dir(const char *path)
{
    return lnx_path_is_dir(path) ? 1 : 0;
}

LNX_API int
lnx_file_is_reg(const char *path)
{
    return lnx_path_is_file(path) ? 1 : 0;
}

LNX_API int
lnx_file_is_symlink(const char *path)
{
    return lnx_path_is_symlink(path) ? 1 : 0;
}

LNX_API int
lnx_file_read_link(const char *path, char *buf, size_t size)
{
    ssize_t n = readlink(path, buf, size - 1);
    if (n < 0)
        return -1;
    buf[n] = '\0';
    return (int)n;
}

LNX_API int
lnx_file_real_path(const char *path, char *buf, size_t size)
{
    char *resolved = realpath(path, NULL);
    if (!resolved)
        return -1;
    int len = (int)strlen(resolved);
    if ((size_t)len >= size) {
        free(resolved);
        return -1;
    }
    strcpy(buf, resolved);
    free(resolved);
    return len;
}

LNX_API int
lnx_file_touch(const char *path)
{
    int fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd < 0)
        return -1;
    close(fd);
    return 0;
}

LNX_API int
lnx_file_copy_permissions(const char *src, const char *dst)
{
    struct stat st;
    if (stat(src, &st) != 0)
        return -1;
    return chmod(dst, st.st_mode & 07777);
}

LNX_API int
lnx_file_access(const char *path, int mode)
{
    return access(path, mode);
}
