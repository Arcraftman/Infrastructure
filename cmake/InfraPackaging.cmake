# InfraPackaging.cmake - 安装配置（全部用 macro）

if(DEFINED INFRA_PACKAGING_INCLUDED)
    return()
endif()
infra_set(INFRA_PACKAGING_INCLUDED TRUE)

include(GNUInstallDirs)

macro(infra_setup_install)
    if(NOT INFRA_INSTALL)
        infra_print_info("Installation disabled")
        return()
    endif()
    
    # 安装所有已注册的模块
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        if(TARGET ${MODULE})
            install(TARGETS ${MODULE}
                EXPORT infra-targets
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
            infra_print_success("Installed module '${MODULE}'")
        endif()
    endforeach()
    
    # 导出目标
    install(EXPORT infra-targets
        FILE infra-targets.cmake
        NAMESPACE infra::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
    )
    
    # 安装配置文件
    if(EXISTS ${CMAKE_BINARY_DIR}/infra-config.cmake)
        install(FILES
            ${CMAKE_BINARY_DIR}/infra-config.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
        )
    endif()
endmacro()