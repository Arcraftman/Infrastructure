# modules/stk/CMakeLists.txt

# ============================================================
# 构建选项
# ============================================================
option(BUILD_STK "Build STK library" ON)
option(BUILD_TESTS "Build tests" OFF)

# ============================================================
# 目录变量
# ============================================================
set(STK_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(STK_INCLUDE_DIR ${STK_ROOT_DIR}/include)
set(STK_SRC_DIR ${STK_ROOT_DIR}/src)
set(STK_INTERNAL_DIR ${STK_ROOT_DIR}/internal)
set(STK_TESTS_DIR ${STK_ROOT_DIR}/tests)
set(STK_EXAMPLES_DIR ${STK_ROOT_DIR}/examples)

# ============================================================
# 配置信息
# ============================================================
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

# ============================================================
# 设置输出目录
# ============================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================
# 包含目录（构建时）
# ============================================================
include_directories(
    ${STK_INCLUDE_DIR}
    ${STK_INTERNAL_DIR}
    ${STK_SRC_DIR}
)

# ============================================================
# 子目录
# ============================================================
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_STK AND TARGET stk)
    # ============================================================
    # 安装
    # ============================================================
    include(GNUInstallDirs)

    install(
        TARGETS stk
        EXPORT stkTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stk
    )

    # 安装公共头文件
    install(
        DIRECTORY ${STK_INCLUDE_DIR}/stk/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stk
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )

    # 安装 internal 头文件（如果需要）
    install(
        DIRECTORY ${STK_INTERNAL_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stk/internal
        FILES_MATCHING PATTERN "*.h"
    )

    # ============================================================
    # 导出 CMake 配置
    # ============================================================
    include(CMakePackageConfigHelpers)

    # 生成导出文件
    install(EXPORT stkTargets
        FILE stkTargets.cmake
        NAMESPACE stk::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/stk
    )

    # 生成版本文件
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/stkConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # 生成配置文件
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/stkConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/stkConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/stk
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/stkConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/stkConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/stk
    )
endif()