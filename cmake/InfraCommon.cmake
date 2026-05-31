# InfraCommon.cmake - Common utilities and messaging infrastructure
#
# This module provides foundational utilities for the Infra CMake build system.
# It establishes consistent messaging, directory setup, and basic validation patterns.
#
# REQUIRES: CMake 3.19+
# PLATFORM: Cross-platform (Windows, Linux, macOS)

if(DEFINED INFRA_COMMON_INCLUDED)
    return()
endif()
set(INFRA_COMMON_INCLUDED TRUE)

# ============================================================================
# Color Codes for Terminal Output (if supported)
# ============================================================================

if(NOT WIN32 AND NOT APPLE)
    string(ASCII 27 _ESCAPE)
    set(_RESET "${_ESCAPE}[m")
    set(_BOLD_CYAN "${_ESCAPE}[1;36m")
    set(_BOLD_GREEN "${_ESCAPE}[1;32m")
    set(_BOLD_YELLOW "${_ESCAPE}[1;33m")
    set(_BOLD_RED "${_ESCAPE}[1;31m")
else()
    set(_RESET "")
    set(_BOLD_CYAN "")
    set(_BOLD_GREEN "")
    set(_BOLD_YELLOW "")
    set(_BOLD_RED "")
endif()

# ============================================================================
# Unified Message System
# ============================================================================

# infra_message(TYPE MESSAGE)
#
# Output formatted status message with consistent prefix.
#
# PARAMETERS:
#   TYPE    - Message type: STATUS, WARNING, FATAL_ERROR
#   MESSAGE - Message text
#
# EXAMPLE:
#   infra_message(STATUS "Configuration complete")
#   infra_message(WARNING "Feature not available")
#
function(infra_message TYPE MESSAGE)
    if(TYPE STREQUAL "STATUS")
        message(STATUS "${_BOLD_CYAN}[Infra]${_RESET} ${MESSAGE}")
    elseif(TYPE STREQUAL "WARNING")
        message(WARNING "${_BOLD_YELLOW}[Infra]${_RESET} ${MESSAGE}")
    elseif(TYPE STREQUAL "FATAL_ERROR")
        message(FATAL_ERROR "${_BOLD_RED}[Infra]${_RESET} ${MESSAGE}")
    else()
        message("${MESSAGE}")
    endif()
endfunction()

# infra_info(MESSAGE)
#
# Output informational status message (shorthand for STATUS).
#
function(infra_info MESSAGE)
    infra_message(STATUS "${MESSAGE}")
endfunction()

# infra_warn(MESSAGE)
#
# Output warning message (shorthand for WARNING).
#
function(infra_warn MESSAGE)
    infra_message(WARNING "${MESSAGE}")
endfunction()

# infra_fatal(MESSAGE)
#
# Output fatal error message and stop configuration (shorthand for FATAL_ERROR).
#
function(infra_fatal MESSAGE)
    infra_message(FATAL_ERROR "${MESSAGE}")
endfunction()

# infra_success(MESSAGE)
#
# Output success message with green color.
#
function(infra_success MESSAGE)
    message(STATUS "${_BOLD_GREEN}[Infra]${_RESET} ✓ ${MESSAGE}")
endfunction()

# infra_separator()
#
# Output visual separator line for readability.
#
function(infra_separator)
    message(STATUS "${_BOLD_CYAN}========================================${_RESET}")
endfunction()

# ============================================================================
# Argument Validation
# ============================================================================

# _infra_check_required_args(FUNCTION_NAME [KEYWORD...])
#
# Validate that required keyword arguments were provided.
# Sets variable INFRA_VALID to TRUE/FALSE.
#
# INTERNAL HELPER - do not use directly
#
function(_infra_check_required_args FUNCTION_NAME)
    set(INFRA_VALID TRUE PARENT_SCOPE)

    foreach(keyword ${ARGN})
        if(NOT DEFINED ${keyword})
            set(INFRA_VALID FALSE PARENT_SCOPE)
            infra_fatal("${FUNCTION_NAME}() requires ${keyword} argument")
            return()
        endif()
    endforeach()
endfunction()

# ============================================================================
# Directory Management
# ============================================================================

