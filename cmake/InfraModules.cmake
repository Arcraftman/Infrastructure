# [file name]: InfraModules.cmake
# [file content begin]
# InfraModules.cmake - 模块注册与构建流程
#
# 定义模块生命周期：注册 → 初始化 → 注册组件 → 完成 → 添加模块
# 每个模块位于 modules/${MODULE_NAME} 目录下，包含自己的 CMakeLists.txt
#
# 宏说明：
#   infra_register_module(NAME)       - 在全局列表中注册模块名称
#   infra_init_module(NAME)           - 完整初始化：注册 + 目录设置 + 输出目录
#   infra_register_component(...)     - 向模块添加源文件（直接添加到最终库）
#   infra_finalize_module(NAME)       - 创建最终的模块库
#   infra_add_module(NAME)            - 条件性地包含模块子目录
#
# 全局状态变量：
#   INFRA_REGISTERED_MODULES          - 所有已注册模块名称的列表
#   INFRA_MODULE_${NAME}_SOURCES      - 为模块收集的源文件列表
#   INFRA_MODULE_${NAME}_LINK_LIBS    - 模块的链接库
#   INFRA_MODULE_${NAME}_INCLUDE_DIRS - 模块的头文件目录
#   INFRA_MODULE_${NAME}_COMPILE_DEFS - 模块的编译宏定义
#
# 使用的选项：INFRA_ENABLE_{MODULE_UPPER}
# 依赖：InfraUtils, InfraCompiler
# 平台：跨平台

# 防止重复包含
if(DEFINED INFRA_MODULES_INCLUDED)
    return()
endif()
set(INFRA_MODULES_INCLUDED TRUE)

# 包含依赖的工具模块
include(InfraUtils)
include(InfraCompiler)

# ============================================================================
# 全局状态初始化
# ============================================================================
# 存储所有已注册的模块名称列表
set(INFRA_REGISTERED_MODULES "")
# 标记输出目录是否已设置（避免重复设置）
set(INFRA_OUTPUT_DIRECTORIES_SET FALSE)

# ============================================================================
# 输出目录设置
# ============================================================================
# 功能：设置项目的输出目录（bin、lib等）
# 说明：使用标志位确保只执行一次，避免多个模块重复设置
# ============================================================================
macro(infra_setup_output_dirs)
    if(INFRA_OUTPUT_DIRECTORIES_SET)
        return()
    endif()
    # 调用 InfraUtils 中的函数设置输出目录
    infra_set_output_dirs()
    set(INFRA_OUTPUT_DIRECTORIES_SET TRUE)
endmacro()

