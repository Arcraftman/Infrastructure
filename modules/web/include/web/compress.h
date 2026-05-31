#ifndef WEB_COMPRESS_H
#define WEB_COMPRESS_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file compress.h
 * @brief Response body compression (gzip/deflate) using zlib.
 *
 * If zlib is not available, all functions return -1 and set errno to ENOSYS.
 */

/** Supported compression algorithms. */
typedef enum web_compress_algo {
    WEB_COMPRESS_NONE    = 0, /**< No compression (passthrough). */
    WEB_COMPRESS_GZIP    = 1, /**< gzip format (RFC 1952). */
    WEB_COMPRESS_DEFLATE = 2  /**< raw deflate (RFC 1951). */
} web_compress_algo_t;

/**
 * Detect the best compression algorithm from an Accept-Encoding header.
 * @param accept_encoding  Value of the Accept-Encoding request header.
 * @return The preferred algorithm, or WEB_COMPRESS_NONE if no match.
 */
WEB_API web_compress_algo_t
web_compress_negotiate(const char *accept_encoding);

/**
 * Compress a buffer using the given algorithm.
 * @param algo     Compression algorithm.
 * @param in       Input data.
 * @param in_len   Input length.
 * @param[out] out      Receives the compressed data (caller must free with free()).
 * @param[out] out_len  Receives the compressed length.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_compress(web_compress_algo_t algo,
             const void *in, size_t in_len,
             void **out, size_t *out_len);

/**
 * Name string for a compression algorithm ("gzip", "deflate").
 * Returns NULL for WEB_COMPRESS_NONE.
 */
WEB_API const char *
web_compress_name(web_compress_algo_t algo);

/**
 * Compress a response body in-place.
 * Replaces the response body with the compressed version and
 * adds the Content-Encoding header. The response must have a body set.
 * @param algo Compression algorithm.
 * @param resp Response to compress.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_response_compress(web_compress_algo_t algo, web_response_t *resp);

#ifdef __cplusplus
}
#endif

#endif /* WEB_COMPRESS_H */
