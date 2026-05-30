# InfraUtils.cmake - 基础工具函数（全部用 macro）

if(DEFINED INFRA_UTILS_INCLUDED)
    return()
endif()
infra_set(INFRA_UTILS_INCLUDED TRUE)

# ============================================================================
# 变量操作
# ============================================================================
macro(infra_set VAR VALUE)
    set(${VAR} ${VALUE})
endmacro()

macro(infra_append VAR VALUE)
    list(APPEND ${VAR} ${VALUE})
endmacro()

# ============================================================================
# 打印函数
# ============================================================================
macro(infra_msg MSG)
    message(STATUS "${MSG}")
endmacro()

macro(infra_msg_warn MSG)
    message(WARNING "${MSG}")
endmacro()

macro(infra_print_info MSG)
    infra_msg("Infra: ${MSG}")
endmacro()

macro(infra_print_warning MSG)
    infra_msg_warn("Infra: ${MSG}")
endmacro()

macro(infra_print_success MSG)
    infra_msg("Infra: ${MSG}")
endmacro()

macro(infra_separator)
    infra_msg("")
    infra_msg("========================================")
endmacro()

macro(infra_print_config)
    infra_separator()
    infra_msg("Infra Configuration Summary")
    infra_separator()
    infra_msg("  Build Type:       ${CMAKE_BUILD_TYPE}")
    infra_msg("  Library Type:     ${INFRA_LIBRARY_TYPE}")
    infra_msg("  Compiler:         ${INFRA_COMPILER_ID}")
    infra_msg("  Tests:            ${INFRA_BUILD_TESTS}")
    infra_msg("  Install:          ${INFRA_INSTALL}")
    # 动态打印所有启用的模块
    foreach(MODULE ${INFRA_REGISTERED_MODULES})
        string(TOUPPER ${MODULE} MODULE_UPPER)
        infra_msg("  ${MODULE}:           ${INFRA_ENABLE_${MODULE_UPPER}}")
    endforeach()
    infra_separator()
    infra_msg("")
endmacro()

# ============================================================================
# 目录设置
# ============================================================================
macro(infra_setup_dirs)
    infra_set(INFRA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
    infra_set(INFRA_BUILD_DIR ${CMAKE_BINARY_DIR})
endmacro()

macro(infra_set_output_dirs)
    infra_set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    infra_set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    infra_set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endmacro()

# ============================================================================
# 文件操作
# ============================================================================
macro(infra_update_file FILEPATH CONTENT)
    if(EXISTS "${FILEPATH}")
        file(READ "${FILEPATH}" OLD)
        if("${OLD}" STREQUAL "${CONTENT}")
            return()
        endif()
    endif()
    file(WRITE "${FILEPATH}" "${CONTENT}")
endmacro()

macro(infra_generate_version DEST_FILE)
    infra_set(CONTENT "#ifndef INFRA_VERSION_H\n")
    infra_append(CONTENT "#define INFRA_VERSION_H\n\n")
    infra_append(CONTENT "#define INFRA_VERSION_STRING \"${INFRA_VERSION_STRING}\"\n")
    infra_append(CONTENT "#define INFRA_VERSION_MAJOR ${INFRA_VERSION_MAJOR}\n")
    infra_append(CONTENT "#define INFRA_VERSION_MINOR ${INFRA_VERSION_MINOR}\n")
    infra_append(CONTENT "#define INFRA_VERSION_PATCH ${INFRA_VERSION_PATCH}\n\n")
    infra_append(CONTENT "#endif\n")
    infra_update_file(${DEST_FILE} "${CONTENT}")
endmacro()