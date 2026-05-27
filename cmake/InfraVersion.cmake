# InfraVersion.cmake - 版本管理

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