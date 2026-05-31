# InfraCompiler.cmake - Compiler detection and per-target configuration
#
# Detects the active compiler and provides a macro to apply project-wide
# compile options to any target.
#
# MACROS:
#   infra_detect_compiler()  - Detect compiler, set INFRA_COMPILER_ID
#   infra_setup_target(TARGET) - Apply warnings, optimization, debug, PIC
#
# VARIABLES SET:
#   INFRA_COMPILER_ID - One of: MSVC, GCC, Clang, Unknown
#
# OPTIONS USED: INFRA_ENABLE_WARNINGS, INFRA_ENABLE_OPTIMIZATION,
#               INFRA_ENABLE_DEBUG_SYMBOLS, INFRA_POSITION_INDEPENDENT_CODE
# PLATFORM: Cross-platform (GCC, Clang, MSVC)

if(DEFINED INFRA_COMPILER_INCLUDED)
    return()
endif()
set(INFRA_COMPILER_INCLUDED TRUE)

macro(infra_detect_compiler)
    if(MSVC)
        set(INFRA_COMPILER_ID "MSVC")
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(INFRA_COMPILER_ID "GCC")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(INFRA_COMPILER_ID "Clang")
    else()
        set(INFRA_COMPILER_ID "Unknown")
    endif()
    infra_info("Detected compiler: ${INFRA_COMPILER_ID}")
endmacro()

macro(infra_setup_target TARGET)
    if(INFRA_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /W4)
        else()
            target_compile_options(${TARGET} PRIVATE -Wall -Wextra)
        endif()
    endif()
    
    if(INFRA_ENABLE_OPTIMIZATION AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /O2)
        else()
            target_compile_options(${TARGET} PRIVATE -O2)
        endif()
    endif()
    
    if(INFRA_ENABLE_DEBUG_SYMBOLS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /Zi)
        else()
            target_compile_options(${TARGET} PRIVATE -g)
        endif()
    endif()
    
    if(INFRA_POSITION_INDEPENDENT_CODE)
        set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
endmacro()

infra_detect_compiler()