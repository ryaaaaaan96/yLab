# ==============================================================================
# yDev设备抽象层 CMake 配置文件
# ==============================================================================
# 此文件定义了yDev设备抽象层的编译配置
# 提供统一的设备操作接口，类似Linux设备驱动模型
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# yDev源文件定义
# ------------------------------------------------------------------------------

# 头文件路径
set(YDEV_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# 源文件列表
set(yDev_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDev.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDev_gpio.c
)

# ------------------------------------------------------------------------------
# yDev INTERFACE 库 - 接口库，用于头文件和编译定义
# ------------------------------------------------------------------------------

add_library(yDev_Interface INTERFACE)

target_include_directories(yDev_Interface 
    INTERFACE 
        ${YDEV_INC_DIR}
)

target_link_libraries(yDev_Interface 
    INTERFACE 
        yPlatform
)

# ------------------------------------------------------------------------------
# yDev OBJECT 库 - 对象库，用于编译源文件
# ------------------------------------------------------------------------------

# 创建 yDev 目标文件库
add_library(yDev OBJECT ${yDev_Src})

# 链接接口库
target_link_libraries(yDev 
    PRIVATE
        yDev_Interface
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        yDev
)
