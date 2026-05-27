# modules/CMakeLists.txt

# ============================================================================
# 注册主模块（必须先注册）
# ============================================================================
if(INFRA_ENABLE_STK)
    infra_register_module(stk)
endif()

if(INFRA_ENABLE_LNX)
    infra_register_module(lnx)
endif()

# ============================================================================
# 添加子目录（子目录中会注册子模块）
# ============================================================================
if(INFRA_ENABLE_STK AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stk/CMakeLists.txt)
    add_subdirectory(stk)
endif()

if(INFRA_ENABLE_LNX AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lnx/CMakeLists.txt)
    add_subdirectory(lnx)
endif()