# InfraConfig.cmake - 构建配置选项

if(DEFINED INFRA_CONFIG_INCLUDED)
    return()
endif()
set(INFRA_CONFIG_INCLUDED TRUE)

# ============================================================================
# 构建类型和语言
# ============================================================================
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type: Debug, Release, RelWithDebInfo, MinSizeRel")

# 语言支持
option(INFRA_ENABLE_C "Enable C language support" ON)
option(INFRA_ENABLE_CXX "Enable C++ language support" ON)
option(INFRA_ENABLE_ASM "Enable Assembly language support" OFF)

# ============================================================================
# 库类型和链接
# ============================================================================
set(INFRA_LIBRARY_TYPE "SHARED" CACHE STRING "Library type: SHARED, STATIC, MODULE")
set_property(CACHE INFRA_LIBRARY_TYPE PROPERTY STRINGS "SHARED" "STATIC" "MODULE")

option(INFRA_POSITION_INDEPENDENT_CODE "Build position independent code" ON)
option(INFRA_LINK_AS_WHOLE_ARCHIVE "Link whole archive" OFF)
option(INFRA_STRIP_SYMBOLS "Strip symbols in release build" ON)

# ============================================================================
# 优化选项
# ============================================================================
option(INFRA_ENABLE_OPTIMIZATION "Enable compiler optimization" ON)
option(INFRA_ENABLE_LTO "Enable Link Time Optimization" OFF)
option(INFRA_ENABLE_IPO "Enable Interprocedural Optimization" OFF)
option(INFRA_ENABLE_FAT_LTO "Enable Fat LTO objects" OFF)

set(INFRA_OPTIMIZATION_LEVEL "O2" CACHE STRING "Optimization level: O0, O1, O2, O3, Os, Oz, Ofast")
set_property(CACHE INFRA_OPTIMIZATION_LEVEL PROPERTY STRINGS "O0" "O1" "O2" "O3" "Os" "Oz" "Ofast")

