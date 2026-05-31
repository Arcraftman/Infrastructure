infra_init_module(lnx)

# ============================================================================
# LNX kernel component — system monitoring, kernel interfaces
# ============================================================================
set(LNX_KERNEL_DIR ${INFRA_MODULE_LNX_SRC_DIR}/kernel)
set(LNX_KERNEL_SOURCES "")
if(EXISTS ${LNX_KERNEL_DIR}/monitor.c)
    list(APPEND LNX_KERNEL_SOURCES ${LNX_KERNEL_DIR}/monitor.c)
endif()

if(LNX_KERNEL_SOURCES)
    infra_register_component(lnx kernel
        SOURCES ${LNX_KERNEL_SOURCES}
        PRIVATE_DIRS
            ${LNX_KERNEL_DIR}
            ${INFRA_MODULE_LNX_INC_DIR}/../
    )
endif()

# ============================================================================
# LNX core component — process, signal, file system, scheduling
# ============================================================================
set(LNX_CORE_SOURCES
    ${INFRA_MODULE_LNX_SRC_DIR}/process.c
    ${INFRA_MODULE_LNX_SRC_DIR}/signal.c
    ${INFRA_MODULE_LNX_SRC_DIR}/fsnotify.c
    ${INFRA_MODULE_LNX_SRC_DIR}/file/file.c
    ${INFRA_MODULE_LNX_SRC_DIR}/sched/sched.c
)

# Filter to only existing files
set(LNX_CORE_EXISTING "")
foreach(SRC ${LNX_CORE_SOURCES})
    if(EXISTS ${SRC})
        list(APPEND LNX_CORE_EXISTING ${SRC})
    endif()
endforeach()

if(LNX_CORE_EXISTING)
    infra_register_component(lnx core
        SOURCES ${LNX_CORE_EXISTING}
        PRIVATE_DIRS
            ${INFRA_MODULE_LNX_SRC_DIR}
            ${INFRA_MODULE_LNX_SRC_DIR}/file
            ${INFRA_MODULE_LNX_SRC_DIR}/sched
            ${INFRA_MODULE_LNX_INC_DIR}/../
    )
endif()

infra_finalize_module(lnx)

if(INFRA_BUILD_TESTS)
    if(EXISTS ${INFRA_MODULE_LNX_ROOT_DIR}/tests/CMakeLists.txt)
        add_subdirectory(${INFRA_MODULE_LNX_ROOT_DIR}/tests)
    else()
        infra_message(STATUS "LNX tests directory not found, skipping")
    endif()
endif()