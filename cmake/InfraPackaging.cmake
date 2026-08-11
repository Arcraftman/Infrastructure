# [file name]: InfraPackaging.cmake
# [file content begin]
# InfraPackaging.cmake - 安装和打包配置

if(DEFINED INFRA_PACKAGING_INCLUDED)
    return()
else()
    set(INFRA_PACKAGING_INCLUDED TRUE)
endif()

include(GNUInstallDirs)

# 设置安装
macro(infra_setup_install)
    # 默认启用安装
    if(NOT DEFINED INFRA_INSTALL)
        set(INFRA_INSTALL ON CACHE BOOL "Enable installation targets" FORCE)
        infra_debug("INFRA_INSTALL default set to ON")
    else()
        infra_debug("INFRA_INSTALL = ${INFRA_INSTALL}")
    endif()
    
    if(NOT INFRA_INSTALL)
        infra_info("Installation disabled")
        return()
    endif()
    
    infra_info("Setting up installation...")
    
    # 安装所有已注册的模块
    if(INFRA_REGISTERED_MODULES)
        infra_debug("Installing modules: ${INFRA_REGISTERED_MODULES}")
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
                infra_debug("Install target ${MODULE} -> ${CMAKE_INSTALL_LIBDIR}")
            else()
                infra_warn("Module '${MODULE}' is not a valid target, skipping")
            endif()
        endforeach()
    else()
        infra_debug("No registered modules to install")
    endif()
    
    # 导出 CMake 目标文件
    # 仅在确有模块被注册时才导出；否则 infra-targets 为空，
    # 无条件 install(EXPORT) 会在配置阶段报 unknown export 错误。
    if(INFRA_REGISTERED_MODULES)
        install(EXPORT infra-targets
            FILE infra-targets.cmake
            NAMESPACE infra::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
        )
        infra_debug("Exporting targets to ${CMAKE_INSTALL_LIBDIR}/cmake/infra")
    else()
        infra_debug("No targets registered, skipping export (infra-targets)")
    endif()
    
    # 安装头文件（所有模块）
    # 注意：不再依赖任何特定模块（如 stk）的开关，统一安装模块 include 目录下
    # 实际存在的所有头文件类型（.h / .hpp / .tcc）。
    if(INFRA_REGISTERED_MODULES)
        foreach(MODULE ${INFRA_REGISTERED_MODULES})
            set(INC_DIR "${CMAKE_SOURCE_DIR}/modules/${MODULE}/include")
            if(EXISTS ${INC_DIR})
                install(DIRECTORY ${INC_DIR}/
                    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                    FILES_MATCHING
                        PATTERN "*.h"
                        PATTERN "*.hpp"
                        PATTERN "*.tcc"
                )
                infra_info("Added install rule for headers of module '${MODULE}'")
                infra_debug("Headers from ${INC_DIR} -> ${CMAKE_INSTALL_INCLUDEDIR}")
            else()
                infra_debug("No headers found for module ${MODULE} at ${INC_DIR}")
            endif()
        endforeach()
    else()
        infra_debug("No modules to install headers for")
    endif()
    
    # 重要：生成一个显式的消息，确保 CMake 知道有安装规则
    message(STATUS "[Infra] Install rules have been generated")
    
    infra_success("Installation setup complete")
endmacro()