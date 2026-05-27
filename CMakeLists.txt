cmake_minimum_required(VERSION 3.15)
project(infra VERSION 1.0.0 LANGUAGES C CXX)

# 根据构建类型设置默认选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(INFRA_ENABLE_OPTIMIZATION OFF CACHE BOOL "" FORCE)
    set(INFRA_ENABLE_DEBUG_SYMBOLS ON CACHE BOOL "" FORCE)
else()
    set(INFRA_ENABLE_OPTIMIZATION ON CACHE BOOL "" FORCE)
    set(INFRA_OPTIMIZATION_LEVEL "O2" CACHE STRING "" FORCE)
    set(INFRA_STRIP_SYMBOLS ON CACHE BOOL "" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(InfraCompiler)
include(InfraConfig)
include(InfraUtils)
include(InfraVersion)
include(InfraModules)

# 目录变量
set(INFRA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)


# 生成版本头文件
infra_generate_version_file(${CMAKE_BINARY_DIR}/infra_version.h)

# 复制公共头文件
if(INFRA_ENABLE_STK)
    infra_copy_headers(${INFRA_ROOT_DIR}/stk/include ${INFRA_INCLUDE_DIR})
endif()
if(INFRA_ENABLE_LNX)
    infra_copy_headers(${INFRA_ROOT_DIR}/lnx/include ${INFRA_INCLUDE_DIR})
endif()

# 添加子目录
add_subdirectory(modules)

# 根据配置构建模块
infra_build_modules_by_config()

# 打印配置
infra_print_config()