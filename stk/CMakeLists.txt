# ============== 目录变量 ==============
set(STK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(STK_SRC_CXX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp)
set(STK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(STK_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(STK_UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utils)

# ============== 选项 ==============
option(BUILD_CLIB "Build C version" ON)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_VECTOR "Build Vector module" ON)
option(BUILD_RBTREE "Build RBTree module" ON)
option(BUILD_STRING "Build String module" ON)
option(BUILD_TESTS "Build tests" ON)

set(INTERNAL_INCLUDE_DIR ${STK_INCLUDE_DIR}/stk/internal)
file(MAKE_DIRECTORY ${INTERNAL_INCLUDE_DIR})

# ============== 内部头文件 ==============
set(C_INTERNAL_HEADERS
    ${STK_SRC_DIR}/stk_export.h
    ${STK_SRC_DIR}/stk_rbtree_config.h
    ${STK_SRC_DIR}/stk_vector_config.h
)

set(VECTOR_C_HEADERS
    ${STK_SRC_DIR}/stk_vector_int.h
    ${STK_SRC_DIR}/stk_vector_float.h
    ${STK_SRC_DIR}/stk_vector_double.h
    ${STK_SRC_DIR}/stk_vector_pointer.h
    ${STK_SRC_DIR}/stk_vector_string.h
)

set(RBTREE_C_HEADERS
    ${STK_SRC_DIR}/stk_rbtree_int.h
    ${STK_SRC_DIR}/stk_rbtree_float.h
    ${STK_SRC_DIR}/stk_rbtree_double.h
    ${STK_SRC_DIR}/stk_rbtree_pointer.h
    ${STK_SRC_DIR}/stk_rbtree_string.h
)

set(STRING_C_HEADERS
    ${STK_SRC_DIR}/stk_string.h
)

# 只保留 .hpp 和 .tcc 文件
set(CPP_INTERNAL_HEADERS
    ${STK_SRC_CXX_DIR}/stk_vector.hpp
    ${STK_SRC_CXX_DIR}/stk_vector.tcc
    ${STK_SRC_CXX_DIR}/stk_rbtree.hpp
    ${STK_SRC_CXX_DIR}/stk_rbtree.tcc
    ${STK_SRC_CXX_DIR}/stk_string.hpp
    ${STK_SRC_CXX_DIR}/stk_string.tcc
)

# 复制所有头文件
file(COPY ${C_INTERNAL_HEADERS} ${VECTOR_C_HEADERS} ${RBTREE_C_HEADERS} ${STRING_C_HEADERS}
    DESTINATION ${INTERNAL_INCLUDE_DIR}
)
file(COPY ${CPP_INTERNAL_HEADERS}
    DESTINATION ${INTERNAL_INCLUDE_DIR}/cpp
)

# ============== 收集所有 OBJECT 文件 ==============
set(ALL_OBJECTS "")

# ============== 1. Vector 模块 ==============
if(BUILD_VECTOR)
    set(VECTOR_TYPES
        int
        float
        double 
        pointer
        string
    )
    
    foreach(TYPE ${VECTOR_TYPES})
        add_library(stk_vector_${TYPE} OBJECT 
            ${STK_SRC_DIR}/stk_vector_${TYPE}.c
        )
        
        target_include_directories(stk_vector_${TYPE}
            PUBLIC  $<BUILD_INTERFACE:${STK_INCLUDE_DIR}> $<INSTALL_INTERFACE:include/stk>
            PRIVATE ${STK_SRC_DIR} ${STK_UTIL_DIR}
        )
        
        if(BUILD_SHARED_LIBS)
            target_compile_definitions(stk_vector_${TYPE}
                PUBLIC STK_LIBC PRIVATE STK_EXPORTING
            )
        else()
            target_compile_definitions(stk_vector_${TYPE}
                PUBLIC STK_LIBC
            )
        endif()
        
        if(MSVC)
            target_compile_options(stk_vector_${TYPE} PRIVATE /W4 /wd4013 /wd4047 /wd4996)
        else()
            target_compile_options(stk_vector_${TYPE} PRIVATE -Wall -Wextra -Wpedantic -fPIC)
        endif()

        list(APPEND ALL_OBJECTS $<TARGET_OBJECTS:stk_vector_${TYPE}>)
    endforeach()
endif()

# ============== 2. RBTree 模块 ==============
if(BUILD_RBTREE)
    set(RBTREE_TYPES
        int
        float
        double
        pointer
        string
    )
    
    foreach(TYPE ${RBTREE_TYPES})
        add_library(stk_rbtree_${TYPE} OBJECT 
            ${STK_SRC_DIR}/stk_rbtree_${TYPE}.c
        )
        
        target_include_directories(stk_rbtree_${TYPE}
            PUBLIC  $<BUILD_INTERFACE:${STK_INCLUDE_DIR}> $<INSTALL_INTERFACE:include/stk>
            PRIVATE ${STK_SRC_DIR} ${STK_UTIL_DIR}
        )
        
        if(BUILD_SHARED_LIBS)
            target_compile_definitions(stk_rbtree_${TYPE}
                PUBLIC STK_LIBC PRIVATE STK_EXPORTING
            )
        else()
            target_compile_definitions(stk_rbtree_${TYPE}
                PUBLIC STK_LIBC
            )
        endif()
        
        if(MSVC)
            target_compile_options(stk_rbtree_${TYPE} PRIVATE /W4 /wd4013 /wd4047 /wd4996)
        else()
            target_compile_options(stk_rbtree_${TYPE} PRIVATE -Wall -Wextra -Wpedantic  -fPIC)
        endif()

        list(APPEND ALL_OBJECTS $<TARGET_OBJECTS:stk_rbtree_${TYPE}>)
    endforeach()
endif()

# ============== 3. String 模块 ==============
if(BUILD_STRING)
    add_library(stk_string OBJECT ${STK_SRC_DIR}/stk_string.c)
    
    target_include_directories(stk_string
        PUBLIC  $<BUILD_INTERFACE:${STK_INCLUDE_DIR}> $<INSTALL_INTERFACE:include/stk>
        PRIVATE ${STK_SRC_DIR} ${STK_UTIL_DIR}
    )
    
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(stk_string PUBLIC STK_LIBC PRIVATE STK_EXPORTING)
    else()
        target_compile_definitions(stk_string PUBLIC STK_LIBC)
    endif()
    
    if(MSVC)
        target_compile_options(stk_string PRIVATE /W4 /wd4013 /wd4047 /wd4996)
    else()
        target_compile_options(stk_string PRIVATE -Wall -Wextra -Wpedantic -fPIC)
    endif()

    list(APPEND ALL_OBJECTS $<TARGET_OBJECTS:stk_string>)
endif()

# ============== 主库 ==============
if(BUILD_CLIB)
    if(BUILD_SHARED_LIBS)
        message(STATUS "Building Shared Library ...")
        
        add_library(stk SHARED ${ALL_OBJECTS})
        add_library(stk::stk ALIAS stk)
        
        target_include_directories(stk 
            PUBLIC  $<BUILD_INTERFACE:${STK_INCLUDE_DIR}> $<INSTALL_INTERFACE:include/stk>
            PRIVATE ${STK_SRC_DIR} ${STK_UTIL_DIR}
        )
        
        if(MSVC)
            target_compile_definitions(stk PRIVATE STK_EXPORTING PUBLIC STK_LIBC STK_DLL)
            set_target_properties(stk PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
        else()
            target_compile_definitions(stk PRIVATE STK_EXPORTING PUBLIC STK_LIBC)
        endif()
        
        if(MSVC)
            target_compile_options(stk PRIVATE /W4 /wd4013 /wd4047 /wd4996)
        else()
            target_compile_options(stk PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            target_link_libraries(stk PRIVATE m)
        endif()
        
        set_target_properties(stk PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            OUTPUT_NAME "stk"
        )
        
    else()
        message(STATUS "Building Static Library ...")
        
        add_library(stk STATIC ${ALL_OBJECTS})
        add_library(stk::stk ALIAS stk)
        
        target_include_directories(stk 
            PUBLIC  $<BUILD_INTERFACE:${STK_INCLUDE_DIR}> $<INSTALL_INTERFACE:include/stk>
            PRIVATE ${STK_SRC_DIR} ${STK_UTIL_DIR}
        )
        
        target_compile_definitions(stk PUBLIC STK_LIBC)
        
        if(MSVC)
            target_compile_options(stk PRIVATE /W4 /wd4013 /wd4047 /wd4996)
        else()
            target_compile_options(stk PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            target_link_libraries(stk PRIVATE m)
        endif()
        
        set_target_properties(stk PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            OUTPUT_NAME "stk"
        )
    endif()
    
    # ============== 安装 ==============
    install(TARGETS stk
        EXPORT StkTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/stk
    )
    
    # ============== 公共头文件 ==============
    set(PUBLIC_HEADERS
        ${STK_INCLUDE_DIR}/stk/vector.h
        ${STK_INCLUDE_DIR}/stk/string.h
        ${STK_INCLUDE_DIR}/stk/rbtree.h
    )
    
    install(FILES ${PUBLIC_HEADERS} DESTINATION include/stk)
    
    # ============== 内部头文件 ==============
    install(FILES ${C_INTERNAL_HEADERS} DESTINATION include/stk/internal)
    install(FILES ${VECTOR_C_HEADERS} DESTINATION include/stk/internal)
    install(FILES ${RBTREE_C_HEADERS} DESTINATION include/stk/internal)
    install(FILES ${STRING_C_HEADERS} DESTINATION include/stk/internal)
    install(FILES ${CPP_INTERNAL_HEADERS} DESTINATION include/stk/internal/cpp)
    
    # ============== 导出配置 ==============
    include(CMakePackageConfigHelpers)
    
    install(EXPORT StkTargets
        FILE StkTargets.cmake
        NAMESPACE stk::
        DESTINATION lib/cmake/stk
    )
    
    write_basic_package_version_file(
        ${CMAKE_BINARY_DIR}/StkConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    
    install(FILES ${CMAKE_BINARY_DIR}/StkConfigVersion.cmake
        DESTINATION lib/cmake/stk
    )
endif()

# ============== 编译信息 ==============
message(STATUS "")
message(STATUS "STK Library Configuration:")
message(STATUS "  Source dir:      ${STK_SRC_DIR}")
message(STATUS "  Include dir:     ${STK_INCLUDE_DIR}")
message(STATUS "  Modules:")
message(STATUS "    Vector:        ${BUILD_VECTOR}")
message(STATUS "    RBTree:        ${BUILD_RBTREE}")
message(STATUS "    String:        ${BUILD_STRING}")
message(STATUS "  Shared:          ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests:           ${BUILD_TESTS}")
message(STATUS "")


# ============== 测试子目录 ==============
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

