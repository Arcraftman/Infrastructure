# InfraTesting.cmake - CTest integration helpers
#
# Provides macros for adding unit tests, performance benchmarks, and test data.
# All tests are registered with CTest and include timeout / environment config.
#
# MACROS:
#   infra_add_test(NAME SOURCE [LIBS...])         - Add a unit test executable
#   infra_add_perf_test(NAME SOURCE [LIBS...])    - Add a benchmark executable
#   infra_add_test_data(DATA_DIR DEST_DIR)         - Copy test data files
#
# OPTIONS: INFRA_BUILD_TESTS, INFRA_BUILD_BENCHMARKS
# PLATFORM: Cross-platform

if(DEFINED INFRA_TESTING_INCLUDED)
    return()
endif()
set(INFRA_TESTING_INCLUDED TRUE)

set(INFRA_TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data")

macro(infra_add_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_TESTS)
        return()
    endif()
    
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${ARGN})
    
    # Ensure working directory exists
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    
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
endmacro()

macro(infra_add_perf_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_BENCHMARKS)
        return()
    endif()
    
    add_executable(${TEST_NAME}_perf ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME}_perf PRIVATE ${ARGN})
    
    add_test(NAME ${TEST_NAME}_perf COMMAND ${TEST_NAME}_perf)
    set_tests_properties(${TEST_NAME}_perf PROPERTIES LABELS "perf")
endmacro()

macro(infra_add_test_data DATA_DIR DEST_DIR)
    if(INFRA_BUILD_TESTS AND EXISTS ${DATA_DIR})
        file(COPY ${DATA_DIR} DESTINATION ${INFRA_TEST_DATA_DIR}/${DEST_DIR})
    endif()
endmacro()