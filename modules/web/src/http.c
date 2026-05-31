#include "web/http.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Method strings
 * ========================================================================= */

static const char *const METHOD_STR[WEB_METHOD_COUNT] = {
    [WEB_METHOD_UNKNOWN] = NULL,
    [WEB_METHOD_GET]     = "GET",
    [WEB_METHOD_POST]    = "POST",
    [WEB_METHOD_PUT]     = "PUT",
    [WEB_METHOD_DELETE]  = "DELETE",
    [WEB_METHOD_PATCH]   = "PATCH",
    [WEB_METHOD_HEAD]    = "HEAD",
    [WEB_METHOD_OPTIONS] = "OPTIONS",
    [WEB_METHOD_CONNECT] = "CONNECT",
    [WEB_METHOD_TRACE]   = "TRACE",
};

WEB_API const char *
web_method_str(web_method_t method)
{
    if (method > 0 && method < WEB_METHOD_COUNT)
        return METHOD_STR[method];
    return NULL;
}

WEB_API web_method_t
web_method_from_str(const char *str, size_t len)
{
    if (!str) return WEB_METHOD_UNKNOWN;
    for (int i = 1; i < WEB_METHOD_COUNT; i++) {
        size_t slen = strlen(METHOD_STR[i]);
        if (slen == len && memcmp(METHOD_STR[i], str, len) == 0)
            return (web_method_t)i;
    }
    return WEB_METHOD_UNKNOWN;
}

/* =========================================================================
 * Status strings
 * ========================================================================= */

typedef struct { web_status_t code; const char *reason; } status_entry;

