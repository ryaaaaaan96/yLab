# ==============================================================================
# core的编译的文件
# ==============================================================================
# 暂无说明
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# core 驱动源文件列表
# ------------------------------------------------------------------------------

# 添加core的驱动源文件
set(Core_Src
    # 系统核心文件
    ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32f4xx.c              # 系统初始化和时钟配置
    ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32f407xx.s         # 启动汇编文件（复位向量表等）
)

# ------------------------------------------------------------------------------
# 静态库目标定义和配置
# ------------------------------------------------------------------------------

# 创建Core静态库（包含实际源文件）
add_library(Core_Interface INTERFACE)

# 设置头文件包含路径
target_include_directories(Core_Interface 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS
)

# 设置编译定义
target_compile_definitions(Core_Interface 
    INTERFACE
        STM32F407xx                 # 这个宏告诉编译器使用F407芯片
        USE_HAL_DRIVER              # 启用HAL库
        USE_FULL_LL_DRIVER          # 启用完整LL库
)

# ------------------------------------------------------------------------------
# 静态库目标定义和配置
# ------------------------------------------------------------------------------

# 创建Core静态库（包含实际源文件）
add_library(Core 
    STATIC 
        ${Core_Src}
)

# 设置编译定义
target_compile_definitions(Core 
    PRIVATE
        STM32F407xx                 # 这个宏告诉编译器使用F407芯片
        USE_HAL_DRIVER              # 启用HAL库
        USE_FULL_LL_DRIVER          # 启用完整LL库
)

# 链接接口库
target_link_libraries(Core 
    PRIVATE
        Core_Interface
        Fwlib_Interface
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        Core
)

