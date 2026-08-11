# [file name]: InfraDoc.cmake
# [file content begin]
# InfraDoc.cmake - 文档生成 (Doxygen + Sphinx)
#
# 提供 Doxygen API 文档和 Sphinx 用户文档的设置。
# 由 INFRA_BUILD_DOCS 选项控制。
#
# 函数:
#   infra_find_doxygen()          - 定位 Doxygen，如果未找到则禁用 INFRA_BUILD_DOCS
#   infra_setup_doxygen()         - 配置 Doxygen 构建目标
#   infra_setup_sphinx()          - 配置 Sphinx 构建目标
#   infra_setup_documentation()   - 完整的文档构建流程（Doxygen + Sphinx）
#
# 选项: INFRA_BUILD_DOCS, INFRA_INSTALL, INFRA_INSTALL_HEADERS
# 依赖: Doxygen (可选), Sphinx (可选)
# 平台: 跨平台

# 防止重复包含
if(DEFINED INFRA_DOC_INCLUDED)
    return()
else()
    set(INFRA_DOC_INCLUDED TRUE)
endif()

# 文档构建选项
option(INFRA_BUILD_DOCS "Build documentation" OFF)

# 查找 Doxygen
function(infra_find_doxygen)
    find_package(Doxygen QUIET)
    
    if(DOXYGEN_FOUND)
        infra_info("Doxygen found - ${DOXYGEN_VERSION}")
    else()
        infra_info("Doxygen not found, documentation disabled")
        set(INFRA_BUILD_DOCS OFF PARENT_SCOPE)
    endif()
endfunction()

# 配置 Doxygen
function(infra_setup_doxygen)
    if(NOT INFRA_BUILD_DOCS)
        infra_debug("Doxygen setup skipped (INFRA_BUILD_DOCS is OFF)")
        return()
    endif()
    
    infra_find_doxygen()
    
    if(NOT DOXYGEN_FOUND)
        infra_debug("Doxygen not found, skipping setup")
        return()
    endif()
    
    # Doxygen 配置变量；统一使用 Infra 源码根目录，避免被调用上下文改变。
    set(DOXYGEN_INPUT "${PROJECT_SOURCE_DIR}/modules")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs/html")
    
    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER "${INFRA_VERSION_STRING}")
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_USE_MATHJAX YES)
    
    # 如果存在 Doxyfile.in 模板，使用它
    set(INFRA_DOXYFILE_IN "${PROJECT_SOURCE_DIR}/docs/Doxyfile.in")
    set(INFRA_DOXYFILE_OUT "${CMAKE_BINARY_DIR}/Doxyfile")
    if(EXISTS "${INFRA_DOXYFILE_IN}")
        configure_file("${INFRA_DOXYFILE_IN}" "${INFRA_DOXYFILE_OUT}" @ONLY)
        add_custom_target(doxygen-docs
            COMMAND ${DOXYGEN_EXECUTABLE} "${INFRA_DOXYFILE_OUT}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generate API documentation with Doxygen"
        )
        infra_debug("Generated Doxyfile from template")
    else()
        doxygen_add_docs(doxygen-docs
            ${DOXYGEN_INPUT}
            COMMENT "Generate API documentation with Doxygen"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        infra_debug("Using default doxygen_add_docs")
    endif()
    
    # 创建 docs 主目标，依赖 doxygen-docs
    add_custom_target(docs DEPENDS doxygen-docs)
    
    # 安装文档
    if(INFRA_INSTALL AND INFRA_INSTALL_HEADERS)
        install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}
            DESTINATION share/doc/${PROJECT_NAME}/html
            COMPONENT documentation
        )
        infra_debug("Added install rule for Doxygen docs")
    else()
        infra_debug("Doxygen docs installation skipped")
    endif()
    
    infra_success("Doxygen documentation configured")
endfunction()

# 配置 Sphinx
function(infra_setup_sphinx)
    if(NOT INFRA_BUILD_DOCS)
        infra_debug("Sphinx setup skipped (INFRA_BUILD_DOCS is OFF)")
        return()
    endif()
    
    find_program(SPHINX_BUILD sphinx-build)
    
    if(NOT SPHINX_BUILD)
        infra_info("Sphinx not found")
        return()
    endif()
    
    set(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/docs)
    set(SPHINX_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs/html)
    
    # 如果存在 Sphinx 配置文件 conf.py
    if(EXISTS ${SPHINX_SOURCE_DIR}/conf.py)
        add_custom_target(sphinx-docs
            COMMAND ${SPHINX_BUILD} -b html ${SPHINX_SOURCE_DIR} ${SPHINX_OUTPUT_DIR}
            COMMENT "Generate documentation with Sphinx"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        # 将 sphinx-docs 添加为 docs 目标的依赖
        add_dependencies(docs sphinx-docs)
        infra_success("Sphinx documentation configured")
        infra_debug("Sphinx source: ${SPHINX_SOURCE_DIR}, output: ${SPHINX_OUTPUT_DIR}")
    else()
        infra_debug("Sphinx conf.py not found at ${SPHINX_SOURCE_DIR}/conf.py")
    endif()
endfunction()

# 完整的文档设置（Doxygen + Sphinx）
function(infra_setup_documentation)
    if(NOT INFRA_BUILD_DOCS)
        infra_info("Documentation disabled")
        return()
    endif()
    
    # 由 Doxygen/Sphinx 子函数按需创建 docs 目标。
    # 不在这里预先创建，避免与 doxygen-docs 依赖目标重复。
    infra_debug("Documentation target will be created by available generators")
    
    # 配置 Doxygen
    infra_setup_doxygen()
    
    # 配置 Sphinx
    infra_setup_sphinx()
    
    infra_success("Documentation configured (run 'make docs')")
endfunction()