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
        ${INFRA_MODULE_WEB_INC_DIR}/../
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
        ${INFRA_MODULE_WEB_INC_DIR}/../
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
        ${INFRA_MODULE_WEB_INC_DIR}/../
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
        ${INFRA_MODULE_WEB_INC_DIR}/../
)

infra_finalize_module(web)

if(INFRA_BUILD_TESTS)
    if(EXISTS ${INFRA_MODULE_WEB_ROOT_DIR}/tests/CMakeLists.txt)
        add_subdirectory(${INFRA_MODULE_WEB_ROOT_DIR}/tests)
    else()
        infra_message(STATUS "WEB tests directory not found, skipping")
    endif()
endif()
