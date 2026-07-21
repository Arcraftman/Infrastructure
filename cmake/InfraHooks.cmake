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
endif()
set(INFRA_HOOKS_INCLUDED TRUE)

# 向钩子追加回调
# 参数: HOOK_NAME - 钩子名称, 后续参数为回调（文件路径或命令名）
macro(infra_hook_append HOOK_NAME)
    # 如果钩子变量不存在，初始化为空
    if(NOT DEFINED INFRA_HOOK_${HOOK_NAME})
        set(INFRA_HOOK_${HOOK_NAME} "" CACHE INTERNAL "")
    endif()
    # 将回调追加到钩子列表
    list(APPEND INFRA_HOOK_${HOOK_NAME} ${ARGN})
endmacro()

# 执行钩子
# 参数: HOOK_NAME - 要执行的钩子名称
macro(infra_hook_run HOOK_NAME)
    if(DEFINED "INFRA_HOOK_${HOOK_NAME}")
        foreach(H IN LISTS "INFRA_HOOK_${HOOK_NAME}")
            if(EXISTS "${H}")
                # 如果是文件路径，包含该文件
                include("${H}")
            elseif(COMMAND "${H}")
                # 如果是命令名，调用该命令
                cmake_language(CALL "${H}")
            endif()
        endforeach()
    endif()
endmacro()

# 从目录加载钩子
# 参数: DIR_PATH - 包含 .cmake 文件的目录路径
macro(infra_hook_load_dir DIR_PATH)
    if(EXISTS "${DIR_PATH}")
        # 获取目录下所有 .cmake 文件
        file(GLOB HOOK_FILES "${DIR_PATH}/*.cmake")
        foreach(H IN LISTS HOOK_FILES)
            # 提取文件名（不含扩展名）作为钩子名
            get_filename_component(N "${H}" NAME_WE)
            # 将文件作为钩子追加
            infra_hook_append("${N}" "${H}")
        endforeach()
    endif()
endmacro()

# 重置所有钩子
macro(infra_hook_reset)
    get_cmake_property(VARS VARIABLES)
    foreach(V IN LISTS VARS)
        # 清除所有 INFRA_HOOK_ 开头的变量
        if(V MATCHES "^INFRA_HOOK_")
            unset("${V}" CACHE)
        endif()
    endforeach()
endmacro()