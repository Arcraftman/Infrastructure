if(NOT INFRA_BUILD_TESTS)
    return()
endif()

infra_add_test(lnx_test_open
    ${CMAKE_CURRENT_SOURCE_DIR}/test_open.c
    infra::lnx
)

add_executable(lnx_test_file_share
    ${CMAKE_CURRENT_SOURCE_DIR}/test_file_share.c
)
target_link_libraries(lnx_test_file_share PRIVATE infra::lnx)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
add_test(NAME lnx_test_file_share COMMAND lnx_test_file_share all)
set_tests_properties(lnx_test_file_share PROPERTIES
    TIMEOUT 30
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
