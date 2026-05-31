#include "web/cookie.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* =========================================================================
 * Cookie string helpers
 * ========================================================================= */

static char *
trim_dup(const char *s, size_t len)
{
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
           s[len - 1] == '\r' || s[len - 1] == '\n'))
        len--;
    while (len > 0 && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')) {
        s++;
        len--;
    }
    if (len == 0) return NULL;
    char *copy = (char *)malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, s, len);
    copy[len] = '\0';
    return copy;
}

/* =========================================================================
 * Parse: Cookie header (name=value pairs separated by "; ")
 * ========================================================================= */

WEB_API web_cookie_t *
web_cookie_parse(const char *header)
{
    if (!header || !*header) return NULL;

    web_cookie_t *head = NULL, **tail = &head;
    const char *p = header;

    while (*p) {
        /* skip leading whitespace and semicolon */
        while (*p == ' ' || *p == ';') p++;
        if (!*p) break;

        /* find the '=' */
        const char *eq = strchr(p, '=');
        const char *semi = strchr(p, ';');
        if (!eq) break;

        const char *val_end = semi ? semi : p + strlen(p);
        const char *name_end = eq;

        /* trim name */
        while (name_end > p && (name_end[-1] == ' ' || name_end[-1] == '\t'))
            name_end--;

        web_cookie_t *ck = (web_cookie_t *)calloc(1, sizeof(*ck));
        if (!ck) break;
        ck->max_age = -1;

        ck->name = trim_dup(p, (size_t)(name_end - p));
        if (eq + 1 < val_end)
            ck->value = trim_dup(eq + 1, (size_t)(val_end - eq - 1));
        else
            ck->value = strdup("");

        if (ck->name && ck->value) {
            *tail = ck;
            tail  = &ck->next;
        } else {
            web_cookie_free(ck);
        }

        p = *val_end ? val_end + 1 : val_end;
    }

    return head;
}

/* =========================================================================
 * Parse: Set-Cookie header
 * ========================================================================= */

WEB_API web_cookie_t *
web_cookie_parse_set(const char *header)
{
    if (!header || !*header) return NULL;

    web_cookie_t *ck = (web_cookie_t *)calloc(1, sizeof(*ck));
    if (!ck) return NULL;
    ck->max_age = -1;

    const char *p = header;

    /* name=value */
    const char *eq = strchr(p, '=');
    if (!eq) { free(ck); return NULL; }
    ck->name = trim_dup(p, (size_t)(eq - p));
    p = eq + 1;

    /* value ends at ';' or end-of-string */
    const char *semi = strchr(p, ';');
    if (semi)
        ck->value = trim_dup(p, (size_t)(semi - p));
    else
        ck->value = trim_dup(p, strlen(p));

    if (!ck->name || !ck->value) { web_cookie_free(ck); return NULL; }
    p = semi ? semi + 1 : p + strlen(p);

    /* parse attributes */
    while (*p) {
        while (*p == ' ' || *p == ';') p++;
        if (!*p) break;

        const char *attr_start = p;
        while (*p && *p != '=' && *p != ';') p++;
        size_t attr_len = (size_t)(p - attr_start);

        const char *val_start = NULL;
        size_t val_len = 0;
        if (*p == '=') {
            p++;
            val_start = p;
            while (*p && *p != ';') p++;
            val_len = (size_t)(p - val_start);
        }

        if (attr_len == 6 && strncasecmp(attr_start, "domain", 6) == 0 && val_start) {
            ck->domain = trim_dup(val_start, val_len);
        } else if (attr_len == 4 && strncasecmp(attr_start, "path", 4) == 0 && val_start) {
            ck->path = trim_dup(val_start, val_len);
        } else if (attr_len == 6 && strncasecmp(attr_start, "secure", 6) == 0) {
            ck->secure = 1;
        } else if (attr_len == 8 && strncasecmp(attr_start, "httponly", 8) == 0) {
            ck->httponly = 1;
        } else if (attr_len == 7 && strncasecmp(attr_start, "max-age", 7) == 0 && val_start) {
            char *tmp = trim_dup(val_start, val_len);
            if (tmp) { ck->max_age = atol(tmp); free(tmp); }
        }

        if (*p) p++;
    }

    return ck;
}

