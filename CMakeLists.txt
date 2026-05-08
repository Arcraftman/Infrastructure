cmake_minimum_required(VERSION 3.12)
project(infrastructure VERSION 0.0.1 LANGUAGES C CXX)

# ============== 编译标准 ==============
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============== 输出目录 ==============
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============== 安装前缀 ==============
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/stk" CACHE PATH "Install path" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path" FORCE)
    endif()
endif()

# ============== 根目录选项 ==============
option(BUILD_STK "Build STK library" ON)


# ============== GTest 配置 ==============
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(GTest REQUIRED)
    set(GTEST_INCLUDE_DIR ${GTEST_INCLUDE_DIRS})
    set(GTEST_LIBRARY ${GTEST_LIBRARIES})
    set(GTEST_MAIN_LIBRARY ${GTEST_LIBRARIES})
    set(GTEST_DLL_DIR "")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(GTEST_INCLUDE_DIR "D:/gtest/include" CACHE PATH "GTest include")
    set(GTEST_LIBRARY "D:/gtest/lib/gtest.lib" CACHE FILEPATH "GTest lib")
    set(GTEST_MAIN_LIBRARY "D:/gtest/lib/gtest_main.lib" CACHE FILEPATH "GTest main")
    set(GTEST_DLL_DIR "D:/gtest/bin" CACHE PATH "GTest DLL")
else()
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        message(WARNING "GTest not found, tests will be disabled")
        set(BUILD_TESTS OFF)
    endif()
endif()

if(NOT BUILD_TESTS)
    message(STATUS "Tests disabled")
elseif(NOT GTEST_INCLUDE_DIR AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(WARNING "GTest not configured, disabling tests")
    set(BUILD_TESTS OFF)
endif()


# ============== 安装头文件 ==============
install(DIRECTORY stk/include/stk/
    DESTINATION include/stk
    FILES_MATCHING PATTERN "*.h"
    PATTERN ".git*" EXCLUDE
)

# ============== 导出配置 ==============
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/StkConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(EXPORT StkTargets
    FILE StkTargets.cmake
    NAMESPACE stk::
    DESTINATION lib/cmake/stk
)

install(FILES
    ${CMAKE_BINARY_DIR}/StkConfigVersion.cmake
    DESTINATION lib/cmake/stk
)

# ============== CPack 打包 ==============
include(CPack)

set(CPACK_PACKAGE_NAME "stk")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "stk - Simple Tool Kit")
set(CPACK_PACKAGE_VENDOR "stk")
set(CPACK_PACKAGE_CONTACT "user@example.com")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "stk")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;Bundle")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "user@example.com")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

# ============== 配置信息 ==============
message(STATUS "")
message(STATUS "==================== Stk Configuration ====================")
message(STATUS "Project:           Stk ${PROJECT_VERSION}")
message(STATUS "Platform:          ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture:      ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  BUILD_STK:       ${BUILD_STK}")
message(STATUS "  BUILD_TESTS:     ${BUILD_TESTS}")
message(STATUS "")
message(STATUS "Output directories:")
message(STATUS "  Runtime:         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Library:         ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "  Archive:         ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "============================================================")
message(STATUS "")

enable_testing()

# ============== 子目录 ==============
if(BUILD_STK)
    add_subdirectory(stk)
endif()

