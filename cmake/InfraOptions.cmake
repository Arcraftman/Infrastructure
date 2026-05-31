# InfraOptions.cmake - Build option management per module
#
# Provides option discovery by scanning per-module option files.
#
# FUNCTIONS:
#   infra_define_options() - Load option definitions from per-module cmake files
#
# DEPENDENCY: none
# PLATFORM: Cross-platform

if(DEFINED INFRA_OPTIONS_INCLUDED)
    return()
endif()
set(INFRA_OPTIONS_INCLUDED TRUE)

function(infra_define_options)
    foreach(MODULE ${INFRA_MODULES})
        set(OPTIONS_FILE "${PROJECT_SOURCE_DIR}/modules/${MODULE}/${MODULE}.cmake")
        if(EXISTS "${OPTIONS_FILE}")
            include("${OPTIONS_FILE}")
        endif()
    endforeach()
endfunction()
