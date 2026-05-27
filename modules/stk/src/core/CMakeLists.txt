# modules/stk/src/core/CMakeLists.txt


set(DETAIL_DIR ${SRC_CORE_DIR}/detail)

# ============================================================================
# 复制头文件
# ============================================================================
file(COPY ${SRC_CORE_DIR}/vector.h ${SRC_CORE_DIR}/rbtree.h ${SRC_CORE_DIR}/str.h ${SRC_CORE_DIR}/list.h
     DESTINATION ${INFRA_INCLUDE_DIR}/stk/core)

if(EXISTS ${DETAIL_DIR})
    file(COPY ${DETAIL_DIR}/rbtree.h ${DETAIL_DIR}/vector.h ${DETAIL_DIR}/list.h
         DESTINATION ${INFRA_INCLUDE_DIR}/stk/core/detail)
endif()

if(EXISTS ${SRC_CORE_DIR}/config.h)
    file(COPY ${SRC_CORE_DIR}/config.h DESTINATION ${INFRA_INCLUDE_DIR}/stk/core)
endif()

# ============================================================================
# 设置额外的头文件路径（internal 目录）
# ============================================================================
infra_module_headers(stk core
    ${STK_INTERNAL_DIR}
    ${SRC_CORE_DIR}
)

# ============================================================================
# 注册子模块
# ============================================================================
if(INFRA_STK_ENABLE_CORE)
    if(INFRA_STK_CORE_ENABLE_VECTOR AND EXISTS ${SRC_CORE_DIR}/vector.c)
        infra_register_submodule(stk core vector ${SRC_CORE_DIR}/vector.c)
    endif()
    
    if(INFRA_STK_CORE_ENABLE_LIST AND EXISTS ${SRC_CORE_DIR}/list.c)
        infra_register_submodule(stk core list ${SRC_CORE_DIR}/list.c)
    endif()
    
    if(INFRA_STK_CORE_ENABLE_RBTREE AND EXISTS ${SRC_CORE_DIR}/rbtree.c)
        infra_register_submodule(stk core rbtree ${SRC_CORE_DIR}/rbtree.c)
    endif()
    
    if(INFRA_STK_CORE_ENABLE_STRING AND EXISTS ${SRC_CORE_DIR}/str.c)
        infra_register_submodule(stk core string ${SRC_CORE_DIR}/str.c)
    endif()
endif()