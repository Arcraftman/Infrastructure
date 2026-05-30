# InfraDoc.cmake - 文档生成配置（全部用 macro）

if(DEFINED INFRA_DOC_INCLUDED)
    return()
endif()
infra_set(INFRA_DOC_INCLUDED TRUE)

option(INFRA_BUILD_DOCS "Build documentation" OFF)

macro(infra_find_doxygen)
    find_package(Doxygen QUIET)
    
    if(DOXYGEN_FOUND)
        infra_print_info("Doxygen found - ${DOXYGEN_VERSION}")
    else()
        infra_print_info("Doxygen not found, documentation disabled")
        infra_set(INFRA_BUILD_DOCS OFF)
    endif()
endmacro()

macro(infra_setup_doxygen)
    if(NOT INFRA_BUILD_DOCS)
        return()
    endif()
    
    infra_find_doxygen()
    
    if(NOT DOXYGEN_FOUND)
        return()
    endif()
    
    infra_set(DOXYGEN_INPUT ${INFRA_INCLUDE_DIR})
    infra_set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs/html)
    
    infra_set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    infra_set(DOXYGEN_PROJECT_NUMBER ${INFRA_VERSION_STRING})
    infra_set(DOXYGEN_RECURSIVE YES)
    infra_set(DOXYGEN_GENERATE_LATEX NO)
    infra_set(DOXYGEN_GENERATE_TREEVIEW YES)
    infra_set(DOXYGEN_USE_MATHJAX YES)
    
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
    
    infra_print_success("Doxygen documentation configured")
endmacro()

macro(infra_setup_sphinx)
    if(NOT INFRA_BUILD_DOCS)
        return()
    endif()
    
    find_program(SPHINX_BUILD sphinx-build)
    
    if(NOT SPHINX_BUILD)
        infra_print_info("Sphinx not found")
        return()
    endif()
    
    infra_set(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/docs)
    infra_set(SPHINX_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs/html)
    
    if(EXISTS ${SPHINX_SOURCE_DIR}/conf.py)
        add_custom_target(sphinx-docs
            COMMAND ${SPHINX_BUILD} -b html ${SPHINX_SOURCE_DIR} ${SPHINX_OUTPUT_DIR}
            COMMENT "Generate documentation with Sphinx"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        add_dependencies(docs sphinx-docs)
        infra_print_success("Sphinx documentation configured")
    endif()
endmacro()

macro(infra_setup_documentation)
    if(NOT INFRA_BUILD_DOCS)
        infra_print_info("Documentation disabled")
        return()
    endif()
    
    add_custom_target(docs)
    infra_setup_doxygen()
    infra_setup_sphinx()
    
    infra_print_success("Documentation configured (run 'make docs')")
endmacro()