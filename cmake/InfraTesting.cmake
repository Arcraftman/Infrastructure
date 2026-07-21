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
endif()
set(INFRA_TESTING_INCLUDED TRUE)

# 测试数据目录
set(INFRA_TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data")

# 添加单元测试
macro(infra_add_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_TESTS)
        return()
    endif()
    
    # 创建测试可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    # 链接依赖库
    target_link_libraries(${TEST_NAME} PRIVATE ${ARGN})
    
    # 确保测试工作目录存在
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    
    # 向 CTest 注册测试
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # 设置测试属性
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30                                   # 超时时间 30 秒
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests  # 工作目录
    )
    
    # 设置测试数据目录环境变量
    if(INFRA_TEST_DATA_DIR)
        set_tests_properties(${TEST_NAME} PROPERTIES
            ENVIRONMENT "INFRA_TEST_DATA_DIR=${INFRA_TEST_DATA_DIR}"
        )
    endif()
endmacro()

# 添加性能基准测试
macro(infra_add_perf_test TEST_NAME TEST_SOURCE)
    if(NOT INFRA_BUILD_BENCHMARKS)
        return()
    endif()
    
    # 创建性能测试可执行文件（名称加 _perf 后缀）
    add_executable(${TEST_NAME}_perf ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME}_perf PRIVATE ${ARGN})
    
    # 注册为性能测试（添加 perf 标签）
    add_test(NAME ${TEST_NAME}_perf COMMAND ${TEST_NAME}_perf)
    set_tests_properties(${TEST_NAME}_perf PROPERTIES LABELS "perf")
endmacro()

# 添加测试数据
macro(infra_add_test_data DATA_DIR DEST_DIR)
    if(INFRA_BUILD_TESTS AND EXISTS ${DATA_DIR})
        # 复制测试数据到目标目录
        file(COPY ${DATA_DIR} DESTINATION ${INFRA_TEST_DATA_DIR}/${DEST_DIR})
    endif()
endmacro()