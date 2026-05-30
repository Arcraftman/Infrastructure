if(NOT INFRA_BUILD_TESTS)
    return()
endif()

set(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test_smoke.c)

infra_add_test(stk_smoke ${TEST_SRC} infra::stk)
