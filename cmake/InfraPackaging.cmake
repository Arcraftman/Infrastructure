# InfraPackaging.cmake - Install and packaging configuration
#
# Handles module installation, CMake package export, and config file deployment.
#
# MACROS:
#   infra_setup_install() - Register install targets for all modules
#
# OPTIONS: INFRA_INSTALL, INFRA_INSTALL_HEADERS
# DEPENDENCY: GNUInstallDirs
# PLATFORM: Cross-platform

if(DEFINED INFRA_PACKAGING_INCLUDED)
    return()
endif()
set(INFRA_PACKAGING_INCLUDED TRUE)

include(GNUInstallDirs)

macro(infra_setup_install)
    if(NOT INFRA_INSTALL)
        infra_info("Installation disabled")
        return()
    endif()
    
    # Install all registered modules
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        if(TARGET ${MODULE})
            install(TARGETS ${MODULE}
                EXPORT infra-targets
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
            infra_success("Installed module '${MODULE}'")
        endif()
    endforeach()
    
    # Export CMake targets
    install(EXPORT infra-targets
        FILE infra-targets.cmake
        NAMESPACE infra::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
    )
    
    # Install config file
    if(EXISTS ${CMAKE_BINARY_DIR}/infra-config.cmake)
        install(FILES
            ${CMAKE_BINARY_DIR}/infra-config.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/infra
        )
    endif()
endmacro()