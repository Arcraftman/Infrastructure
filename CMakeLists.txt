# 顶层 CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(Infra VERSION 1.0.0 LANGUAGES C CXX)

# 添加 CMake 模块路径（必须在 include 之前！）
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# 根据构建类型设置默认选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(INFRA_ENABLE_OPTIMIZATION OFF CACHE BOOL "" FORCE)
    set(INFRA_ENABLE_DEBUG_SYMBOLS ON CACHE BOOL "" FORCE)
else()
    set(INFRA_ENABLE_OPTIMIZATION ON CACHE BOOL "" FORCE)
    set(INFRA_OPTIMIZATION_LEVEL "O2" CACHE STRING "" FORCE)
    set(INFRA_STRIP_SYMBOLS ON CACHE BOOL "" FORCE)
endif()

# 包含核心模块（不需要写 cmake/ 前缀）
include(InfraCompiler)
include(InfraConfig)
include(InfraDoc)
include(InfraModules)
include(InfraPackaging)
include(InfraTesting)
include(InfraUtils)
include(InfraVersion)

# 设置目录变量
set(INFRA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(INFRA_SRC_DIR ${CMAKE_SOURCE_DIR}/modules)

# 生成版本头文件
infra_generate_version_file(${CMAKE_BINARY_DIR}/infra_version.h)

# 复制公共头文件
if(INFRA_ENABLE_STK)
    infra_copy_headers(${INFRA_SRC_DIR}/stk/include ${INFRA_INCLUDE_DIR})
endif()
if(INFRA_ENABLE_LNX)
    infra_copy_headers(${INFRA_SRC_DIR}/lnx/include ${INFRA_INCLUDE_DIR})
endif()

# 添加子目录
add_subdirectory(modules)

# 构建所有模块
infra_build_all_modules()

# 打印配置
infra_print_config()

# 设置安装（在模块构建之后）
infra_setup_install()