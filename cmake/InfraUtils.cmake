# InfraUtils.cmake — Utility helpers for the Infra build system

if(DEFINED INFRA_UTILS_INCLUDED)
    return()
endif()
set(INFRA_UTILS_INCLUDED TRUE)

# ---------------------------------------------------------------------------
# Logging helpers — thin wrappers with a consistent "Infra:" prefix
# ---------------------------------------------------------------------------
function(infra_print_info MSG)
    message(STATUS "Infra: ${MSG}")
endfunction()

function(infra_print_warning MSG)
    message(WARNING "Infra: ${MSG}")
endfunction()

function(infra_print_success MSG)
    message(STATUS "Infra: ${MSG}")
endfunction()

function(infra_print_error MSG)
    message(FATAL_ERROR "Infra: ${MSG}")
endfunction()

function(infra_separator)
    message(STATUS "")
    message(STATUS "========================================")
endfunction()

# ---------------------------------------------------------------------------
# File helpers — content-gated write to avoid unnecessary rebuilds
# ---------------------------------------------------------------------------
function(infra_update_file FILEPATH CONTENT)
    if(EXISTS "${FILEPATH}")
        file(READ "${FILEPATH}" OLD)
        if("${OLD}" STREQUAL "${CONTENT}")
            return()
        endif()
    endif()
    file(WRITE "${FILEPATH}" "${CONTENT}")
endfunction()