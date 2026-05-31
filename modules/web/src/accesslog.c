#include "web/accesslog.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =========================================================================
 * Internal structure
 * ========================================================================= */

struct web_accesslog {
    FILE               *fp;
    web_accesslog_format_t format;
};

/* =========================================================================
 * Helpers
 * ========================================================================= */

/* Format a timestamp like "01/Jan/2025:12:00:00 +0000" into a static buffer */
static const char *
format_timestamp(void)
{
    static char buf[64];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);

    strftime(buf, sizeof(buf), "%d/%b/%Y:%H:%M:%S %z", &tm);
    return buf;
}

/* Quote-safe string output (replace non-printable chars, escape quotes) */
static void
fprint_quoted(FILE *fp, const char *s)
{
    if (!s) { fputs("-", fp); return; }
    if (*s == '\0') { fputs("-", fp); return; }
    fputc('"', fp);
    while (*s) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\')
            fputc('\\', fp);
        if (c >= 32 && c < 127)
            fputc(c, fp);
        else
            fprintf(fp, "\\x%02x", c);
        s++;
    }
    fputc('"', fp);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

WEB_API web_accesslog_t *
web_accesslog_open(const char *path)
{
    return web_accesslog_open_fmt(path, WEB_ACCESSLOG_COMMON);
}

WEB_API web_accesslog_t *
web_accesslog_open_fmt(const char *path, web_accesslog_format_t fmt)
{
    if (!path) { errno = EINVAL; return NULL; }

    web_accesslog_t *log = (web_accesslog_t *)calloc(1, sizeof(*log));
    if (!log) return NULL;

    log->fp = fopen(path, "a");
    if (!log->fp) {
        free(log);
        return NULL;
    }
    log->format = fmt;

    /* Disable stdio buffering for immediate writes */
    setbuf(log->fp, NULL);
    return log;
}

WEB_API int
web_accesslog_write(web_accesslog_t *log,
                    const char *remote_addr,
                    const char *method,
                    const char *path,
                    int         status,
                    size_t      body_size,
                    const char *referer,
                    const char *user_agent)
{
    if (!log || !log->fp) return -1;

    const char *dash = "-";
    if (!remote_addr) remote_addr = dash;
    if (!method)      method      = dash;
    if (!path)        path        = dash;

    /* Common format:
     * remote_addr - - [timestamp] "METHOD path HTTP/1.1" status body_size
     */
    const char *ts = format_timestamp();

    if (log->format == WEB_ACCESSLOG_COMMON) {
        fprintf(log->fp, "%s - - [%s] \"%s %s HTTP/1.1\" %d %zu\n",
                remote_addr, ts, method, path, status, body_size);
    } else {
        /* Combined format: adds Referer and User-Agent */
        fprintf(log->fp, "%s - - [%s] \"%s %s HTTP/1.1\" %d %zu ",
                remote_addr, ts, method, path, status, body_size);
        fprint_quoted(log->fp, referer);
        fputc(' ', log->fp);
        fprint_quoted(log->fp, user_agent);
        fputc('\n', log->fp);
    }

    fflush(log->fp);
    return 0;
}

WEB_API void
web_accesslog_close(web_accesslog_t *log)
{
    if (!log) return;
    if (log->fp)
        fclose(log->fp);
    memset(log, 0, sizeof(*log));
    free(log);
}
