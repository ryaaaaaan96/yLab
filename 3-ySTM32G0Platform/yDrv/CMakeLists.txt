# ==============================================================================
# yDrv驱动层CMake构建配置文件
# ==============================================================================
# 描述: STM32G0系列yDrv驱动层的CMake构建配置
# 作者: YLab Development Team  
# 版本: 2.0
# 日期: 2025
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# yDrv驱动层头文件包含路径配置
# ------------------------------------------------------------------------------

# yDrv驱动层头文件目录
set(yDrv_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/inc    # yDrv驱动层公共头文件目录
)

# ------------------------------------------------------------------------------
# yDrv驱动层源文件配置
# ------------------------------------------------------------------------------

# yDrv驱动层源文件列表
set(yDrv_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv_basic.c        # 基础功能实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv_gpio.c         # GPIO驱动实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv.c              # 核心初始化实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv_usart.c        # USART驱动实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv_dma.c          # DMA驱动实现
)

# ------------------------------------------------------------------------------
# 库目标定义和配置
# ------------------------------------------------------------------------------

# stm32hal
add_library(yDrv_Interface 
    INTERFACE 
)    # INTERFACE库不包含源文件，仅提供包含路径和编译定义给其他目标使用
target_include_directories(yDrv_Interface 
    INTERFACE 
        ${yDrv_Include_Dirs}
)    # 设置头文件包含路径
target_link_libraries(yDrv_Interface 
    INTERFACE 
        Fwlib_Interface         
        Core_Interface
        yLib_Interface
)         # 继承yPlatform的包含路径和宏定义

# stm32hal
add_library(yDrv
    STATIC
        ${yDrv_Src} 
)    # STATIC库包含源文件，编译成静态库

target_link_libraries(yDrv 
    PRIVATE 
        yDrv_Interface
        Fwlib                    # 添加这行：链接到 Fwlib 静态库
)         # 继承yPlatform的包含路径和宏定义

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        yDrv
)