# ============================================================================
# 模块目录设置
# ============================================================================
# 功能：为指定模块设置源代码目录、头文件目录等路径变量
# 参数：
#   MODULE_NAME - 模块名称
# 输出的变量：
#   INFRA_MODULE_${_MODULE}_ROOT_DIR - 模块根目录
#   INFRA_MODULE_${_MODULE}_INC_DIR  - 模块头文件目录（include/）
#   INFRA_MODULE_${_MODULE}_SRC_DIR  - 模块源代码目录（src/）
# ============================================================================
macro(infra_setup_module_dirs MODULE_NAME)
    # 将模块名转换为大写，用于变量命名（如 stk -> STK）
    string(TOUPPER ${MODULE_NAME} _MODULE)
    # 设置模块根目录为当前 CMakeLists.txt 所在目录
    set(INFRA_MODULE_${_MODULE}_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    # 设置头文件目录：根目录下的 include 文件夹
    set(INFRA_MODULE_${_MODULE}_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    # 设置源代码目录：根目录下的 src 文件夹
    set(INFRA_MODULE_${_MODULE}_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
endmacro()

# ============================================================================
# 模块注册
# ============================================================================
# 功能：将模块名称注册到全局列表中，并初始化该模块的状态变量
# 参数：
#   MODULE_NAME - 模块名称
# 说明：
#   - 避免重复注册相同的模块
#   - 为模块初始化空的源文件列表、链接库列表、包含目录列表和编译定义列表
#   - 将注册结果传递到父作用域（PARENT_SCOPE）
# ============================================================================
macro(infra_register_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    # 检查模块是否已经注册，避免重复
    if(NOT ${MODULE_NAME} IN_LIST INFRA_REGISTERED_MODULES)
        # 将模块名添加到全局列表
        list(APPEND INFRA_REGISTERED_MODULES ${MODULE_NAME})
        # 将更新后的列表传递到父作用域（供调用者使用）
        set(INFRA_REGISTERED_MODULES "${INFRA_REGISTERED_MODULES}" PARENT_SCOPE)
        # 初始化该模块的源文件列表（空）
        set(INFRA_MODULE_${_MODULE}_SOURCES "")
        # 初始化该模块的链接库列表（空）
        set(INFRA_MODULE_${_MODULE}_LINK_LIBS "")
        # 初始化该模块的包含目录列表（空）
        set(INFRA_MODULE_${_MODULE}_INCLUDE_DIRS "")
        # 初始化该模块的编译定义列表（空）
        set(INFRA_MODULE_${_MODULE}_COMPILE_DEFS "")
        # 输出信息日志
        infra_info("Registered module '${MODULE_NAME}'")
    endif()
endmacro()

# ============================================================================
# 模块完整初始化（一站式）
# ============================================================================
# 功能：一次性完成模块的注册、目录设置和输出目录设置
# 参数：
#   MODULE_NAME - 模块名称
# 说明：这是最常用的模块初始化宏，集成了三个步骤：
#   1. infra_register_module   - 注册模块
#   2. infra_setup_module_dirs  - 设置模块目录
#   3. infra_setup_output_dirs  - 设置全局输出目录
# ============================================================================
macro(infra_init_module MODULE_NAME)
    infra_register_module(${MODULE_NAME})      # 注册模块
    infra_setup_module_dirs(${MODULE_NAME})    # 设置模块目录
    infra_setup_output_dirs()                  # 设置输出目录
    infra_info("Initialized module '${MODULE_NAME}'")
endmacro()

# ============================================================================
# 组件注册
# ============================================================================
# 功能：将组件的源文件添加到模块的源文件列表中
# 参数：
#   MODULE_NAME     - 所属模块名称
#   COMPONENT_NAME  - 组件名称（仅用于日志）
# 关键字参数：
#   SOURCES         - 源文件列表（必需）
#   PRIVATE_DIRS    - 私有头文件目录列表
#   LINK_LIBS       - 需要链接的库列表
#   COMPILE_DEFS    - 编译宏定义列表
# 说明：
#   - 所有组件的源文件收集到 INFRA_MODULE_${MODULE}_SOURCES 列表中
#   - 最终在 infra_finalize_module 中一次性创建库
#   - 组件名称仅用于日志输出和调试
# ============================================================================
macro(infra_register_component MODULE_NAME COMPONENT_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    
    # 定义支持的参数类型
    set(options "")                          # 布尔选项（无）
    set(oneValueArgs "")                     # 单值参数（无）
    set(multiValueArgs SOURCES PRIVATE_DIRS LINK_LIBS COMPILE_DEFS)  # 多值参数
    
    # 解析传入的参数
    cmake_parse_arguments(COMP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # 检查是否提供了源文件
    if(NOT COMP_SOURCES)
        infra_warn("Component ${MODULE_NAME}/${COMPONENT_NAME} has no sources")
        return()
    endif()
    
    # 将源文件添加到模块的源文件列表
    list(APPEND INFRA_MODULE_${_MODULE}_SOURCES ${COMP_SOURCES})
    
    # 添加私有头文件目录
    foreach(DIR ${COMP_PRIVATE_DIRS})
        list(APPEND INFRA_MODULE_${_MODULE}_INCLUDE_DIRS ${DIR})
    endforeach()
    
    # 添加编译宏定义
    foreach(DEF ${COMP_COMPILE_DEFS})
        list(APPEND INFRA_MODULE_${_MODULE}_COMPILE_DEFS ${DEF})
    endforeach()
    
    # 收集链接库（去重）
    foreach(LIB ${COMP_LINK_LIBS})
        if(NOT LIB IN_LIST INFRA_MODULE_${_MODULE}_LINK_LIBS)
            list(APPEND INFRA_MODULE_${_MODULE}_LINK_LIBS ${LIB})
        endif()
    endforeach()
    
    infra_success("Component '${COMPONENT_NAME}' added to module '${MODULE_NAME}'")
endmacro()

# ============================================================================
# 模块完成（创建库）
# ============================================================================
# 功能：使用收集到的所有源文件创建最终的模块库（共享库或静态库）
# 参数：
#   MODULE_NAME - 模块名称
# 说明：
#   - 使用 INFRA_MODULE_${MODULE}_SOURCES 中收集的所有源文件
#   - 根据 BUILD_SHARED_LIBS 决定创建共享库还是静态库
#   - 创建 `infra::${MODULE_NAME}` 别名目标，方便使用
# ============================================================================
macro(infra_finalize_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    
    # 最终库的目标名称（与模块名相同）
    set(LIBRARY_TARGET "${MODULE_NAME}")
    
    # 如果最终库目标已存在，直接返回
    if(TARGET ${LIBRARY_TARGET})
        return()
    endif()
    
    # 获取收集到的源文件
    set(SOURCES ${INFRA_MODULE_${_MODULE}_SOURCES})
    
    # 检查是否有源文件
    if(NOT SOURCES)
        infra_warn("Module '${MODULE_NAME}' has no sources")
        return()
    endif()
    
    # 根据 BUILD_SHARED_LIBS 决定创建共享库还是静态库
    if(BUILD_SHARED_LIBS)
        add_library(${LIBRARY_TARGET} SHARED ${SOURCES})
    else()
        add_library(${LIBRARY_TARGET} STATIC ${SOURCES})
    endif()
    
    # 创建别名目标，方便使用 `infra::module_name` 的形式链接
    add_library(infra::${MODULE_NAME} ALIAS ${LIBRARY_TARGET})
    
    # 设置头文件包含目录（PUBLIC，对外可见）
    target_include_directories(${LIBRARY_TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${INFRA_MODULE_${_MODULE}_INC_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )
    
    # 添加额外的私有头文件目录（PRIVATE，仅编译时使用）
    foreach(DIR ${INFRA_MODULE_${_MODULE}_INCLUDE_DIRS})
        target_include_directories(${LIBRARY_TARGET} PRIVATE ${DIR})
    endforeach()
    
    # 添加编译宏定义
    foreach(DEF ${INFRA_MODULE_${_MODULE}_COMPILE_DEFS})
        target_compile_definitions(${LIBRARY_TARGET} PRIVATE ${DEF})
    endforeach()
    
    # 构建共享库时自动添加 DLL 导出宏
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${LIBRARY_TARGET} PRIVATE
            ${_MODULE}_DLL
            ${_MODULE}_EXPORTING
        )
    endif()
    
    # 链接收集到的依赖库
    if(INFRA_MODULE_${_MODULE}_LINK_LIBS)
        target_link_libraries(${LIBRARY_TARGET} PRIVATE ${INFRA_MODULE_${_MODULE}_LINK_LIBS})
    endif()
    
    # 应用通用的编译器设置
    infra_setup_target(${LIBRARY_TARGET})
    
    infra_success("Module '${MODULE_NAME}' created")
endmacro()

# ============================================================================
# 模块包含（条件性）
# ============================================================================
# 功能：根据 INFRA_ENABLE_{MODULE_UPPER} 选项条件性地包含模块子目录
# 参数：
#   MODULE_NAME - 模块名称
# 说明：
#   - 如果 INFRA_ENABLE_{MODULE_UPPER} 未设置或为 FALSE，则跳过该模块
#   - 模块子目录必须包含 CMakeLists.txt 文件
# ============================================================================
macro(infra_add_module MODULE_NAME)
    # 将模块名转换为大写，用于构建选项变量名
    # 例如：MODULE_NAME=stk -> INFRA_ENABLE_STK
    string(TOUPPER ${MODULE_NAME} MODULE_UPPER)
    
    # 检查是否启用了该模块
    if(NOT INFRA_ENABLE_${MODULE_UPPER})
        infra_info("Module '${MODULE_NAME}' disabled")
        return()
    endif()

    # 构建模块子目录的完整路径
    set(MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/${MODULE_NAME}")
    
    # 检查模块的 CMakeLists.txt 是否存在
    if(NOT EXISTS "${MODULE_PATH}/CMakeLists.txt")
        infra_warn("Module '${MODULE_NAME}' not found")
        return()
    endif()

    # 包含模块子目录
    add_subdirectory(${MODULE_PATH})
    infra_success("Added module '${MODULE_NAME}'")
endmacro()

# ============================================================================
# 批量添加模块
# ============================================================================
# 功能：批量添加所有已注册的模块
# 说明：遍历 INFRA_MODULES 列表，逐个调用 infra_add_module
# ============================================================================
macro(infra_add_modules)
    foreach(MODULE ${INFRA_MODULES})
        infra_add_module(${MODULE})
    endforeach()
endmacro()