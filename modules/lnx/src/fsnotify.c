#include "lnx/def.h"
#include "lnx/fsnotify.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LNX_FSNOTIFY_BUF_SIZE (sizeof(struct inotify_event) + NAME_MAX + 1)

struct lnx_fsnotify {
    int fd;
};

LNX_API lnx_fsnotify_t *
lnx_fsnotify_create(void)
{
    lnx_fsnotify_t *nf = calloc(1, sizeof(*nf));
    if (!nf)
        return NULL;
    nf->fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    if (nf->fd < 0) {
        free(nf);
        return NULL;
    }
    return nf;
}

LNX_API void
lnx_fsnotify_destroy(lnx_fsnotify_t *nf)
{
    if (!nf)
        return;
    close(nf->fd);
    free(nf);
}

LNX_API int
lnx_fsnotify_fd(lnx_fsnotify_t *nf)
{
    return nf ? nf->fd : -1;
}

LNX_API int
lnx_fsnotify_add_watch(lnx_fsnotify_t *nf, const char *path,
                        uint32_t mask)
{
    if (!nf)
        return -1;
    return inotify_add_watch(nf->fd, path, mask);
}

LNX_API int
lnx_fsnotify_rm_watch(lnx_fsnotify_t *nf, int wd)
{
    if (!nf)
        return -1;
    return inotify_rm_watch(nf->fd, wd);
}

LNX_API int
lnx_fsnotify_read(lnx_fsnotify_t *nf, lnx_fsnotify_event_t *events, int count)
{
    if (!nf || !events || count <= 0)
        return -1;

    unsigned char buf[LNX_FSNOTIFY_BUF_SIZE * 4];
    ssize_t n = read(nf->fd, buf, sizeof(buf));
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        return -1;
    }

    int idx = 0;
    size_t off = 0;
    while ((size_t)off < (size_t)n && idx < count) {
        struct inotify_event *ie = (struct inotify_event *)(buf + off);
        lnx_fsnotify_event_t *ev = &events[idx];
        ev->wd       = ie->wd;
        ev->mask     = ie->mask;
        ev->cookie   = ie->cookie;
        ev->name_len = ie->len;
        if (ie->len > 0) {
            ev->name = strdup(ie->name);
            if (!ev->name)
                return -1;
        } else {
            ev->name = NULL;
        }
        off += sizeof(struct inotify_event) + ie->len;
        idx++;
    }
    return idx;
}
