# InfraConfig.cmake - 构建选项（通用版本）

if(DEFINED INFRA_CONFIG_INCLUDED)
    return()
endif()
infra_set(INFRA_CONFIG_INCLUDED TRUE)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    infra_set(CMAKE_BUILD_TYPE "Debug")
endif()

# 库类型
infra_set(INFRA_LIBRARY_TYPE "SHARED")
if(NOT DEFINED BUILD_SHARED_LIBS)
    if(INFRA_LIBRARY_TYPE STREQUAL "SHARED")
        infra_set(BUILD_SHARED_LIBS ON)
    else()
        infra_set(BUILD_SHARED_LIBS OFF)
    endif()
endif()

# 基本选项（通用）
option(INFRA_ENABLE_OPTIMIZATION "Enable optimization" ON)
option(INFRA_ENABLE_DEBUG_SYMBOLS "Enable debug symbols" ON)
option(INFRA_POSITION_INDEPENDENT_CODE "Enable PIC" ON)
option(INFRA_ENABLE_WARNINGS "Enable warnings" ON)
option(INFRA_BUILD_TESTS "Build tests" OFF)
option(INFRA_INSTALL "Enable installation" ON)

# 版本
infra_set(INFRA_VERSION_STRING "${PROJECT_VERSION}")
infra_set(INFRA_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
infra_set(INFRA_VERSION_MINOR ${PROJECT_VERSION_MINOR})
infra_set(INFRA_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# 模块列表（由外部配置）
if(NOT DEFINED INFRA_MODULES)
    infra_set(INFRA_MODULES "")
endif()

# 为每个模块生成启用选项
foreach(MODULE ${INFRA_MODULES})
    string(TOUPPER ${MODULE} MODULE_UPPER)
    option(INFRA_ENABLE_${MODULE_UPPER} "Enable ${MODULE} module" OFF)
endforeach()