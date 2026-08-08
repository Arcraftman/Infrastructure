# [file name]: InfraDependencies.cmake
# [file content begin]
# InfraDependencies.cmake - 外部依赖辅助函数

# 防止重复包含
if(DEFINED INFRA_DEPENDENCIES_INCLUDED)
    return()
else()
    set(INFRA_DEPENDENCIES_INCLUDED TRUE)
endif()

# 设置策略 CMP0074: find_package 使用 <PackageName>_ROOT 变量
cmake_policy(SET CMP0074 NEW)

# 依赖管理选项
option(INFRA_ENABLE_FETCHCONTENT "Enable FetchContent fallback for dependencies" OFF)
option(INFRA_ENABLE_PKGCONFIG "Enable pkg-config support when available" ON)

include(FindPackageHandleStandardArgs)

# 查找包（支持 find_package 和可选的 FetchContent 回退）
function(infra_find_package NAME)
    set(oneValueArgs VERSION)
    set(multiValueArgs COMPONENTS)
    cmake_parse_arguments(PKG "" "VERSION" "COMPONENTS" ${ARGN})

    # 调用 find_package
    if(PKG_VERSION)
        find_package(${NAME} ${PKG_VERSION} ${PKG_COMPONENTS} QUIET)
    else()
        find_package(${NAME} ${PKG_COMPONENTS} QUIET)
    endif()

    # 如果未找到且启用了 FetchContent
    if(NOT ${NAME}_FOUND AND INFRA_ENABLE_FETCHCONTENT)
        if(NOT COMMAND FetchContent_Declare)
            include(FetchContent)
            message(STATUS "[Infra] FetchContent included for ${NAME}")
        endif()
        message(STATUS "Infra: Dependency '${NAME}' not found with find_package(). FetchContent fallback is enabled, but must be configured per project.")
    elseif(${NAME}_FOUND)
        message(STATUS "[Infra] Found dependency: ${NAME} (${${NAME}_VERSION})")
    else()
        message(STATUS "[Infra] Dependency not found: ${NAME}")
    endif()

    # 将结果传递给父作用域
    set(${NAME}_FOUND ${${NAME}_FOUND} PARENT_SCOPE)
    if(PKG_VERSION)
        set(${NAME}_VERSION ${${NAME}_VERSION} PARENT_SCOPE)
    endif()
endfunction()

# 使用 pkg-config 查找包
function(infra_find_pkg_config NAME)
    # 如果未启用 pkg-config，直接返回
    if(NOT INFRA_ENABLE_PKGCONFIG)
        set(${NAME}_PKG_CONFIG_FOUND OFF PARENT_SCOPE)
        message(STATUS "[Infra] pkg-config disabled for ${NAME}")
        return()
    endif()

    # 查找 PkgConfig 模块
    find_package(PkgConfig QUIET)
    if(NOT PkgConfig_FOUND)
        set(${NAME}_PKG_CONFIG_FOUND OFF PARENT_SCOPE)
        message(STATUS "[Infra] pkg-config not found, skipping ${NAME}")
        return()
    endif()

    # 使用 pkg-config 检查包
    pkg_check_modules(${NAME} QUIET ${NAME})
    set(${NAME}_PKG_CONFIG_FOUND ${${NAME}_FOUND} PARENT_SCOPE)
    
    if(${NAME}_FOUND)
        message(STATUS "[Infra] Found ${NAME} via pkg-config")
    else()
        message(STATUS "[Infra] ${NAME} not found via pkg-config")
    endif()
endfunction()