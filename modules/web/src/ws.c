#include "web/ws.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Minimal SHA-1 implementation for WebSocket key digest.
 *
 * RFC 6455 requires: SHA-1(key + "258EAFA5-E914-47DA-95CA-5AB9DC11B85B")
 * then Base64-encode the 20-byte digest.
 *
 * We include a compact SHA-1 so the module has zero external dependencies
 * for WebSocket support.
 */

/* =========================================================================
 * Compact SHA-1
 * ========================================================================= */

#define SHA1_BLOCK_SIZE 64

typedef struct {
    uint32_t state[5];
    uint64_t count;
    unsigned char buffer[SHA1_BLOCK_SIZE];
} sha1_ctx;

static void
sha1_transform(uint32_t state[5], const unsigned char block[SHA1_BLOCK_SIZE])
{
    uint32_t w[80];
    for (int i = 0; i < 16; i++)
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               ((uint32_t)block[i * 4 + 3]);
    for (int i = 16; i < 80; i++)
        w[i] = (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]) << 1 |
               ((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]) >> 31);

    uint32_t a = state[0], b = state[1], c = state[2],
             d = state[3], e = state[4], f, tmp;

#define SHA1_ROUND(i, K) do { \
    if (i < 20)      f = (b & c) | (~b & d); \
    else if (i < 40) f = b ^ c ^ d; \
    else if (i < 60) f = (b & c) | (b & d) | (c & d); \
    else             f = b ^ c ^ d; \
    tmp = ((a << 5) | (a >> 27)) + f + e + K + w[i]; \
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = tmp; \
} while (0)

    for (int i = 0; i < 80; i++) {
        if (i < 20)      SHA1_ROUND(i, 0x5A827999);
        else if (i < 40) SHA1_ROUND(i, 0x6ED9EBA1);
        else if (i < 60) SHA1_ROUND(i, 0x8F1BBCDC);
        else             SHA1_ROUND(i, 0xCA62C1D6);
    }

#undef SHA1_ROUND

    state[0] += a; state[1] += b; state[2] += c;
    state[3] += d; state[4] += e;
}

static void
sha1_init(sha1_ctx *ctx)
{
    ctx->count = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

static void
sha1_update(sha1_ctx *ctx, const unsigned char *data, size_t len)
{
    size_t i = (size_t)(ctx->count & 63);
    ctx->count += (uint64_t)len;

    while (len > 0) {
        size_t part = SHA1_BLOCK_SIZE - i;
        if (part > len) part = len;
        memcpy(ctx->buffer + i, data, part);
        i += part;
        data += part;
        len -= part;
        if (i == SHA1_BLOCK_SIZE) {
            sha1_transform(ctx->state, ctx->buffer);
            i = 0;
        }
    }
}

static void
sha1_final(sha1_ctx *ctx, unsigned char digest[20])
{
    uint64_t bits = ctx->count * 8;
    size_t i = (size_t)(ctx->count & 63);

    /* Pad */
    ctx->buffer[i++] = 0x80;
    if (i > 56) {
        memset(ctx->buffer + i, 0, SHA1_BLOCK_SIZE - i);
        sha1_transform(ctx->state, ctx->buffer);
        i = 0;
    }
    memset(ctx->buffer + i, 0, 56 - i);

    /* Append length in bits */
    for (int j = 7; j >= 0; j--)
        ctx->buffer[56 + (size_t)j] = (unsigned char)(bits >> ((7 - j) * 8));
    sha1_transform(ctx->state, ctx->buffer);

    for (int j = 0; j < 5; j++)
        digest[j * 4]     = (unsigned char)(ctx->state[j] >> 24),
        digest[j * 4 + 1] = (unsigned char)(ctx->state[j] >> 16),
        digest[j * 4 + 2] = (unsigned char)(ctx->state[j] >> 8),
        digest[j * 4 + 3] = (unsigned char)(ctx->state[j]);
}

/* Base64 encode */
static void
base64_encode(const unsigned char *in, size_t in_len, char *out)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i, o = 0;
    for (i = 0; i + 3 <= in_len; i += 3) {
        uint32_t v = ((uint32_t)in[i] << 16) |
                     ((uint32_t)in[i+1] << 8) |
                     (uint32_t)in[i+2];
        out[o++] = b64[(v >> 18) & 0x3F];
        out[o++] = b64[(v >> 12) & 0x3F];
        out[o++] = b64[(v >> 6) & 0x3F];
        out[o++] = b64[v & 0x3F];
    }
    size_t remain = in_len - i;
    if (remain == 1) {
        uint32_t v = (uint32_t)in[i] << 16;
        out[o++] = b64[(v >> 18) & 0x3F];
        out[o++] = b64[(v >> 12) & 0x3F];
        out[o++] = '=';
        out[o++] = '=';
    } else if (remain == 2) {
        uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8);
        out[o++] = b64[(v >> 18) & 0x3F];
        out[o++] = b64[(v >> 12) & 0x3F];
        out[o++] = b64[(v >> 6) & 0x3F];
        out[o++] = '=';
    }
    out[o] = '\0';
}

