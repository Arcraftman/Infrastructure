# modules/lnx/CMakeLists.txt





# 内核模块目录


# 内核源码路径
if(EXISTS "/lib/modules/${CMAKE_HOST_SYSTEM_PROCESSOR}/build")
    set(KERNEL_BUILD_DIR "/lib/modules/${CMAKE_HOST_SYSTEM_PROCESSOR}/build")
elseif(EXISTS "/usr/src/linux-headers-$(uname -r)")
    execute_process(
        COMMAND uname -r
        OUTPUT_VARIABLE KERNEL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(KERNEL_BUILD_DIR "/usr/src/linux-headers-${KERNEL_VERSION}")
else()
    set(KERNEL_BUILD_DIR "")
endif()



# ============================================================
# 头文件
# ============================================================
set(PUBLIC_HEADERS
    ${LNX_INCLUDE_DIR}/lnx/core.h
    ${LNX_INCLUDE_DIR}/lnx/monitor.h
)

set(INTERNAL_HEADERS
    ${LNX_INTERNAL_DIR}/lnxdef.h
)

# ============================================================
# 用户空间库 (liblnx)
# ============================================================
if(BUILD_LNX)
    set(LNX_SOURCES
        ${LNX_CORE_DIR}/monitor_client.c
        ${LNX_CORE_DIR}/logger.c
        ${LNX_CORE_DIR}/utils.c
    )
    
    set(LNX_HEADERS
        ${PUBLIC_HEADERS}
        ${INTERNAL_HEADERS}
    )
    
    # 创建库
    if(BUILD_SHARED_LIBS)
        add_library(lnx SHARED ${LNX_SOURCES})
    else()
        add_library(lnx STATIC ${LNX_SOURCES})
    endif()
    
    add_library(lnx::lnx ALIAS lnx)
    
    # 包含目录
    target_include_directories(lnx
        PUBLIC
            $<BUILD_INTERFACE:${LNX_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${LNX_SRC_DIR}
            ${LNX_CORE_DIR}
            ${LNX_INTERNAL_DIR}
    )
    
    # 编译定义
    target_compile_definitions(lnx
        PUBLIC ${LNX_COMPILE_DEFS}
        PRIVATE LNX_LIBRARY
    )
    
    # 编译选项
    target_compile_options(lnx
        PRIVATE ${LNX_COMPILE_OPTIONS}
    )
    
    # 链接库
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(lnx PRIVATE pthread m)
    endif()
    
    # 设置版本
    set_target_properties(lnx
        PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            OUTPUT_NAME "lnx"
    )
    
    message(STATUS "LNX library configured: ${LNX_SOURCES}")
endif()

# ============================================================
# 内核模块 (monitor.ko)
# ============================================================
if(BUILD_KERNEL_MODULE AND KERNEL_BUILD_DIR)
    # 设置内核模块源文件
    set(KERNEL_MODULE_SOURCES
        ${LNX_KERNEL_DIR}/monitor.c
    )
    
    set(KERNEL_MODULE_HEADERS
        ${LNX_KERNEL_DIR}/monitor.h
        ${LNX_INTERNAL_DIR}/lnxdef.h
    )
    
    # 创建内核模块构建目录
    set(KERNEL_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/kernel_build)
    file(MAKE_DIRECTORY ${KERNEL_BUILD_PATH})
    
    # 复制源文件到构建目录
    foreach(KERNEL_SRC ${KERNEL_MODULE_SOURCES})
        configure_file(
            ${KERNEL_SRC}
            ${KERNEL_BUILD_PATH}/${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL_SRC}
            COPYONLY
        )
    endforeach()
    
    foreach(KERNEL_HDR ${KERNEL_MODULE_HEADERS})
        configure_file(
            ${KERNEL_HDR}
            ${KERNEL_BUILD_PATH}/${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL_HDR}
            COPYONLY
        )
    endforeach()
    
    # 创建内核模块 Makefile
    set(KERNEL_MAKEFILE ${KERNEL_BUILD_PATH}/Makefile)
    file(WRITE ${KERNEL_MAKEFILE} "
# Auto-generated Makefile for kernel module
obj-m := monitor.o
monitor-objs := monitor.o

KERNEL_DIR := ${KERNEL_BUILD_DIR}
PWD := ${KERNEL_BUILD_PATH}

default:
\t\$(MAKE) -C \$(KERNEL_DIR) M=\$(PWD) modules

clean:
\t\$(MAKE) -C \$(KERNEL_DIR) M=\$(PWD) clean
")
    
    # 添加自定义目标构建内核模块
    add_custom_target(lnx_kernel_module
        COMMAND ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_BUILD_PATH}
        COMMENT "Building kernel module monitor.ko"
        WORKING_DIRECTORY ${KERNEL_BUILD_PATH}
        DEPENDS ${KERNEL_MODULE_SOURCES} ${KERNEL_MODULE_HEADERS}
    )
    
    # 设置输出文件
    set(KERNEL_MODULE_OUTPUT ${KERNEL_BUILD_PATH}/monitor.ko)
    
    # 添加安装内核模块的目标
    add_custom_target(install_kernel_module
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_MODULE_OUTPUT} /lib/modules/${CMAKE_HOST_SYSTEM_PROCESSOR}/extra/
        COMMAND depmod -a
        COMMENT "Installing kernel module to /lib/modules/extra/"
        DEPENDS lnx_kernel_module
    )
    
    # 添加加载/卸载辅助目标
    add_custom_target(load_kernel_module
        COMMAND insmod ${KERNEL_MODULE_OUTPUT}
        COMMENT "Loading kernel module"
        DEPENDS lnx_kernel_module
    )
    
    add_custom_target(unload_kernel_module
        COMMAND rmmod monitor
        COMMENT "Unloading kernel module"
    )
    
    message(STATUS "Kernel module configured: monitor.ko")
else()
    if(BUILD_KERNEL_MODULE)
        message(WARNING "Kernel build directory not found! Kernel module will not be built.")
        message(WARNING "Please install kernel headers: apt-get install linux-headers-$(uname -r)")
    endif()
endif()

# ============================================================
# 子目录
# ============================================================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================
# 安装
# ============================================================
if(BUILD_LNX AND TARGET lnx)
    include(GNUInstallDirs)
    
    # 安装库
    install(
        TARGETS lnx
        EXPORT lnxTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lnx
    )
    
    # 安装公共头文件
    install(
        DIRECTORY ${LNX_INCLUDE_DIR}/lnx/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lnx
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    
    # 安装 internal 头文件（如果需要）
    install(
        DIRECTORY ${LNX_INTERNAL_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lnx/internal
        FILES_MATCHING PATTERN "*.h"
    )
    
    # ============================================================
    # 导出 CMake 配置
    # ============================================================
    include(CMakePackageConfigHelpers)
    
    # 生成导出文件
    install(EXPORT lnxTargets
        FILE lnxTargets.cmake
        NAMESPACE lnx::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lnx
    )
    
    # 生成版本文件
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/lnxConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    
    # 生成配置文件
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/lnxConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/lnxConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lnx
    )
    
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/lnxConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/lnxConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lnx
    )
    
    message(STATUS "Installation configured for LNX")
endif()

# ============================================================
# 帮助信息
# ============================================================
add_custom_target(lnx_help
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "LNX Build Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  lnx                 - Build LNX library"
    COMMAND ${CMAKE_COMMAND} -E echo "  lnx_kernel_module   - Build kernel module (monitor.ko)"
    COMMAND ${CMAKE_COMMAND} -E echo "  load_kernel_module  - Load kernel module"
    COMMAND ${CMAKE_COMMAND} -E echo "  unload_kernel_module - Unload kernel module"
    COMMAND ${CMAKE_COMMAND} -E echo "  install_kernel_module - Install kernel module"
    COMMAND ${CMAKE_COMMAND} -E echo "  tests               - Build tests"
    COMMAND ${CMAKE_COMMAND} -E echo "  examples            - Build examples"
    COMMAND ${CMAKE_COMMAND} -E echo "  install             - Install LNX library"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Display LNX build help"
)

message(STATUS "")
message(STATUS "LNX configuration complete!")
message(STATUS "Run 'make lnx_help' for build targets")
message(STATUS "")