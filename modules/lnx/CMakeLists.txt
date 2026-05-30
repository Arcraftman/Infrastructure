infra_init_module(lnx)

set(LNX_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LNX_SRC_DIR ${LNX_ROOT_DIR}/src)
set(LNX_INTERNAL_DIR ${LNX_ROOT_DIR}/internal)
set(LNX_TESTS_DIR ${LNX_ROOT_DIR}/tests)
set(LNX_KERNEL_DIR ${LNX_SRC_DIR}/kernel)

# ============================================================================
# LNX kernel component
# ============================================================================
set(LNX_KERNEL_SOURCES "")
if(EXISTS ${LNX_KERNEL_DIR}/monitor.c)
    list(APPEND LNX_KERNEL_SOURCES ${LNX_KERNEL_DIR}/monitor.c)
endif()

if(LNX_KERNEL_SOURCES)
    infra_register_component(lnx kernel
        SOURCES ${LNX_KERNEL_SOURCES}
        PRIVATE_DIRS
            ${LNX_KERNEL_DIR}
            ${LNX_INTERNAL_DIR}
    )
else()
    message(STATUS "Infra: No LNX kernel sources found, skipping lnx component")
endif()

infra_finalize_module(lnx)

if(INFRA_BUILD_TESTS AND EXISTS ${LNX_TESTS_DIR})
    enable_testing()
    add_subdirectory(${LNX_TESTS_DIR})
endif()