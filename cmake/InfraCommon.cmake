# [file name]: InfraCommon.cmake
# [file content begin]
# InfraCommon.cmake - 通用工具和消息基础设施
#
# 该模块为 Infra CMake 构建系统提供基础工具函数。
# 它建立统一的消息输出、目录设置和基本的参数验证模式。
#
# 要求: CMake 3.19+
# 平台: 跨平台 (Windows, Linux, macOS)

# 防止重复包含
if(DEFINED INFRA_COMMON_INCLUDED)
    return()
endif()
set(INFRA_COMMON_INCLUDED TRUE)

# ============================================================================
# 终端输出颜色代码（如果支持）
# ============================================================================

# 非 Windows 且非 macOS 平台支持 ANSI 转义颜色
if(NOT WIN32 AND NOT APPLE)
    string(ASCII 27 _ESCAPE)                           # ESC 字符
    set(_RESET "${_ESCAPE}[m")                         # 重置颜色
    set(_BOLD_CYAN "${_ESCAPE}[1;36m")                 # 粗体青色
    set(_BOLD_GREEN "${_ESCAPE}[1;32m")                # 粗体绿色
    set(_BOLD_YELLOW "${_ESCAPE}[1;33m")               # 粗体黄色
    set(_BOLD_RED "${_ESCAPE}[1;31m")                  # 粗体红色
else()
    # Windows 或 macOS 不支持颜色
    set(_RESET "")
    set(_BOLD_CYAN "")
    set(_BOLD_GREEN "")
    set(_BOLD_YELLOW "")
    set(_BOLD_RED "")
endif()

# ============================================================================
# 统一消息系统
# ============================================================================

# infra_message(TYPE MESSAGE)
#
# 输出格式化的状态消息，带有统一的前缀。
#
# 参数:
#   TYPE    - 消息类型: STATUS, WARNING, FATAL_ERROR
#   MESSAGE - 消息内容
#
# 示例:
#   infra_message(STATUS "Configuration complete")
#   infra_message(WARNING "Feature not available")
#
function(infra_message TYPE MESSAGE)
    if(TYPE STREQUAL "STATUS")
        # 状态信息 - 青色前缀
        message(STATUS "${_BOLD_CYAN}[Infra]${_RESET} ${MESSAGE}")
    elseif(TYPE STREQUAL "WARNING")
        # 警告信息 - 黄色前缀
        message(WARNING "${_BOLD_YELLOW}[Infra]${_RESET} ${MESSAGE}")
    elseif(TYPE STREQUAL "FATAL_ERROR")
        # 致命错误 - 红色前缀，停止配置
        message(FATAL_ERROR "${_BOLD_RED}[Infra]${_RESET} ${MESSAGE}")
    else()
        # 未知类型，直接输出
        message("${MESSAGE}")
    endif()
endfunction()

# infra_info(MESSAGE)
#
# 输出信息性状态消息（STATUS 的简写）。
#
function(infra_info MESSAGE)
    infra_message(STATUS "${MESSAGE}")
endfunction()

# infra_warn(MESSAGE)
#
# 输出警告消息（WARNING 的简写）。
#
function(infra_warn MESSAGE)
    infra_message(WARNING "${MESSAGE}")
endfunction()

# infra_fatal(MESSAGE)
#
# 输出致命错误并停止配置（FATAL_ERROR 的简写）。
#
function(infra_fatal MESSAGE)
    infra_message(FATAL_ERROR "${MESSAGE}")
endfunction()

# infra_success(MESSAGE)
#
# 输出成功消息，使用绿色显示。
#
function(infra_success MESSAGE)
    message(STATUS "${_BOLD_GREEN}[Infra]${_RESET} ✓ ${MESSAGE}")
endfunction()

# infra_separator()
#
# 输出视觉分隔线，提高可读性。
#
function(infra_separator)
    message(STATUS "${_BOLD_CYAN}========================================${_RESET}")
endfunction()

# ============================================================================
# 参数验证
# ============================================================================

# _infra_check_required_args(FUNCTION_NAME [KEYWORD...])
#
# 验证必需的命名参数是否已提供。
# 设置变量 INFRA_VALID 为 TRUE/FALSE。
#
# 内部辅助函数 - 不要直接使用
#
function(_infra_check_required_args FUNCTION_NAME)
    set(INFRA_VALID TRUE PARENT_SCOPE)  # 默认有效

    # 遍历所有必需的参数关键字
    foreach(keyword ${ARGN})
        if(NOT DEFINED ${keyword})
            # 参数未定义，标记为无效并报错
            set(INFRA_VALID FALSE PARENT_SCOPE)
            infra_fatal("${FUNCTION_NAME}() requires ${keyword} argument")
            return()
        endif()
    endforeach()
