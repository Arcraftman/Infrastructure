set(SRC_CORE_DIR ${STK_SRC_DIR}/core)

if(INFRA_STK_ENABLE_CORE AND EXISTS ${STK_SRC_DIR}/core)
    add_subdirectory(core)
endif()

if(INFRA_STK_ENABLE_UTIL AND EXISTS ${STK_SRC_DIR}/util)
    add_subdirectory(util)
endif()
