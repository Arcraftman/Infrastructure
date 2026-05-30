# InfraSanitizers.cmake - Sanitizer 配置（全部用 macro）

if(DEFINED INFRA_SANITIZERS_INCLUDED)
    return()
endif()
infra_set(INFRA_SANITIZERS_INCLUDED TRUE)

include(CheckCXXSourceCompiles)

option(INFRA_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(INFRA_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(INFRA_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(INFRA_ENABLE_MSAN "Enable MemorySanitizer" OFF)

macro(infra_check_sanitizer SANITIZER_NAME FLAG)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    infra_set(CMAKE_REQUIRED_FLAGS ${FLAG})
    
    check_cxx_source_compiles("
        int main() { return 0; }
    " INFRA_HAVE_${SANITIZER_NAME})
    
    infra_set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endmacro()

macro(infra_setup_sanitizers TARGET)
    if(NOT INFRA_ENABLE_ASAN AND NOT INFRA_ENABLE_UBSAN AND NOT INFRA_ENABLE_TSAN AND NOT INFRA_ENABLE_MSAN)
        return()
    endif()
    
    infra_set(SANITIZER_FLAGS "")
    
    if(INFRA_ENABLE_ASAN)
        infra_check_sanitizer(ASAN "-fsanitize=address")
        if(INFRA_HAVE_ASAN)
            infra_append(SANITIZER_FLAGS "-fsanitize=address")
            infra_append(SANITIZER_FLAGS "-fno-omit-frame-pointer")
            infra_append(SANITIZER_FLAGS "-fno-common")
            infra_print_info("AddressSanitizer enabled for ${TARGET}")
        else()
            infra_print_warning("AddressSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_UBSAN)
        infra_check_sanitizer(UBSAN "-fsanitize=undefined")
        if(INFRA_HAVE_UBSAN)
            infra_append(SANITIZER_FLAGS "-fsanitize=undefined")
            infra_append(SANITIZER_FLAGS "-fno-omit-frame-pointer")
            infra_print_info("UndefinedBehaviorSanitizer enabled for ${TARGET}")
        else()
            infra_print_warning("UndefinedBehaviorSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_TSAN)
        infra_check_sanitizer(TSAN "-fsanitize=thread")
        if(INFRA_HAVE_TSAN)
            infra_append(SANITIZER_FLAGS "-fsanitize=thread")
            infra_print_info("ThreadSanitizer enabled for ${TARGET}")
        else()
            infra_print_warning("ThreadSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_MSAN)
        infra_check_sanitizer(MSAN "-fsanitize=memory")
        if(INFRA_HAVE_MSAN)
            infra_append(SANITIZER_FLAGS "-fsanitize=memory")
            infra_append(SANITIZER_FLAGS "-fsanitize-memory-track-origins")
            infra_print_info("MemorySanitizer enabled for ${TARGET}")
        else()
            infra_print_warning("MemorySanitizer not supported by compiler")
        endif()
    endif()
    
    if(SANITIZER_FLAGS)
        target_compile_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
    endif()
endmacro()