endfunction()

# ============================================================================
# 目录管理
# ============================================================================

# infra_setup_dirs()
#
# 配置标准的项目目录变量。
# 设置:
#   INFRA_INCLUDE_DIR    - ${CMAKE_SOURCE_DIR}/include
#   INFRA_BUILD_DIR      - ${CMAKE_BINARY_DIR}
#   INFRA_CMAKE_DIR      - ${CMAKE_CURRENT_LIST_DIR}
#
# 平台: 跨平台
#
function(infra_setup_dirs)
    set(INFRA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" PARENT_SCOPE)
    set(INFRA_BUILD_DIR "${CMAKE_BINARY_DIR}" PARENT_SCOPE)
    set(INFRA_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)
endfunction()

# infra_set_output_dirs()
#
# 配置 CMake 输出目录，确保跨平台一致性。
# 设置:
#   CMAKE_RUNTIME_OUTPUT_DIRECTORY  - ${CMAKE_BINARY_DIR}/bin（可执行文件）
#   CMAKE_LIBRARY_OUTPUT_DIRECTORY  - ${CMAKE_BINARY_DIR}/lib（动态库）
#   CMAKE_ARCHIVE_OUTPUT_DIRECTORY  - ${CMAKE_BINARY_DIR}/lib（静态库）
#
# 平台: 跨平台
# 注意: 在 project() 之后、add_library/add_executable 之前调用
#
function(infra_set_output_dirs)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" PARENT_SCOPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" PARENT_SCOPE)
endfunction()

# ============================================================================
# 文件操作
# ============================================================================

# infra_write_if_different(FILEPATH CONTENT)
#
# 仅当内容与现有文件不同时才写入内容到 FILEPATH。
# 这可以避免因时间戳变化而导致的不必要重新构建。
#
# 参数:
#   FILEPATH - 目标文件路径
#   CONTENT  - 要写入的文件内容
#
# 示例:
#   infra_write_if_different(
#       "${CMAKE_BINARY_DIR}/version.h"
#       "#define VERSION \"${PROJECT_VERSION}\""
#   )
#
function(infra_write_if_different FILEPATH CONTENT)
    if(EXISTS "${FILEPATH}")
        # 读取现有内容
        file(READ "${FILEPATH}" OLD_CONTENT)
        # 如果内容相同则跳过写入
        if("${OLD_CONTENT}" STREQUAL "${CONTENT}")
            return()
        endif()
    endif()
    # 写入新内容
    file(WRITE "${FILEPATH}" "${CONTENT}")
    infra_info("Generated ${FILEPATH}")
endfunction()

# ============================================================================
# 配置信息显示
# ============================================================================

# infra_print_config_header(TITLE)
#
# 打印格式化的配置段落标题。
#
# 参数:
#   TITLE - 段落标题
#
# 示例:
#   infra_print_config_header("Build Configuration")
#
function(infra_print_config_header TITLE)
    infra_separator()
    message(STATUS "${_BOLD_CYAN}${TITLE}${_RESET}")
    infra_separator()
endfunction()

# infra_print_config_value(NAME VALUE)
#
# 打印配置键值对，格式统一。
#
# 参数:
#   NAME  - 配置项名称
#   VALUE - 配置项值
#
# 示例:
#   infra_print_config_value("Compiler" "${CMAKE_CXX_COMPILER_ID}")
#
function(infra_print_config_value NAME VALUE)
    message(STATUS "  ${NAME} : ${VALUE}")
endfunction()

# ============================================================================
# CMake 版本和功能支持
# ============================================================================

# infra_require_cmake_version(VERSION)
#
# 强制要求最低 CMake 版本。
#
# 参数:
#   VERSION - 最低要求的版本（例如 "3.19"）
#
# 致命: 如果 CMAKE_VERSION < VERSION 则停止配置
#
# 示例:
#   infra_require_cmake_version("3.19")
#
function(infra_require_cmake_version VERSION)
    if(CMAKE_VERSION VERSION_LESS "${VERSION}")
        # 版本不满足要求
        infra_fatal("CMake ${VERSION}+ required (found ${CMAKE_VERSION})")
    endif()
    infra_info("CMake ${CMAKE_VERSION} (>= ${VERSION}) ✓")
