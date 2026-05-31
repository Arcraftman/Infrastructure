# InfraModules.cmake - Module registration and build pipeline
#
# Defines the module lifecycle: register → init → register_component →
# finalize → add_module. Each module lives under modules/${MODULE_NAME}
# with its own CMakeLists.txt.
#
# MACROS:
#   infra_register_module(NAME)       - Register a module name in the global list
#   infra_init_module(NAME)           - Full init: register + dir setup + output
#   infra_register_component(...)     - Add an object library component to a module
#   infra_finalize_module(NAME)       - Assemble module library from components
#   infra_add_module(NAME)            - conditionally include a module subdirectory
#
# GLOBAL STATE:
#   INFRA_REGISTERED_MODULES          - List of all registered module names
#   INFRA_MODULE_${NAME}_OBJECTS      - Object files collected for the module
#   INFRA_MODULE_${NAME}_LINK_LIBS    - Link libraries for the module
#
# OPTIONS USED: INFRA_ENABLE_{MODULE_UPPER}
# DEPENDENCY: InfraUtils, InfraCompiler
# PLATFORM: Cross-platform

if(DEFINED INFRA_MODULES_INCLUDED)
    return()
endif()
set(INFRA_MODULES_INCLUDED TRUE)

include(InfraUtils)
include(InfraCompiler)

# Global state
set(INFRA_REGISTERED_MODULES "")
set(INFRA_OUTPUT_DIRECTORIES_SET FALSE)

# ---------------------------------------------------------------------------
# Output directories
# ---------------------------------------------------------------------------
macro(infra_setup_output_dirs)
    if(INFRA_OUTPUT_DIRECTORIES_SET)
        return()
    endif()
    infra_set_output_dirs()
    set(INFRA_OUTPUT_DIRECTORIES_SET TRUE)
endmacro()

