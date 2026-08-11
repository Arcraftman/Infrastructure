if(NOT INFRA_BUILD_TESTS)
    return()
endif()

set(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test_web.c)

infra_add_test(web_core ${TEST_SRC} infra::web)
