# [file name]: InfraConfig.cmake
# [file content begin]
# InfraConfig.cmake - 构建配置和选项
#
# 该模块为 Infra 项目建立完整的构建选项集。
# 处理平台检测、构建类型配置，并启用/禁用功能。
#
# 要求: InfraCommon.cmake (必须先包含)
# 要求: CMake 3.19+

# 防止重复包含
if(DEFINED INFRA_CONFIG_INCLUDED)
    return()
else()
    set(INFRA_CONFIG_INCLUDED TRUE)
endif()

# 包含基础工具模块
include(InfraCommon)

# ============================================================================
# CMake 版本要求
# ============================================================================

infra_require_cmake_version("3.19")

# ============================================================================
# 平台检测
# ============================================================================

infra_get_platform_name(INFRA_PLATFORM)
infra_info("Platform: ${INFRA_PLATFORM}")

# ============================================================================
# 构建类型配置
# ============================================================================

# 如果未指定构建类型，设置默认值
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Build type (Debug|Release|RelWithDebInfo|MinSizeRel)" FORCE)
    set(CMAKE_BUILD_TYPE "Debug")
    infra_info("Build type not specified, defaulting to 'Debug'")
else()
    infra_debug("Build type specified: ${CMAKE_BUILD_TYPE}")
endif()

