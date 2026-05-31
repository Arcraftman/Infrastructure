#ifndef WEB_H
#define WEB_H

/**
 * @file web.h
 * @brief Umbrella header for the Infra web (HTTP server) module.
 *
 * Include this single header to get all public web module APIs.
 *
 * ## Module Overview
 *
 * The web module provides a lightweight, single-threaded HTTP/1.1 server
 * built on epoll (Linux) with a select() fallback. It supports:
 *
 *   - Route registration (exact and wildcard-prefix matching)
 *   - Static file serving from configurable mount points
 *   - WebSocket upgrade (RFC 6455 handshake + frame I/O)
 *   - MIME type detection by file extension
 *   - Configurable backlog and thread count
 *   - Keep-Alive connections with configurable timeout
 *
 * ## Quick Start
 *
 * ```c
 * #include "web/web.h"
 *
 * static web_response_t *hello_handler(const web_request_t *req,
 *                                       void *userdata) {
 *     (void)req; (void)userdata;
 *     return web_response_text(WEB_STATUS_OK, "Hello, world!");
 * }
 *
 * int main(void) {
 *     web_server_t *srv = web_server_create("0.0.0.0", 8080);
 *     web_server_add_route(srv, "GET", "/hello", hello_handler, NULL);
 *     web_server_add_static(srv, "/static", "/var/www/html");
 *     return web_server_start(srv);
 * }
 * ```
 *
 * @defgroup web_module Web (HTTP Server)
 */

#include "web/def.h"
#include "web/http.h"
#include "web/mime.h"
#include "web/server.h"
#include "web/ws.h"

#endif /* WEB_H */
