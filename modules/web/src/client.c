#include "web/client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLIENT_MAX_RESPONSE (16U * 1024U * 1024U)
#define CLIENT_MAX_HEADERS (64U * 1024U)

typedef struct web_client {
    long connect_timeout;
    long request_timeout;
    char* proxy_url;
    char error[256];
} web_client_impl_t;

static void set_error(web_client_impl_t* client, const char* message)
{
    if (client)
        snprintf(client->error, sizeof(client->error), "%s", message);
}

static int wait_fd(int fd, short events, long timeout_sec)
{
    struct pollfd pfd = {fd, events, 0};
    int timeout_ms = timeout_sec > 0 && timeout_sec <= INT_MAX / 1000
                         ? (int)(timeout_sec * 1000)
                         : -1;
    int rc;
    do {
        rc = poll(&pfd, 1, timeout_ms);
    } while (rc < 0 && errno == EINTR);
    return rc > 0 && !(pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) ? 0 : -1;
}

static int connect_host(const char* host, const char* port, long timeout_sec)
{
    struct addrinfo hints;
    struct addrinfo* result = NULL;
    struct addrinfo* it;
    int fd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    if (getaddrinfo(host, port, &hints, &result) != 0)
        return -1;
    for (it = result; it; it = it->ai_next) {
        int flags;
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0)
            continue;
        flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            close(fd);
            fd = -1;
            continue;
        }
        if (connect(fd, it->ai_addr, it->ai_addrlen) < 0 && errno != EINPROGRESS) {
            close(fd);
            fd = -1;
            continue;
        }
        if (wait_fd(fd, POLLOUT, timeout_sec) == 0) {
            int error = 0;
            socklen_t error_len = sizeof(error);
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_len) == 0 && error == 0)
                break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(result);
    if (fd >= 0) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags >= 0)
            (void)fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
    return fd;
}

static int send_all(int fd, const void* data, size_t length, long timeout_sec)
{
    const unsigned char* cursor = (const unsigned char*)data;
    while (length > 0) {
        ssize_t n;
        if (wait_fd(fd, POLLOUT, timeout_sec) < 0)
            return -1;
        do {
            n = send(fd, cursor, length, MSG_NOSIGNAL);
        } while (n < 0 && errno == EINTR);
        if (n <= 0)
            return -1;
        cursor += (size_t)n;
        length -= (size_t)n;
    }
    return 0;
}

static int append_bytes(unsigned char** buffer, size_t* length, size_t* capacity,
                        const unsigned char* data, size_t data_length)
{
    size_t needed;
    unsigned char* resized;
    if (data_length > CLIENT_MAX_RESPONSE - *length)
        return -1;
    needed = *length + data_length;
    if (needed <= *capacity) {
        memcpy(*buffer + *length, data, data_length);
        *length = needed;
        return 0;
    }
    *capacity = *capacity ? *capacity : 4096;
    while (*capacity < needed) {
        if (*capacity > CLIENT_MAX_RESPONSE / 2)
            *capacity = CLIENT_MAX_RESPONSE;
        else
            *capacity *= 2;
    }
    resized = (unsigned char*)realloc(*buffer, *capacity);
    if (!resized)
        return -1;
    *buffer = resized;
    memcpy(*buffer + *length, data, data_length);
    *length = needed;
    return 0;
}

static const unsigned char* find_bytes(const unsigned char* data, size_t length,
                                       const char* needle, size_t needle_length)
{
    size_t i;
    if (needle_length == 0 || needle_length > length)
        return NULL;
    for (i = 0; i + needle_length <= length; ++i)
        if (memcmp(data + i, needle, needle_length) == 0)
            return data + i;
    return NULL;
}

