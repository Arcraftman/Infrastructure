# InfraOptions.cmake - 选项管理

if(DEFINED INFRA_OPTIONS_INCLUDED)
    return()
endif()
infra_set(INFRA_OPTIONS_INCLUDED TRUE)

# 简单的模块选项定义 macro
macro(infra_define_options)
    foreach(MODULE ${INFRA_MODULES})
        string(TOUPPER ${MODULE} MODULE_UPPER)
        option(INFRA_ENABLE_${MODULE_UPPER} "Enable ${MODULE} module" OFF)
        
        # 如果存在模块选项文件，则包含它
        set(OPTIONS_FILE "${CMAKE_SOURCE_DIR}/modules/${MODULE}/${MODULE}.cmake")
        if(EXISTS ${OPTIONS_FILE})
            include(${OPTIONS_FILE})
        endif()
    endforeach()
endmacro()