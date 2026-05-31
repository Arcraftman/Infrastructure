#include "web/mime.h"

#include <string.h>

/* =========================================================================
 * MIME type table
 * ========================================================================= */

typedef struct { const char *ext; const char *mime; } mime_entry;

static const mime_entry MIME_TABLE[] = {
    {".html",  "text/html; charset=utf-8"},
    {".htm",   "text/html; charset=utf-8"},
    {".css",   "text/css"},
    {".js",    "application/javascript"},
    {".mjs",   "application/javascript"},
    {".json",  "application/json"},
    {".xml",   "application/xml"},
    {".txt",   "text/plain; charset=utf-8"},
    {".md",    "text/markdown"},
    {".csv",   "text/csv"},

    {".png",   "image/png"},
    {".jpg",   "image/jpeg"},
    {".jpeg",  "image/jpeg"},
    {".gif",   "image/gif"},
    {".svg",   "image/svg+xml"},
    {".ico",   "image/x-icon"},
    {".webp",  "image/webp"},
    {".avif",  "image/avif"},
    {".bmp",   "image/bmp"},

    {".woff",  "font/woff"},
    {".woff2", "font/woff2"},
    {".ttf",   "font/ttf"},
    {".otf",   "font/otf"},
    {".eot",   "application/vnd.ms-fontobject"},

    {".mp4",   "video/mp4"},
    {".webm",  "video/webm"},
    {".ogg",   "video/ogg"},
    {".mp3",   "audio/mpeg"},
    {".wav",   "audio/wav"},

    {".pdf",   "application/pdf"},
    {".zip",   "application/zip"},
    {".gz",    "application/gzip"},
    {".tar",   "application/x-tar"},
    {".bz2",   "application/x-bzip2"},
    {".7z",    "application/x-7z-compressed"},

    {".wasm",  "application/wasm"},
    {".map",   "application/json"},
    {".yaml",  "text/yaml"},
    {".yml",   "text/yaml"},
    {".toml",  "application/toml"},
    {NULL, NULL}
};

WEB_API const char *
web_mime_by_ext(const char *ext)
{
    if (!ext) return "application/octet-stream";

    /* Ensure leading dot */
    size_t elen = strlen(ext);
    char dot_ext[64];
    if (elen == 0) return "application/octet-stream";

    const char *key = ext;
    if (ext[0] != '.') {
        if (elen + 1 >= sizeof(dot_ext))
            return "application/octet-stream";
        dot_ext[0] = '.';
        memcpy(dot_ext + 1, ext, elen + 1);
        key = dot_ext;
    }

    for (const mime_entry *e = MIME_TABLE; e->ext; e++) {
        if (strcasecmp(e->ext, key) == 0)
            return e->mime;
    }
    return "application/octet-stream";
}

WEB_API const char *
web_mime_by_path(const char *path)
{
    if (!path) return "application/octet-stream";

    const char *dot = strrchr(path, '.');
    if (!dot) return "application/octet-stream";

    return web_mime_by_ext(dot);
}
