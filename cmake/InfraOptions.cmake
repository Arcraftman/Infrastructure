# [file name]: InfraOptions.cmake
# [file content begin]
# InfraOptions.cmake - 每个模块的构建选项管理
#
# 通过扫描每个模块的选项文件来发现选项。
#
# 函数:
#   infra_define_options() - 从每个模块的 cmake 文件加载选项定义
#
# 依赖: 无
# 平台: 跨平台

# 防止重复包含
if(DEFINED INFRA_OPTIONS_INCLUDED)
    return()
else()
    set(INFRA_OPTIONS_INCLUDED TRUE)
endif()

# 定义模块选项
function(infra_define_options)
    if(INFRA_MODULES)
        infra_debug("Defining options for modules: ${INFRA_MODULES}")
        foreach(MODULE ${INFRA_MODULES})
            # 构建选项文件路径: modules/${MODULE}/${MODULE}.cmake
            set(OPTIONS_FILE "${PROJECT_SOURCE_DIR}/modules/${MODULE}/${MODULE}.cmake")
            if(EXISTS "${OPTIONS_FILE}")
                # 如果选项文件存在，包含它
                include("${OPTIONS_FILE}")
                infra_debug("Loaded options from: ${OPTIONS_FILE}")
            else()
                infra_debug("No options file for module ${MODULE}: ${OPTIONS_FILE}")
            endif()
        endforeach()
    else()
        infra_debug("No modules defined, skipping options")
    endif()
endfunction()