# 验证构建类型是否有效
set(VALID_BUILD_TYPES "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
list(FIND VALID_BUILD_TYPES "${CMAKE_BUILD_TYPE}" _BUILD_TYPE_VALID)
if(_BUILD_TYPE_VALID EQUAL -1)
    infra_fatal("Invalid CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}'. "
                "Must be one of: ${VALID_BUILD_TYPES}")
    message(FATAL_ERROR "${INFRA_FATAL_TEXT}")
else()
    infra_debug("Build type '${CMAKE_BUILD_TYPE}' is valid")
endif()

infra_info("Build Type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# 库类型配置
# ============================================================================

# 库类型选项：SHARED（共享库）或 STATIC（静态库）
set(INFRA_LIBRARY_TYPE "SHARED" CACHE STRING "Library type (SHARED|STATIC)")
set_property(CACHE INFRA_LIBRARY_TYPE PROPERTY STRINGS SHARED STATIC)

if(INFRA_LIBRARY_TYPE STREQUAL "SHARED")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
    infra_info("Library Type: SHARED")
elseif(INFRA_LIBRARY_TYPE STREQUAL "STATIC")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    infra_info("Library Type: STATIC")
else()
    infra_fatal("Invalid INFRA_LIBRARY_TYPE '${INFRA_LIBRARY_TYPE}'. Must be SHARED or STATIC")
    message(FATAL_ERROR "${INFRA_FATAL_TEXT}")
endif()

# ============================================================================
# 核心构建选项
# ============================================================================

# 优化
option(INFRA_ENABLE_OPTIMIZATION
    "Enable compiler optimizations (-O2/-O3)" ON)

# 调试符号
option(INFRA_ENABLE_DEBUG_SYMBOLS
    "Enable debug symbols (-g for GCC/Clang, /Zi for MSVC)" ON)

# 位置无关代码
option(INFRA_POSITION_INDEPENDENT_CODE
    "Enable PIC (-fPIC for GCC/Clang)" ON)

# 编译器警告
option(INFRA_ENABLE_WARNINGS
    "Enable compiler warnings (-Wall -Wextra for GCC/Clang, /W4 for MSVC)" ON)

# 严格模式 - 将警告视为错误
option(INFRA_ENABLE_STRICT_WARNINGS
    "Treat compiler warnings as errors (-Werror for GCC/Clang, /WX for MSVC)" OFF)

# 地址消毒剂 (AddressSanitizer)
option(INFRA_ENABLE_ASAN
    "Enable AddressSanitizer for memory debugging" OFF)

# 覆盖率（GCC/Clang + gcov/lcov/gcovr）
option(INFRA_ENABLE_COVERAGE
    "Enable source coverage instrumentation for test builds" OFF)

# ============================================================================
# 测试配置
# ============================================================================

option(BUILD_TESTING
    "Build tests" ON)

option(INFRA_BUILD_TESTS
    "Build unit tests" ${BUILD_TESTING})

option(INFRA_BUILD_BENCHMARKS
    "Build performance benchmarks" OFF)

option(INFRA_BUILD_EXAMPLES
    "Build example programs" OFF)

# ============================================================================
# 安装配置
# ============================================================================

option(INFRA_INSTALL
    "Enable installation targets" ON)

option(INFRA_INSTALL_HEADERS
    "Install public header files" ON)

option(INFRA_INSTALL_DOCS
    "Install documentation" ON)

option(INFRA_INSTALL_CMAKE_CONFIG
    "Generate and install CMake config files" ON)

option(INFRA_INSTALL_PKGCONFIG
    "Generate and install pkg-config .pc files" ON)

# ============================================================================
# 文档配置
# ============================================================================

option(INFRA_BUILD_DOCS
    "Build API documentation (requires Doxygen/Sphinx)" OFF)

option(INFRA_DOCS_WITH_LATEX
    "Generate LaTeX documentation (requires Doxygen+LaTeX)" OFF)

# ============================================================================
# 依赖管理
# ============================================================================

option(INFRA_ENABLE_FETCHCONTENT
    "Enable FetchContent for missing dependencies" OFF)

option(INFRA_ENABLE_PKGCONFIG
    "Enable pkg-config support for dependencies" ON)

# ============================================================================
# 版本信息
# ============================================================================

set(INFRA_VERSION_STRING "${PROJECT_VERSION}" CACHE STRING "Project version string")
set(INFRA_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE STRING "Major version")
set(INFRA_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE STRING "Minor version")
set(INFRA_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE STRING "Patch version")

infra_info("Version: ${INFRA_VERSION_STRING}")

# ============================================================================
# 模块管理
# ============================================================================

# 要构建的模块列表
# 通过 -DINFRA_MODULES='stk;lnx;web' 显式指定。
# 未指定时不报错，仅警告并构建空模块集（什么都不构建）。
if(NOT DEFINED INFRA_MODULES OR INFRA_MODULES STREQUAL "")
    set(INFRA_MODULES "" CACHE STRING "Semicolon-separated list of modules to build")
    infra_warn("INFRA_MODULES not specified - no modules will be built. "
               "Specify e.g. -DINFRA_MODULES='stk;lnx;web' to build modules.")
else()
    infra_info("Configured modules: ${INFRA_MODULES}")
endif()

# 为每个模块生成启用/禁用选项
#
# 约定：INFRA_MODULES 中列出的模块即代表“要构建的模块”，
# 因此只要出现在 INFRA_MODULES 里就强制启用（FORCE），
# 不再需要额外传 -DINFRA_ENABLE_*=ON。
# 若想排除某个模块，直接不把它写进 INFRA_MODULES 即可。
foreach(MODULE ${INFRA_MODULES})
    string(TOUPPER "${MODULE}" MODULE_UPPER)
    set(INFRA_ENABLE_${MODULE_UPPER} ON CACHE BOOL
        "Enable ${MODULE} module (auto-enabled because listed in INFRA_MODULES)" FORCE)
    infra_info("Module '${MODULE}' enabled (via INFRA_MODULES)")
endforeach()

# stk c++
option(INFRA_STK_ENABLE_CXX
    "Enable C++ for stk module" OFF)

# stk 核心组件选项
option(INFRA_STK_ENABLE_CORE
    "Enable stk core component" ON)
option(INFRA_STK_CORE_ENABLE_VECTOR
    "Enable stk core vector implementation" ON)
option(INFRA_STK_CORE_ENABLE_LIST
    "Enable stk core list implementation" ON)
option(INFRA_STK_CORE_ENABLE_RBTREE
    "Enable stk core red-black tree implementation" ON)
option(INFRA_STK_CORE_ENABLE_SLIST
    "Enable stk core singly-linked list implementation" ON)
option(INFRA_STK_CORE_ENABLE_SET
    "Enable stk core set (hash-based) implementation" ON)
option(INFRA_STK_CORE_ENABLE_STRING
    "Enable stk core string implementation" ON)
option(INFRA_STK_CORE_ENABLE_ARENA
    "Enable stk core arena allocator" ON)
option(INFRA_STK_CORE_ENABLE_BUFFER
    "Enable stk core dynamic buffer" ON)
option(INFRA_STK_CORE_ENABLE_HASHMAP
    "Enable stk core hashmap" ON)
option(INFRA_STK_CORE_ENABLE_POOL
    "Enable stk core memory pool" ON)
option(INFRA_STK_CORE_ENABLE_HEAP
    "Enable stk core binary heap" ON)
option(INFRA_STK_CORE_ENABLE_BITSET
    "Enable stk core bitset" ON)
option(INFRA_STK_CORE_ENABLE_STACK
     "Enable stk core stack" ON)
option(INFRA_STK_CORE_ENABLE_QUEUE
     "Enable stk core queue" ON)
option(INFRA_STK_CORE_ENABLE_DEQUE
     "Enable stk core deque" ON)
option(INFRA_STK_CORE_ENABLE_RINGBUF
     "Enable stk core ring buffer" ON)
option(INFRA_STK_CORE_ENABLE_UF
     "Enable stk core union-find" ON)

option(INFRA_STK_ENABLE_UTILS
    "Enable stk utility functions (string, path, env, math, hash)" ON)

# ============================================================================
# 调试选项
# ============================================================================

option(INFRA_DEBUG
    "Enable debug output during CMake configuration" OFF)

option(INFRA_VERBOSE_CMAKE
    "Enable verbose CMake messages" OFF)

if(INFRA_VERBOSE_CMAKE)
    set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)
    infra_info("Verbose CMake output enabled")
else()
    infra_debug("Verbose CMake output disabled")
endif()

# ============================================================================
# 验证和冲突检测
# ============================================================================

# 检查冲突的选项
if(INFRA_ENABLE_ASAN AND INFRA_ENABLE_STRICT_WARNINGS)
    infra_warn("AddressSanitizer may produce warnings that conflict with -Werror")
else()
    infra_debug("No conflicts detected between ASAN and strict warnings")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT INFRA_ENABLE_DEBUG_SYMBOLS)
        infra_warn("Debug build without debug symbols (-g) recommended for Debug builds")
    else()
        infra_debug("Debug build with debug symbols enabled")
    endif()
