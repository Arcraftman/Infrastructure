cmake_minimum_required(VERSION 3.19)
project(Infra VERSION 1.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(INFRA_MODULES stk lnx web)

include(InfraUtils)
include(InfraCommon)
include(InfraCompiler)
include(CTest)
include(InfraConfig)
include(InfraOptions)
include(InfraModules)
include(InfraPackaging)
include(InfraVersion)
include(InfraTesting)

infra_setup_dirs()
infra_define_options()
infra_generate_version("${INFRA_BUILD_DIR}/infra_version.h")
infra_add_modules()
infra_print_build_config()
infra_setup_install()
