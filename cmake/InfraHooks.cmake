# [file name]: InfraHooks.cmake
# [file content begin]
# InfraHooks.cmake - 钩子机制
#
# 提供模块化的钩子系统，允许在构建过程的特定点注入自定义逻辑。
# 支持从文件加载钩子或直接调用命令。
#
# 宏:
#   infra_hook_append(HOOK_NAME ...)    - 向指定钩子追加回调
#   infra_hook_run(HOOK_NAME)           - 执行指定钩子的所有回调
#   infra_hook_load_dir(DIR_PATH)       - 从目录加载所有 .cmake 文件作为钩子
#   infra_hook_reset()                  - 重置所有钩子状态

# 防止重复包含
if(DEFINED INFRA_HOOKS_INCLUDED)
    return()
else()
    set(INFRA_HOOKS_INCLUDED TRUE)
endif()

# 向钩子追加回调
# 参数: HOOK_NAME - 钩子名称, 后续参数为回调（文件路径或命令名）
macro(infra_hook_append HOOK_NAME)
    # 如果钩子变量不存在，初始化为空
    if(NOT DEFINED INFRA_HOOK_${HOOK_NAME})
        set(INFRA_HOOK_${HOOK_NAME} "" CACHE INTERNAL "")
        infra_debug("Created hook: ${HOOK_NAME}")
    endif()
    # 将回调追加到钩子列表
    list(APPEND INFRA_HOOK_${HOOK_NAME} ${ARGN})
    infra_debug("Appended to hook ${HOOK_NAME}: ${ARGN}")
endmacro()

# 执行钩子
# 参数: HOOK_NAME - 要执行的钩子名称
macro(infra_hook_run HOOK_NAME)
    if(DEFINED "INFRA_HOOK_${HOOK_NAME}")
        set(_HOOK_COUNT 0)
        foreach(H IN LISTS "INFRA_HOOK_${HOOK_NAME}")
            if(EXISTS "${H}")
                # 如果是文件路径，包含该文件
                include("${H}")
                math(EXPR _HOOK_COUNT "${_HOOK_COUNT} + 1")
                infra_debug("Executed hook file: ${H}")
            elseif(COMMAND "${H}")
                # 如果是命令名，调用该命令
                cmake_language(CALL "${H}")
                math(EXPR _HOOK_COUNT "${_HOOK_COUNT} + 1")
                infra_debug("Executed hook command: ${H}")
            else()
                infra_warn("Hook callback not found: ${H}")
            endif()
        endforeach()
        if(_HOOK_COUNT GREATER 0)
            infra_debug("Hook ${HOOK_NAME} executed ${_HOOK_COUNT} callbacks")
        else()
            infra_debug("Hook ${HOOK_NAME} has no valid callbacks")
        endif()
    else()
        infra_debug("Hook ${HOOK_NAME} is not defined")
    endif()
endmacro()

# 从目录加载钩子
# 参数: DIR_PATH - 包含 .cmake 文件的目录路径
macro(infra_hook_load_dir DIR_PATH)
    if(EXISTS "${DIR_PATH}")
        # 获取目录下所有 .cmake 文件
        file(GLOB HOOK_FILES "${DIR_PATH}/*.cmake")
        if(HOOK_FILES)
            foreach(H IN LISTS HOOK_FILES)
                # 提取文件名（不含扩展名）作为钩子名
                get_filename_component(N "${H}" NAME_WE)
                # 将文件作为钩子追加
                infra_hook_append("${N}" "${H}")
            endforeach()
            infra_debug("Loaded ${_HOOK_COUNT} hooks from ${DIR_PATH}")
        else()
            infra_debug("No .cmake files found in ${DIR_PATH}")
        endif()
    else()
        infra_debug("Hook directory not found: ${DIR_PATH}")
    endif()
endmacro()

# 重置所有钩子
macro(infra_hook_reset)
    get_cmake_property(VARS VARIABLES)
    set(_RESET_COUNT 0)
    foreach(V IN LISTS VARS)
        # 清除所有 INFRA_HOOK_ 开头的变量
        if(V MATCHES "^INFRA_HOOK_")
            unset("${V}" CACHE)
            math(EXPR _RESET_COUNT "${_RESET_COUNT} + 1")
        endif()
    endforeach()
    if(_RESET_COUNT GREATER 0)
        infra_debug("Reset ${_RESET_COUNT} hooks")
    else()
        infra_debug("No hooks to reset")
    endif()
endmacro()