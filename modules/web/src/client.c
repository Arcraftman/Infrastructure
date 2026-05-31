#include "web/client.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

/* =========================================================================
 * Internal types
 * ========================================================================= */

#define CLIENT_BUF_SIZE 32768

typedef struct client_header {
    char *name;
    char *value;
    struct client_header *next;
} client_header_t;

struct web_client {
    char       *host;
    int         port;
    int         use_ssl;   /* reserved for future use */
    int         fd;
    int         timeout_ms;

    /* Request headers */
    client_header_t *req_headers;

    /* Response */
    int         status;
    client_header_t *resp_headers;
    unsigned char   *body;
    size_t      body_len;
    size_t      body_cap;
};

/* =========================================================================
 * Socket helpers
 * ========================================================================= */

static int
connect_with_timeout(const char *host, int port, int timeout_ms)
{
    struct addrinfo hints, *ai, *rp;
    char port_str[16];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = 0;

    snprintf(port_str, sizeof(port_str), "%d", port);
    int ret = getaddrinfo(host, port_str, &hints, &ai);
    if (ret != 0) return -1;

    int fd = -1;
    for (rp = ai; rp; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol);
        if (fd < 0) continue;

        /* Connect with poll-based timeout */
        connect(fd, rp->ai_addr, rp->ai_addrlen);

        struct pollfd pfd;
        pfd.fd     = fd;
        pfd.events = POLLOUT;
        ret = poll(&pfd, 1, timeout_ms > 0 ? timeout_ms : 5000);

        if (ret <= 0) {
            close(fd);
            fd = -1;
            continue;
        }

        /* Check connection result via SO_ERROR */
        int so_err = 0;
        socklen_t err_len = sizeof(so_err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_err, &err_len);
        if (so_err != 0) {
            close(fd);
            fd = -1;
            continue;
        }

        /* Restore blocking for simplicity */
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags != -1)
            fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        break;
    }

    freeaddrinfo(ai);
    return fd;
}

static client_header_t *
header_list_append(client_header_t **list, const char *name, const char *value)
{
    client_header_t *h = (client_header_t *)malloc(sizeof(*h));
    if (!h) return NULL;
    h->name  = strdup(name);
    h->value = strdup(value);
    if (!h->name || !h->value) {
        free(h->name);
        free(h->value);
        free(h);
        return NULL;
    }
    h->next = NULL;

    /* Append */
    while (*list) list = &(*list)->next;
    *list = h;
    return h;
}

static void
header_list_free(client_header_t *list)
{
    while (list) {
        client_header_t *next = list->next;
        free(list->name);
        free(list->value);
        free(list);
        list = next;
    }
}

static const char *
header_list_find(const client_header_t *list, const char *name)
{
    for (const client_header_t *h = list; h; h = h->next)
        if (strcasecmp(h->name, name) == 0)
            return h->value;
    return NULL;
}

/* =========================================================================
 * Response parsing
 * ========================================================================= */

static int
parse_status_line(const char *line, int *status)
{
    /* Expect "HTTP/1.x STATUS ..." */
    if (strncmp(line, "HTTP/", 5) != 0) return -1;
    const char *p = line + 5;
    /* skip version */
    while (*p && *p != ' ') p++;
    if (!*p) return -1;
    while (*p == ' ') p++;
    *status = (int)strtol(p, NULL, 10);
    return 0;
}

