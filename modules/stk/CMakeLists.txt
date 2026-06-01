# ============================================================================
# STK 模块构建配置
# ============================================================================
# 功能：配置 STK（标准工具包）模块的构建选项和组件
# 
# 组件结构：
#   - core   : 核心数据结构（向量、链表、红黑树、字符串等）
#   - utils  : 工具组件（日志、其他实用功能）
#
# 可选功能（通过 CMake 选项控制）：
#   INFRA_STK_ENABLE_CORE           - 启用核心组件
#   INFRA_STK_ENABLE_UTILS          - 启用工具组件
#   INFRA_STK_CORE_ENABLE_VECTOR    - 启用动态数组
#   INFRA_STK_CORE_ENABLE_LIST      - 启用链表
#   INFRA_STK_CORE_ENABLE_RBTREE    - 启用红黑树
#   INFRA_STK_CORE_ENABLE_STRING    - 启用字符串
#   INFRA_STK_CORE_ENABLE_ARENA     - 启用竞技场分配器
#   INFRA_STK_CORE_ENABLE_BUFFER    - 启用缓冲区
#   INFRA_STK_CORE_ENABLE_HASHMAP   - 启用哈希映射
#   INFRA_STK_CORE_ENABLE_POOL      - 启用对象池
#   INFRA_STK_CORE_ENABLE_HEAP      - 启用堆
#   INFRA_STK_CORE_ENABLE_BITSET    - 启用位集
#   INFRA_BUILD_TESTS               - 构建测试
# ============================================================================

# ----------------------------------------------------------------------------
# 初始化模块
# ----------------------------------------------------------------------------
# 注册模块、设置目录变量、配置输出目录
infra_init_module(stk)

# ============================================================================
# 核心组件 (Core Component)
# ============================================================================
# 提供基础数据结构和内存管理功能
# ============================================================================
if(INFRA_STK_ENABLE_CORE)
    
    # 收集核心组件的源文件
    set(CORE_SOURCES "")

    # 动态数组 - 泛型向量实现
    if(INFRA_STK_CORE_ENABLE_VECTOR)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/vector.c
        )
    endif()
    
    # 双向链表 - 通用链表实现
    if(INFRA_STK_CORE_ENABLE_LIST)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/list.c
        )
    endif()
    
    # 红黑树 - 自平衡二叉搜索树
    if(INFRA_STK_CORE_ENABLE_RBTREE)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/rbtree.c
        )
    endif()
    
    # 字符串 - 动态字符串操作
    if(INFRA_STK_CORE_ENABLE_STRING)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/str.c
        )
    endif()
    
    # 竞技场分配器 - 线性内存分配器
    if(INFRA_STK_CORE_ENABLE_ARENA)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/arena.c
        )
    endif()
    
    # 缓冲区 - 自动增长的字节缓冲区
    if(INFRA_STK_CORE_ENABLE_BUFFER)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/buffer.c
        )
    endif()
    
    # 哈希映射 - 开放寻址哈希表
    if(INFRA_STK_CORE_ENABLE_HASHMAP)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/hashmap.c
        )
    endif()
    
    # 对象池 - 固定大小的对象分配器
    if(INFRA_STK_CORE_ENABLE_POOL)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/pool.c
        )
    endif()
    
    # 堆 - 二叉堆优先级队列
    if(INFRA_STK_CORE_ENABLE_HEAP)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/heap.c
        )
    endif()
    
    # 位集 - 固定大小位数组
    if(INFRA_STK_CORE_ENABLE_BITSET)
        list(APPEND CORE_SOURCES 
            ${INFRA_MODULE_STK_SRC_DIR}/core/bitset.c
        )
    endif()
    
    # 注册核心组件
    # 将编译生成的 .o 文件收集到模块对象列表中
    infra_register_component(stk core 
        SOURCES ${CORE_SOURCES}
        PRIVATE_DIRS ${INFRA_MODULE_STK_SRC_DIR}/core
    )
    
endif()

# ============================================================================
# 工具组件 (Utils Component)
# ============================================================================
# 提供日志、辅助函数等实用工具
# ============================================================================
if(INFRA_STK_ENABLE_UTILS)
    
    # 工具组件源文件列表
    set(UTIL_SOURCES
        ${INFRA_MODULE_STK_SRC_DIR}/utils/other.c      # 其他实用函数
        ${INFRA_MODULE_STK_SRC_DIR}/utils/logger.c     # 日志系统
        ${INFRA_MODULE_STK_SRC_DIR}/utils/status.c     # 状态
    )
    
    # 注册工具组件
    infra_register_component(stk utils 
        SOURCES ${UTIL_SOURCES}
        PRIVATE_DIRS ${INFRA_MODULE_STK_SRC_DIR}/utils
    )
    
endif()

# ----------------------------------------------------------------------------
# 完成模块组装
# ----------------------------------------------------------------------------
# 将所有组件对象链接成最终的库文件（共享库或静态库）
infra_finalize_module(stk)

# ----------------------------------------------------------------------------
# 测试子目录
# ----------------------------------------------------------------------------
# 如果需要构建测试，包含 tests 子目录
if(INFRA_BUILD_TESTS)
    add_subdirectory(tests)
endif()