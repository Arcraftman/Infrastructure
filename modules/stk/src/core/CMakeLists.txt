# stk/core/CMakeLists.txt
set(STK_INCLUDE_STK_CORE_DIR ${STK_INCLUDE_DIR}/stk/core)

set(DETAIL_DIR ${STK_SRC_CORE_DIR}/detail)
set(CXX_DIR ${STK_SRC_CORE_DIR}/cpp)

set(PUBLIC_HEADER
    ${STK_INCLUDE_DIR}/stk/core.h
)

set(CONFIG_HEADER
    ${STK_SRC_CORE_DIR}/config.h
)

set(VECTOR_HEADER
    ${STK_SRC_CORE_DIR}/vector.h
)

set(RBTREE_HEADER
    ${STK_SRC_CORE_DIR}/rbtree.h
)

set(STRING_HEADER
    ${STK_SRC_CORE_DIR}/str.h
)

set(CORE_HEADERS
    ${VECTOR_HEADER}
    ${RBTREE_HEADER}
    ${STRING_HEADER}
)

set(DETAIL_HEADERS
    ${DETAIL_DIR}/rbtree.h
    ${DETAIL_DIR}/vector.h
)

set(CPP_HEADERS
    ${CXX_DIR}/vector.hpp
    ${CXX_DIR}/vector.tcc
    ${CXX_DIR}/rbtree.hpp
    ${CXX_DIR}/rbtree.tcc
    ${CXX_DIR}/string.hpp
    ${CXX_DIR}/string.tcc
)

set(C_HEADERS
    ${DETAIL_HEADERS}
    ${CORE_HEADERS}
)

# 复制所有头文件
file(
    COPY
        ${CORE_HEADERS}
        ${DETAIL_HEADERS}
    DESTINATION ${STK_INCLUDE_STK_CORE_DIR}
)

file(
    COPY ${CPP_HEADERS}
    DESTINATION ${STK_INCLUDE_STK_CORE_DIR}/cpp
)

option(BUILD_SHARED_LIBS "Build shared library" ON)

option(BUILD_CLIB "Build C version" ON)
option(BUILD_CPPLIB "Build C++ wrapper" ON)

option(BUILD_VECTOR "Build Vector module" ON)
option(BUILD_RBTREE "Build RBTree module" ON)
option(BUILD_STRING "Build String module" ON)

# ============================================================================
# Compiler Settings
# ============================================================================
if(MSVC)
    set(STK_COMPILE_OPTIONS /W4 /wd4013 /wd4047 /wd4996)
    set(STK_COMPILE_DEFS _CRT_SECURE_NO_WARNINGS)