# infra_setup_dirs()
#
# Configure standard project directory variables.
# Sets:
#   INFRA_INCLUDE_DIR    - ${CMAKE_SOURCE_DIR}/include
#   INFRA_BUILD_DIR      - ${CMAKE_BINARY_DIR}
#   INFRA_CMAKE_DIR      - ${CMAKE_CURRENT_LIST_DIR}
#
# PLATFORM: Cross-platform
#
function(infra_setup_dirs)
    set(INFRA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" PARENT_SCOPE)
    set(INFRA_BUILD_DIR "${CMAKE_BINARY_DIR}" PARENT_SCOPE)
    set(INFRA_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)
endfunction()

# infra_set_output_dirs()
#
# Configure CMake output directories for consistency across platforms.
# Sets:
#   CMAKE_RUNTIME_OUTPUT_DIRECTORY  - ${CMAKE_BINARY_DIR}/bin
#   CMAKE_LIBRARY_OUTPUT_DIRECTORY  - ${CMAKE_BINARY_DIR}/lib
#   CMAKE_ARCHIVE_OUTPUT_DIRECTORY  - ${CMAKE_BINARY_DIR}/lib
#
# PLATFORM: Cross-platform
# NOTE: Call this after project() and before add_library/add_executable
#
function(infra_set_output_dirs)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" PARENT_SCOPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" PARENT_SCOPE)
endfunction()

# ============================================================================
# File Operations
# ============================================================================

# infra_write_if_different(FILEPATH CONTENT)
#
# Write CONTENT to FILEPATH only if it differs from existing content.
# This avoids unnecessary rebuilds due to timestamp changes.
#
# PARAMETERS:
#   FILEPATH - Destination file path
#   CONTENT  - File content to write
#
# EXAMPLE:
#   infra_write_if_different(
#       "${CMAKE_BINARY_DIR}/version.h"
#       "#define VERSION \"${PROJECT_VERSION}\""
#   )
#
function(infra_write_if_different FILEPATH CONTENT)
    if(EXISTS "${FILEPATH}")
        file(READ "${FILEPATH}" OLD_CONTENT)
        if("${OLD_CONTENT}" STREQUAL "${CONTENT}")
            return()
        endif()
    endif()
    file(WRITE "${FILEPATH}" "${CONTENT}")
    infra_info("Generated ${FILEPATH}")
endfunction()

# ============================================================================
# Configuration Display
# ============================================================================

# infra_print_config_header(TITLE)
#
# Print formatted configuration section header.
#
# PARAMETERS:
#   TITLE - Section title
#
# EXAMPLE:
#   infra_print_config_header("Build Configuration")
#
function(infra_print_config_header TITLE)
    infra_separator()
    message(STATUS "${_BOLD_CYAN}${TITLE}${_RESET}")
    infra_separator()
endfunction()

# infra_print_config_value(NAME VALUE)
#
# Print configuration key-value pair with consistent formatting.
#
# PARAMETERS:
#   NAME  - Configuration name
#   VALUE - Configuration value
#
# EXAMPLE:
#   infra_print_config_value("Compiler" "${CMAKE_CXX_COMPILER_ID}")
#
function(infra_print_config_value NAME VALUE)
    message(STATUS "  ${NAME} : ${VALUE}")
endfunction()

# ============================================================================
# CMake Version and Feature Support
# ============================================================================

# infra_require_cmake_version(VERSION)
#
# Enforce minimum CMake version requirement.
#
# PARAMETERS:
#   VERSION - Minimum required version (e.g., "3.19")
#
# FATAL: Stops configuration if CMAKE_VERSION < VERSION
#
# EXAMPLE:
#   infra_require_cmake_version("3.19")
#
function(infra_require_cmake_version VERSION)
    if(CMAKE_VERSION VERSION_LESS "${VERSION}")
        infra_fatal("CMake ${VERSION}+ required (found ${CMAKE_VERSION})")
    endif()
    infra_info("CMake ${CMAKE_VERSION} (>= ${VERSION}) ✓")
endfunction()

# ============================================================================
# Target Properties Helper
# ============================================================================

# infra_normalize_target_name(NAME OUTPUT_VAR)
#
# Convert name to valid CMake target name (replace spaces/special chars).
#
# PARAMETERS:
#   NAME        - Input name
#   OUTPUT_VAR  - Output variable to store normalized name
#
# INTERNAL HELPER
#
function(infra_normalize_target_name NAME OUTPUT_VAR)
    string(REGEX REPLACE "[^a-zA-Z0-9_.-]" "_" NORMALIZED "${NAME}")
    set(${OUTPUT_VAR} "${NORMALIZED}" PARENT_SCOPE)