# ---------------------------------------------------------------------------
# Module directory setup
# ---------------------------------------------------------------------------
macro(infra_setup_module_dirs MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    set(INFRA_MODULE_${_MODULE}_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(INFRA_MODULE_${_MODULE}_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(INFRA_MODULE_${_MODULE}_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
endmacro()

# ---------------------------------------------------------------------------
# Module registration
# ---------------------------------------------------------------------------
macro(infra_register_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    if(NOT ${MODULE_NAME} IN_LIST INFRA_REGISTERED_MODULES)
        list(APPEND INFRA_REGISTERED_MODULES ${MODULE_NAME})
        set(INFRA_REGISTERED_MODULES "${INFRA_REGISTERED_MODULES}" PARENT_SCOPE)
        set(INFRA_MODULE_${_MODULE}_OBJECTS "")
        set(INFRA_MODULE_${_MODULE}_LINK_LIBS "")
        infra_info("Registered module '${MODULE_NAME}'")
    endif()
endmacro()

# ---------------------------------------------------------------------------
# Module initialization (one-stop)
# ---------------------------------------------------------------------------
macro(infra_init_module MODULE_NAME)
    infra_register_module(${MODULE_NAME})
    infra_setup_module_dirs(${MODULE_NAME})
    infra_setup_output_dirs()
    infra_info("Initialized module '${MODULE_NAME}'")
endmacro()

# ---------------------------------------------------------------------------
# Component registration
#
# Creates an OBJECT library for the component and collects its objects
# into the parent module's object list.
#
# Keywords: SOURCES PRIVATE_DIRS LINK_LIBS COMPILE_DEFS
# ---------------------------------------------------------------------------
macro(infra_register_component MODULE_NAME COMPONENT_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES PRIVATE_DIRS LINK_LIBS COMPILE_DEFS)
    cmake_parse_arguments(COMP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT COMP_SOURCES)
        infra_warn("Component ${MODULE_NAME}/${COMPONENT_NAME} has no sources")
        return()
    endif()
    
    # Record component sources
    set(INFRA_MODULE_${_MODULE}_${COMPONENT_NAME}_SOURCES ${COMP_SOURCES})
    
    set(TARGET_NAME "infra_${MODULE_NAME}_${COMPONENT_NAME}")
    add_library(${TARGET_NAME} OBJECT ${COMP_SOURCES})
    
    # Include directories
    target_include_directories(${TARGET_NAME} 
        PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${INFRA_MODULE_${_MODULE}_INC_DIR}
    )
    
    # Private directories
    foreach(DIR ${COMP_PRIVATE_DIRS})
        target_include_directories(${TARGET_NAME} PRIVATE ${DIR})
    endforeach()
    
    # Compile definitions
    foreach(DEF ${COMP_COMPILE_DEFS})
        target_compile_definitions(${TARGET_NAME} PRIVATE ${DEF})
    endforeach()
    
    # Auto-define DLL export macros when building shared libraries.
    # For a module named "stk", this adds -DSTK_DLL -DSTK_EXPORTING so the
    # STK_API macro in stk/def.h emits dllexport/visibility("default").
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${TARGET_NAME} PRIVATE
            ${_MODULE}_DLL
            ${_MODULE}_EXPORTING
        )
    endif()
    
    # Apply compiler settings
    infra_setup_target(${TARGET_NAME})
    
    # Collect object files
    set(INFRA_MODULE_${_MODULE}_OBJECTS 
        ${INFRA_MODULE_${_MODULE}_OBJECTS} 
        $<TARGET_OBJECTS:${TARGET_NAME}>)
    
    # Collect link libraries
    foreach(LIB ${COMP_LINK_LIBS})
        list(APPEND INFRA_MODULE_${_MODULE}_LINK_LIBS ${LIB})
    endforeach()
    
    infra_success("Component: ${TARGET_NAME}")
endmacro()

# ---------------------------------------------------------------------------
# Module finalization
#
# Assembles all component objects into the module library target
# (shared or static depending on BUILD_SHARED_LIBS).
# ---------------------------------------------------------------------------
macro(infra_finalize_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} _MODULE)
    if(TARGET ${MODULE_NAME})
        return()
    endif()
    
    set(OBJECTS ${INFRA_MODULE_${_MODULE}_OBJECTS})
    if(NOT OBJECTS)
        infra_warn("Module '${MODULE_NAME}' has no objects")
        return()
    endif()
    
    if(BUILD_SHARED_LIBS)
        add_library(${MODULE_NAME} SHARED ${OBJECTS})
    else()
        add_library(${MODULE_NAME} STATIC ${OBJECTS})
    endif()
    
    add_library(infra::${MODULE_NAME} ALIAS ${MODULE_NAME})
    
    # Public header directory - use the module's own include directory
    target_include_directories(${MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${INFRA_MODULE_${_MODULE}_INC_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )
    
    # Private header directory
    if(INFRA_MODULE_${_MODULE}_INC_DIR)
        target_include_directories(${MODULE_NAME}
            PRIVATE ${INFRA_MODULE_${_MODULE}_INC_DIR}
        )
    endif()
    
    # Link libraries
    if(INFRA_MODULE_${_MODULE}_LINK_LIBS)
        target_link_libraries(${MODULE_NAME} PRIVATE ${INFRA_MODULE_${_MODULE}_LINK_LIBS})
    endif()
    
    infra_setup_target(${MODULE_NAME})
    infra_success("Module '${MODULE_NAME}' created")
endmacro()

# ---------------------------------------------------------------------------
# Module inclusion
#
# Conditionally includes a module's CMakeLists.txt based on its
# INFRA_ENABLE_{UPPER} option.
# ---------------------------------------------------------------------------
macro(infra_add_module MODULE_NAME)
    string(TOUPPER ${MODULE_NAME} MODULE_UPPER)
    if(NOT INFRA_ENABLE_${MODULE_UPPER})
        infra_info("Module '${MODULE_NAME}' disabled")
        return()
    endif()

    set(MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/${MODULE_NAME}")
    if(NOT EXISTS "${MODULE_PATH}/CMakeLists.txt")
        infra_warn("Module '${MODULE_NAME}' not found")
        return()
    endif()

    add_subdirectory(${MODULE_PATH})
    infra_success("Added module '${MODULE_NAME}'")
endmacro()

macro(infra_add_modules)
    foreach(MODULE ${INFRA_MODULES})
        infra_add_module(${MODULE})
    endforeach()
endmacro()