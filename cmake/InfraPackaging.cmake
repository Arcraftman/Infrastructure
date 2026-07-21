# [file name]: InfraPackaging.cmake (修复版)
# [file content begin]
# InfraPackaging.cmake - 安装和打包配置

if(DEFINED INFRA_PACKAGING_INCLUDED)
    return()
endif()
set(INFRA_PACKAGING_INCLUDED TRUE)

include(GNUInstallDirs)

# 设置安装
macro(infra_setup_install)
    # 默认启用安装
    if(NOT DEFINED INFRA_INSTALL)
        set(INFRA_INSTALL ON CACHE BOOL "Enable installation targets" FORCE)
    endif()
    
    if(NOT INFRA_INSTALL)
        infra_info("Installation disabled")
        return()
    endif()
    
    infra_info("Setting up installation...")
    
    # 安装所有已注册的模块
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        if(TARGET ${MODULE})
            # 关键：这里必须有 install() 命令
            install(TARGETS ${MODULE}
                EXPORT infra-targets
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
            infra_success("Added install rule for module '${MODULE}'")
        endif()
    endforeach()
    
    # 导出 CMake 目标文件
    install(EXPORT infra-targets
        FILE infra-targets.cmake
        NAMESPACE infra::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
    )
    
    # 安装头文件（所有模块）
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        set(INC_DIR "${CMAKE_SOURCE_DIR}/modules/${MODULE}/include")
        if(EXISTS ${INC_DIR})
            install(DIRECTORY ${INC_DIR}/
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
                FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
            )
            infra_info("Added install rule for headers of module '${MODULE}'")
        endif()
    endforeach()
    
    # 重要：生成一个显式的消息，确保 CMake 知道有安装规则
    message(STATUS "[Infra] Install rules have been generated")
    
    infra_success("Installation setup complete")
endmacro()