static int parse_response(web_client_impl_t* client, const unsigned char* raw, size_t raw_len,
                          web_response_t** out)
{
    const unsigned char* header_end = find_bytes(raw, raw_len, "\r\n\r\n", 4);
    const unsigned char* line_end;
    const unsigned char* cursor;
    web_response_t* response = NULL;
    char status_line[128];
    int status;

    if (!header_end || (size_t)(header_end - raw) > CLIENT_MAX_HEADERS) {
        set_error(client, "invalid or oversized HTTP response headers");
        return -1;
    }
    line_end = find_bytes(raw, (size_t)(header_end - raw), "\r\n", 2);
    if (!line_end || (size_t)(line_end - raw) >= sizeof(status_line)) {
        set_error(client, "invalid HTTP response status line");
        return -1;
    }
    memcpy(status_line, raw, (size_t)(line_end - raw));
    status_line[line_end - raw] = '\0';
    if (sscanf(status_line, "HTTP/1.1 %d", &status) != 1 || status < 100 || status > 999) {
        set_error(client, "invalid HTTP response status line");
        return -1;
    }
    response = web_response_new((web_status_t)status);
    if (!response)
        return -1;
    cursor = line_end + 2;
    while (cursor < header_end) {
        const unsigned char* eol = find_bytes(cursor, (size_t)(header_end - cursor) + 2, "\r\n", 2);
        const unsigned char* colon;
        char* name;
        char* value;
        if (!eol) {
            set_error(client, "malformed response header line");
            goto invalid;
        }
        colon = memchr(cursor, ':', (size_t)(eol - cursor));
        if (!colon || colon == cursor) {
            set_error(client, "malformed response header colon");
            goto invalid;
        }
        name = strndup((const char*)cursor, (size_t)(colon - cursor));
        value = strndup((const char*)colon + 1, (size_t)(eol - colon - 1));
        if (!name || !value) {
            free(name);
            free(value);
            goto invalid;
        }
        while (*value == ' ' || *value == '\t')
            memmove(value, value + 1, strlen(value));
        if (web_response_set_header(response, name, value) < 0) {
            free(name);
            free(value);
            goto invalid;
        }
        free(name);
        free(value);
        cursor = eol + 2;
    }
    if ((size_t)(header_end + 4 - raw) < raw_len) {
        size_t body_len = raw_len - (size_t)(header_end + 4 - raw);
        if (web_response_set_body_copy(response, header_end + 4, body_len) < 0)
            goto invalid;
    }
    *out = response;
    return 0;
invalid:
    web_response_destroy(response);
    set_error(client, "malformed HTTP response headers");
    return -1;
}

static int parse_http_url(const char* url, char** host, char** port, char** path)
{
    const char* start;
    const char* authority_end;
    const char* colon;
    if (!url || strncmp(url, "http://", 7) != 0)
        return -1;
    start = url + 7;
    authority_end = strchr(start, '/');
    if (!authority_end)
        authority_end = start + strlen(start);
    if (authority_end == start)
        return -1;
    colon = memchr(start, ':', (size_t)(authority_end - start));
    if (colon) {
        *host = strndup(start, (size_t)(colon - start));
        *port = strndup(colon + 1, (size_t)(authority_end - colon - 1));
    } else {
        *host = strndup(start, (size_t)(authority_end - start));
        *port = strdup("80");
    }
    *path = *authority_end ? strdup(authority_end) : strdup("/");
    if (!*host || !*port || !*path || (*port)[0] == '\0') {
        free(*host); free(*port); free(*path);
        *host = NULL; *port = NULL; *path = NULL;
        return -1;
    }
    return 0;
}

WEB_API web_client_t* web_client_create(long connect_timeout, long request_timeout)
{
    web_client_impl_t* client = (web_client_impl_t*)calloc(1, sizeof(*client));
    if (!client)
        return NULL;
    client->connect_timeout = connect_timeout > 0 ? connect_timeout : 10;
    client->request_timeout = request_timeout > 0 ? request_timeout : 30;
    return (web_client_t*)client;
}

