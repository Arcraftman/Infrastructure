# modules/web/CMakeLists.txt
#
# Web module — HTTP server, client helpers, WebSocket, and MIME types.
# Uses epoll on Linux with select() fallback.

infra_init_module(web)

# ============================================================================
# HTTP core — request/response parsing, formatting
# ============================================================================
set(WEB_HTTP_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/http.c
)

infra_register_component(web http
    SOURCES ${WEB_HTTP_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Server — event loop (epoll / select), connection management, routing
# ============================================================================
set(WEB_SERVER_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/server.c
)

infra_register_component(web server
    SOURCES ${WEB_SERVER_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# MIME — content-type lookup by extension
# ============================================================================
set(WEB_MIME_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/mime.c
)

infra_register_component(web mime
    SOURCES ${WEB_MIME_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# WebSocket — RFC 6455 framing, handshake, SHA-1 helper
# ============================================================================
set(WEB_WS_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/ws.c
)

infra_register_component(web ws
    SOURCES ${WEB_WS_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# URI — RFC 3986 parser, percent encoding
# ============================================================================
set(WEB_URI_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/uri.c
)
infra_register_component(web uri
    SOURCES ${WEB_URI_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Cookie — parse, serialize, cookie jar
# ============================================================================
set(WEB_COOKIE_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/cookie.c
)
infra_register_component(web cookie
    SOURCES ${WEB_COOKIE_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Form — URL-encoded form parser
# ============================================================================
set(WEB_FORM_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/form.c
)
infra_register_component(web form
    SOURCES ${WEB_FORM_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Compress — gzip/deflate via zlib
# ============================================================================
set(WEB_COMPRESS_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/compress.c
)
infra_register_component(web compress
    SOURCES ${WEB_COMPRESS_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Router — trie-based URL router
# ============================================================================
set(WEB_ROUTER_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/router.c
)
infra_register_component(web router
    SOURCES ${WEB_ROUTER_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Middleware — chain-based request processing
# ============================================================================
set(WEB_MIDDLEWARE_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/middleware.c
)
infra_register_component(web middleware
    SOURCES ${WEB_MIDDLEWARE_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Auth — Basic / Bearer authentication helpers
# ============================================================================
set(WEB_AUTH_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/auth.c
)
infra_register_component(web auth
    SOURCES ${WEB_AUTH_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# CORS — cross-origin resource sharing helpers
# ============================================================================
set(WEB_CORS_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/cors.c
)
infra_register_component(web cors
    SOURCES ${WEB_CORS_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Thread pool — pthread-backed worker pool
# ============================================================================
set(WEB_THREADPOOL_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/threadpool.c
)
infra_register_component(web threadpool
    SOURCES ${WEB_THREADPOOL_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Client — HTTP client
# ============================================================================
set(WEB_CLIENT_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/client.c
)
infra_register_component(web client
    SOURCES ${WEB_CLIENT_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Cache — response cache with TTL / LRU eviction
# ============================================================================
set(WEB_CACHE_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/cache.c
)
infra_register_component(web cache
    SOURCES ${WEB_CACHE_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Access log — Common / Combined log format writer
# ============================================================================
set(WEB_ACCESSLOG_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/accesslog.c
)
infra_register_component(web accesslog
    SOURCES ${WEB_ACCESSLOG_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Rate limit — token-bucket rate limiter
# ============================================================================
set(WEB_RATELIMIT_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/ratelimit.c
)
infra_register_component(web ratelimit
    SOURCES ${WEB_RATELIMIT_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

# ============================================================================
# Session — in-memory session store
# ============================================================================
set(WEB_SESSION_SOURCES
    ${INFRA_MODULE_WEB_SRC_DIR}/session.c
)
infra_register_component(web session
    SOURCES ${WEB_SESSION_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_WEB_INC_DIR}
)

infra_finalize_module(web)

if(INFRA_BUILD_TESTS)
    if(EXISTS ${INFRA_MODULE_WEB_ROOT_DIR}/tests/CMakeLists.txt)
        add_subdirectory(${INFRA_MODULE_WEB_ROOT_DIR}/tests)
    else()
        infra_message(STATUS "WEB tests directory not found, skipping")
    endif()
endif()
