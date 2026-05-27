set(STK_ROOT_DIR ${INFRA_MODULES_DIR}/stk)
set(STK_OS_DIR ${INFRA_MODULES_DIR}/os)


option(BUILD_STK "Build STK project" ON)
option(BUILD_LNX "Build LNX project" ON)


if(BUILD_STK)
    add_subdirectory(stk)
endif()

if(BUILD_LNX)
    add_subdirectory(lnx)
endif()