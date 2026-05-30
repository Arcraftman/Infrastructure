# modules/lnx/CMakeLists.txt
infra_init_module(lnx)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    infra_print_warning("LNX module only available on Linux, skipping")
    return()
endif()

if(INFRA_LNX_ENABLE_KERNEL)
    infra_set(KERNEL_SOURCES "")
    
    infra_append(KERNEL_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/kernel/monitor.c)
    infra_append(KERNEL_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/kernel/syscall.c)
    infra_append(KERNEL_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/kernel/proc.c)
    
    if(KERNEL_SOURCES)
        infra_register_component(lnx kernel SOURCES ${KERNEL_SOURCES})
    endif()
endif()

if(INFRA_LNX_ENABLE_CORE)
    infra_set(CORE_SOURCES "")
    
    infra_append(CORE_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/core/file.c)
    infra_append(CORE_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/core/process.c)
    infra_append(CORE_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/core/signal.c)
    
    if(CORE_SOURCES)
        infra_register_component(lnx core SOURCES ${CORE_SOURCES})
    endif()
endif()

if(INFRA_LNX_ENABLE_DRIVER)
    infra_set(DRIVER_SOURCES "")
    
    infra_append(DRIVER_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/driver/device.c)
    infra_append(DRIVER_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/driver/pci.c)
    infra_append(DRIVER_SOURCES ${INFRA_MODULE_LNX_SRC_DIR}/driver/usb.c)
    
    if(DRIVER_SOURCES)
        infra_register_component(lnx driver SOURCES ${DRIVER_SOURCES})
    endif()
endif()

infra_finalize_module(lnx)

if(TARGET lnx)
    target_link_libraries(lnx PRIVATE ${CMAKE_DL_LIBS} rt)
    
    if(INFRA_LNX_ENABLE_CORE OR INFRA_LNX_ENABLE_DRIVER)
        find_package(Threads REQUIRED)
        target_link_libraries(lnx PRIVATE Threads::Threads)
    endif()
endif()
