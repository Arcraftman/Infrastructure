#include "web/accesslog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct web_accesslog {
    FILE* fp;
    web_accesslog_format_t format;
};

static const char* timestamp_now(void)
{
    static char buffer[64];
    time_t now = time(NULL);
    struct tm local;
    localtime_r(&now, &local);
    strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", &local);
    return buffer;
}

static void print_json_string(FILE* fp, const char* value)
{
    const unsigned char* p = (const unsigned char*)(value ? value : "");
    fputc('"', fp);
    while (*p) {
        if (*p == '"' || *p == '\\')
            fputc('\\', fp);
        if (*p >= 32 && *p < 127)
            fputc(*p, fp);
        else
            fprintf(fp, "\\u%04x", *p);
        ++p;
    }
    fputc('"', fp);
}

WEB_API web_accesslog_t* web_accesslog_create(const char* path, web_accesslog_format_t format)
{
    web_accesslog_t* log = (web_accesslog_t*)calloc(1, sizeof(*log));
    if (!log)
        return NULL;
    log->fp = path ? fopen(path, "a") : stderr;
    if (!log->fp) {
        free(log);
        return NULL;
    }
    log->format = format;
    return log;
}

WEB_API int web_accesslog_log(web_accesslog_t* log,
                              const web_request_t* req,
                              const web_response_t* resp,
                              int status,
                              size_t bytes,
                              const char* remote,
                              long ms)
{
    const char* method;
    const char* path;
    const char* referer;
    const char* user_agent;
    if (!log || !log->fp || !req)
        return -1;
    method = web_method_str(req->method);
    path = req->path ? req->path : "-";
    referer = web_request_header(req, "Referer");
    user_agent = web_request_header(req, "User-Agent");
    if (status == 0 && resp)
        status = (int)resp->status;
    if (bytes == 0 && resp)
        bytes = resp->body_len;

    if (log->format == WEB_ACCESSLOG_JSON) {
        fprintf(log->fp, "{\"remote\":");
        print_json_string(log->fp, remote ? remote : "-");
        fprintf(log->fp, ",\"method\":");
        print_json_string(log->fp, method ? method : "UNKNOWN");
        fprintf(log->fp, ",\"path\":");
        print_json_string(log->fp, path);
        fprintf(log->fp, ",\"status\":%d,\"bytes\":%zu,\"ms\":%ld}\n", status, bytes, ms);
    } else {
        fprintf(log->fp, "%s - - [%s] \"%s %s HTTP/1.1\" %d %zu",
                remote ? remote : "-", timestamp_now(), method ? method : "UNKNOWN", path, status, bytes);
        if (log->format == WEB_ACCESSLOG_COMBINED)
            fprintf(log->fp, " \"%s\" \"%s\"", referer ? referer : "-", user_agent ? user_agent : "-");
        fputc('\n', log->fp);
    }
    return fflush(log->fp) == 0 ? 0 : -1;
}

WEB_API int web_accesslog_flush(web_accesslog_t* log)
{
    return log && log->fp ? (fflush(log->fp) == 0 ? 0 : -1) : -1;
}

WEB_API void web_accesslog_destroy(web_accesslog_t* log)
{
    if (!log)
        return;
    if (log->fp && log->fp != stderr)
        fclose(log->fp);
    free(log);
}
