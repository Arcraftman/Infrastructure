# InfraConfig.cmake - Build configuration and options
#
# This module establishes the complete set of build options for the Infra project.
# It handles platform detection, build type configuration, and enables/disables features.
#
# REQUIRES: InfraCommon.cmake (must be included first)
# REQUIRES: CMake 3.19+

if(DEFINED INFRA_CONFIG_INCLUDED)
    return()
endif()
set(INFRA_CONFIG_INCLUDED TRUE)

include(InfraCommon)

# ============================================================================
# CMake Version Requirements
# ============================================================================

infra_require_cmake_version("3.19")

# ============================================================================
# Platform Detection
# ============================================================================

infra_get_platform_name(INFRA_PLATFORM)
infra_info("Platform: ${INFRA_PLATFORM}")

# ============================================================================
# Build Type Configuration
# ============================================================================

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Build type (Debug|Release|RelWithDebInfo|MinSizeRel)")
    infra_info("Build type not specified, defaulting to 'Debug'")
endif()

# Validate build type
set(VALID_BUILD_TYPES "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
list(FIND VALID_BUILD_TYPES "${CMAKE_BUILD_TYPE}" _BUILD_TYPE_VALID)
if(_BUILD_TYPE_VALID EQUAL -1)
    infra_fatal("Invalid CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}'. "
                "Must be one of: ${VALID_BUILD_TYPES}")
endif()

infra_info("Build Type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# Library Type Configuration
# ============================================================================

set(INFRA_LIBRARY_TYPE "SHARED" CACHE STRING "Library type (SHARED|STATIC)")
set_property(CACHE INFRA_LIBRARY_TYPE PROPERTY STRINGS SHARED STATIC)

if(INFRA_LIBRARY_TYPE STREQUAL "SHARED")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
    infra_info("Library Type: SHARED")
elseif(INFRA_LIBRARY_TYPE STREQUAL "STATIC")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    infra_info("Library Type: STATIC")
else()
    infra_fatal("Invalid INFRA_LIBRARY_TYPE. Must be SHARED or STATIC")
endif()

# ============================================================================
# Core Build Options
# ============================================================================

# Optimization
option(INFRA_ENABLE_OPTIMIZATION
    "Enable compiler optimizations (-O2/-O3)" ON)

# Debug Symbols
option(INFRA_ENABLE_DEBUG_SYMBOLS
    "Enable debug symbols (-g for GCC/Clang, /Zi for MSVC)" ON)

# Position Independent Code
option(INFRA_POSITION_INDEPENDENT_CODE
    "Enable PIC (-fPIC for GCC/Clang)" ON)

# Compiler Warnings
option(INFRA_ENABLE_WARNINGS
    "Enable compiler warnings (-Wall -Wextra for GCC/Clang, /W4 for MSVC)" ON)

# Strict Mode - treat warnings as errors
option(INFRA_ENABLE_STRICT_WARNINGS
    "Treat compiler warnings as errors (-Werror for GCC/Clang, /WX for MSVC)" OFF)

# Address Sanitizer
option(INFRA_ENABLE_ASAN
    "Enable AddressSanitizer for memory debugging" OFF)

# ============================================================================
# Testing Configuration
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
# Installation Configuration
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
# Documentation Configuration
# ============================================================================

option(INFRA_BUILD_DOCS
    "Build API documentation (requires Doxygen/Sphinx)" OFF)

option(INFRA_DOCS_WITH_LATEX
    "Generate LaTeX documentation (requires Doxygen+LaTeX)" OFF)

# ============================================================================
# Dependency Management
# ============================================================================

option(INFRA_ENABLE_FETCHCONTENT
    "Enable FetchContent for missing dependencies" OFF)

option(INFRA_ENABLE_PKGCONFIG
    "Enable pkg-config support for dependencies" ON)

# ============================================================================
# Version Information
# ============================================================================

set(INFRA_VERSION_STRING "${PROJECT_VERSION}" CACHE STRING "Project version string")
set(INFRA_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE STRING "Major version")
set(INFRA_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE STRING "Minor version")
set(INFRA_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE STRING "Patch version")

infra_info("Version: ${INFRA_VERSION_STRING}")

# ============================================================================
# Module Management
# ============================================================================

# List of modules to build
# Should be set by parent CMakeLists.txt before including this file
if(NOT DEFINED INFRA_MODULES)
    set(INFRA_MODULES "" CACHE STRING "Semicolon-separated list of modules to build")
    infra_warn("INFRA_MODULES not defined - no modules will be built")
endif()

if(INFRA_MODULES)
    infra_info("Configured modules: ${INFRA_MODULES}")
endif()

# Generate enable/disable option for each module
foreach(MODULE ${INFRA_MODULES})
    string(TOUPPER "${MODULE}" MODULE_UPPER)
    if(MODULE STREQUAL "stk")
        option(INFRA_ENABLE_${MODULE_UPPER}
            "Enable ${MODULE} module" ON)
    else()
        option(INFRA_ENABLE_${MODULE_UPPER}
            "Enable ${MODULE} module" OFF)
    endif()
endforeach()

option(INFRA_STK_ENABLE_CORE
    "Enable stk core component" ON)
option(INFRA_STK_CORE_ENABLE_VECTOR
    "Enable stk core vector implementation" ON)
option(INFRA_STK_CORE_ENABLE_LIST
    "Enable stk core list implementation" ON)
option(INFRA_STK_CORE_ENABLE_RBTREE
    "Enable stk core red-black tree implementation" ON)
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

option(INFRA_STK_ENABLE_UTILS
    "Enable stk utility functions (string, path, env, math, hash)" ON)

# ============================================================================
# Debug Options
# ============================================================================

option(INFRA_DEBUG
    "Enable debug output during CMake configuration" OFF)

option(INFRA_VERBOSE_CMAKE
    "Enable verbose CMake messages" OFF)

if(INFRA_VERBOSE_CMAKE)
    set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)
endif()

# ============================================================================
# Validation and Conflict Detection
# ============================================================================

# Check for conflicting options
if(INFRA_ENABLE_ASAN AND INFRA_ENABLE_STRICT_WARNINGS)
    infra_warn("AddressSanitizer may produce warnings that conflict with -Werror")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT INFRA_ENABLE_DEBUG_SYMBOLS)
        infra_warn("Debug build without debug symbols (-g) recommended for Debug builds")
    endif()
endif()

if(BUILD_SHARED_LIBS AND INFRA_POSITION_INDEPENDENT_CODE)
    # This is expected and good
elseif(NOT BUILD_SHARED_LIBS AND NOT INFRA_POSITION_INDEPENDENT_CODE)
    infra_info("Static library build without PIC")
endif()

# ============================================================================
# Configuration Summary
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
        message(STATUS "  (none)")
    endif()

    infra_separator()
    message(STATUS "")
endfunction()

# Export function so it can be called from parent CMakeLists
function(infra_config_summary)
    infra_print_build_config()
endfunction()
