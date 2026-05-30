# modules/stk/CMakeLists.txt
infra_init_module(stk)

if(INFRA_STK_ENABLE_CORE)
    infra_set(CORE_SOURCES "")

    if(INFRA_STK_CORE_ENABLE_VECTOR)
        infra_append(CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/vector.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_LIST)
        infra_append(CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/list.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_RBTREE)
        infra_append(CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/rbtree.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_STRING)
        infra_append(CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/str.c)
    endif()
    
    infra_register_component(stk core SOURCES ${CORE_SOURCES})
endif()

infra_finalize_module(stk)