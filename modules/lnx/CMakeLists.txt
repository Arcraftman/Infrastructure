infra_init_module(lnx)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    infra_message(STATUS "LNX module is only available on Linux, skipping")
    return()
endif()

set(LNX_CORE_SOURCES
    ${INFRA_MODULE_LNX_SRC_DIR}/core/file.c
    ${INFRA_MODULE_LNX_SRC_DIR}/core/fsnotify.c
    ${INFRA_MODULE_LNX_SRC_DIR}/core/monitor.c
    ${INFRA_MODULE_LNX_SRC_DIR}/core/process.c
    ${INFRA_MODULE_LNX_SRC_DIR}/core/sched.c
    ${INFRA_MODULE_LNX_SRC_DIR}/core/signal.c
)

infra_register_component(lnx core
    SOURCES ${LNX_CORE_SOURCES}
    PRIVATE_DIRS
        ${INFRA_MODULE_LNX_SRC_DIR}
        ${INFRA_MODULE_LNX_INC_DIR}
)

infra_finalize_module(lnx)

target_compile_definitions(lnx PRIVATE _GNU_SOURCE)
target_link_libraries(lnx PRIVATE ${CMAKE_DL_LIBS} rt)
find_package(Threads REQUIRED)
target_link_libraries(lnx PRIVATE Threads::Threads)

if(INFRA_BUILD_TESTS AND EXISTS "${INFRA_MODULE_LNX_ROOT_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

set(LNX_EXAMPLES_CMAKE "${INFRA_MODULE_LNX_ROOT_DIR}/examples/CMakeLists.txt")
if(INFRA_BUILD_EXAMPLES AND EXISTS "${LNX_EXAMPLES_CMAKE}")
    add_subdirectory("${INFRA_MODULE_LNX_ROOT_DIR}/examples" "${CMAKE_CURRENT_BINARY_DIR}/examples")
endif()