/* =========================================================================
 * Serialize
 * ========================================================================= */

WEB_API char *
web_cookie_serialize(const web_cookie_t *cookie)
{
    if (!cookie || !cookie->name) return NULL;

    size_t nlen = strlen(cookie->name);
    size_t vlen = cookie->value ? strlen(cookie->value) : 0;
    char *out = (char *)malloc(nlen + 1 + vlen + 1);
    if (!out) return NULL;

    if (cookie->value)
        sprintf(out, "%s=%s", cookie->name, cookie->value);
    else
        sprintf(out, "%s=", cookie->name);
    return out;
}

WEB_API char *
web_cookie_serialize_set(const web_cookie_t *cookie)
{
    if (!cookie || !cookie->name) return NULL;

    /* estimate size */
    size_t len = strlen(cookie->name) + 1 + strlen(cookie->value ? cookie->value : "") + 1;

    char attr_buf[128];
    if (cookie->domain)  len += 8 + strlen(cookie->domain);
    if (cookie->path)    len += 6 + strlen(cookie->path);
    if (cookie->secure)  len += 8;
    if (cookie->httponly) len += 10;
    if (cookie->max_age >= 0) {
        int n = snprintf(attr_buf, sizeof(attr_buf), "; Max-Age=%ld", cookie->max_age);
        if (n > 0) len += (size_t)n;
    }
    len += 2; /* ; \0 */

    char *out = (char *)malloc(len);
    if (!out) return NULL;

    size_t pos = sprintf(out, "%s=%s",
                 cookie->name, cookie->value ? cookie->value : "");
    if (cookie->path)
        pos += (size_t)sprintf(out + pos, "; Path=%s", cookie->path);
    if (cookie->domain)
        pos += (size_t)sprintf(out + pos, "; Domain=%s", cookie->domain);
    if (cookie->secure)
        pos += (size_t)sprintf(out + pos, "; Secure");
    if (cookie->httponly)
        pos += (size_t)sprintf(out + pos, "; HttpOnly");
    if (cookie->max_age >= 0)
        pos += (size_t)sprintf(out + pos, "; Max-Age=%ld", cookie->max_age);

    return out;
}

WEB_API void
web_cookie_free(web_cookie_t *cookie)
{
    while (cookie) {
        web_cookie_t *next = cookie->next;
        free(cookie->name);
        free(cookie->value);
        free(cookie->domain);
        free(cookie->path);
        free(cookie);
        cookie = next;
    }
}

/* =========================================================================
 * Cookie Jar
 * ========================================================================= */

typedef struct web_cookie_jar_entry {
    char            *domain;
    char            *path;
    web_cookie_t    *cookie;
    time_t           expires_at; /* 0 = session cookie (no expiry) */
    struct web_cookie_jar_entry *next;
} jar_entry_t;

struct web_cookie_jar {
    jar_entry_t *entries;
};

WEB_API web_cookie_jar_t *
web_cookie_jar_create(void)
{
    web_cookie_jar_t *jar = (web_cookie_jar_t *)calloc(1, sizeof(*jar));
    return jar;
}

