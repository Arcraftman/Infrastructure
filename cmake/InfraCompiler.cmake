# [file name]: InfraCompiler.cmake
# [file content begin]
# InfraCompiler.cmake - 编译器检测和按目标配置
#
# 检测当前使用的编译器，并提供宏来为任何目标应用项目范围的编译选项。
#
# 宏:
#   infra_detect_compiler()  - 检测编译器，设置 INFRA_COMPILER_ID
#   infra_setup_target(TARGET) - 应用警告、优化、调试、PIC 设置
#
# 设置的变量:
#   INFRA_COMPILER_ID - 取值: MSVC, GCC, Clang, Unknown
#
# 使用的选项: INFRA_ENABLE_WARNINGS, INFRA_ENABLE_OPTIMIZATION,
#              INFRA_ENABLE_DEBUG_SYMBOLS, INFRA_POSITION_INDEPENDENT_CODE
# 平台: 跨平台 (GCC, Clang, MSVC)

# 防止重复包含
if(DEFINED INFRA_COMPILER_INCLUDED)
    return()
else()
    set(INFRA_COMPILER_INCLUDED TRUE)
endif()

# 检测编译器类型
macro(infra_detect_compiler)
    if(MSVC)
        # Microsoft Visual C++
        set(INFRA_COMPILER_ID "MSVC")
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # GNU GCC
        set(INFRA_COMPILER_ID "GCC")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        # Clang (包括 Apple Clang)
        set(INFRA_COMPILER_ID "Clang")
    else()
        # 未知编译器
        set(INFRA_COMPILER_ID "Unknown")
        infra_warn("Unknown compiler detected: ${CMAKE_C_COMPILER_ID}")
    endif()
    infra_info("Detected compiler: ${INFRA_COMPILER_ID}")
endmacro()

# 为目标应用编译选项
macro(infra_setup_target TARGET)
    # 检查目标是否存在
    if(NOT TARGET ${TARGET})
        infra_warn("Target '${TARGET}' does not exist, skipping setup")
        return()
    endif()
    
    # 警告选项
    if(INFRA_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /W4)      # MSVC 高警告级别
            infra_debug("Applied MSVC warnings to ${TARGET}")
        else()
            target_compile_options(${TARGET} PRIVATE -Wall -Wextra)  # GCC/Clang 常用警告
            infra_debug("Applied GCC/Clang warnings to ${TARGET}")
        endif()
    else()
        infra_debug("Warnings disabled for ${TARGET}")
    endif()
    
    # 优化选项（非 Debug 构建时）
    if(INFRA_ENABLE_OPTIMIZATION AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /O2)      # MSVC 最大优化
            infra_debug("Applied MSVC optimization to ${TARGET}")
        else()
            target_compile_options(${TARGET} PRIVATE -O2)      # GCC/Clang O2 优化
            infra_debug("Applied GCC/Clang optimization to ${TARGET}")
        endif()
    else()
        infra_debug("Optimization disabled for ${TARGET} (build type: ${CMAKE_BUILD_TYPE})")
    endif()
    
    # 调试符号（仅 Debug 构建时）
    if(INFRA_ENABLE_DEBUG_SYMBOLS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /Zi)      # MSVC 调试信息
            infra_debug("Applied MSVC debug symbols to ${TARGET}")
        else()
            target_compile_options(${TARGET} PRIVATE -g)       # GCC/Clang 调试信息
            infra_debug("Applied GCC/Clang debug symbols to ${TARGET}")
        endif()
    else()
        if(NOT INFRA_ENABLE_DEBUG_SYMBOLS)
            infra_debug("Debug symbols disabled for ${TARGET}")
        else()
            infra_debug("Debug symbols skipped for ${TARGET} (build type: ${CMAKE_BUILD_TYPE})")
        endif()
    endif()
    
    # 覆盖率插桩（GCC/Clang；通常与 Debug + CTest 配合）
    if(INFRA_ENABLE_COVERAGE)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${TARGET} PRIVATE --coverage)
            target_link_options(${TARGET} PRIVATE --coverage)
            infra_debug("Applied coverage instrumentation to ${TARGET}")
        else()
            infra_warn("Coverage is enabled but the current compiler is not GCC/Clang")
        endif()
    endif()

    # 位置无关代码（用于共享库）
    if(INFRA_POSITION_INDEPENDENT_CODE)
        set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        infra_debug("Applied PIC to ${TARGET}")
    else()
        infra_debug("PIC disabled for ${TARGET}")
    endif()
endmacro()

# 自动检测编译器
infra_detect_compiler()