#ifndef WEB_MIME_H
#define WEB_MIME_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Look up MIME type by file extension.
 * @param ext  Extension including the dot (".html", ".css") or without.
 * @return MIME type string (e.g. "text/html"), or "application/octet-stream".
 */
WEB_API const char *
web_mime_by_ext(const char *ext);

/**
 * Look up MIME type from a file path.
 * @param path  File path or URL path.
 * @return MIME type string.
 */
WEB_API const char *
web_mime_by_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* WEB_MIME_H */
