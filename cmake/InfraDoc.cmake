# InfraDoc.cmake - Documentation generation (Doxygen + Sphinx)
#
# Provides Doxygen API-doc and Sphinx user-doc setup.
# Controlled by INFRA_BUILD_DOCS option.
#
# FUNCTIONS:
#   infra_find_doxygen()          - Locate Doxygen, set INFRA_BUILD_DOCS if absent
#   infra_setup_doxygen()         - Configure Doxygen targets
#   infra_setup_sphinx()          - Configure Sphinx targets
#   infra_setup_documentation()   - Complete doc pipeline (both Doxygen + Sphinx)
#
# OPTIONS: INFRA_BUILD_DOCS, INFRA_INSTALL, INFRA_INSTALL_HEADERS
# DEPENDENCY: Doxygen (optional), Sphinx (optional)
# PLATFORM: Cross-platform

if(DEFINED INFRA_DOC_INCLUDED)
    return()
endif()
set(INFRA_DOC_INCLUDED TRUE)

option(INFRA_BUILD_DOCS "Build documentation" OFF)

function(infra_find_doxygen)
    find_package(Doxygen QUIET)
    
    if(DOXYGEN_FOUND)
        infra_info("Doxygen found - ${DOXYGEN_VERSION}")
    else()
        infra_info("Doxygen not found, documentation disabled")
        set(INFRA_BUILD_DOCS OFF PARENT_SCOPE)
    endif()
endfunction()

function(infra_setup_doxygen)
    if(NOT INFRA_BUILD_DOCS)
        return()
    endif()
    
    infra_find_doxygen()
    
    if(NOT DOXYGEN_FOUND)
        return()
    endif()
    
    set(DOXYGEN_INPUT ${INFRA_INCLUDE_DIR})
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs/html)
    
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_NUMBER ${INFRA_VERSION_STRING})
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_USE_MATHJAX YES)
    
    if(EXISTS ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(doxygen-docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generate API documentation with Doxygen"
        )
    else()
        doxygen_add_docs(doxygen-docs
            ${DOXYGEN_INPUT}
            COMMENT "Generate API documentation with Doxygen"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
    
    add_custom_target(docs DEPENDS doxygen-docs)
    
    if(INFRA_INSTALL AND INFRA_INSTALL_HEADERS)
        install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}
            DESTINATION share/doc/${PROJECT_NAME}/html
            COMPONENT documentation
        )
    endif()
    
    infra_success("Doxygen documentation configured")
endfunction()

function(infra_setup_sphinx)
    if(NOT INFRA_BUILD_DOCS)
        return()
    endif()
    
    find_program(SPHINX_BUILD sphinx-build)
    
    if(NOT SPHINX_BUILD)
        infra_info("Sphinx not found")
        return()
    endif()
    
    set(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/docs)
    set(SPHINX_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs/html)
    
    if(EXISTS ${SPHINX_SOURCE_DIR}/conf.py)
        add_custom_target(sphinx-docs
            COMMAND ${SPHINX_BUILD} -b html ${SPHINX_SOURCE_DIR} ${SPHINX_OUTPUT_DIR}
            COMMENT "Generate documentation with Sphinx"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        add_dependencies(docs sphinx-docs)
        infra_success("Sphinx documentation configured")
    endif()
endfunction()

function(infra_setup_documentation)
    if(NOT INFRA_BUILD_DOCS)
        infra_info("Documentation disabled")
        return()
    endif()
    
    add_custom_target(docs)
    infra_setup_doxygen()
    infra_setup_sphinx()
    
    infra_success("Documentation configured (run 'make docs')")
endfunction()