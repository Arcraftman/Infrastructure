# modules/stk/CMakeLists.txt
infra_init_module(stk)

if(INFRA_STK_ENABLE_CORE)
    set(CORE_SOURCES "")

    if(INFRA_STK_CORE_ENABLE_VECTOR)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/vector.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_LIST)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/list.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_RBTREE)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/rbtree.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_STRING)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/str.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_ARENA)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/arena.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_BUFFER)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/buffer.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_HASHMAP)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/hashmap.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_POOL)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/pool.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_HEAP)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/heap.c)
    endif()
    if(INFRA_STK_CORE_ENABLE_BITSET)
        list(APPEND CORE_SOURCES ${INFRA_MODULE_STK_SRC_DIR}/core/bitset.c)
    endif()
    
    infra_register_component(stk core SOURCES ${CORE_SOURCES}
        PRIVATE_DIRS ${INFRA_MODULE_STK_SRC_DIR}/core
    )
endif()

if(INFRA_STK_ENABLE_UTILS)
    set(UTIL_SOURCES
        ${INFRA_MODULE_STK_SRC_DIR}/utils/other.c
        ${INFRA_MODULE_STK_SRC_DIR}/utils/logger.c
    )
    infra_register_component(stk utils SOURCES ${UTIL_SOURCES}
        PRIVATE_DIRS ${INFRA_MODULE_STK_SRC_DIR}/utils
    )
endif()

infra_finalize_module(stk)

if(INFRA_BUILD_TESTS)
    add_subdirectory(tests)
endif()