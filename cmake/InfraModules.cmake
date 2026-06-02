# InfraModules.cmake - 模块注册与构建流程
#
# 定义模块生命周期：注册 → 初始化 → 注册组件 → 完成 → 添加模块
# 每个模块位于 modules/${MODULE_NAME} 目录下，包含自己的 CMakeLists.txt
#
# 宏说明：
#   infra_register_module(NAME)       - 在全局列表中注册模块名称
#   infra_init_module(NAME)           - 完整初始化：注册 + 目录设置 + 输出目录
#   infra_register_component(...)     - 向模块添加一个对象库组件
#   infra_finalize_module(NAME)       - 从组件组装模块库
#   infra_add_module(NAME)            - 条件性地包含模块子目录
#
# 全局状态变量：
#   INFRA_REGISTERED_MODULES          - 所有已注册模块名称的列表
#   INFRA_MODULE_${NAME}_OBJECTS      - 为模块收集的对象文件
#   INFRA_MODULE_${NAME}_LINK_LIBS    - 模块的链接库
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
#   - 为模块初始化空的对象列表和链接库列表
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
        # 初始化该模块的对象文件列表（空）
        set(INFRA_MODULE_${_MODULE}_OBJECTS "")
        # 初始化该模块的链接库列表（空）
        set(INFRA_MODULE_${_MODULE}_LINK_LIBS "")
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
# 组件注册（方案3：共享 OBJECT 库）
# ============================================================================
# 功能：将所有组件的源文件添加到同一个 OBJECT 库中
# 参数：
#   MODULE_NAME     - 所属模块名称
#   COMPONENT_NAME  - 组件名称（仅用于日志，不用于目标命名）
# 关键字参数：
#   SOURCES         - 源文件列表（必需）
#   PRIVATE_DIRS    - 私有头文件目录列表
#   LINK_LIBS       - 需要链接的库列表
#   COMPILE_DEFS    - 编译宏定义列表
# 说明：
#   - 所有组件共享同一个 OBJECT 库目标：infra_${MODULE_NAME}_objs
#   - 编译时只有一个 .dir 目录，例如 infra_stk_objs.dir/
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
    
    # 记录组件的源文件（用于调试）
    set(INFRA_MODULE_${_MODULE}_${COMPONENT_NAME}_SOURCES ${COMP_SOURCES})
    

    if(BUILD_SHARED_LIBS)
        set(TARGET_NAME "${MODULE_NAME}")
    else()
        set(TARGET_NAME "${MODULE_NAME}")
    endif()
    
    # 如果目标还不存在，创建它
    if(NOT TARGET ${TARGET_NAME})
        add_library(${TARGET_NAME} OBJECT)
        infra_info("Created object library: ${TARGET_NAME}")
    endif()
    
    # 将源文件添加到共享 OBJECT 库
    target_sources(${TARGET_NAME} PRIVATE ${COMP_SOURCES})
    
    # 设置头文件包含路径
    target_include_directories(${TARGET_NAME} 
        PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${INFRA_MODULE_${_MODULE}_INC_DIR}
    )
    
    # 添加额外的私有头文件目录
    foreach(DIR ${COMP_PRIVATE_DIRS})
        target_include_directories(${TARGET_NAME} PRIVATE ${DIR})
    endforeach()
    
    # 添加编译宏定义
    foreach(DEF ${COMP_COMPILE_DEFS})
        target_compile_definitions(${TARGET_NAME} PRIVATE ${DEF})
    endforeach()
    
    # 构建共享库时自动添加 DLL 导出宏
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${TARGET_NAME} PRIVATE
            ${_MODULE}_DLL
            ${_MODULE}_EXPORTING
        )
    endif()
    
    # 应用通用的编译器设置
    infra_setup_target(${TARGET_NAME})
    
    # 收集链接库（去重）
    foreach(LIB ${COMP_LINK_LIBS})
        if(NOT LIB IN_LIST INFRA_MODULE_${_MODULE}_LINK_LIBS)
            list(APPEND INFRA_MODULE_${_MODULE}_LINK_LIBS ${LIB})
        endif()
    endforeach()
    
    infra_success("Component '${COMPONENT_NAME}' added to ${TARGET_NAME}")
endmacro()
# ============================================================================
# 模块完成（组装）
# ============================================================================
# 功能：将所有组件对象文件组装成最终的模块库（共享库或静态库）
# 参数：
#   MODULE_NAME - 模块名称
# 说明：
#   - 使用共享 OBJECT 库 infra_${MODULE_NAME}_objects 收集所有对象
#   - 根据 BUILD_SHARED_LIBS 决定创建共享库还是静态库
#   - 创建 `infra::${MODULE_NAME}` 别名目标，方便使用
# ============================================================================
macro(infra_finalize_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    
    # 共享 OBJECT 库的目标名称
    set(OBJECTS_TARGET "${MODULE_NAME}")
    
    # 如果目标已存在，直接返回（避免重复创建）
    if(TARGET ${MODULE_NAME})
        return()
    endif()
    
    # 检查 OBJECT 库是否存在
    if(NOT TARGET ${OBJECTS_TARGET})
        infra_warn("Module '${MODULE_NAME}' has no objects target")
        return()
    endif()
    
    # 获取 OBJECT 库的对象文件
    set(OBJECTS $<TARGET_OBJECTS:${OBJECTS_TARGET}>)
    
    # 根据 BUILD_SHARED_LIBS 决定创建共享库还是静态库
    if(BUILD_SHARED_LIBS)
        add_library(${MODULE_NAME} SHARED ${OBJECTS})
    else()
        add_library(${MODULE_NAME} STATIC ${OBJECTS})
    endif()
    
    # 创建别名目标，方便使用 `infra::module_name` 的形式链接
    add_library(infra::${MODULE_NAME} ALIAS ${MODULE_NAME})
    
    # 设置公共头文件目录
    # BUILD_INTERFACE: 构建时使用的路径（模块自己的 include 目录）
    # INSTALL_INTERFACE: 安装后使用的路径（${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}）
    target_include_directories(${MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${INFRA_MODULE_${_MODULE}_INC_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )
    
    # 添加私有头文件目录（仅编译时使用，不对外暴露）
    if(INFRA_MODULE_${_MODULE}_INC_DIR)
        target_include_directories(${MODULE_NAME}
            PRIVATE ${INFRA_MODULE_${_MODULE}_INC_DIR}
        )
    endif()
    
    # 链接收集到的依赖库
    if(INFRA_MODULE_${_MODULE}_LINK_LIBS)
        target_link_libraries(${MODULE_NAME} PRIVATE ${INFRA_MODULE_${_MODULE}_LINK_LIBS})
    endif()
    
    # 应用通用的编译器设置
    infra_setup_target(${MODULE_NAME})
    
    infra_success("Module '${MODULE_NAME}' created from ${OBJECTS_TARGET}")
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