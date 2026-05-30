# InfraModules.cmake - 模块管理（通用版本）

if(DEFINED INFRA_MODULES_INCLUDED)
    return()
endif()
infra_set(INFRA_MODULES_INCLUDED TRUE)

include(InfraUtils)
include(InfraCompiler)

# 全局状态
infra_set(INFRA_REGISTERED_MODULES "")
infra_set(INFRA_OUTPUT_DIRECTORIES_SET FALSE)

# 输出目录
macro(infra_setup_output_dirs)
    if(INFRA_OUTPUT_DIRECTORIES_SET)
        return()
    endif()
    infra_set_output_dirs()
    infra_set(INFRA_OUTPUT_DIRECTORIES_SET TRUE)
endmacro()

# 模块目录
macro(infra_setup_module_dirs MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
    infra_set(INFRA_MODULE_${MODULE_NAME_UPPER}_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    infra_set(INFRA_MODULE_${MODULE_NAME_UPPER}_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    infra_set(INFRA_MODULE_${MODULE_NAME_UPPER}_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
endmacro()

# 注册模块
macro(infra_register_module MODULE_NAME)
    if(NOT ${MODULE_NAME} IN_LIST INFRA_REGISTERED_MODULES)
        infra_append(INFRA_REGISTERED_MODULES ${MODULE_NAME})
        infra_set(INFRA_MODULE_${MODULE_NAME}_OBJECTS "")
        infra_set(INFRA_MODULE_${MODULE_NAME}_LINK_LIBS "")
        infra_print_info("Registered module '${MODULE_NAME}'")
    endif()
endmacro()

# 初始化模块
macro(infra_init_module MODULE_NAME)
    infra_register_module(${MODULE_NAME})
    infra_setup_module_dirs(${MODULE_NAME})
    infra_setup_output_dirs()
    infra_print_info("Initialized module '${MODULE_NAME}'")
endmacro()

# 注册组件（通用）
macro(infra_register_component MODULE_NAME COMPONENT_NAME)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES PRIVATE_DIRS LINK_LIBS COMPILE_DEFS)
    cmake_parse_arguments(COMP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT COMP_SOURCES)
        infra_print_warning("Component ${MODULE_NAME}/${COMPONENT_NAME} has no sources")
        return()
    endif()
    
    # 记录组件源文件
    infra_set(INFRA_MODULE_${MODULE_NAME}_${COMPONENT_NAME}_SOURCES ${COMP_SOURCES})
    
    infra_set(TARGET_NAME "infra_${MODULE_NAME}_${COMPONENT_NAME}")
    add_library(${TARGET_NAME} OBJECT ${COMP_SOURCES})
    
    # 包含目录
    target_include_directories(${TARGET_NAME} 
        PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${INFRA_MODULE_${MODULE_NAME}_INC_DIR}
    )
    
    # 添加私有目录
    foreach(DIR ${COMP_PRIVATE_DIRS})
        target_include_directories(${TARGET_NAME} PRIVATE ${DIR})
    endforeach()
    
    # 编译定义
    foreach(DEF ${COMP_COMPILE_DEFS})
        target_compile_definitions(${TARGET_NAME} PRIVATE ${DEF})
    endforeach()
    
    # 应用编译器设置
    infra_setup_target(${TARGET_NAME})
    
    # 收集对象文件
    infra_set(INFRA_MODULE_${MODULE_NAME}_OBJECTS 
        ${INFRA_MODULE_${MODULE_NAME}_OBJECTS} 
        $<TARGET_OBJECTS:${TARGET_NAME}>)
    
    # 收集链接库
    foreach(LIB ${COMP_LINK_LIBS})
        infra_append(INFRA_MODULE_${MODULE_NAME}_LINK_LIBS ${LIB})
    endforeach()
    
    infra_print_success("Component: ${TARGET_NAME}")
endmacro()

# 完成模块（通用）
macro(infra_finalize_module MODULE_NAME)
    if(TARGET ${MODULE_NAME})
        return()
    endif()
    
    infra_set(OBJECTS ${INFRA_MODULE_${MODULE_NAME}_OBJECTS})
    if(NOT OBJECTS)
        infra_print_warning("Module '${MODULE_NAME}' has no objects")
        return()
    endif()
    
    if(BUILD_SHARED_LIBS)
        add_library(${MODULE_NAME} SHARED ${OBJECTS})
    else()
        add_library(${MODULE_NAME} STATIC ${OBJECTS})
    endif()
    
    add_library(infra::${MODULE_NAME} ALIAS ${MODULE_NAME})
    
    # 公共头文件目录
    target_include_directories(${MODULE_NAME}
        PUBLIC 
            $<BUILD_INTERFACE:${INFRA_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )
    
    # 私有头文件目录
    if(INFRA_MODULE_${MODULE_NAME}_INC_DIR)
        target_include_directories(${MODULE_NAME}
            PRIVATE ${INFRA_MODULE_${MODULE_NAME}_INC_DIR}
        )
    endif()
    
    # 链接库
    if(INFRA_MODULE_${MODULE_NAME}_LINK_LIBS)
        target_link_libraries(${MODULE_NAME} PRIVATE ${INFRA_MODULE_${MODULE_NAME}_LINK_LIBS})
    endif()
    
    infra_setup_target(${MODULE_NAME})
    infra_print_success("Module '${MODULE_NAME}' created")
endmacro()


# 批量添加模块
function(infra_add_modules)
    foreach(MODULE ${INFRA_MODULES})
        infra_add_module(${MODULE})
    endforeach()
endfunction()

# InfraModules.cmake - 模块管理（修正部分）

# ... 前面保持不变 ...

# 添加模块（改成 macro）
macro(infra_add_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} MODULE_UPPER)
    if(NOT INFRA_ENABLE_${MODULE_UPPER})
        infra_print_info("Module '${MODULE_NAME}' disabled")
        return()
    endif()
    
    infra_set(MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}")
    if(NOT EXISTS "${MODULE_PATH}/CMakeLists.txt")
        infra_print_warning("Module '${MODULE_NAME}' not found")
        return()
    endif()
    
    add_subdirectory(${MODULE_PATH})
    infra_print_success("Added module '${MODULE_NAME}'")
endmacro()

# 批量添加模块（改成 macro）
macro(infra_add_modules)
    foreach(MODULE ${INFRA_MODULES})
        infra_add_module(${MODULE})
    endforeach()
endmacro()