static const status_entry STATUS_TABLE[] = {
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {204, "No Content"},
    {206, "Partial Content"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {304, "Not Modified"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {413, "Payload Too Large"},
    {426, "Upgrade Required"},
    {429, "Too Many Requests"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {0, NULL}
};

WEB_API const char *
web_status_str(web_status_t status)
{
    for (const status_entry *e = STATUS_TABLE; e->reason; e++)
        if (e->code == status) return e->reason;
    return "Unknown";
}

WEB_API const char *
web_status_reason(web_status_t status)
{
    return web_status_str(status);
}

/* =========================================================================
 * Header linked list
 * ========================================================================= */

WEB_API web_header_t *
web_header_new(const char *name, const char *value)
{
    web_header_t *h = (web_header_t *)calloc(1, sizeof(*h));
    if (!h) return NULL;
    h->name  = strdup(name);
    h->value = strdup(value);
    if (!h->name || !h->value) {
        free(h->name);
        free(h->value);
        free(h);
        return NULL;
    }
    return h;
}

WEB_API void
web_header_free(web_header_t *hdr)
{
    if (!hdr) return;
    free(hdr->name);
    free(hdr->value);
    free(hdr);
}

static int
header_append(web_header_t **list, const char *name, const char *value)
{
    web_header_t *h = web_header_new(name, value);
    if (!h) return -1;
    if (!*list) {
        *list = h;
    } else {
        web_header_t *last = *list;
        while (last->next) last = last->next;
        last->next = h;
    }
    return 0;
}

/* =========================================================================
 * Request parsing
 * ========================================================================= */

WEB_API int
web_request_parse(const char *data, size_t len, web_request_t **out)
{
    if (!data || !out || len == 0) return -1;

    web_request_t *req = (web_request_t *)calloc(1, sizeof(*req));
    if (!req) return -1;

    /* Save raw reference (non-owning) */
    req->_raw     = (char *)data;
    req->_raw_len = len;

    /* ---- Parse request line ---- */
    const char *p = data;
    const char *end = data + len;

    /* Method: skip to first space */
    const char *sp1 = memchr(p, ' ', (size_t)(end - p));
    if (!sp1) { web_request_destroy(req); return -1; }
    req->method = web_method_from_str(p, (size_t)(sp1 - p));
    p = sp1 + 1;

    /* Path + optional query */
    const char *sp2 = memchr(p, ' ', (size_t)(end - p));
    if (!sp2) { web_request_destroy(req); return -1; }

    /* Check for query string '?' */
    const char *qs = memchr(p, '?', (size_t)(sp2 - p));
    if (qs) {
        req->path  = strndup(p, (size_t)(qs - p));
        req->query = strndup(qs + 1, (size_t)(sp2 - qs - 1));
    } else {
        req->path  = strndup(p, (size_t)(sp2 - p));
        req->query = NULL;
    }
    p = sp2 + 1;

    /* HTTP version */
    const char *eol = memchr(p, '\r', (size_t)(end - p));
    if (!eol) eol = memchr(p, '\n', (size_t)(end - p));
    if (eol) {
        req->version = strndup(p, (size_t)(eol - p));
        p = eol;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    } else {
        req->version = strndup(p, (size_t)(end - p));
        p = end;
    }

    /* ---- Parse headers ---- */
    while (p < end) {
        /* End of headers: blank line */
        if (*p == '\r' && p + 1 < end && *(p + 1) == '\n') {
            p += 2;
            break;
        }
        if (*p == '\n') { p++; break; }

        const char *eol2 = memchr(p, '\r', (size_t)(end - p));
        if (!eol2) eol2 = memchr(p, '\n', (size_t)(end - p));
        if (!eol2) eol2 = end;

        const char *colon = memchr(p, ':', (size_t)(eol2 - p));
        if (colon && colon > p) {
            const char *vstart = colon + 1;
            while (vstart < eol2 && isspace((unsigned char)*vstart)) vstart++;
            size_t vlen = (size_t)(eol2 - vstart);
            /* name is case-insensitive but we store as-is */
            if (header_append(&req->headers,
                              strndup(p, (size_t)(colon - p)),
                              strndup(vstart, vlen)) < 0) {
                web_request_destroy(req);
                return -1;
            }
        }
        p = eol2;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    }

    /* ---- Body ---- */
    if (p < end) {
        req->body_len = (size_t)(end - p);
        req->body = (unsigned char *)malloc(req->body_len + 1);
        if (req->body) {
            memcpy(req->body, p, req->body_len);
            req->body[req->body_len] = '\0';
        }
    }

    *out = req;
    return 0;
}

WEB_API const char *
web_request_header(const web_request_t *req, const char *name)
{
    if (!req || !name) return NULL;
    for (web_header_t *h = req->headers; h; h = h->next) {
        if (strcasecmp(h->name, name) == 0)
            return h->value;
    }
    return NULL;
}

WEB_API void
web_request_destroy(web_request_t *req)
{
    if (!req) return;
    free(req->path);
    free(req->query);
    free(req->version);
    web_header_t *h = req->headers;
    while (h) {
        web_header_t *next = h->next;
        web_header_free(h);
        h = next;
    }
    free(req->body);
    /* _raw is a borrow, do not free */
    free(req);
}

/* =========================================================================
 * Response
 * ========================================================================= */

WEB_API web_response_t *
web_response_new(web_status_t status)
{
    web_response_t *r = (web_response_t *)calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->status = status;
    r->owns_body = 0;
    return r;
}

WEB_API int
web_response_set_header(web_response_t *resp, const char *name,
                        const char *value)
{
    if (!resp || !name || !value) return -1;

    /* Overwrite existing header with same name (case-insensitive) */
    web_header_t *prev = NULL;
    (void)prev;
    for (web_header_t *h = resp->headers; h; prev = h, h = h->next) {
        if (strcasecmp(h->name, name) == 0) {
            char *nv = strdup(value);
            if (!nv) return -1;
            free(h->value);
            h->value = nv;
            return 0;
        }
    }
    /* Append new */
    return header_append(&resp->headers, name, value);
}

WEB_API int
web_response_set_body(web_response_t *resp, const void *data, size_t len)
{
    if (!resp) return -1;
    if (resp->owns_body && resp->body) {
        free(resp->body);
    }
    resp->body     = (unsigned char *)data;
    resp->body_len = len;
    resp->owns_body = 0; /* caller retains ownership */
    return 0;
}

WEB_API int
web_response_set_body_copy(web_response_t *resp, const void *data, size_t len)
{
    if (!resp) return -1;
    unsigned char *copy = (unsigned char *)malloc(len ? len : 1);
    if (!copy) return -1;
    if (data && len) memcpy(copy, data, len);
    if (resp->owns_body) free(resp->body);
    resp->body     = copy;
    resp->body_len = len;
    resp->owns_body = 1;
    return 0;
}

WEB_API char *
web_response_format(const web_response_t *resp, size_t *out_len)
{
    if (!resp) return NULL;

    /* Build status line */
    const char *reason = web_status_reason(resp->status);
    char status_line[128];
    int slen = snprintf(status_line, sizeof(status_line),
                        "HTTP/1.1 %d %s\r\n", resp->status, reason);
    if (slen < 0 || (size_t)slen >= sizeof(status_line)) return NULL;

    /* Calculate total length: status line + headers + body */
    size_t total = (size_t)slen;

    /* Build header strings */
    for (web_header_t *h = resp->headers; h; h = h->next) {
        total += strlen(h->name) + 2 + strlen(h->value) + 2;  /* "name: value\r\n" */
    }
    /* Content-Length if body present and not already set */
    if (resp->body && resp->body_len > 0) {
        int has_cl = 0;
        for (web_header_t *h = resp->headers; h; h = h->next)
            if (strcasecmp(h->name, "Content-Length") == 0) { has_cl = 1; break; }
        if (!has_cl)
            total += 48; /* overhead for Content-Length header */
    }
    total += 2; /* final \r\n */
    total += resp->body ? resp->body_len : 0;

    char *buf = (char *)malloc(total + 1);
    if (!buf) return NULL;

    char *wp = buf;
    size_t remain = total + 1;

    int n = snprintf(wp, remain, "%s", status_line);
    wp += n; remain -= (size_t)n;

    /* Write headers */
    for (web_header_t *h = resp->headers; h; h = h->next) {
        n = snprintf(wp, remain, "%s: %s\r\n", h->name, h->value);
        wp += n; remain -= (size_t)n;
    }

    /* Auto Content-Length */
    if (resp->body && resp->body_len > 0) {
        int has_cl = 0;
        for (web_header_t *h = resp->headers; h; h = h->next)
            if (strcasecmp(h->name, "Content-Length") == 0) { has_cl = 1; break; }
        if (!has_cl) {
            n = snprintf(wp, remain, "Content-Length: %zu\r\n", resp->body_len);
            wp += n; remain -= (size_t)n;
        }
    }

    /* End headers */
    n = snprintf(wp, remain, "\r\n");
    wp += n; remain -= (size_t)n;

    /* Body */
    if (resp->body && resp->body_len > 0) {
        memcpy(wp, resp->body, resp->body_len);
        wp += resp->body_len;
    }

    *wp = '\0';
    if (out_len) *out_len = (size_t)(wp - buf);
    return buf;
}

WEB_API void
web_response_destroy(web_response_t *resp)
{
    if (!resp) return;
    web_header_t *h = resp->headers;
    while (h) {
        web_header_t *next = h->next;
        web_header_free(h);
        h = next;
    }
    if (resp->owns_body) free(resp->body);
    free(resp);
}

/* =========================================================================
 * Convenience response builders
 * ========================================================================= */

WEB_API web_response_t *
web_response_text(web_status_t status, const char *text)
{
    web_response_t *r = web_response_new(status);
    if (!r) return NULL;
    web_response_set_header(r, "Content-Type", "text/plain; charset=utf-8");
    web_response_set_body_copy(r, text, strlen(text));
    return r;
}

WEB_API web_response_t *
web_response_json(web_status_t status, const char *json_str)
{
    web_response_t *r = web_response_new(status);
    if (!r) return NULL;
    web_response_set_header(r, "Content-Type", "application/json");
    web_response_set_body_copy(r, json_str, strlen(json_str));
    return r;
}

WEB_API web_response_t *
web_response_file(const char *path)
{
    (void)path;
    /* Static file serving is handled at the server level;
     * this convenience stub returns 501. */
    return web_response_text(WEB_STATUS_NOT_IMPLEMENTED,
                             "Static file serving is not yet implemented");
}

WEB_API web_response_t *
web_response_error(web_status_t status)
{
    const char *reason = web_status_reason(status);
    return web_response_text(status, reason);
}