endfunction()

# ============================================================================
# 目标属性辅助函数
# ============================================================================

# infra_normalize_target_name(NAME OUTPUT_VAR)
#
# 将名称转换为有效的 CMake 目标名称（替换空格和特殊字符）。
#
# 参数:
#   NAME        - 输入名称
#   OUTPUT_VAR  - 存储规范化名称的输出变量
#
# 内部辅助函数
#
function(infra_normalize_target_name NAME OUTPUT_VAR)
    # 将非字母数字、点、下划线、连字符的字符替换为下划线
    string(REGEX REPLACE "[^a-zA-Z0-9_.-]" "_" NORMALIZED "${NAME}")
    set(${OUTPUT_VAR} "${NORMALIZED}" PARENT_SCOPE)
endfunction()

# ============================================================================
# 平台检测
# ============================================================================

# infra_get_platform_name(OUTPUT_VAR)
#
# 检测并返回平台名称。
#
# 参数:
#   OUTPUT_VAR - 输出变量
#
# 返回值:
#   "Windows" - Windows 平台
#   "Linux"   - Linux 平台
#   "macOS"   - macOS 平台
#   "Unix"    - 其他类 Unix 系统
#   "Unknown" - 未知平台
#
# 示例:
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
# 字符串操作辅助函数
# ============================================================================

# infra_string_to_identifier(STRING OUTPUT_VAR)
#
# 将任意字符串转换为有效的 C 标识符。
#
# 参数:
#   STRING     - 输入字符串
#   OUTPUT_VAR - 输出变量
#
# 示例:
#   infra_string_to_identifier("my-module v2.0" IDENT)
#   # IDENT = "my_module_v2_0"
#
function(infra_string_to_identifier STRING OUTPUT_VAR)
    # 将非字母数字和下划线的字符替换为下划线
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" IDENTIFIER "${STRING}")
    # 如果以数字开头，添加前缀下划线
    string(REGEX REPLACE "^[0-9]" "_" IDENTIFIER "${IDENTIFIER}")
    set(${OUTPUT_VAR} "${IDENTIFIER}" PARENT_SCOPE)
endfunction()

# ============================================================================
# 变量检查
# ============================================================================

# infra_var_exists(VARIABLE_NAME RESULT_VAR)
#
# 检查 CMake 变量是否已定义。
#
# 参数:
#   VARIABLE_NAME - 要检查的变量名
#   RESULT_VAR    - 输出变量 (TRUE/FALSE)
#
function(infra_var_exists VARIABLE_NAME RESULT_VAR)
    if(DEFINED ${VARIABLE_NAME})
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# 调试支持
# ============================================================================

# infra_debug(MESSAGE)
#
# 如果启用了 INFRA_DEBUG，输出调试消息。
#
# 参数:
#   MESSAGE - 调试消息
#
# 用法:
#   cmake -DINFRA_DEBUG=ON ..
#
function(infra_debug MESSAGE)
    if(INFRA_DEBUG)
        message(STATUS "${_BOLD_YELLOW}[Infra Debug]${_RESET} ${MESSAGE}")
    endif()
endfunction()

# ============================================================================
# 功能摘要
# ============================================================================

# infra_feature_summary(FEATURE STATUS)
#
# 将功能添加到配置摘要报告中。
# 应在配置期间调用以构建功能列表。
#
# 参数:
#   FEATURE - 功能名称
#   STATUS  - "ON"、"OFF" 或描述性字符串
#
function(infra_feature_summary FEATURE STATUS)
    # 获取全局属性中的功能摘要列表
    get_property(SUMMARY GLOBAL PROPERTY INFRA_FEATURE_SUMMARY)
    list(APPEND SUMMARY "${FEATURE}: ${STATUS}")
    set_property(GLOBAL PROPERTY INFRA_FEATURE_SUMMARY "${SUMMARY}")
endfunction()

# infra_print_feature_summary()
#
# 输出累积的功能摘要。
#
function(infra_print_feature_summary)
    # 获取全局属性中的功能摘要列表
    get_property(SUMMARY GLOBAL PROPERTY INFRA_FEATURE_SUMMARY)
    if(NOT SUMMARY)
        return()
    endif()

    # 打印标题和所有功能状态
    infra_print_config_header("Feature Summary")
    foreach(FEATURE_STATUS ${SUMMARY})
        message(STATUS "  ${FEATURE_STATUS}")
    endforeach()
    infra_separator()
endfunction()