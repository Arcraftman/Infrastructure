#include "web/compress.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/*
 * Compression support using zlib.  If zlib is unavailable at link time
 * the functions return -1 with errno = ENOSYS.
 */

#if defined(WEB_HAVE_ZLIB)
#  include <zlib.h>
#endif

/* =========================================================================
 * Negotiate
 * ========================================================================= */

WEB_API web_compress_algo_t
web_compress_negotiate(const char *accept_encoding)
{
    if (!accept_encoding) return WEB_COMPRESS_NONE;

    /* Simple token scan — prefers gzip over deflate */
    const char *p = accept_encoding;
    while (*p) {
        while (*p == ' ' || *p == ',') p++;
        if (strncasecmp(p, "gzip", 4) == 0) {
            return WEB_COMPRESS_GZIP;
        }
        if (strncasecmp(p, "deflate", 7) == 0) {
            return WEB_COMPRESS_DEFLATE;
        }
        while (*p && *p != ',') p++;
    }
    return WEB_COMPRESS_NONE;
}

WEB_API const char *
web_compress_name(web_compress_algo_t algo)
{
    switch (algo) {
        case WEB_COMPRESS_GZIP:    return "gzip";
        case WEB_COMPRESS_DEFLATE: return "deflate";
        default:                   return NULL;
    }
}

/* =========================================================================
 * Compress
 * ========================================================================= */

WEB_API int
web_compress(web_compress_algo_t algo,
             const void *in, size_t in_len,
             void **out, size_t *out_len)
{
    if (!in || !out || !out_len) return -1;
    *out = NULL;
    *out_len = 0;

    if (algo == WEB_COMPRESS_NONE) {
        *out = malloc(in_len ? in_len : 1);
        if (!*out) return -1;
        memcpy(*out, in, in_len);
        *out_len = in_len;
        return 0;
    }

#if defined(WEB_HAVE_ZLIB)
    int window_bits;
    switch (algo) {
        case WEB_COMPRESS_GZIP:    window_bits = 15 | 16; break; /* gzip */
        case WEB_COMPRESS_DEFLATE: window_bits = -15;      break; /* raw deflate */
        default: errno = EINVAL; return -1;
    }

    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    int ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                           window_bits, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) return -1;

    /* Upper bound: source + 0.1% + 12 bytes */
    uLong bound = deflateBound(&strm, (uLong)in_len);
    *out = malloc((size_t)bound);
    if (!*out) { deflateEnd(&strm); return -1; }

    strm.next_in   = (Bytef *)in;
    strm.avail_in  = (uInt)in_len;
    strm.next_out  = (Bytef *)*out;
    strm.avail_out = (uInt)bound;

    ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&strm);
        free(*out);
        *out = NULL;
        return -1;
    }

    *out_len = (size_t)strm.total_out;
    deflateEnd(&strm);
    return 0;

#else
    (void)algo;
    (void)in;
    (void)in_len;
    (void)out;
    (void)out_len;
    errno = ENOSYS;
    return -1;
#endif
}

/* =========================================================================
 * Response compression
 * ========================================================================= */

WEB_API int
web_response_compress(web_compress_algo_t algo, web_response_t *resp)
{
    if (!resp || algo == WEB_COMPRESS_NONE) return -1;

    if (!resp->body || resp->body_len == 0) {
        /* Nothing to compress — just set the header */
        return web_response_set_header(resp, "Content-Encoding",
                                       web_compress_name(algo));
    }

    void *compressed = NULL;
    size_t compressed_len = 0;
    if (web_compress(algo, resp->body, resp->body_len,
                     &compressed, &compressed_len) != 0)
        return -1;

    free(resp->body);
    resp->body     = (unsigned char *)compressed;
    resp->body_len = compressed_len;

    /* Transfer-Encoding not needed for static compression; set Content-Encoding */
    if (web_response_set_header(resp, "Content-Encoding",
                                web_compress_name(algo)) != 0) {
        /* header set failed but body is already compressed */
    }
    return 0;
}
