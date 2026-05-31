#include "web/uri.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Hex helpers
 * ========================================================================= */

static int
hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static const char HEX_DIGITS[17] = "0123456789ABCDEF";

/* RFC 3986 unreserved characters (not percent-encoded) */
static int
is_unreserved(char c)
{
    return isalnum((unsigned char)c) ||
           c == '-' || c == '.' || c == '_' || c == '~';
}

/* =========================================================================
 * Encode / Decode
 * ========================================================================= */

WEB_API int
web_uri_encode(const char *src, size_t len, char **out)
{
    if (!out) return -1;
    if (!src) { *out = NULL; return 0; }
    if (len == 0) len = strlen(src);

    size_t dst_len = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)src[i];
        if (is_unreserved((char)c))
            dst_len++;
        else
            dst_len += 3; /* %XX */
    }

    *out = (char *)malloc(dst_len + 1);
    if (!*out) return -1;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)src[i];
        if (is_unreserved((char)c)) {
            (*out)[j++] = c;
        } else {
            (*out)[j++] = '%';
            (*out)[j++] = HEX_DIGITS[c >> 4];
            (*out)[j++] = HEX_DIGITS[c & 0x0F];
        }
    }
    (*out)[j] = '\0';
    return 0;
}

WEB_API size_t
web_uri_decode(char *str)
{
    if (!str) return 0;
    size_t r = 0, w = 0;
    while (str[r]) {
        if (str[r] == '%' && str[r + 1] && str[r + 2]) {
            int hi = hex_val(str[r + 1]);
            int lo = hex_val(str[r + 2]);
            if (hi >= 0 && lo >= 0) {
                str[w++] = (char)((hi << 4) | lo);
                r += 3;
                continue;
            }
        }
        if (str[r] == '+') {
            str[w++] = ' ';
            r++;
            continue;
        }
        str[w++] = str[r++];
    }
    str[w] = '\0';
    return w;
}

/* =========================================================================
 * URI parse
 * ========================================================================= */

WEB_API int
web_uri_parse(const char *str, size_t len, web_uri_t **out)
{
    if (!str || !out) return -1;
    if (len == 0) len = strlen(str);
    if (len == 0) return -1;

    web_uri_t *uri = (web_uri_t *)calloc(1, sizeof(*uri));
    if (!uri) return -1;

    const char *p   = str;
    const char *end = str + len;

    /* ---- Scheme ---- */
    if (isalpha((unsigned char)*p)) {
        const char *s = p + 1;
        while (s < end && (isalnum((unsigned char)*s) ||
               *s == '+' || *s == '-' || *s == '.'))
            s++;
        if (s < end && *s == ':' && s > p) {
            uri->scheme = strndup(p, (size_t)(s - p));
            if (!uri->scheme) { free(uri); return -1; }
            for (char *c = uri->scheme; *c; c++)
                *c = (char)tolower((unsigned char)*c);
            p = s + 1; /* skip ':' */
        }
    }

    /* ---- Authority "//" ---- */
    if (end - p >= 2 && p[0] == '/' && p[1] == '/') {
        p += 2;
        /* find end of authority — '/' or '?' or '#' or end */
        const char *auth_end = p;
        while (auth_end < end && *auth_end != '/' && *auth_end != '?' && *auth_end != '#')
            auth_end++;

        /* userinfo */
        const char *at = (const char *)memchr(p, '@', (size_t)(auth_end - p));
        if (at) {
            uri->userinfo = strndup(p, (size_t)(at - p));
            if (uri->userinfo) web_uri_decode(uri->userinfo);
            p = at + 1;
        }

        /* host */
        const char *host_start = p;
        const char *host_end   = auth_end;

        if (p < auth_end && *p == '[') {
            /* IPv6 literal */
            const char *close = (const char *)memchr(p, ']', (size_t)(auth_end - p));
            if (!close) { web_uri_destroy(uri); return -1; }
            uri->host = strndup(p + 1, (size_t)(close - p - 1));
            p = close + 1;
        } else {
            /* Find last colon for port */
            const char *colon = NULL;
            for (const char *cp = p; cp < auth_end; cp++)
                if (*cp == ':') colon = cp;
            if (colon) {
                uri->host = strndup(p, (size_t)(colon - p));
                char buf[16];
                size_t plen = (size_t)(auth_end - colon - 1);
                if (plen > 15) plen = 15;
                memcpy(buf, colon + 1, plen);
                buf[plen] = '\0';
                uri->port = atoi(buf);
                p = colon;
            } else {
                uri->host = strndup(p, (size_t)(auth_end - p));
            }
        }
        if (uri->host) web_uri_decode(uri->host);
        p = auth_end;
    }

    /* ---- Path ---- */
    if (p < end && *p != '?' && *p != '#') {
        const char *pe = p;
        while (pe < end && *pe != '?' && *pe != '#')
            pe++;
        uri->path = strndup(p, (size_t)(pe - p));
        if (uri->path) web_uri_decode(uri->path);
        p = pe;
    } else {
        uri->path = strdup("");
        if (!uri->path) { web_uri_destroy(uri); return -1; }
    }

    /* ---- Query ---- */
    if (p < end && *p == '?') {
        p++;
        const char *qe = (const char *)memchr(p, '#', (size_t)(end - p));
        if (!qe) qe = end;
        uri->query = strndup(p, (size_t)(qe - p));
        p = qe;
    }

    /* ---- Fragment ---- */
    if (p < end && *p == '#') {
        p++;
        uri->fragment = strndup(p, (size_t)(end - p));
    }

    *out = uri;
    return 0;
}

/* =========================================================================
 * URI build
 * ========================================================================= */

WEB_API char *
web_uri_build(const web_uri_t *uri)
{
    if (!uri || !uri->path) return NULL;

    /* Estimate total length */
    size_t len = 0;
    if (uri->scheme)  len += strlen(uri->scheme) + 3; /* "://" */
    if (uri->userinfo) len += strlen(uri->userinfo) + 1; /* "@" */
    if (uri->host)    len += strlen(uri->host);
    if (uri->port > 0) len += 7; /* ":65535" */
    len += strlen(uri->path);
    if (uri->query)   len += strlen(uri->query) + 1; /* "?" */
    if (uri->fragment) len += strlen(uri->fragment) + 1; /* "#" */
    len++; /* nul */

    char *buf = (char *)malloc(len);
    if (!buf) return NULL;
    char *p = buf;

    if (uri->scheme) {
        p += sprintf(p, "%s://", uri->scheme);
    }
    if (uri->userinfo) {
        p += sprintf(p, "%s@", uri->userinfo);
    }
    if (uri->host) {
        if (strchr(uri->host, ':'))
            p += sprintf(p, "[%s]", uri->host);
        else
            p += sprintf(p, "%s", uri->host);
    }
    if (uri->port > 0)
        p += sprintf(p, ":%d", uri->port);
    if (uri->path) {
        p += sprintf(p, "%s", uri->path);
    }
    if (uri->query)
        p += sprintf(p, "?%s", uri->query);
    if (uri->fragment)
        p += sprintf(p, "#%s", uri->fragment);

    return buf;
}

/* =========================================================================
 * Destroy
 * ========================================================================= */

WEB_API void
web_uri_destroy(web_uri_t *uri)
{
    if (!uri) return;
    free(uri->scheme);
    free(uri->host);
    free(uri->path);
    free(uri->query);
    free(uri->fragment);
    free(uri->userinfo);
    free(uri);
}