# ============================================================================
# 调试选项
# ============================================================================
option(INFRA_ENABLE_DEBUG_SYMBOLS "Enable debug symbols" ON)
option(INFRA_ENABLE_DEBUG_INFO "Enable debug information" ON)
option(INFRA_ENABLE_PROFILING "Enable profiling (-pg)" OFF)
option(INFRA_ENABLE_GCOV "Enable gcov coverage" OFF)
option(INFRA_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(INFRA_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(INFRA_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(INFRA_ENABLE_MSAN "Enable MemorySanitizer" OFF)
option(INFRA_ENABLE_LSAN "Enable LeakSanitizer" OFF)

# ============================================================================
# 警告选项
# ============================================================================
option(INFRA_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(INFRA_TREAT_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(INFRA_ENABLE_EXTRA_WARNINGS "Enable extra warnings" OFF)
option(INFRA_ENABLE_EVERYTHING_WARNINGS "Enable everything warnings (Clang)" OFF)

# ============================================================================
# 构建模式开关
# ============================================================================
option(INFRA_BUILD_ALL_MODULES "Build all registered modules" ON)
option(INFRA_BUILD_STK "Build STK module" ON)
option(INFRA_BUILD_LNX "Build LNX module" OFF)

# ============================================================================
# 主模块开关
# ============================================================================
option(INFRA_ENABLE_STK "Enable STK module" ON)
option(INFRA_ENABLE_LNX "Enable LNX module" OFF)

# ============================================================================
# STK 子模块
# ============================================================================
if(INFRA_ENABLE_STK)
    option(INFRA_STK_ENABLE_CORE "Enable STK core" ON)
    option(INFRA_STK_ENABLE_UTIL "Enable STK util" OFF)
    
    # 语言选项
    option(INFRA_STK_ENABLE_C_API "Enable C API" ON)
    option(INFRA_STK_ENABLE_CPP_API "Enable C++ API" OFF)
    
    # Core 子模块
    option(INFRA_STK_CORE_ENABLE_VECTOR "Enable vector" ON)
    option(INFRA_STK_CORE_ENABLE_LIST "Enable list" ON)
    option(INFRA_STK_CORE_ENABLE_RBTREE "Enable rbtree" ON)
    option(INFRA_STK_CORE_ENABLE_STRING "Enable string" ON)
    option(INFRA_STK_CORE_ENABLE_HASHMAP "Enable hashmap" OFF)
    option(INFRA_STK_CORE_ENABLE_QUEUE "Enable queue" OFF)
    
    # Util 子模块
    option(INFRA_STK_UTIL_ENABLE_LOGGER "Enable logger" ON)
    option(INFRA_STK_UTIL_ENABLE_CONFIG "Enable config" OFF)
    option(INFRA_STK_UTIL_ENABLE_TIMER "Enable timer" OFF)
    option(INFRA_STK_UTIL_ENABLE_THREAD "Enable thread pool" OFF)
endif()

# ============================================================================
# LNX 子模块
# ============================================================================
if(INFRA_ENABLE_LNX)
    option(INFRA_LNX_ENABLE_CORE "Enable LNX core" OFF)
    option(INFRA_LNX_ENABLE_DRIVER "Enable LNX driver" OFF)
    option(INFRA_LNX_ENABLE_KERNEL "Enable LNX kernel" OFF)
endif()

# ============================================================================
# 测试和示例
# ============================================================================
option(INFRA_BUILD_TESTS "Build tests" OFF)
option(INFRA_BUILD_EXAMPLES "Build examples" OFF)
option(INFRA_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(INFRA_BUILD_DOCS "Build documentation" OFF)

# ============================================================================
# 安装选项
# ============================================================================
option(INFRA_INSTALL "Enable installation" ON)
option(INFRA_INSTALL_HEADERS "Install header files" ON)
option(INFRA_INSTALL_LIBRARIES "Install libraries" ON)
option(INFRA_INSTALL_CMAKE_CONFIG "Install CMake config files" ON)
option(INFRA_INSTALL_PDB "Install PDB files (Windows)" OFF)

set(INFRA_INSTALL_CMAKEDIR "share/infra/cmake" CACHE STRING "CMake config dir")
set(INFRA_INSTALL_LIBDIR "lib" CACHE STRING "Library installation directory")
set(INFRA_INSTALL_INCLUDEDIR "include" CACHE STRING "Include installation directory")
set(INFRA_INSTALL_BINDIR "bin" CACHE STRING "Binary installation directory")

# ============================================================================
# 版本信息
# ============================================================================
set(INFRA_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(INFRA_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(INFRA_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(INFRA_VERSION_TWEAK 0)
set(INFRA_VERSION_STRING "${PROJECT_VERSION}")

# ============================================================================
# 配置摘要函数
# ============================================================================
function(infra_print_config)
    message(STATUS "")
    message(STATUS "Infra Configuration Summary")
    message(STATUS "========================================")
    message(STATUS "")
    message(STATUS "Build Configuration:")
    message(STATUS "  Build Type:         ${CMAKE_BUILD_TYPE}")
    message(STATUS "  Library Type:       ${INFRA_LIBRARY_TYPE}")
    message(STATUS "  C Library:          ${INFRA_ENABLE_C}")
    message(STATUS "  C++ Library:        ${INFRA_ENABLE_CXX}")
    message(STATUS "  Assembly:           ${INFRA_ENABLE_ASM}")
    message(STATUS "")
    message(STATUS "Optimization:")
    message(STATUS "  Optimizations:      ${INFRA_ENABLE_OPTIMIZATION}")
    message(STATUS "  Optimization Level: ${INFRA_OPTIMIZATION_LEVEL}")
    message(STATUS "  LTO:                ${INFRA_ENABLE_LTO}")
    message(STATUS "  IPO:                ${INFRA_ENABLE_IPO}")
    message(STATUS "  PIC/PIE:            ${INFRA_POSITION_INDEPENDENT_CODE}")
    message(STATUS "")
    message(STATUS "Debugging:")
    message(STATUS "  Debug Symbols:      ${INFRA_ENABLE_DEBUG_SYMBOLS}")
    message(STATUS "  Debug Info:         ${INFRA_ENABLE_DEBUG_INFO}")
    message(STATUS "  Profiling:          ${INFRA_ENABLE_PROFILING}")
    message(STATUS "  Coverage:           ${INFRA_ENABLE_GCOV}")
    message(STATUS "  AddressSanitizer:   ${INFRA_ENABLE_ASAN}")
    message(STATUS "  UBSan:              ${INFRA_ENABLE_UBSAN}")
    message(STATUS "  TSan:               ${INFRA_ENABLE_TSAN}")
    message(STATUS "")
    message(STATUS "Warnings:")
    message(STATUS "  Warnings:           ${INFRA_ENABLE_WARNINGS}")
    message(STATUS "  Warnings as Errors: ${INFRA_TREAT_WARNINGS_AS_ERRORS}")
    message(STATUS "  Extra Warnings:     ${INFRA_ENABLE_EXTRA_WARNINGS}")
    message(STATUS "")
    message(STATUS "Modules:")
    message(STATUS "  Build All:          ${INFRA_BUILD_ALL_MODULES}")
    message(STATUS "  STK:                ${INFRA_BUILD_STK}")
    message(STATUS "  LNX:                ${INFRA_BUILD_LNX}")
    message(STATUS "")
    message(STATUS "STK Configuration:")
    message(STATUS "  Core:               ${INFRA_STK_ENABLE_CORE}")
    message(STATUS "  Util:               ${INFRA_STK_ENABLE_UTIL}")
    message(STATUS "  C API:              ${INFRA_STK_ENABLE_C_API}")
    message(STATUS "  C++ API:            ${INFRA_STK_ENABLE_CPP_API}")
    message(STATUS "")
    message(STATUS "  STK Core Submodules:")
    message(STATUS "    vector:           ${INFRA_STK_CORE_ENABLE_VECTOR}")
    message(STATUS "    list:             ${INFRA_STK_CORE_ENABLE_LIST}")
    message(STATUS "    rbtree:           ${INFRA_STK_CORE_ENABLE_RBTREE}")
    message(STATUS "    string:           ${INFRA_STK_CORE_ENABLE_STRING}")
    message(STATUS "    hashmap:          ${INFRA_STK_CORE_ENABLE_HASHMAP}")
    message(STATUS "    queue:            ${INFRA_STK_CORE_ENABLE_QUEUE}")
    message(STATUS "")
    if(INFRA_STK_ENABLE_UTIL)
        message(STATUS "  STK Util Submodules:")
        message(STATUS "    logger:           ${INFRA_STK_UTIL_ENABLE_LOGGER}")
        message(STATUS "    config:           ${INFRA_STK_UTIL_ENABLE_CONFIG}")
        message(STATUS "    timer:            ${INFRA_STK_UTIL_ENABLE_TIMER}")
        message(STATUS "    thread:           ${INFRA_STK_UTIL_ENABLE_THREAD}")
        message(STATUS "")
    endif()
    message(STATUS "Build Targets:")
    message(STATUS "  Tests:              ${INFRA_BUILD_TESTS}")
    message(STATUS "  Examples:           ${INFRA_BUILD_EXAMPLES}")
    message(STATUS "  Benchmarks:         ${INFRA_BUILD_BENCHMARKS}")
    message(STATUS "  Docs:               ${INFRA_BUILD_DOCS}")
    message(STATUS "")
    message(STATUS "Installation:")
    message(STATUS "  Install:            ${INFRA_INSTALL}")
    message(STATUS "  Headers:            ${INFRA_INSTALL_HEADERS}")
    message(STATUS "  Libraries:          ${INFRA_INSTALL_LIBRARIES}")
    message(STATUS "  CMake Config:       ${INFRA_INSTALL_CMAKE_CONFIG}")
    message(STATUS "  Install Prefix:     ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "========================================")
    message(STATUS "")
endfunction()