else()
    infra_debug("Not a Debug build, debug symbols: ${INFRA_ENABLE_DEBUG_SYMBOLS}")
endif()

if(BUILD_SHARED_LIBS AND INFRA_POSITION_INDEPENDENT_CODE)
    # 这是预期的，也是好的
    infra_debug("Shared library with PIC enabled")
elseif(NOT BUILD_SHARED_LIBS AND NOT INFRA_POSITION_INDEPENDENT_CODE)
    infra_info("Static library build without PIC")
else()
    infra_debug("Shared/PIC configuration: SHARED=${BUILD_SHARED_LIBS}, PIC=${INFRA_POSITION_INDEPENDENT_CODE}")
endif()

# ============================================================================
# 配置摘要
# ============================================================================

function(infra_print_build_config)
    infra_print_config_header("Infra Build Configuration")
    infra_print_config_value("CMAKE_BUILD_TYPE" "${CMAKE_BUILD_TYPE}")
    infra_print_config_value("CMAKE_CXX_COMPILER" "${CMAKE_CXX_COMPILER_ID}")
    infra_print_config_value("Platform" "${INFRA_PLATFORM}")
    infra_print_config_value("Library Type" "${INFRA_LIBRARY_TYPE}")
    infra_print_config_value("Version" "${INFRA_VERSION_STRING}")

    infra_separator()
    message(STATUS "Feature Options:")
    infra_print_config_value("Build Tests" "${INFRA_BUILD_TESTS}")
    infra_print_config_value("Build Benchmarks" "${INFRA_BUILD_BENCHMARKS}")
    infra_print_config_value("Build Examples" "${INFRA_BUILD_EXAMPLES}")
    infra_print_config_value("Build Docs" "${INFRA_BUILD_DOCS}")

    infra_separator()
    message(STATUS "Optimization Options:")
    infra_print_config_value("Optimization" "${INFRA_ENABLE_OPTIMIZATION}")
    infra_print_config_value("Debug Symbols" "${INFRA_ENABLE_DEBUG_SYMBOLS}")
    infra_print_config_value("PIC" "${INFRA_POSITION_INDEPENDENT_CODE}")
    infra_print_config_value("Warnings" "${INFRA_ENABLE_WARNINGS}")
    infra_print_config_value("Strict Warnings" "${INFRA_ENABLE_STRICT_WARNINGS}")
    infra_print_config_value("Coverage" "${INFRA_ENABLE_COVERAGE}")

    infra_separator()
    message(STATUS "Enabled Modules:")
    if(INFRA_MODULES)
        foreach(MODULE ${INFRA_MODULES})
            string(TOUPPER "${MODULE}" MODULE_UPPER)
            if(INFRA_ENABLE_${MODULE_UPPER})
                message(STATUS "  ✓ ${MODULE}")
            else()
                message(STATUS "  ✗ ${MODULE} (disabled)")
            endif()
        endforeach()
    else()
        message(STATUS "  none")
    endif()

    infra_separator()
    message(STATUS "")
endfunction()

# 导出函数以便从父级 CMakeLists 调用
function(infra_config_summary)
    infra_print_build_config()
endfunction()