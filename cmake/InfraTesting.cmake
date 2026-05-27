# InfraTesting.cmake - 测试系统

if(DEFINED INFRA_TESTING_INCLUDED)
    return()
endif()
set(INFRA_TESTING_INCLUDED TRUE)

# 测试数据目录
set(INFRA_TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data" CACHE PATH "Test data directory")

# 添加单元测试
function(infra_add_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_TESTS)
        return()
    endif()
    
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${ARGN})
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
    
    if(INFRA_TEST_DATA_DIR)
        set_tests_properties(${TEST_NAME} PROPERTIES
            ENVIRONMENT "INFRA_TEST_DATA_DIR=${INFRA_TEST_DATA_DIR}"
        )
    endif()
endfunction()

# 添加性能测试
function(infra_add_perf_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_BENCHMARKS)
        return()
    endif()
    
    add_executable(${TEST_NAME}_perf ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME}_perf PRIVATE ${ARGN})
    
    add_test(NAME ${TEST_NAME}_perf COMMAND ${TEST_NAME}_perf)
    set_tests_properties(${TEST_NAME}_perf PROPERTIES LABELS "perf")
endfunction()

# 添加测试数据
function(infra_add_test_data DATA_DIR DEST_DIR)
    if(INFRA_BUILD_TESTS AND EXISTS ${DATA_DIR})
        file(COPY ${DATA_DIR} DESTINATION ${INFRA_TEST_DATA_DIR}/${DEST_DIR})
    endif()
endfunction()