WEB_API int
web_cookie_jar_store(web_cookie_jar_t *jar,
                      const char *set_cookie_header,
                      const char *domain, const char *path)
{
    if (!jar || !set_cookie_header) return -1;

    web_cookie_t *ck = web_cookie_parse_set(set_cookie_header);
    if (!ck) return -1;

    /* apply defaults */
    if (!ck->domain) ck->domain = strdup(domain ? domain : "");
    if (!ck->path)   ck->path   = strdup(path ? path : "/");

    /* calculate expiry */
    time_t expires_at = 0;
    if (ck->max_age >= 0)
        expires_at = time(NULL) + ck->max_age;

    /* check for existing entry with same name/domain/path, replace it */
    jar_entry_t **pp = &jar->entries;
    while (*pp) {
        jar_entry_t *e = *pp;
        if (strcmp(e->cookie->name, ck->name) == 0 &&
            strcmp(e->domain, ck->domain) == 0 &&
            strcmp(e->path, ck->path) == 0) {
            /* replace */
            web_cookie_free(e->cookie);
            e->cookie = ck;
            e->expires_at = expires_at;
            return 1;
        }
        pp = &e->next;
    }

    jar_entry_t *entry = (jar_entry_t *)malloc(sizeof(*entry));
    if (!entry) { web_cookie_free(ck); return -1; }
    entry->domain     = strdup(ck->domain);
    entry->path       = strdup(ck->path);
    entry->cookie     = ck;
    entry->expires_at = expires_at;
    entry->next       = NULL;

    *pp = entry;
    return 1;
}

/* simple domain suffix match */
static int
domain_match(const char *cookie_domain, const char *request_domain)
{
    if (!cookie_domain || !request_domain) return 0;
    size_t cdlen = strlen(cookie_domain);
    size_t rdlen = strlen(request_domain);
    if (cdlen > rdlen) return 0;
    const char *suffix = request_domain + rdlen - cdlen;
    if (strcasecmp(cookie_domain, suffix) != 0) return 0;
    if (cdlen == rdlen) return 1;
    /* check that suffix starts at a dot boundary */
    return suffix[-1] == '.';
}

/* simple path prefix match */
static int
path_match(const char *cookie_path, const char *request_path)
{
    if (!cookie_path || !request_path) return 0;
    size_t plen = strlen(cookie_path);
    if (strncmp(cookie_path, request_path, plen) != 0) return 0;
    /* exact match or request path continues with '/' */
    return request_path[plen] == '\0' || request_path[plen] == '/';
}

WEB_API char *
web_cookie_jar_get(web_cookie_jar_t *jar,
                    const char *domain, const char *path)
{
    if (!jar || !domain || !path) return NULL;

    time_t now = time(NULL);
    size_t total = 0;
    /* First pass: count and filter */
    int count = 0;
    for (jar_entry_t *e = jar->entries; e; e = e->next) {
        if (e->expires_at > 0 && now >= e->expires_at)
            continue; /* expired */
        if (!domain_match(e->domain, domain))
            continue;
        if (!path_match(e->path, path))
            continue;
        total += strlen(e->cookie->name) + 1 +
                 (e->cookie->value ? strlen(e->cookie->value) : 0) + 2; /* "; " */
        count++;
    }

    if (count == 0) return strdup("");

    char *buf = (char *)malloc(total + 1);
    if (!buf) return NULL;
    buf[0] = '\0';

    int first = 1;
    for (jar_entry_t *e = jar->entries; e; e = e->next) {
        if (e->expires_at > 0 && now >= e->expires_at)
            continue;
        if (!domain_match(e->domain, domain))
            continue;
        if (!path_match(e->path, path))
            continue;
        if (!first)
            strcat(buf, "; ");
        strcat(buf, e->cookie->name);
        strcat(buf, "=");
        if (e->cookie->value)
            strcat(buf, e->cookie->value);
        first = 0;
    }
    return buf;
}

WEB_API void
web_cookie_jar_cleanup(web_cookie_jar_t *jar)
{
    if (!jar) return;
    time_t now = time(NULL);
    jar_entry_t **pp = &jar->entries;
    while (*pp) {
        jar_entry_t *e = *pp;
        if (e->expires_at > 0 && now >= e->expires_at) {
            *pp = e->next;
            free(e->domain);
            free(e->path);
            web_cookie_free(e->cookie);
            free(e);
        } else {
            pp = &e->next;
        }
    }
}

WEB_API void
web_cookie_jar_destroy(web_cookie_jar_t *jar)
{
    if (!jar) return;
    jar_entry_t *e = jar->entries;
    while (e) {
        jar_entry_t *next = e->next;
        free(e->domain);
        free(e->path);
        web_cookie_free(e->cookie);
        free(e);
        e = next;
    }
    free(jar);
}
