# InfraSanitizers.cmake - Sanitizer 配置

if(DEFINED INFRA_SANITIZERS_INCLUDED)
    return()
endif()
set(INFRA_SANITIZERS_INCLUDED TRUE)

# 检查编译器是否支持特定的 Sanitizer
macro(infra_check_sanitizer SANITIZER_NAME FLAG)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS ${FLAG})
    
    check_cxx_source_compiles("
        int main() { return 0; }
    " INFRA_HAVE_${SANITIZER_NAME})
    
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endmacro()

# 配置 Sanitizer 选项
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
            message(STATUS "Infra: AddressSanitizer enabled for ${TARGET}")
        else()
            message(WARNING "Infra: AddressSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_UBSAN)
        infra_check_sanitizer(UBSAN "-fsanitize=undefined")
        if(INFRA_HAVE_UBSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=undefined")
            list(APPEND SANITIZER_FLAGS "-fno-omit-frame-pointer")
            message(STATUS "Infra: UndefinedBehaviorSanitizer enabled for ${TARGET}")
        else()
            message(WARNING "Infra: UndefinedBehaviorSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_TSAN)
        infra_check_sanitizer(TSAN "-fsanitize=thread")
        if(INFRA_HAVE_TSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=thread")
            message(STATUS "Infra: ThreadSanitizer enabled for ${TARGET}")
        else()
            message(WARNING "Infra: ThreadSanitizer not supported by compiler")
        endif()
    endif()
    
    if(INFRA_ENABLE_MSAN)
        infra_check_sanitizer(MSAN "-fsanitize=memory")
        if(INFRA_HAVE_MSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=memory")
            list(APPEND SANITIZER_FLAGS "-fsanitize-memory-track-origins")
            message(STATUS "Infra: MemorySanitizer enabled for ${TARGET}")
        else()
            message(WARNING "Infra: MemorySanitizer not supported by compiler")
        endif()
    endif()
    
    if(SANITIZER_FLAGS)
        target_compile_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
    endif()
endfunction()