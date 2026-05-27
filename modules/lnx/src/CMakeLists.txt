set(LNX_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LNX_INCLUDE_DIR ${LNX_ROOT_DIR}/include)
set(LNX_SRC_DIR ${LNX_ROOT_DIR}/src)
set(LNX_INTERNAL_DIR ${LNX_ROOT_DIR}/internal)
set(LNX_TESTS_DIR ${LNX_ROOT_DIR}/tests)
set(LNX_EXAMPLES_DIR ${LNX_ROOT_DIR}/examples)

set(LNX_KERNEL_DIR ${LNX_SRC_DIR}/kernel)
set(LNX_CORE_DIR ${LNX_SRC_DIR}/core)
set(LNX_DRIVER_DIR ${LNX_SRC_DIR}/driver)

# ============================================================
# 配置信息
# ============================================================
message(STATUS "")
message(STATUS "==================== LNX Configuration ====================")
message(STATUS "Project:           LNX ${PROJECT_VERSION}")
message(STATUS "Platform:          ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture:      ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  BUILD_LNX:            ${BUILD_LNX}")
message(STATUS "  BUILD_KERNEL_MODULE:  ${BUILD_KERNEL_MODULE}")
message(STATUS "  BUILD_TESTS:          ${BUILD_TESTS}")
message(STATUS "  BUILD_EXAMPLES:       ${BUILD_EXAMPLES}")
message(STATUS "")
message(STATUS "Directories:")
message(STATUS "  Kernel module:    ${LNX_KERNEL_DIR}")
message(STATUS "  Core library:     ${LNX_CORE_DIR}")
message(STATUS "  Driver:           ${LNX_DRIVER_DIR}")
message(STATUS "  Kernel build:     ${KERNEL_BUILD_DIR}")
message(STATUS "============================================================")
message(STATUS "")

# ============================================================
# 设置输出目录
# ============================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================
# 编译选项
# ============================================================
if(MSVC)
    set(LNX_COMPILE_OPTIONS /W4 /wd4013 /wd4047 /wd4996)
    set(LNX_COMPILE_DEFS _CRT_SECURE_NO_WARNINGS)
else()
    set(LNX_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror)
    set(LNX_COMPILE_DEFS _GNU_SOURCE)
    
    if(BUILD_SHARED_LIBS)
        set(LNX_COMPILE_OPTIONS ${LNX_COMPILE_OPTIONS} -fPIC)
    endif()
endif()