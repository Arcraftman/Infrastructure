# InfraUtils.cmake - 工具函数

if(DEFINED INFRA_UTILS_INCLUDED)
    return()
endif()
set(INFRA_UTILS_INCLUDED TRUE)

function(infra_debug MSG)
    if(INFRA_CMAKE_DEBUG)
        message(STATUS "[Infra] ${MSG}")
    endif()
endfunction()

function(infra_copy_headers SOURCE_DIR DEST_DIR)
    if(NOT EXISTS ${SOURCE_DIR})
        return()
    endif()
    file(GLOB_RECURSE HEADERS "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.hpp" "${SOURCE_DIR}/*.tcc")
    foreach(HEADER ${HEADERS})
        file(RELATIVE_PATH REL ${SOURCE_DIR} ${HEADER})
        get_filename_component(DIR ${REL} DIRECTORY)
        if(DIR)
            set(DEST ${DEST_DIR}/${DIR})
        else()
            set(DEST ${DEST_DIR})
        endif()
        file(COPY ${HEADER} DESTINATION ${DEST})
    endforeach()
endfunction()

function(infra_update_file FILEPATH CONTENT)
    if(EXISTS "${FILEPATH}")
        file(READ "${FILEPATH}" OLD)
        if("${OLD}" STREQUAL "${CONTENT}")
            return()
        endif()
    endif()
    file(WRITE "${FILEPATH}" "${CONTENT}")
endfunction()

# ============================================================================
# 生成版本头文件（之前遗漏的函数）
# ============================================================================
function(infra_generate_version_file DEST_FILE)
    set(VERSION_CONTENT "#ifndef INFRA_VERSION_H\n")
    set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_VERSION_H\n\n")
    set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_VERSION_STRING \"${INFRA_VERSION_STRING}\"\n")
    set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_VERSION_MAJOR ${INFRA_VERSION_MAJOR}\n")
    set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_VERSION_MINOR ${INFRA_VERSION_MINOR}\n")
    set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_VERSION_PATCH ${INFRA_VERSION_PATCH}\n")
    set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_VERSION_TWEAK ${INFRA_VERSION_TWEAK}\n\n")
    
    if(GIT_FOUND)
        infra_git_describe(INFRA_GIT_HASH ${PROJECT_SOURCE_DIR})
        set(VERSION_CONTENT "${VERSION_CONTENT}#define INFRA_GIT_HASH \"${INFRA_GIT_HASH}\"\n")
    endif()
    
    set(VERSION_CONTENT "${VERSION_CONTENT}\n#endif /* INFRA_VERSION_H */\n")
    
    infra_update_file(${DEST_FILE} "${VERSION_CONTENT}")
endfunction()