# [file name]: InfraUtils.cmake
# [file content begin]
# InfraUtils.cmake - Infra 构建系统的实用辅助函数
#
# 提供简单的日志包装器和文件操作辅助函数。

# 防止重复包含
if(DEFINED INFRA_UTILS_INCLUDED)
    return()
else()
    set(INFRA_UTILS_INCLUDED TRUE)
endif()

# ---------------------------------------------------------------------------
# 日志辅助函数 - 带有 "Infra:" 前缀的薄包装器
# ---------------------------------------------------------------------------

# 输出信息消息
function(infra_print_info MSG)
    message(STATUS "Infra: ${MSG}")
endfunction()

# 输出警告消息
function(infra_print_warning MSG)
    message(WARNING "Infra: ${MSG}")
endfunction()

# 输出成功消息
function(infra_print_success MSG)
    message(STATUS "Infra: ${MSG}")
endfunction()

# 输出错误消息并终止
function(infra_print_error MSG)
    message(FATAL_ERROR "Infra: ${MSG}")
endfunction()

# 输出分隔线
function(infra_separator)
    message(STATUS "")
    message(STATUS "========================================")
endfunction()

# ---------------------------------------------------------------------------
# 文件辅助函数 - 内容门控写入，避免不必要的重新构建
# ---------------------------------------------------------------------------

# 仅当内容不同时才更新文件
function(infra_update_file FILEPATH CONTENT)
    if(EXISTS "${FILEPATH}")
        file(READ "${FILEPATH}" OLD)
        # 如果内容相同，跳过写入
        if("${OLD}" STREQUAL "${CONTENT}")
            infra_debug("File ${FILEPATH} unchanged, skipping")
            return()
        else()
            infra_debug("File ${FILEPATH} changed, updating")
        endif()
    else()
        infra_debug("File ${FILEPATH} does not exist, creating")
    endif()
    # 写入新内容
    file(WRITE "${FILEPATH}" "${CONTENT}")
endfunction()