static int
parse_response(const char *buf, size_t len, int *status,
               client_header_t **headers, unsigned char **body, size_t *body_len)
{
    *headers = NULL;
    *body    = NULL;
    *body_len = 0;

    /* Find double CRLF separating headers from body */
    const char *crlf = buf;
    const char *header_end = NULL;
    while (crlf && (size_t)(crlf - buf) < len - 3) {
        crlf = strstr(crlf, "\r\n");
        if (!crlf) break;
        /* Check for empty line */
        if (crlf + 2 < buf + len && crlf[2] == '\r' && crlf[3] == '\n') {
            header_end = crlf;
            break;
        }
        if (crlf + 1 < buf + len && crlf[2] == '\n') {
            header_end = crlf;
            break;
        }
        crlf += 2;
    }
    if (!header_end) return -1;

    /* Parse status line */
    size_t status_line_len = (size_t)(header_end - buf);
    char *status_line = (char *)malloc(status_line_len + 1);
    if (!status_line) return -1;
    memcpy(status_line, buf, status_line_len);
    status_line[status_line_len] = '\0';

    char *nl = strchr(status_line, '\n');
    if (nl) *nl = '\0';
    char *cr = strchr(status_line, '\r');
    if (cr) *cr = '\0';

    if (parse_status_line(status_line, status) != 0) {
        free(status_line);
        return -1;
    }
    free(status_line);

    /* Parse headers */
    const char *hp = buf;
    /* Skip status line */
    while (hp < buf + len && *hp != '\n') hp++;
    if (hp < buf + len) hp++;

    while (hp < (const char *)header_end) {
        const char *eol = strchr(hp, '\n');
        if (!eol) eol = header_end;
        const char *cr2 = memchr(hp, '\r', (size_t)(eol - hp));
        size_t line_len = cr2 ? (size_t)(cr2 - hp) : (size_t)(eol - hp);

        if (line_len > 0) {
            const char *colon = memchr(hp, ':', line_len);
            if (colon) {
                size_t name_len = (size_t)(colon - hp);
                char *name = (char *)malloc(name_len + 1);
                memcpy(name, hp, name_len);
                name[name_len] = '\0';

                const char *val_start = colon + 1;
                while (val_start < hp + line_len && *val_start == ' ')
                    val_start++;
                size_t val_len = (size_t)(hp + line_len - val_start);
                char *value = (char *)malloc(val_len + 1);
                memcpy(value, val_start, val_len);
                value[val_len] = '\0';

                header_list_append(headers, name, value);
                free(name);
                free(value);
            }
        }
        hp = eol + 1;
    }

    /* Body */
    const char *body_start = header_end + 4; /* past \r\n\r\n */
    if (body_start < buf + len) {
        size_t remaining = (size_t)(buf + len - body_start);
        *body = (unsigned char *)malloc(remaining + 1);
        if (*body) {
            memcpy(*body, body_start, remaining);
            (*body)[remaining] = '\0';
            *body_len = remaining;
        }
    }
    return 0;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

WEB_API web_client_t *
web_client_create(const char *host, int port, int use_ssl)
{
    if (!host || port <= 0 || port > 65535) {
        errno = EINVAL;
        return NULL;
    }

    web_client_t *c = (web_client_t *)calloc(1, sizeof(*c));
    if (!c) return NULL;

    c->host    = strdup(host);
    if (!c->host) { free(c); return NULL; }
    c->port    = port;
    c->use_ssl = use_ssl;
    c->fd      = -1;
    c->timeout_ms = 5000;
    return c;
}

WEB_API int
web_client_set_timeout(web_client_t *client, int timeout_sec)
{
    if (!client) return -1;
    client->timeout_ms = timeout_sec > 0 ? timeout_sec * 1000 : -1;
    return 0;
}

WEB_API int
web_client_set_header(web_client_t *client, const char *name, const char *value)
{
    if (!client || !name || !value) return -1;
    return header_list_append(&client->req_headers, name, value) ? 0 : -1;
}

WEB_API int
web_client_request(web_client_t *client, const char *method, const char *path,
                   const void *body, size_t body_len,
                   web_client_cb cb, void *userdata)
{
    if (!client || !method || !path) return -1;

    /* Close previous connection if any */
    if (client->fd >= 0) {
        close(client->fd);
        client->fd = -1;
    }
    header_list_free(client->resp_headers);
    client->resp_headers = NULL;
    free(client->body);
    client->body = NULL;
    client->body_len = 0;
    client->body_cap = 0;
    client->status    = 0;

    /* Connect */
    client->fd = connect_with_timeout(client->host, client->port, client->timeout_ms);
    if (client->fd < 0) return -1;

    /* Build request */
    char *request = NULL;
    size_t req_len = 0;
    {
        /* First line: METHOD path HTTP/1.1\r\n */
        const char *fmt = "%s %s HTTP/1.1\r\n";
        int n = snprintf(NULL, 0, fmt, method, path);
        request = (char *)malloc((size_t)n + 1);
        if (!request) return -1;
        snprintf(request, (size_t)n + 1, fmt, method, path);
        req_len = (size_t)n;

        /* Host header */
        char hostport[256];
        snprintf(hostport, sizeof(hostport), "%s:%d", client->host, client->port);
        n = snprintf(NULL, 0, "Host: %s\r\n", hostport);
        char *hdr = (char *)malloc((size_t)n + 1);
        if (!hdr) { free(request); return -1; }
        snprintf(hdr, (size_t)n + 1, "Host: %s\r\n", hostport);
        request = (char *)realloc(request, req_len + (size_t)n + 1);
        memcpy(request + req_len, hdr, (size_t)n);
        req_len += (size_t)n;
        free(hdr);

        /* User headers */
        for (client_header_t *h = client->req_headers; h; h = h->next) {
            n = snprintf(NULL, 0, "%s: %s\r\n", h->name, h->value);
            request = (char *)realloc(request, req_len + (size_t)n + 1);
            snprintf(request + req_len, (size_t)n + 1, "%s: %s\r\n", h->name, h->value);
            req_len += (size_t)n;
        }

        /* Content-Length */
        if (body && body_len > 0) {
            n = snprintf(NULL, 0, "Content-Length: %zu\r\n", body_len);
            request = (char *)realloc(request, req_len + (size_t)n + 1);
            snprintf(request + req_len, (size_t)n + 1, "Content-Length: %zu\r\n", body_len);
            req_len += (size_t)n;
        }

        /* Header terminator */
        request = (char *)realloc(request, req_len + 3);
        memcpy(request + req_len, "\r\n", 2);
        req_len += 2;

        /* Body */
        if (body && body_len > 0) {
            request = (char *)realloc(request, req_len + body_len);
            memcpy(request + req_len, body, body_len);
            req_len += body_len;
        }
        request[req_len] = '\0';
    }

    /* Send */
    size_t sent = 0;
    while (sent < req_len) {
        ssize_t n = write(client->fd, request + sent, req_len - sent);
        if (n <= 0) {
            free(request);
            return -1;
        }
        sent += (size_t)n;
    }
    free(request);

    /* Receive response */
    size_t cap = CLIENT_BUF_SIZE;
    size_t off = 0;
    char *resp_buf = (char *)malloc(cap);
    if (!resp_buf) return -1;

    while (1) {
        if (off >= cap) {
            cap *= 2;
            resp_buf = (char *)realloc(resp_buf, cap);
            if (!resp_buf) return -1;
        }
        ssize_t n = read(client->fd, resp_buf + off, cap - off);
        if (n <= 0) break;
        off += (size_t)n;
    }

    /* Parse */
    int ret = parse_response(resp_buf, off, &client->status,
                             &client->resp_headers,
                             &client->body, &client->body_len);
    free(resp_buf);
    if (ret != 0) return -1;

    /* Callback */
    if (cb) cb(client, userdata);

    return 0;
}

WEB_API int
web_client_status(const web_client_t *client)
{
    return client ? client->status : 0;
}

WEB_API const char *
web_client_header(const web_client_t *client, const char *name)
{
    if (!client || !name) return NULL;
    return header_list_find(client->resp_headers, name);
}

WEB_API const unsigned char *
web_client_body(const web_client_t *client, size_t *out_len)
{
    if (!client) { if (out_len) *out_len = 0; return NULL; }
    if (out_len) *out_len = client->body_len;
    return client->body;
}

WEB_API void
web_client_destroy(web_client_t *client)
{
    if (!client) return;
    if (client->fd >= 0) close(client->fd);
    header_list_free(client->req_headers);
    header_list_free(client->resp_headers);
    free(client->host);
    free(client->body);
    memset(client, 0, sizeof(*client));
    free(client);
}
