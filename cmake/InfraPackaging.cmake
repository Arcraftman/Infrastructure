# InfraPackaging.cmake - 打包和安装配置

if(DEFINED INFRA_PACKAGING_INCLUDED)
    return()
endif()
set(INFRA_PACKAGING_INCLUDED TRUE)

# ============================================================================
# 安装配置
# ============================================================================
include(GNUInstallDirs)

# 安装目标
function(infra_install_target TARGET_NAME)
    if(NOT INFRA_INSTALL)
        return()
    endif()
    
    if(NOT TARGET ${TARGET_NAME})
        return()
    endif()
    
    install(TARGETS ${TARGET_NAME}
        EXPORT infraTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endfunction()

# 安装头文件
function(infra_install_headers HEADER_DIR)
    if(NOT INFRA_INSTALL_HEADERS)
        return()
    endif()
    
    if(NOT EXISTS ${HEADER_DIR})
        return()
    endif()
    
    install(DIRECTORY ${HEADER_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.tcc"
    )
endfunction()

# 安装 CMake 配置文件
function(infra_install_cmake_config)
    if(NOT INFRA_INSTALL_CMAKE_CONFIG)
        return()
    endif()
    
    include(CMakePackageConfigHelpers)
    
    # 生成配置文件
    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/cmake/InfraConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/infra-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
    )
    
    # 生成版本文件
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/infra-config-version.cmake
        VERSION ${INFRA_VERSION_STRING}
        COMPATIBILITY AnyNewerVersion
    )
    
    # 安装配置文件
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/infra-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/infra-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
    )
endfunction()

# 安装导出目标（在有目标时调用）
function(infra_install_export_targets)
    get_cmake_property(ALL_TARGETS TARGETS)
    foreach(TGT ${ALL_TARGETS})
        if(TGT STREQUAL "stk" OR TGT STREQUAL "lnx")
            install(EXPORT infraTargets
                FILE infra-targets.cmake
                NAMESPACE infra::
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
            )
            break()
        endif()
    endforeach()
endfunction()

# ============================================================================
# CPack 打包配置
# ============================================================================
function(infra_setup_cpack)
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VERSION ${INFRA_VERSION_STRING})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Infra - Infrastructure Library")
    set(CPACK_PACKAGE_VENDOR "Infra Project")
    set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
    
    if(WIN32)
        set(CPACK_GENERATOR "ZIP")
        set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})
    elseif(APPLE)
        set(CPACK_GENERATOR "DragNDrop;TGZ")
    else()
        set(CPACK_GENERATOR "TGZ")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Infra Developer")
    endif()
    
    include(CPack)
endfunction()

# ============================================================================
# 创建完整的安装规则
# ============================================================================
function(infra_setup_install)
    if(NOT INFRA_INSTALL)
        message(STATUS "Infra: Installation disabled")
        return()
    endif()
    
    # 检查是否有库被构建
    set(HAS_LIBRARIES OFF)
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        if(TARGET ${MODULE})
            set(HAS_LIBRARIES ON)
            infra_install_target(${MODULE})
        endif()
    endforeach()
    
    if(NOT HAS_LIBRARIES)
        message(STATUS "Infra: No libraries built, skipping installation")
        return()
    endif()
    
    # 安装头文件
    infra_install_headers(${INFRA_INCLUDE_DIR})
    
    # 安装 CMake 配置
    infra_install_cmake_config()
    
    # 安装导出目标
    infra_install_export_targets()
    
    # 设置 CPack
    infra_setup_cpack()
    
    message(STATUS "Infra: Installation configured")
endfunction()