/* =========================================================================
 * WebSocket API
 * ========================================================================= */

static const char WS_MAGIC[] = "258EAFA5-E914-47DA-95CA-5AB9DC11B85B";

WEB_API int
web_ws_is_upgrade(const web_request_t *req)
{
    if (!req) return 0;
    const char *upgrade = web_request_header(req, "Upgrade");
    if (!upgrade) return 0;
    if (strcasecmp(upgrade, "websocket") != 0) return 0;

    const char *conn = web_request_header(req, "Connection");
    if (!conn) return 0;
    if (strcasestr(conn, "Upgrade") == NULL) return 0;

    return 1;
}

WEB_API int
web_ws_accept(const web_request_t *req, web_response_t *resp)
{
    if (!req || !resp) return -1;

    const char *key = web_request_header(req, "Sec-WebSocket-Key");
    if (!key) return -1;

    /* Compute SHA-1(key + magic) */
    size_t key_len = strlen(key);
    size_t total_len = key_len + strlen(WS_MAGIC);
    char *concat = (char *)malloc(total_len);
    if (!concat) return -1;
    memcpy(concat, key, key_len);
    memcpy(concat + key_len, WS_MAGIC, strlen(WS_MAGIC));

    sha1_ctx ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char *)concat, total_len);
    unsigned char digest[20];
    sha1_final(&ctx, digest);
    free(concat);

    /* Base64 encode the digest */
    char accept_key[40];
    base64_encode(digest, 20, accept_key);

    /* Set response headers */
    resp->status = WEB_STATUS_SWITCHING_PROTOCOLS;
    web_response_set_header(resp, "Upgrade", "websocket");
    web_response_set_header(resp, "Connection", "Upgrade");
    web_response_set_header(resp, "Sec-WebSocket-Accept", accept_key);

    const char *protocol = web_request_header(req, "Sec-WebSocket-Protocol");
    if (protocol)
        web_response_set_header(resp, "Sec-WebSocket-Protocol", protocol);

    return 0;
}

WEB_API int
web_ws_read_frame(int fd, web_ws_frame_t *frame)
{
    if (!frame) return -1;
    memset(frame, 0, sizeof(*frame));

    unsigned char header[2];
    ssize_t n = read(fd, header, 2);
    if (n <= 0) return (int)n;
    if (n < 2) return -1;

    frame->opcode  = header[0] & 0x0F;
    frame->masked  = (header[1] & 0x80) ? 1 : 0;
    uint64_t plen  = header[1] & 0x7F;

    if (plen == 126) {
        unsigned char ext[2];
        if (read(fd, ext, 2) < 2) return -1;
        plen = ((uint64_t)ext[0] << 8) | (uint64_t)ext[1];
    } else if (plen == 127) {
        unsigned char ext[8];
        if (read(fd, ext, 8) < 8) return -1;
        plen = 0;
        for (int i = 0; i < 8; i++)
            plen = (plen << 8) | (uint64_t)ext[i];
    }

    /* Read mask key */
    if (frame->masked) {
        if (read(fd, frame->mask_key, 4) < 4) return -1;
    }

    /* Read payload */
    if (plen > 0) {
        frame->payload = (unsigned char *)malloc((size_t)plen + 1);
        if (!frame->payload) return -1;
        if (read(fd, frame->payload, (size_t)plen) < (ssize_t)plen) {
            free(frame->payload);
            frame->payload = NULL;
            return -1;
        }
        frame->payload[plen] = '\0';

        /* Unmask if needed */
        if (frame->masked) {
            for (size_t i = 0; i < (size_t)plen; i++)
                frame->payload[i] ^= frame->mask_key[i & 3];
        }
    }

    frame->payload_len = (size_t)plen;
    return (int)(2 + (plen > 125 ? (plen > 65535 ? 8 : 2) : 0) +
                 (frame->masked ? 4 : 0) + plen);
}

WEB_API int
web_ws_write_frame(int fd, const void *data, size_t len,
                   unsigned char opcode)
{
    unsigned char header[10];
    size_t hdr_len = 0;

    header[0] = 0x80 | (opcode & 0x0F);  /* FIN + opcode */

    /* Server-to-client frames are unmasked */
    if (len < 126) {
        header[1] = (unsigned char)len;
        hdr_len = 2;
    } else if (len <= 65535) {
        header[1] = 126;
        header[2] = (unsigned char)(len >> 8);
        header[3] = (unsigned char)(len);
        hdr_len = 4;
    } else {
        header[1] = 127;
        uint64_t llen = (uint64_t)len;
        for (int i = 7; i >= 0; i--) {
            header[2 + (size_t)i] = (unsigned char)(llen & 0xFF);
            llen >>= 8;
        }
        hdr_len = 10;
    }

    if (write(fd, header, hdr_len) < (ssize_t)hdr_len) return -1;
    if (len > 0 && write(fd, data, len) < (ssize_t)len) return -1;

    return 0;
}

WEB_API void
web_ws_frame_destroy(web_ws_frame_t *frame)
{
    if (frame) free(frame->payload);
}
