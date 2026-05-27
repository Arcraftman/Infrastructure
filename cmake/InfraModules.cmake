# InfraModules.cmake - 模块化构建系统（增强版）

if(DEFINED INFRA_MODULES_INCLUDED)
    return()
endif()
set(INFRA_MODULES_INCLUDED TRUE)

include(InfraUtils)
include(InfraCompiler)
include(InfraHooks)

# 全局注册表
set(INFRA_REGISTERED_MODULES "" CACHE INTERNAL "")
set(INFRA_MODULE_DEPENDS "" CACHE INTERNAL "")

# 注册主模块
macro(infra_register_module MODULE_NAME)
    if(NOT ${MODULE_NAME} IN_LIST INFRA_REGISTERED_MODULES)
        list(APPEND INFRA_REGISTERED_MODULES ${MODULE_NAME})
        set(INFRA_OBJS_${MODULE_NAME} "" CACHE INTERNAL "")
        set(INFRA_MODULE_${MODULE_NAME}_DEPENDS "" CACHE INTERNAL "")
        set(INFRA_MODULE_${MODULE_NAME}_OPTIONAL "" CACHE INTERNAL "")
        message(STATUS "Infra: Registered module: ${MODULE_NAME}")
    endif()
endmacro()

# 设置模块依赖
macro(infra_module_depends MODULE_NAME)
    set(INFRA_MODULE_${MODULE_NAME}_DEPENDS ${ARGN} CACHE INTERNAL "")
endmacro()  # ✅ 修复：改为 endmacro()

# 设置可选依赖
macro(infra_module_optional MODULE_NAME)
    set(INFRA_MODULE_${MODULE_NAME}_OPTIONAL ${ARGN} CACHE INTERNAL "")
endmacro()

# 设置模块额外的头文件路径
macro(infra_module_headers MODULE_NAME GROUP_NAME)
    set(INFRA_${MODULE_NAME}_${GROUP_NAME}_EXTRA_HEADERS ${ARGN} CACHE INTERNAL "")
endmacro()

# 检查模块依赖
function(infra_check_dependencies MODULE_NAME)
    foreach(DEP ${INFRA_MODULE_${MODULE_NAME}_DEPENDS})
        if(NOT ${DEP} IN_LIST INFRA_REGISTERED_MODULES)
            message(WARNING "Module ${MODULE_NAME} depends on ${DEP}, but ${DEP} is not enabled")
            return()
        endif()
    endforeach()
    
    foreach(OPT ${INFRA_MODULE_${MODULE_NAME}_OPTIONAL})
        if(${OPT} IN_LIST INFRA_REGISTERED_MODULES)
            message(STATUS "Module ${MODULE_NAME}: optional dependency ${OPT} is available")
        endif()
    endforeach()
endfunction()

# 注册子模块
macro(infra_register_submodule MODULE_NAME GROUP_NAME SUB_NAME SOURCE_FILE)
    if(NOT ${MODULE_NAME} IN_LIST INFRA_REGISTERED_MODULES)
        message(FATAL_ERROR "Infra: Module '${MODULE_NAME}' not registered before submodule '${SUB_NAME}'")
    endif()
    
    string(TOUPPER "${MODULE_NAME}" M_UPPER)
    string(TOUPPER "${GROUP_NAME}" G_UPPER)
    string(TOUPPER "${SUB_NAME}" S_UPPER)
    set(ENABLE_VAR "INFRA_${M_UPPER}_${G_UPPER}_ENABLE_${S_UPPER}")
    
    if(${ENABLE_VAR})
        set(FULL_NAME "${GROUP_NAME}_${SUB_NAME}")
        set(TARGET_NAME "infra_${MODULE_NAME}_${FULL_NAME}")
        
        add_library(${TARGET_NAME} OBJECT ${SOURCE_FILE})
        
        set(INC_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${INFRA_SRC_DIR}/${MODULE_NAME}
            ${INFRA_SRC_DIR}/${MODULE_NAME}/${GROUP_NAME}
            ${INFRA_SRC_DIR}/${MODULE_NAME}/${GROUP_NAME}/detail
        )
        
        if(DEFINED INFRA_${MODULE_NAME}_${GROUP_NAME}_EXTRA_HEADERS)
            list(APPEND INC_DIRS ${INFRA_${MODULE_NAME}_${GROUP_NAME}_EXTRA_HEADERS})
        endif()
        
        target_include_directories(${TARGET_NAME} PRIVATE ${INC_DIRS})
        
        infra_set_compile_options(${TARGET_NAME})
        infra_set_export_definitions(${TARGET_NAME})
        
        set(INFRA_OBJS_${MODULE_NAME} 
            ${INFRA_OBJS_${MODULE_NAME}} 
            $<TARGET_OBJECTS:${TARGET_NAME}>
            CACHE INTERNAL "")
        
        message(STATUS "  ✓ ${TARGET_NAME} -> module: ${MODULE_NAME}")
    endif()
