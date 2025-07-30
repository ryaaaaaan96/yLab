# ==============================================================================
# yLib库的编译配置文件
# ==============================================================================
# yLib是一个通用的C语言库，提供常用的数据结构和算法
# 包含FIFO、堆、链表、内存池、红黑树、环形缓冲区等
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# yLib库的头文件包含路径
# ------------------------------------------------------------------------------

# 包含了项目所需的所有头文件目录
set(yLib_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/inc                     # yLib头文件目录
    ${CMAKE_CURRENT_SOURCE_DIR}                         # 包含yLib_config.h
)

# 包含了项目所需的源文件目录
set(yLib_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yLib_fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yLib_heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yLib_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yLib_mempool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yLib_rbtree.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yLib_ring.c
)

# ------------------------------------------------------------------------------
# 库目标定义和配置
# ------------------------------------------------------------------------------
# 创建yLib静态库
add_library(yLib_Interface
    INTERFACE 
)    # 创建静态库，包含所有yLib源文件

target_include_directories(yLib_Interface 
    INTERFACE 
        ${yLib_Include_Dirs}
)    # 设置头文件包含路径为PUBLIC，其他链接此库的目标可以访问

# 创建yLib静态库
add_library(yLib 
    STATIC 
        ${yLib_Src}
)    # 创建静态库，包含所有yLib源文件

target_link_libraries(yLib
    PRIVATE
        yLib_Interface
)


target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        yLib
)


