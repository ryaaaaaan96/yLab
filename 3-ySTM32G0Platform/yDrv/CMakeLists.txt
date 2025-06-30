# ==============================================================================
# yDrv的编译的文件
# ==============================================================================
# 暂无说明
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# STM32的HAL库的 头文件包含路径
# ------------------------------------------------------------------------------

# 包含了项目所需的所有头文件目录
set(yDrv_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/inc                     # 
)

# 包含了项目所需的源文件目录
set(yDrv_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv_basic.c        
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv_gpio.c        
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yDrv.c        
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