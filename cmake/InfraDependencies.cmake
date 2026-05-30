# InfraDependencies.cmake - External dependency helpers

if(DEFINED INFRA_DEPENDENCIES_INCLUDED)
    return()
endif()
set(INFRA_DEPENDENCIES_INCLUDED TRUE)

cmake_policy(SET CMP0074 NEW)

option(INFRA_ENABLE_FETCHCONTENT "Enable FetchContent fallback for dependencies" OFF)
option(INFRA_ENABLE_PKGCONFIG "Enable pkg-config support when available" ON)

include(FindPackageHandleStandardArgs)

function(infra_find_package NAME)
    set(oneValueArgs VERSION)
    set(multiValueArgs COMPONENTS)
    cmake_parse_arguments(PKG "" "VERSION" "COMPONENTS" ${ARGN})

    if(PKG_VERSION)
        find_package(${NAME} ${PKG_VERSION} ${PKG_COMPONENTS} QUIET)
    else()
        find_package(${NAME} ${PKG_COMPONENTS} QUIET)
    endif()

    if(NOT ${NAME}_FOUND AND INFRA_ENABLE_FETCHCONTENT)
        if(NOT COMMAND FetchContent_Declare)
            include(FetchContent)
        endif()
        message(STATUS "Infra: Dependency '${NAME}' not found with find_package(). FetchContent fallback is enabled, but must be configured per project.")
    endif()

    set(${NAME}_FOUND ${${NAME}_FOUND} PARENT_SCOPE)
    if(PKG_VERSION)
        set(${NAME}_VERSION ${${NAME}_VERSION} PARENT_SCOPE)
    endif()
endfunction()

function(infra_find_pkg_config NAME)
    if(NOT INFRA_ENABLE_PKGCONFIG)
        set(${NAME}_PKG_CONFIG_FOUND OFF PARENT_SCOPE)
        return()
    endif()

    find_package(PkgConfig QUIET)
    if(NOT PkgConfig_FOUND)
        set(${NAME}_PKG_CONFIG_FOUND OFF PARENT_SCOPE)
        return()
    endif()

    pkg_check_modules(${NAME} QUIET ${NAME})
    set(${NAME}_PKG_CONFIG_FOUND ${${NAME}_FOUND} PARENT_SCOPE)
endfunction()