else()
    set(STK_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
    set(STK_COMPILE_DEFS)

    if(BUILD_SHARED_LIBS)
        set(STK_COMPILE_OPTIONS ${STK_COMPILE_OPTIONS} -fPIC)
    endif()
endif()

# ============================================================================
# Export Definitions
# ============================================================================
if(BUILD_SHARED_LIBS)
    set(STK_PUBLIC_DEFS STK_LIBC STK_DLL)
    set(STK_PRIVATE_DEFS STK_EXPORTING)
else()
    set(STK_PUBLIC_DEFS STK_LIBC)
    set(STK_PRIVATE_DEFS)
endif()

# ----------------------------------------------------------------------------
# 收集所有模块源文件
# ----------------------------------------------------------------------------
set(CORE_OBJECTS "")

# ----------------------------------------------------------------------------
# Module: Vector
# ----------------------------------------------------------------------------
if(BUILD_VECTOR)
    add_library(vector OBJECT
        ${STK_SRC_CORE_DIR}/vector.c
    )

    target_include_directories(vector
        PUBLIC
            $<BUILD_INTERFACE:${STK_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${STK_SRC_CORE_DIR}
            ${DETAIL_DIR}
            ${STK_INTERNAL_DIR}
    )

    target_compile_definitions(vector
        PUBLIC ${STK_PUBLIC_DEFS}
        PRIVATE ${STK_PRIVATE_DEFS}
    )

    target_compile_options(vector
        PRIVATE ${STK_COMPILE_OPTIONS}
    )

    list(APPEND CORE_OBJECTS $<TARGET_OBJECTS:vector>)

    message(STATUS "  Vector module:   ENABLED")
else()
    message(STATUS "  Vector module:   DISABLED")
endif()

# ----------------------------------------------------------------------------
# Module: RBTree
# ----------------------------------------------------------------------------
if(BUILD_RBTREE)
    add_library(rbtree OBJECT
        ${STK_SRC_CORE_DIR}/rbtree.c
    )

    target_include_directories(rbtree
        PUBLIC
            $<BUILD_INTERFACE:${STK_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${STK_SRC_CORE_DIR}
            ${DETAIL_DIR}
            ${STK_INTERNAL_DIR}
    )

    target_compile_definitions(rbtree
        PUBLIC ${STK_PUBLIC_DEFS}
        PRIVATE ${STK_PRIVATE_DEFS}
    )

    target_compile_options(rbtree
        PRIVATE ${STK_COMPILE_OPTIONS}
    )

    list(APPEND CORE_OBJECTS $<TARGET_OBJECTS:rbtree>)

    message(STATUS "  RBTree module:   ENABLED")
else()
    message(STATUS "  RBTree module:   DISABLED")
endif()

# ----------------------------------------------------------------------------
# Module: String
# ----------------------------------------------------------------------------
if(BUILD_STRING)
    add_library(str OBJECT
        ${STK_SRC_CORE_DIR}/str.c
    )

    target_include_directories(str
        PUBLIC
            $<BUILD_INTERFACE:${STK_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${STK_SRC_CORE_DIR}
            ${DETAIL_DIR}
            ${STK_INTERNAL_DIR}
    )

    target_compile_definitions(str
        PUBLIC ${STK_PUBLIC_DEFS}
        PRIVATE ${STK_PRIVATE_DEFS}
    )

    target_compile_options(str
        PRIVATE ${STK_COMPILE_OPTIONS}
    )

    list(APPEND CORE_OBJECTS $<TARGET_OBJECTS:str>)

    message(STATUS "  String module:   ENABLED")
else()
    message(STATUS "  String module:   DISABLED")
endif()

# ----------------------------------------------------------------------------
# Module Configuration Summary
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "  STK Core Module Configuration")
message(STATUS "  ============================")
message(STATUS "  Shared Lib:   ${BUILD_SHARED_LIBS}")
message(STATUS "  C Library:    ${BUILD_CLIB}")
message(STATUS "  C++ Wrapper:  ${BUILD_CPPLIB}")
message(STATUS "  Vector:       ${BUILD_VECTOR}")
message(STATUS "  RBTree:       ${BUILD_RBTREE}")
message(STATUS "  String:       ${BUILD_STRING}")
message(STATUS "")

# ----------------------------------------------------------------------------
# Main Library
# ----------------------------------------------------------------------------
if(BUILD_CLIB)
    if(CORE_OBJECTS)
        if(BUILD_SHARED_LIBS)
            message(STATUS "Building STK Shared Library...")
            add_library(stk SHARED ${CORE_OBJECTS})
        else()
            message(STATUS "Building STK Static Library...")
            add_library(stk STATIC ${CORE_OBJECTS})
        endif()

        add_library(stk::stk ALIAS stk)

        target_include_directories(stk
            PUBLIC
                $<BUILD_INTERFACE:${STK_INCLUDE_DIR}>
                $<INSTALL_INTERFACE:include/stk>
            PRIVATE
                ${STK_SRC_CORE_DIR}
                ${DETAIL_DIR}
                ${STK_INTERNAL_DIR}
        )

        target_compile_definitions(stk
            PUBLIC ${STK_PUBLIC_DEFS}
            PRIVATE ${STK_PRIVATE_DEFS}
        )

        target_compile_options(stk
            PRIVATE ${STK_COMPILE_OPTIONS}
        )

        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            target_link_libraries(stk PRIVATE m)
        endif()

        set_target_properties(stk
            PROPERTIES
                VERSION ${PROJECT_VERSION}
                SOVERSION ${PROJECT_VERSION_MAJOR}
                OUTPUT_NAME "stk"
        )

        if(MSVC AND BUILD_SHARED_LIBS)
            set_target_properties(stk
                PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON
            )
        endif()
    else()
        message(WARNING "No core modules enabled! STK library will not be built.")
    endif()
endif()

