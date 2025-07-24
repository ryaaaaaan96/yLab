# ==============================================================================
# yDev设备抽象层CMake构建配置文件
# ==============================================================================
# 描述: yDev设备抽象层的CMake构建配置
# 功能: 提供统一的设备操作接口，类似Linux设备驱动模型
# 作者: YLab Development Team  
# 版本: 1.0
# 日期: 2025
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# yDev设备抽象层文件配置
# ------------------------------------------------------------------------------

# yDev头文件目录路径
set(YDEV_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# yDev源文件列表
set(yDev_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDev.c          # yDev核心实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDev_gpio.c     # GPIO设备抽象层
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDev_usart.c    # USART设备抽象层
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/yDev_dma.c      # DMA设备抽象层
)

# ------------------------------------------------------------------------------
# yDev接口库定义 - 提供头文件和编译定义
# ------------------------------------------------------------------------------

# 创建yDev接口库（仅包含头文件和编译定义）
add_library(yDev_Interface INTERFACE)

# 设置yDev接口库的头文件包含路径
target_include_directories(yDev_Interface 
    INTERFACE 
        ${YDEV_INC_DIR}    # yDev头文件目录
)

# 设置yDev接口库的依赖关系
target_link_libraries(yDev_Interface 
    INTERFACE 
        yPlatform          # 依赖yPlatform库
)

# ------------------------------------------------------------------------------
# yDev对象库定义 - 编译源文件生成目标文件
# ------------------------------------------------------------------------------

# 创建yDev对象库（编译源文件但不链接）
add_library(yDev 
    OBJECT 
        ${yDev_Src}        # yDev源文件列表
)

# 设置yDev对象库的头文件包含路径
target_include_directories(yDev 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接接口库
target_link_libraries(yDev 
    PRIVATE
        yDev_Interface
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        yDev
)