endfunction()

# ============================================================================
# Platform Detection
# ============================================================================

# infra_get_platform_name(OUTPUT_VAR)
#
# Detect and return platform name.
#
# PARAMETERS:
#   OUTPUT_VAR - Output variable
#
# RETURNS:
#   "Windows" - Windows platform
#   "Linux"   - Linux platform
#   "macOS"   - macOS platform
#   "Unix"    - Other Unix-like
#   "Unknown" - Unknown platform
#
# EXAMPLE:
#   infra_get_platform_name(PLATFORM)
#   message(STATUS "Building for ${PLATFORM}")
#
function(infra_get_platform_name OUTPUT_VAR)
    if(WIN32)
        set(PLATFORM "Windows")
    elseif(APPLE)
        set(PLATFORM "macOS")
    elseif(UNIX)
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set(PLATFORM "Linux")
        else()
            set(PLATFORM "Unix")
        endif()
    else()
        set(PLATFORM "Unknown")
    endif()
    set(${OUTPUT_VAR} "${PLATFORM}" PARENT_SCOPE)
endfunction()

# ============================================================================
# String Manipulation Helpers
# ============================================================================

# infra_string_to_identifier(STRING OUTPUT_VAR)
#
# Convert arbitrary string to valid C identifier.
#
# PARAMETERS:
#   STRING     - Input string
#   OUTPUT_VAR - Output variable
#
# EXAMPLE:
#   infra_string_to_identifier("my-module v2.0" IDENT)
#   # IDENT = "my_module_v2_0"
#
function(infra_string_to_identifier STRING OUTPUT_VAR)
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" IDENTIFIER "${STRING}")
    string(REGEX REPLACE "^[0-9]" "_" IDENTIFIER "${IDENTIFIER}")
    set(${OUTPUT_VAR} "${IDENTIFIER}" PARENT_SCOPE)
endfunction()

# ============================================================================
# Variable Checking
# ============================================================================

# infra_var_exists(VARIABLE_NAME RESULT_VAR)
#
# Check if a CMake variable is defined.
#
# PARAMETERS:
#   VARIABLE_NAME - Variable name to check
#   RESULT_VAR    - Output variable (TRUE/FALSE)
#
function(infra_var_exists VARIABLE_NAME RESULT_VAR)
    if(DEFINED ${VARIABLE_NAME})
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# Debug Support
# ============================================================================

# infra_debug(MESSAGE)
#
# Output debug message if INFRA_DEBUG is enabled.
#
# PARAMETERS:
#   MESSAGE - Debug message
#
# USAGE:
#   cmake -DINFRA_DEBUG=ON ..
#
function(infra_debug MESSAGE)
    if(INFRA_DEBUG)
        message(STATUS "${_BOLD_YELLOW}[Infra Debug]${_RESET} ${MESSAGE}")
    endif()
endfunction()

# ============================================================================
# Feature Summary
# ============================================================================

# infra_feature_summary(FEATURE STATUS)
#
# Add feature to configuration summary report.
# Should be called during configuration to build a feature list.
#
# PARAMETERS:
#   FEATURE - Feature name
#   STATUS  - "ON", "OFF", or descriptive string
#
function(infra_feature_summary FEATURE STATUS)
    get_property(SUMMARY GLOBAL PROPERTY INFRA_FEATURE_SUMMARY)
    list(APPEND SUMMARY "${FEATURE}: ${STATUS}")
    set_property(GLOBAL PROPERTY INFRA_FEATURE_SUMMARY "${SUMMARY}")
endfunction()

# infra_print_feature_summary()
#
# Output accumulated feature summary.
#
function(infra_print_feature_summary)
    get_property(SUMMARY GLOBAL PROPERTY INFRA_FEATURE_SUMMARY)
    if(NOT SUMMARY)
        return()
    endif()

    infra_print_config_header("Feature Summary")
    foreach(FEATURE_STATUS ${SUMMARY})
        message(STATUS "  ${FEATURE_STATUS}")
    endforeach()
    infra_separator()
endfunction()
