# InfraDoc.cmake - 文档生成配置

if(DEFINED INFRA_DOC_INCLUDED)
    return()
endif()
set(INFRA_DOC_INCLUDED TRUE)

# 查找 Doxygen
function(infra_find_doxygen)
    find_package(Doxygen QUIET)
    
    if(DOXYGEN_FOUND)
        message(STATUS "Infra: Doxygen found - ${DOXYGEN_VERSION}")
    else()
        message(STATUS "Infra: Doxygen not found, documentation disabled")
        set(INFRA_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    endif()
endfunction()

# 设置 Doxygen 配置
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
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/index.html)
    
    # 配置 Doxyfile
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_NUMBER ${INFRA_VERSION_STRING})
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_USE_MATHJAX YES)
    
    doxygen_add_docs(doxygen-docs
        ${DOXYGEN_INPUT}
        COMMENT "Generate API documentation with Doxygen"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    add_custom_target(docs DEPENDS doxygen-docs)
    
    if(INFRA_INSTALL AND INFRA_INSTALL_HEADERS)
        install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}
            DESTINATION share/doc/${PROJECT_NAME}/html
            COMPONENT documentation
        )
    endif()
    
    message(STATUS "Infra: Doxygen documentation configured")
endfunction()

# 设置 Sphinx 配置（可选）
function(infra_setup_sphinx)
    if(NOT INFRA_BUILD_DOCS)
        return()
    endif()
    
    find_program(SPHINX_BUILD sphinx-build)
    
    if(NOT SPHINX_BUILD)
        message(STATUS "Infra: Sphinx not found")
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
        message(STATUS "Infra: Sphinx documentation configured")
    endif()
endfunction()

# 主文档配置函数
function(infra_setup_documentation)
    if(NOT INFRA_BUILD_DOCS)
        message(STATUS "Infra: Documentation disabled")
        return()
    endif()
    
    add_custom_target(docs)
    infra_setup_doxygen()
    infra_setup_sphinx()
    
    message(STATUS "Infra: Documentation configured (run 'make docs')")
endfunction()