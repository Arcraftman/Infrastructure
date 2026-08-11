# [file name]: InfraTesting.cmake
# [file content begin]
# InfraTesting.cmake - CTest 集成辅助函数
#
# 提供添加单元测试、性能基准测试和测试数据的宏。
# 所有测试都注册到 CTest，包含超时和环境配置。
#
# 宏:
#   infra_add_test(NAME SOURCE [LIBS...])         - 添加单元测试可执行文件
#   infra_add_perf_test(NAME SOURCE [LIBS...])    - 添加基准测试可执行文件
#   infra_add_test_data(DATA_DIR DEST_DIR)         - 复制测试数据文件
#
# 选项: INFRA_BUILD_TESTS, INFRA_BUILD_BENCHMARKS
# 平台: 跨平台

# 防止重复包含
if(DEFINED INFRA_TESTING_INCLUDED)
    return()
else()
    set(INFRA_TESTING_INCLUDED TRUE)
endif()

# 测试数据目录
set(INFRA_TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data")

if(EXISTS ${INFRA_TEST_DATA_DIR})
    infra_debug("Test data directory: ${INFRA_TEST_DATA_DIR}")
else()
    infra_debug("Test data directory does not exist: ${INFRA_TEST_DATA_DIR}")
endif()

# 添加单元测试
macro(infra_add_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_TESTS)
        infra_debug("Test ${TEST_NAME} skipped (INFRA_BUILD_TESTS is OFF)")
        return()
    endif()
    
    # 创建测试可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    # 链接依赖库
    target_link_libraries(${TEST_NAME} PRIVATE ${ARGN})
    infra_debug("Created test executable: ${TEST_NAME}")
    
    # 测试目标也继承项目级编译/覆盖率设置
    if(COMMAND infra_setup_target)
        infra_setup_target(${TEST_NAME})
    endif()

    # 确保测试工作目录存在
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

    # 向 CTest 注册测试
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # 设置测试属性
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30                                   # 超时时间 30 秒
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests  # 工作目录
    )
    infra_debug("Registered test: ${TEST_NAME} (timeout: 30s)")
    
    # 设置测试数据目录环境变量
    if(INFRA_TEST_DATA_DIR)
        set_tests_properties(${TEST_NAME} PROPERTIES
            ENVIRONMENT "INFRA_TEST_DATA_DIR=${INFRA_TEST_DATA_DIR}"
        )
        infra_debug("Set test data environment for ${TEST_NAME}")
    else()
        infra_debug("No test data directory for ${TEST_NAME}")
    endif()
endmacro()

# 添加性能基准测试
macro(infra_add_perf_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_BENCHMARKS)
        infra_debug("Perf test ${TEST_NAME} skipped (INFRA_BUILD_BENCHMARKS is OFF)")
        return()
    endif()
    
    # 创建性能测试可执行文件（名称加 _perf 后缀）
    add_executable(${TEST_NAME}_perf ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME}_perf PRIVATE ${ARGN})
    infra_debug("Created perf test: ${TEST_NAME}_perf")
    
    # 注册为性能测试（添加 perf 标签）
    add_test(NAME ${TEST_NAME}_perf COMMAND ${TEST_NAME}_perf)
    set_tests_properties(${TEST_NAME}_perf PROPERTIES LABELS "perf")
    infra_debug("Registered perf test: ${TEST_NAME}_perf")
endmacro()

# 添加测试数据
macro(infra_add_test_data DATA_DIR DEST_DIR)
    if(INFRA_BUILD_TESTS AND EXISTS ${DATA_DIR})
        # 复制测试数据到目标目录
        file(COPY ${DATA_DIR} DESTINATION ${INFRA_TEST_DATA_DIR}/${DEST_DIR})
        infra_debug("Copied test data from ${DATA_DIR} to ${INFRA_TEST_DATA_DIR}/${DEST_DIR}")
    elseif(NOT INFRA_BUILD_TESTS)
        infra_debug("Test data copy skipped (INFRA_BUILD_TESTS is OFF)")
    elseif(NOT EXISTS ${DATA_DIR})
        infra_warn("Test data directory not found: ${DATA_DIR}")
    endif()
endmacro()