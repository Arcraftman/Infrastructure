#ifndef WEB_WS_H
#define WEB_WS_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * WebSocket Opcodes
 * ========================================================================= */

#define WEB_WS_OPCODE_CONTINUE 0x0
#define WEB_WS_OPCODE_TEXT     0x1
#define WEB_WS_OPCODE_BINARY   0x2
#define WEB_WS_OPCODE_CLOSE    0x8
#define WEB_WS_OPCODE_PING     0x9
#define WEB_WS_OPCODE_PONG     0xA

/* =========================================================================
 * WebSocket Frame
 * ========================================================================= */

typedef struct web_ws_frame {
    unsigned char  opcode;
    int            masked;
    unsigned char  mask_key[4];
    unsigned char  *payload;
    size_t         payload_len;
} web_ws_frame_t;

/**
 * Check if a request is a WebSocket upgrade request.
 * Returns non-zero if the request contains an Upgrade: websocket header.
 */
WEB_API int
web_ws_is_upgrade(const web_request_t *req);

/**
 * Prepare a 101 Switching Protocols response for a WebSocket upgrade.
 * Sets the correct upgrade headers on the response.
 * @return 0 on success, -1 if req is not a valid upgrade request.
 */
WEB_API int
web_ws_accept(const web_request_t *req, web_response_t *resp);

/**
 * Read a single WebSocket frame from a socket descriptor.
 * @param fd     Connected socket.
 * @param frame  Output frame (caller must free payload with free()).
 * @return Number of bytes read on success, -1 on error, 0 on close.
 */
WEB_API int
web_ws_read_frame(int fd, web_ws_frame_t *frame);

/**
 * Write a WebSocket frame to a socket descriptor.
 * @param fd     Connected socket.
 * @param data   Payload data.
 * @param len    Payload length.
 * @param opcode Frame opcode (WEB_WS_OPCODE_TEXT, etc.).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_ws_write_frame(int fd, const void *data, size_t len,
                   unsigned char opcode);

/**
 * Free a WebSocket frame's payload.
 */
WEB_API void
web_ws_frame_destroy(web_ws_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* WEB_WS_H */
