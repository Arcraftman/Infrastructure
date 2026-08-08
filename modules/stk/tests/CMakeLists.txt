if(NOT INFRA_BUILD_TESTS)
    return()
endif()

set(TEST_STK_HASHMAP ${CMAKE_CURRENT_SOURCE_DIR}/test_stk_hashmap.c)

infra_add_test(test_stk_hashmap ${TEST_STK_HASHMAP} infra::stk)