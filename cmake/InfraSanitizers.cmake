# InfraSanitizers.cmake - Sanitizer configuration (ASan, UBSan, TSan, MSan)
#
# Provides per-target sanitizer setup with compiler-capability checks.
# Each sanitizer is enabled via its own option and checked for compiler support
# before applying flags.
#
# FUNCTIONS:
#   infra_setup_sanitizers(TARGET) - Apply enabled sanitizers to a build target
#
# OPTIONS:
#   INFRA_ENABLE_ASAN - AddressSanitizer
#   INFRA_ENABLE_UBSAN - UndefinedBehaviorSanitizer
#   INFRA_ENABLE_TSAN - ThreadSanitizer
#   INFRA_ENABLE_MSAN - MemorySanitizer
#
# PLATFORM: GCC / Clang (MSVC not supported)

if(DEFINED INFRA_SANITIZERS_INCLUDED)
    return()
endif()
set(INFRA_SANITIZERS_INCLUDED TRUE)

include(CheckCXXSourceCompiles)

option(INFRA_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(INFRA_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(INFRA_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(INFRA_ENABLE_MSAN "Enable MemorySanitizer" OFF)

function(infra_check_sanitizer SANITIZER_NAME FLAG)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS ${FLAG})
    
    check_cxx_source_compiles("
        int main() { return 0; }
    " INFRA_HAVE_${SANITIZER_NAME})
    
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    set(INFRA_HAVE_${SANITIZER_NAME} PARENT_SCOPE)
endfunction()

function(infra_setup_sanitizers TARGET)
    if(NOT INFRA_ENABLE_ASAN AND NOT INFRA_ENABLE_UBSAN AND NOT INFRA_ENABLE_TSAN AND NOT INFRA_ENABLE_MSAN)
        return()
    endif()
    
    set(SANITIZER_FLAGS "")
    
    if(INFRA_ENABLE_ASAN)
        infra_check_sanitizer(ASAN "-fsanitize=address")
        if(INFRA_HAVE_ASAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=address")
            list(APPEND SANITIZER_FLAGS "-fno-omit-frame-pointer")
            list(APPEND SANITIZER_FLAGS "-fno-common")
            infra_info("AddressSanitizer enabled for ${TARGET}")
        else()
            infra_warn("AddressSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_UBSAN)
        infra_check_sanitizer(UBSAN "-fsanitize=undefined")
        if(INFRA_HAVE_UBSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=undefined")
            list(APPEND SANITIZER_FLAGS "-fno-omit-frame-pointer")
            infra_info("UndefinedBehaviorSanitizer enabled for ${TARGET}")
        else()
            infra_warn("UndefinedBehaviorSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_TSAN)
        infra_check_sanitizer(TSAN "-fsanitize=thread")
        if(INFRA_HAVE_TSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=thread")
            infra_info("ThreadSanitizer enabled for ${TARGET}")
        else()
            infra_warn("ThreadSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_MSAN)
        infra_check_sanitizer(MSAN "-fsanitize=memory")
        if(INFRA_HAVE_MSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=memory")
            list(APPEND SANITIZER_FLAGS "-fsanitize-memory-track-origins")
            infra_info("MemorySanitizer enabled for ${TARGET}")
        else()
            infra_warn("MemorySanitizer not supported by compiler")
        endif()
    endif()
    
    if(SANITIZER_FLAGS)
        target_compile_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
    endif()
endfunction()