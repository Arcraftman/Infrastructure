cmake_minimum_required(VERSION 3.15)
project(Infra VERSION 1.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(InfraUtils)
include(InfraCompiler)
include(InfraConfig)
include(InfraOptions)
include(InfraModules)
include(InfraPackaging)

# 设置基本目录
infra_setup_dirs()

# 配置模块列表
infra_set(INFRA_MODULES stk lnx)

# 定义选项（一个 macro 搞定）
infra_define_options()

# 生成版本头文件
infra_generate_version(${INFRA_BUILD_DIR}/infra_version.h)

# 添加模块
infra_add_modules()

# 打印配置
infra_print_config()

# 设置安装
infra_setup_install()