endmacro()

# 设置子模块头文件
macro(infra_submodule_header MODULE_NAME GROUP_NAME SUB_NAME HEADER_PATH)
endmacro()

macro(infra_submodule_private_headers MODULE_NAME GROUP_NAME SUB_NAME)
endmacro()

# 构建单个模块
function(infra_build_module MODULE_NAME)
    infra_check_dependencies(${MODULE_NAME})
    
    set(MODULE_OBJS ${INFRA_OBJS_${MODULE_NAME}})
    
    if(NOT MODULE_OBJS)
        message(STATUS "Module ${MODULE_NAME}: no objects")
        return()
    endif()
    
    if(NOT BUILD_CLIB)
        message(STATUS "Module ${MODULE_NAME}: BUILD_CLIB is OFF")
        return()
    endif()
    
    if(BUILD_SHARED_LIBS)
        add_library(${MODULE_NAME} SHARED ${MODULE_OBJS})
        message(STATUS "✓ Created shared library: lib${MODULE_NAME}.so")
    else()
        add_library(${MODULE_NAME} STATIC ${MODULE_OBJS})
        message(STATUS "✓ Created static library: lib${MODULE_NAME}.a")
    endif()
    
    add_library(infra::${MODULE_NAME} ALIAS ${MODULE_NAME})
    
    target_include_directories(${MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${INFRA_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    
    infra_setup_target(${MODULE_NAME})
    set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME})
endfunction()

# 构建所有模块
function(infra_build_all_modules)
    message(STATUS "Infra: Building all registered modules...")
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        infra_build_module(${MODULE})
    endforeach()
endfunction()

# 根据配置智能构建
function(infra_build_modules_by_config)
    infra_hook_run(PRE_BUILD)
    
    if(INFRA_BUILD_ALL_MODULES)
        infra_build_all_modules()
    else()
        if(INFRA_BUILD_STK AND INFRA_ENABLE_STK)
            infra_build_module(stk)
        endif()
        if(INFRA_BUILD_LNX AND INFRA_ENABLE_LNX)
            infra_build_module(lnx)
        endif()
    endif()
    
    infra_hook_run(POST_BUILD)
endfunction()

macro(infra_create_module_library MODULE_NAME)
    if(NOT BUILD_CLIB)
        message(STATUS "Infra: BUILD_CLIB is OFF, skipping ${MODULE_NAME}")
        return()
    endif()
    
    # 收集该模块的所有 OBJECT 库
    set(OBJECTS "")
    
    # 遍历所有以 infra_${MODULE_NAME}_ 开头的目标
    get_cmake_property(ALL_TARGETS TARGETS)
    foreach(TGT ${ALL_TARGETS})
        if(TGT MATCHES "^infra_${MODULE_NAME}_.*")
            list(APPEND OBJECTS $<TARGET_OBJECTS:${TGT}>)
            message(STATUS "Infra: Adding ${TGT} to ${MODULE_NAME}")
        endif()
    endforeach()
    
    if(NOT OBJECTS)
        message(STATUS "Infra: No objects found for module ${MODULE_NAME}")
        return()
    endif()
    
    # 创建库
    if(BUILD_SHARED_LIBS)
        add_library(${MODULE_NAME} SHARED ${OBJECTS})
        message(STATUS "✓ Created shared library: lib${MODULE_NAME}.so")
    else()
        add_library(${MODULE_NAME} STATIC ${OBJECTS})
        message(STATUS "✓ Created static library: lib${MODULE_NAME}.a")
    endif()
    
    target_include_directories(${MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${INFRA_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    
    infra_setup_target(${MODULE_NAME})
    set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME})
    
    add_library(infra::${MODULE_NAME} ALIAS ${MODULE_NAME})
endmacro()