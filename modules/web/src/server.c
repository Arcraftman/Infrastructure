#include "web/server.h"
#include "web/mime.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __linux__
#  include <sys/epoll.h>
#  include <sys/sendfile.h>
#  define WEB_USE_EPOLL 1
#else
#  include <sys/select.h>
#  include <sys/time.h>
#  define WEB_USE_EPOLL 0
#endif

/* =========================================================================
 * Internal types
 * ========================================================================= */

#define WEB_BUF_SIZE 65536
#define WEB_MAX_EVENTS 1024
#define WEB_DEFAULT_BACKLOG 128
#define WEB_DEFAULT_TIMEOUT 60   /* seconds */
#define WEB_DEFAULT_KEEPALIVE 1

/* Per-connection state */
typedef enum {
    CONN_READING,
    CONN_WRITING,
    CONN_CLOSING,
    CONN_UPGRADED  /* WebSocket or other upgrade */
} conn_state_t;

struct web_connection {
    int              fd;
    conn_state_t     state;
    char            *buf;
    size_t           buf_len;
    size_t           buf_cap;
    char            *response;
    size_t           response_len;
    size_t           response_sent;
    web_server_t    *server;
    time_t           last_activity;
    int              keep_alive;
    struct web_connection *next;  /* hash chain or free list */
};

struct web_server {
    char             addr[64];
    int              port;
    int              backlog;
    int              threads;
    int              timeout;
    int              keepalive;
    int              listen_fd;
    volatile int     running;

    /* Routes */
    web_route_t     *routes;

    /* Static mount points */
    struct web_static {
        char *mount;
        size_t mount_len;
        char *dir;
        size_t dir_len;
        struct web_static *next;
    } *statics;

    /* Event loop */
#if WEB_USE_EPOLL
    int              epoll_fd;
    struct epoll_event events[WEB_MAX_EVENTS];
#endif
    fd_set           read_fds;
    fd_set           write_fds;
    int              max_fd;

    /* Connection tracking (simple array indexed by fd) */
    web_connection_t **conns;
    int              conn_cap;
};

/* =========================================================================
 * Helpers
 * ========================================================================= */

