# InfraVersion.cmake - Version string generation
#
# Provides helpers for deriving a version string from Git tags (git describe)
# or falling back to a project-defined version, and generating version headers.
#
# FUNCTIONS:
#   infra_generate_version(OUTPUT_PATH) - Generate version header file
#
# MACROS:
#   infra_git_describe(VAR_NAME PATH) - Run git describe, store result in VAR_NAME
#
# VARIABLES USED:
#   GIT_FOUND, GIT_EXECUTABLE - from FindGit
#   PROJECT_VERSION          - from project() command
# PLATFORM: Cross-platform (requires Git for describe)

if(DEFINED INFRA_VERSION_INCLUDED)
    return()
endif()
set(INFRA_VERSION_INCLUDED TRUE)

macro(infra_git_describe VAR_NAME PATH)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
            WORKING_DIRECTORY ${PATH}
            OUTPUT_VARIABLE ${VAR_NAME}
            RESULT_VARIABLE GIT_RESULT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT GIT_RESULT EQUAL 0)
            set(${VAR_NAME} "unknown")
        endif()
    else()
        set(${VAR_NAME} "unknown")
    endif()
endmacro()

function(infra_generate_version OUTPUT_PATH)
    # Try git describe first
    infra_git_describe(INFRA_GIT_VERSION "${CMAKE_SOURCE_DIR}")
    if(INFRA_GIT_VERSION STREQUAL "unknown")
        set(INFRA_VERSION_STRING "${PROJECT_VERSION}")
    else()
        set(INFRA_VERSION_STRING "${INFRA_GIT_VERSION}")
    endif()

    # Parse version components from PROJECT_VERSION
    set(INFRA_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set(INFRA_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    set(INFRA_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

    # Propagate to caller scope
    set(INFRA_VERSION_STRING "${INFRA_VERSION_STRING}" PARENT_SCOPE)

    # Generate header file
    file(WRITE "${OUTPUT_PATH}"
        "#ifndef INFRA_VERSION_H\n"
        "#define INFRA_VERSION_H\n"
        "\n"
        "#define INFRA_VERSION_STRING \"${INFRA_VERSION_STRING}\"\n"
        "#define INFRA_VERSION_MAJOR ${INFRA_VERSION_MAJOR}\n"
        "#define INFRA_VERSION_MINOR ${INFRA_VERSION_MINOR}\n"
        "#define INFRA_VERSION_PATCH ${INFRA_VERSION_PATCH}\n"
        "#define INFRA_VERSION_TWEAK 0\n"
        "\n"
        "#endif /* INFRA_VERSION_H */\n"
    )
    message(STATUS "Generated version header: ${OUTPUT_PATH}")
endfunction()