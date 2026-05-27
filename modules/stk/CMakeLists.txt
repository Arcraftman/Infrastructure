# modules/stk/CMakeLists.txt

set(STK_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(STK_INCLUDE_DIR ${STK_ROOT_DIR}/include)
set(STK_SRC_DIR ${STK_ROOT_DIR}/src)
set(STK_INTERNAL_DIR ${STK_ROOT_DIR}/internal)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# 添加源文件子目录
add_subdirectory(src)

# 创建 libstk 库
infra_create_module_library(stk)

# 可选：测试目录
if(INFRA_BUILD_TESTS AND EXISTS ${STK_ROOT_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()