static int
set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int
set_reuseaddr(int fd)
{
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

/* =========================================================================
 * Connection management
 * ========================================================================= */

static web_connection_t *
conn_new(web_server_t *srv, int fd)
{
    web_connection_t *c = (web_connection_t *)calloc(1, sizeof(*c));
    if (!c) return NULL;
    c->fd = fd;
    c->state = CONN_READING;
    c->buf_cap = WEB_BUF_SIZE;
    c->buf = (char *)malloc(c->buf_cap);
    if (!c->buf) { free(c); return NULL; }
    c->server = srv;
    c->last_activity = time(NULL);
    c->keep_alive = srv->keepalive;

    /* Store in server's fd-indexed array */
    if (fd >= srv->conn_cap) {
        int new_cap = fd + 64;
        web_connection_t **newc = (web_connection_t **)realloc(
            srv->conns, (size_t)new_cap * sizeof(*newc));
        if (!newc) { free(c->buf); free(c); return NULL; }
        memset(newc + srv->conn_cap, 0,
               (size_t)(new_cap - srv->conn_cap) * sizeof(*newc));
        srv->conns = newc;
        srv->conn_cap = new_cap;
    }
    srv->conns[fd] = c;
    return c;
}

static void
conn_close(web_connection_t *c)
{
    if (!c) return;
    if (c->fd >= 0) {
        if (c->server && c->server->conns && c->fd < c->server->conn_cap)
            c->server->conns[c->fd] = NULL;
#if WEB_USE_EPOLL
        epoll_ctl(c->server->epoll_fd, EPOLL_CTL_DEL, c->fd, NULL);
#endif
        close(c->fd);
    }
    free(c->buf);
    free(c->response);
    free(c);
}

/* =========================================================================
 * Route matching
 * ========================================================================= */

static web_route_t *
find_route(web_server_t *srv, const char *method, const char *path)
{
    for (web_route_t *r = srv->routes; r; r = r->next) {
        /* Method match */
        int method_match = (strcmp(r->method, "*") == 0 ||
                            strcmp(r->method, method) == 0);
        if (!method_match) continue;

        /* Path match: exact, or wildcard suffix */
        size_t plen = strlen(path);
        size_t rlen = strlen(r->path);
        if (rlen > 0 && r->path[rlen - 1] == '*') {
            /* Prefix match */
            if (strncmp(r->path, path, rlen - 1) == 0)
                return r;
        } else if (plen == rlen && memcmp(r->path, path, rlen) == 0) {
            return r;
        }
        /* Trailng slash normalization */
        if (plen > 0 && path[plen - 1] == '/' && rlen == plen - 1 &&
            memcmp(r->path, path, rlen) == 0)
            return r;
        if (rlen > 0 && r->path[rlen - 1] == '/' && plen == rlen - 1 &&
            memcmp(r->path, path, plen) == 0)
            return r;
    }
    return NULL;
}

/* =========================================================================
 * Static file serving
 * ========================================================================= */

static web_response_t *
serve_static_file(web_server_t *srv, const char *path)
{
    /* Find matching static mount */
    struct web_static *st;
    const char *rel = NULL;
    for (st = srv->statics; st; st = st->next) {
        if (strncmp(st->mount, path, st->mount_len) == 0) {
            rel = path + st->mount_len;
            break;
        }
    }
    if (!st) return NULL;

    /* Build filesystem path */
    char fspath[4096];
    int n = snprintf(fspath, sizeof(fspath), "%s%s", st->dir, rel ? rel : "");
    if (n < 0 || (size_t)n >= sizeof(fspath))
        return web_response_error(WEB_STATUS_NOT_FOUND);

    /* Open file */
    FILE *fp = fopen(fspath, "rb");
    if (!fp) return web_response_error(WEB_STATUS_NOT_FOUND);

    /* Stat for size */
    struct stat stbuf;
    if (fstat(fileno(fp), &stbuf) < 0) {
        fclose(fp);
        return web_response_error(WEB_STATUS_NOT_FOUND);
    }

    /* Read into buffer */
    size_t size = stbuf.st_size > 0 ? (size_t)stbuf.st_size : 0;
    unsigned char *content = (unsigned char *)malloc(size ? size : 1);
    if (!content) { fclose(fp); return web_response_error(WEB_STATUS_INTERNAL_ERROR); }

    if (size > 0 && fread(content, 1, size, fp) != size) {
        fclose(fp);
        free(content);
        return web_response_error(WEB_STATUS_INTERNAL_ERROR);
    }
    fclose(fp);

    /* Build response */
    web_response_t *resp = web_response_new(WEB_STATUS_OK);
    if (!resp) { free(content); return NULL; }

    const char *mime = web_mime_by_path(fspath);
    web_response_set_header(resp, "Content-Type", mime);
    web_response_set_body(resp, content, size);
    resp->owns_body = 1;
    return resp;
}

/* =========================================================================
 * Request handling
 * ========================================================================= */

static void
handle_connection(web_connection_t *c)
{
    web_server_t *srv = c->server;

    /* Parse request */
    web_request_t *req = NULL;
    if (web_request_parse(c->buf, c->buf_len, &req) < 0) {
        web_response_t *err = web_response_error(WEB_STATUS_BAD_REQUEST);
        if (err) {
            free(c->response);
            c->response = web_response_format(err, &c->response_len);
            c->response_sent = 0;
            c->state = CONN_WRITING;
            web_response_destroy(err);
        } else {
            c->state = CONN_CLOSING;
        }
        return;
    }

    /* Find route */
    const char *method = web_method_str(req->method);
    web_response_t *resp = NULL;

    if (!method) {
        resp = web_response_error(WEB_STATUS_NOT_IMPLEMENTED);
    } else {
        /* Determine keep-alive from request */
        const char *conn_hdr = web_request_header(req, "Connection");
        if (srv->keepalive) {
            if (conn_hdr && strcasecmp(conn_hdr, "close") == 0)
                c->keep_alive = 0;
            else
                c->keep_alive = 1; /* HTTP/1.1 default is keep-alive */
        } else {
            c->keep_alive = 0;
        }

        web_route_t *route = find_route(srv, method, req->path);
        if (route) {
            resp = route->handler(req, route->userdata);
        } else {
            /* Try static file */
            resp = serve_static_file(srv, req->path);
            if (!resp)
                resp = web_response_error(WEB_STATUS_NOT_FOUND);
        }
    }

    web_request_destroy(req);

    if (resp) {
        free(c->response);
        c->response = web_response_format(resp, &c->response_len);
        c->response_sent = 0;
        c->state = CONN_WRITING;
        web_response_destroy(resp);
    } else {
        c->state = CONN_CLOSING;
    }
}

/* =========================================================================
 * Event loop logic
 * ========================================================================= */

static int
accept_connection(web_server_t *srv)
{
    struct sockaddr_in6 client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int client_fd = accept(srv->listen_fd,
                           (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("accept");
        return -1;
    }

    set_nonblock(client_fd);

    web_connection_t *c = conn_new(srv, client_fd);
    if (!c) {
        close(client_fd);
        return -1;
    }

#if WEB_USE_EPOLL
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;
    if (epoll_ctl(srv->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl ADD");
        conn_close(c);
        return -1;
    }
#else
    if (client_fd >= FD_SETSIZE) {
        conn_close(c);
        return -1;
    }
#endif

    return 0;
}

static int
read_request(web_connection_t *c)
{
    ssize_t n;
    do {
        if (c->buf_len >= c->buf_cap) {
            c->buf_cap *= 2;
            char *nb = (char *)realloc(c->buf, c->buf_cap);
            if (!nb) return -1;
            c->buf = nb;
        }
        n = read(c->fd, c->buf + c->buf_len,
                 c->buf_cap - c->buf_len - 1);
        if (n > 0) {
            c->buf_len += (size_t)n;
        }
    } while (n > 0);

    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        return -1;

    /* Update activity timestamp */
    c->last_activity = time(NULL);

    /* Check if we have a complete request */
    if (c->buf_len > 4 &&
        memcmp(c->buf + c->buf_len - 4, "\r\n\r\n", 4) == 0) {
        c->buf[c->buf_len] = '\0';
        handle_connection(c);
        return 0;
    }
    /* Also handle single \n\n */
    if (c->buf_len > 2 &&
        memcmp(c->buf + c->buf_len - 2, "\n\n", 2) == 0) {
        c->buf[c->buf_len] = '\0';
        handle_connection(c);
        return 0;
    }

    return 0; /* still reading */
}

static int
write_response(web_connection_t *c)
{
    if (!c->response || c->response_sent >= c->response_len) {
        /* Response fully sent — check keep-alive */
        if (c->keep_alive && c->server->keepalive) {
            /* Reset for next request */
            c->buf_len = 0;
            c->response_sent = 0;
            free(c->response);
            c->response = NULL;
            c->response_len = 0;
            c->state = CONN_READING;
            c->last_activity = time(NULL);
#if WEB_USE_EPOLL
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = c->fd;
            epoll_ctl(c->server->epoll_fd, EPOLL_CTL_MOD, c->fd, &ev);
#endif
            return 0;
        }
        c->state = CONN_CLOSING;
        return 0;
    }

    ssize_t n = write(c->fd,
                      c->response + c->response_sent,
                      c->response_len - c->response_sent);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        return -1;
    }
    c->response_sent += (size_t)n;

    if (c->response_sent >= c->response_len) {
        c->last_activity = time(NULL);
        /* keep-alive check happens at top of next call */
    }
    return 0;
}

#if WEB_USE_EPOLL

static void
event_loop_epoll(web_server_t *srv)
{
    while (srv->running) {
        /* Check for timed-out connections */
        if (srv->timeout > 0) {
            time_t now = time(NULL);
            for (int i = 0; i < srv->conn_cap; i++) {
                web_connection_t *c = srv->conns[i];
                if (c && c->state != CONN_CLOSING &&
                    difftime(now, c->last_activity) > srv->timeout) {
                    conn_close(c);
                }
            }
        }

        int nfds = epoll_wait(srv->epoll_fd, srv->events,
                              WEB_MAX_EVENTS, 1000);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            int fd = srv->events[i].data.fd;

            if (fd == srv->listen_fd) {
                accept_connection(srv);
            } else {
                web_connection_t *c = (fd < srv->conn_cap) ? srv->conns[fd] : NULL;
                if (!c) continue;

                if (srv->events[i].events & EPOLLIN) {
                    if (read_request(c) < 0) {
                        conn_close(c);
                        continue;
                    }
                }

                if (srv->events[i].events & EPOLLOUT) {
                    if (write_response(c) < 0) {
                        conn_close(c);
                        continue;
                    }
                }

                if (c->state == CONN_CLOSING) {
                    conn_close(c);
                }
            }
        }
    }
}

#else /* select-based fallback */

static void
event_loop_select(web_server_t *srv)
{
    while (srv->running) {
        /* Check for timed-out connections */
        if (srv->timeout > 0) {
            time_t now = time(NULL);
            for (int i = 0; i < srv->conn_cap; i++) {
                web_connection_t *c = srv->conns[i];
                if (c && c->state != CONN_CLOSING &&
                    difftime(now, c->last_activity) > srv->timeout) {
                    conn_close(c);
                }
            }
        }

        FD_ZERO(&srv->read_fds);
        FD_ZERO(&srv->write_fds);
        FD_SET(srv->listen_fd, &srv->read_fds);
        int max_fd = srv->listen_fd;

        for (int i = 0; i < srv->conn_cap; i++) {
            web_connection_t *c = srv->conns[i];
            if (!c) continue;
            if (c->state == CONN_READING) {
                FD_SET(c->fd, &srv->read_fds);
            } else if (c->state == CONN_WRITING) {
                FD_SET(c->fd, &srv->write_fds);
            }
            if (c->fd > max_fd) max_fd = c->fd;
        }

        struct timeval tv = { 1, 0 };
        int ret = select(max_fd + 1, &srv->read_fds, &srv->write_fds,
                         NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (ret == 0) continue;

        /* Accept new */
        if (FD_ISSET(srv->listen_fd, &srv->read_fds))
            accept_connection(srv);

        /* Read/write existing */
        for (int i = 0; i < srv->conn_cap; i++) {
            web_connection_t *c = srv->conns[i];
            if (!c) continue;

            if (c->state == CONN_READING &&
                FD_ISSET(c->fd, &srv->read_fds)) {
                if (read_request(c) < 0) { conn_close(c); continue; }
            }
            if (c->state == CONN_WRITING &&
                FD_ISSET(c->fd, &srv->write_fds)) {
                if (write_response(c) < 0) { conn_close(c); continue; }
            }
            if (c->state == CONN_CLOSING)
                conn_close(c);
        }
    }
}

#endif /* WEB_USE_EPOLL */

/* =========================================================================
 * Server API
 * ========================================================================= */

WEB_API web_server_t *
web_server_create(const char *addr, int port)
{
    web_server_t *srv = (web_server_t *)calloc(1, sizeof(*srv));
    if (!srv) return NULL;

    if (addr)
        snprintf(srv->addr, sizeof(srv->addr), "%s", addr);
    else
        snprintf(srv->addr, sizeof(srv->addr), "0.0.0.0");

    srv->port = port;
    srv->backlog = WEB_DEFAULT_BACKLOG;
    srv->threads = 0;
    srv->timeout = WEB_DEFAULT_TIMEOUT;
    srv->keepalive = WEB_DEFAULT_KEEPALIVE;
    srv->listen_fd = -1;

    return srv;
}

WEB_API int
web_server_set_backlog(web_server_t *srv, int backlog)
{
    if (!srv) return -1;
    srv->backlog = backlog;
    return 0;
}

WEB_API int
web_server_set_threads(web_server_t *srv, int threads)
{
    if (!srv) return -1;
    srv->threads = threads;
    return 0;
}

WEB_API int
web_server_set_timeout(web_server_t *srv, int seconds)
{
    if (!srv || seconds < 0) return -1;
    srv->timeout = seconds;
    return 0;
}

WEB_API int
web_server_set_keepalive(web_server_t *srv, int enabled)
{
    if (!srv) return -1;
    srv->keepalive = !!enabled;
    return 0;
}

/* Graceful shutdown signal handler */
static volatile int g_srv_shutdown = 0;

static void
sig_shutdown_handler(int signo)
{
    (void)signo;
    g_srv_shutdown = 1;
}

WEB_API int
web_server_add_route(web_server_t *srv, const char *method,
                     const char *path, web_handler_fn handler,
                     void *userdata)
{
    return web_server_add_route_ex(srv, method, path, handler,
                                   userdata, NULL);
}

WEB_API int
web_server_add_route_ex(web_server_t *srv, const char *method,
                        const char *path, web_handler_fn handler,
                        void *userdata, web_handler_dtor dtor)
{
    if (!srv || !method || !path || !handler) return -1;

    web_route_t *r = (web_route_t *)calloc(1, sizeof(*r));
    if (!r) return -1;

    r->method = strdup(method);
    r->path   = strdup(path);

    /* Normalize: remove trailing '/' except for "/" */
    size_t plen = strlen(r->path);
    if (plen > 1 && r->path[plen - 1] == '/')
        r->path[plen - 1] = '\0';

    r->handler = handler;
    r->userdata = userdata;
    r->userdata_dtor = dtor;

    /* Prepend to list */
    r->next = srv->routes;
    srv->routes = r;
    return 0;
}

WEB_API int
web_server_add_static(web_server_t *srv, const char *mount,
                      const char *dir)
{
    if (!srv || !mount || !dir) return -1;

    struct web_static *st = (struct web_static *)
        calloc(1, sizeof(*st));
    if (!st) return -1;

    st->mount = strdup(mount);
    st->mount_len = strlen(mount);
    /* Remove trailing / from mount for clean matching */
    while (st->mount_len > 1 && st->mount[st->mount_len - 1] == '/') {
        st->mount[--st->mount_len] = '\0';
    }
    st->dir = strdup(dir);
    st->dir_len = strlen(dir);
    /* Remove trailing / from dir */
    while (st->dir_len > 1 && st->dir[st->dir_len - 1] == '/') {
        st->dir[--st->dir_len] = '\0';
    }

    st->next = srv->statics;
    srv->statics = st;
    return 0;
}

WEB_API int
web_server_start(web_server_t *srv)
{
    if (!srv) return -1;

    /* Ignore SIGPIPE so we can detect write errors via EPIPE */
    signal(SIGPIPE, SIG_IGN);

    /* Graceful shutdown on SIGINT / SIGTERM */
    g_srv_shutdown = 0;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_shutdown_handler;
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Create socket */
    int fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (fd < 0) {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) { perror("socket"); return -1; }
    }

    set_reuseaddr(fd);
    set_nonblock(fd);

    /* Bind */
    struct sockaddr_in6 addr6;
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_port   = htons((uint16_t)srv->port);

    int use_ipv6 = 1;
    if (strchr(srv->addr, ':') || strcmp(srv->addr, "::1") == 0) {
        inet_pton(AF_INET6, srv->addr, &addr6.sin6_addr);
    } else {
        use_ipv6 = 0;
    }

    if (use_ipv6) {
        if (bind(fd, (struct sockaddr *)&addr6, sizeof(addr6)) < 0) {
            perror("bind IPv6");
            close(fd);
            return -1;
        }
    } else {
        struct sockaddr_in addr4;
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_port   = htons((uint16_t)srv->port);
        inet_pton(AF_INET, srv->addr, &addr4.sin_addr);
        if (bind(fd, (struct sockaddr *)&addr4, sizeof(addr4)) < 0) {
            perror("bind IPv4");
            close(fd);
            return -1;
        }
    }

    if (listen(fd, srv->backlog) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    srv->listen_fd = fd;

#if WEB_USE_EPOLL
    srv->epoll_fd = epoll_create1(0);
    if (srv->epoll_fd < 0) {
        perror("epoll_create1");
        close(fd);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(srv->epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll_ctl ADD listen");
        close(fd);
        close(srv->epoll_fd);
        return -1;
    }
#endif

    srv->running = 1;
    printf("Server listening on %s:%d\n", srv->addr, srv->port);

#if WEB_USE_EPOLL
    event_loop_epoll(srv);
#else
    event_loop_select(srv);
#endif

    return 0;
}

WEB_API void
web_server_stop(web_server_t *srv)
{
    if (srv) srv->running = 0;
}

WEB_API void
web_server_destroy(web_server_t *srv)
{
    if (!srv) return;

    srv->running = 0;

    /* Close all connections */
    if (srv->conns) {
        for (int i = 0; i < srv->conn_cap; i++) {
            if (srv->conns[i])
                conn_close(srv->conns[i]);
        }
        free(srv->conns);
    }

    /* Close listen fd */
    if (srv->listen_fd >= 0) close(srv->listen_fd);
#if WEB_USE_EPOLL
    if (srv->epoll_fd >= 0) close(srv->epoll_fd);
#endif

    /* Free routes */
    web_route_t *r = srv->routes;
    while (r) {
        web_route_t *next = r->next;
        free(r->method);
        free(r->path);
        if (r->userdata_dtor && r->userdata)
            r->userdata_dtor(r->userdata);
        free(r);
        r = next;
    }

    /* Free static mounts */
    struct web_static *st = srv->statics;
    while (st) {
        struct web_static *next = st->next;
        free(st->mount);
        free(st->dir);
        free(st);
        st = next;
    }

    free(srv);
}
