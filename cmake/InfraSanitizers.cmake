# [file name]: InfraSanitizers.cmake
# [file content begin]
# InfraSanitizers.cmake - 消毒剂配置 (ASan, UBSan, TSan, MSan)
#
# 提供按目标的消毒剂设置，包含编译器能力检查。
# 每个消毒剂通过其自己的选项启用，并在应用标志前检查编译器支持。
#
# 函数:
#   infra_setup_sanitizers(TARGET) - 将启用的消毒剂应用于构建目标
#
# 选项:
#   INFRA_ENABLE_ASAN - 地址消毒剂 (AddressSanitizer)
#   INFRA_ENABLE_UBSAN - 未定义行为消毒剂 (UndefinedBehaviorSanitizer)
#   INFRA_ENABLE_TSAN - 线程消毒剂 (ThreadSanitizer)
#   INFRA_ENABLE_MSAN - 内存消毒剂 (MemorySanitizer)
#
# 平台: GCC / Clang (不支持 MSVC)

# 防止重复包含
if(DEFINED INFRA_SANITIZERS_INCLUDED)
    return()
else()
    set(INFRA_SANITIZERS_INCLUDED TRUE)
endif()

# 包含编译测试模块
include(CheckCXXSourceCompiles)

# 消毒剂选项
option(INFRA_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(INFRA_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(INFRA_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(INFRA_ENABLE_MSAN "Enable MemorySanitizer" OFF)

# 检查编译器是否支持指定的消毒剂
function(infra_check_sanitizer SANITIZER_NAME FLAG)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS ${FLAG})
    
    # 编译一个简单的测试程序来检测消毒剂是否可用
    check_cxx_source_compiles("
        int main() { return 0; }
    " INFRA_HAVE_${SANITIZER_NAME})
    
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    set(INFRA_HAVE_${SANITIZER_NAME} PARENT_SCOPE)
    
    if(INFRA_HAVE_${SANITIZER_NAME})
        infra_debug("Compiler supports ${SANITIZER_NAME} (${FLAG})")
    else()
        infra_debug("Compiler does not support ${SANITIZER_NAME} (${FLAG})")
    endif()
endfunction()

# 为目标设置消毒剂
function(infra_setup_sanitizers TARGET)
    # 如果未启用任何消毒剂，直接返回
    if(NOT INFRA_ENABLE_ASAN AND NOT INFRA_ENABLE_UBSAN AND NOT INFRA_ENABLE_TSAN AND NOT INFRA_ENABLE_MSAN)
        infra_debug("No sanitizers enabled for ${TARGET}")
        return()
    endif()
    
    set(SANITIZER_FLAGS "")
    
    # 地址消毒剂
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
    
    # 未定义行为消毒剂
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
    
    # 线程消毒剂
    if(INFRA_ENABLE_TSAN)
        infra_check_sanitizer(TSAN "-fsanitize=thread")
        if(INFRA_HAVE_TSAN)
            list(APPEND SANITIZER_FLAGS "-fsanitize=thread")
            infra_info("ThreadSanitizer enabled for ${TARGET}")
        else()
            infra_warn("ThreadSanitizer not supported by compiler")
        endif()
    endif()
    
    # 内存消毒剂
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
    
    # 如果有消毒剂标志，应用到目标
    if(SANITIZER_FLAGS)
        target_compile_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${TARGET} PRIVATE ${SANITIZER_FLAGS})
        infra_debug("Applied sanitizer flags to ${TARGET}: ${SANITIZER_FLAGS}")
    else()
        infra_debug("No sanitizer flags to apply to ${TARGET}")
    endif()
endfunction()