WEB_API web_response_t* web_client_request(web_client_t* opaque,
                                            const char* method,
                                            const char* url,
                                            const web_header_t* headers,
                                            const void* body,
                                            size_t body_len)
{
    web_client_impl_t* client = (web_client_impl_t*)opaque;
    char* host = NULL; char* port = NULL; char* path = NULL;
    char* request = NULL;
    size_t request_len = 0;
    size_t request_capacity;
    int fd = -1;
    unsigned char* response_data = NULL;
    size_t response_len = 0, response_capacity = 0;
    web_response_t* response = NULL;
    const web_header_t* header;
    int n;

    if (!client || !method || !url || (body_len && !body)) {
        set_error(client, "invalid HTTP request arguments");
        return NULL;
    }
    if (strchr(method, '\r') || strchr(method, '\n') || strchr(url, '\r') || strchr(url, '\n')) {
        set_error(client, "invalid HTTP request token");
        return NULL;
    }
    if (parse_http_url(url, &host, &port, &path) < 0) {
        set_error(client, "only absolute http:// URLs are supported");
        return NULL;
    }
    fd = connect_host(host, port, client->connect_timeout);
    if (fd < 0) {
        set_error(client, "HTTP connection failed");
        goto cleanup;
    }
    request_capacity = strlen(method) + strlen(path) + strlen(host) + body_len + 256;
    request = (char*)malloc(request_capacity);
    if (!request)
        goto cleanup;
    n = snprintf(request, request_capacity, "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n",
                 method, path, host);
    if (n < 0 || (size_t)n >= request_capacity)
        goto cleanup;
    request_len = (size_t)n;
    for (header = headers; header; header = header->next) {
        if (!header->name || !header->value || strchr(header->name, '\r') || strchr(header->name, '\n') ||
            strchr(header->value, '\r') || strchr(header->value, '\n')) {
            set_error(client, "invalid HTTP header");
            goto cleanup;
        }
        n = snprintf(request + request_len, request_capacity - request_len, "%s: %s\r\n",
                     header->name, header->value);
        if (n < 0 || (size_t)n >= request_capacity - request_len)
            goto cleanup;
        request_len += (size_t)n;
    }
    if (body_len) {
        n = snprintf(request + request_len, request_capacity - request_len, "Content-Length: %zu\r\n", body_len);
        if (n < 0 || (size_t)n >= request_capacity - request_len)
            goto cleanup;
        request_len += (size_t)n;
    }
    if (request_len + 2 + body_len > request_capacity)
        goto cleanup;
    memcpy(request + request_len, "\r\n", 2);
    request_len += 2;
    if (body_len) {
        memcpy(request + request_len, body, body_len);
        request_len += body_len;
    }
    if (send_all(fd, request, request_len, client->request_timeout) < 0) {
        set_error(client, "HTTP request send failed");
        goto cleanup;
    }
    for (;;) {
        unsigned char buffer[8192];
        ssize_t received;
        if (wait_fd(fd, POLLIN, client->request_timeout) < 0)
            break;
        do {
            received = recv(fd, buffer, sizeof(buffer), 0);
        } while (received < 0 && errno == EINTR);
        if (received == 0)
            break;
        if (received < 0)
            goto cleanup;
        if (append_bytes(&response_data, &response_len, &response_capacity, buffer, (size_t)received) < 0) {
            set_error(client, "HTTP response exceeds size limit");
            goto cleanup;
        }
    }
    if (response_len == 0) {
        set_error(client, "empty HTTP response");
        goto cleanup;
    }
    if (parse_response(client, response_data, response_len, &response) < 0)
        goto cleanup;
cleanup:
    if (fd >= 0) close(fd);
    free(host); free(port); free(path); free(request); free(response_data);
    return response;
}

WEB_API web_response_t* web_client_request_stream(web_client_t* client,
                                                   const char* method,
                                                   const char* url,
                                                   const web_header_t* headers,
                                                   const void* body,
                                                   size_t body_len,
                                                   web_client_chunk_cb chunk_cb,
                                                   void* cb_userdata)
{
    web_response_t* response = web_client_request(client, method, url, headers, body, body_len);
    if (response && chunk_cb && response->body_len && chunk_cb(response->body, response->body_len, cb_userdata) < 0) {
        web_response_destroy(response);
        return NULL;
    }
    return response;
}

WEB_API const char* web_client_error(const web_client_t* client)
{
    return client ? ((const web_client_impl_t*)client)->error : NULL;
}

WEB_API int web_client_set_proxy(web_client_t* opaque, const char* proxy_url)
{
    web_client_impl_t* client = (web_client_impl_t*)opaque;
    char* copy = proxy_url ? strdup(proxy_url) : NULL;
    if (proxy_url && !copy)
        return -1;
    if (client) {
        free(client->proxy_url);
        client->proxy_url = copy;
        return 0;
    }
    free(copy);
    return -1;
}

WEB_API void web_client_destroy(web_client_t* opaque)
{
    web_client_impl_t* client = (web_client_impl_t*)opaque;
    if (!client)
        return;
    free(client->proxy_url);
    free(client);
}
