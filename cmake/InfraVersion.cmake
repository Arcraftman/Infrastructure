# [file name]: InfraVersion.cmake
# [file content begin]
# InfraVersion.cmake - 版本字符串生成
#
# 提供从 Git 标签（git describe）派生版本字符串的辅助函数，
# 或回退到项目定义的版本，并生成版本头文件。
#
# 函数:
#   infra_generate_version(OUTPUT_PATH) - 生成版本头文件
#
# 宏:
#   infra_git_describe(VAR_NAME PATH) - 运行 git describe，结果存入 VAR_NAME
#
# 使用的变量:
#   GIT_FOUND, GIT_EXECUTABLE - 来自 FindGit
#   PROJECT_VERSION          - 来自 project() 命令
# 平台: 跨平台（需要 Git 支持 describe）

# 防止重复包含
if(DEFINED INFRA_VERSION_INCLUDED)
    return()
endif()
set(INFRA_VERSION_INCLUDED TRUE)

# 获取 Git 描述信息
macro(infra_git_describe VAR_NAME PATH)
    if(GIT_FOUND)
        # 执行 git describe 获取版本标签
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
            WORKING_DIRECTORY ${PATH}
            OUTPUT_VARIABLE ${VAR_NAME}
            RESULT_VARIABLE GIT_RESULT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT GIT_RESULT EQUAL 0)
            set(${VAR_NAME} "unknown")
        endif()
    else()
        set(${VAR_NAME} "unknown")
    endif()
endmacro()

# 生成版本头文件
function(infra_generate_version OUTPUT_PATH)
    # 首先尝试从 Git 获取版本
    infra_git_describe(INFRA_GIT_VERSION "${CMAKE_SOURCE_DIR}")
    if(INFRA_GIT_VERSION STREQUAL "unknown")
        # Git 失败，使用项目版本
        set(INFRA_VERSION_STRING "${PROJECT_VERSION}")
    else()
        # 使用 Git 描述的版本
        set(INFRA_VERSION_STRING "${INFRA_GIT_VERSION}")
    endif()

    # 从 PROJECT_VERSION 解析版本组件
    set(INFRA_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set(INFRA_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    set(INFRA_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

    # 将版本字符串传播到调用者作用域
    set(INFRA_VERSION_STRING "${INFRA_VERSION_STRING}" PARENT_SCOPE)

    # 生成版本头文件
    file(WRITE "${OUTPUT_PATH}"
        "#ifndef INFRA_VERSION_H\n"
        "#define INFRA_VERSION_H\n"
        "\n"
        "#define INFRA_VERSION_STRING \"${INFRA_VERSION_STRING}\"\n"
        "#define INFRA_VERSION_MAJOR ${INFRA_VERSION_MAJOR}\n"
        "#define INFRA_VERSION_MINOR ${INFRA_VERSION_MINOR}\n"
        "#define INFRA_VERSION_PATCH ${INFRA_VERSION_PATCH}\n"
        "#define INFRA_VERSION_TWEAK 0\n"
        "\n"
        "#endif /* INFRA_VERSION_H */\n"
    )
    message(STATUS "Generated version header: ${OUTPUT_PATH}")
endfunction()