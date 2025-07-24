# ==============================================================================
# Letter Shell 中间件CMake构建配置文件
# ==============================================================================
# 描述: Letter Shell 组件的CMake构建配置
# 功能: 提供一个功能强大、可移植的嵌入式命令行解释器
# 作者: YLab Development Team (集成), Letter (原作者)
# 版本: 1.0
# 日期: 2025
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# Letter Shell 文件配置
# ------------------------------------------------------------------------------

# Shell头文件目录路径
set(SHELL_INC_DIR 
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Shell源文件列表
set(shell_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shell.c          # Shell核心实现
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shell_ext.c      # Shell扩展功能
)

# ------------------------------------------------------------------------------
# Shell接口库定义 - 提供头文件和编译定义
# ------------------------------------------------------------------------------

# 创建Shell接口库（仅包含头文件和编译定义）
add_library(shell_Interface INTERFACE)

# 设置Shell接口库的头文件包含路径
target_include_directories(shell_Interface 
    INTERFACE 
        ${SHELL_INC_DIR}    # Shell头文件目录
)

# ------------------------------------------------------------------------------
# Shell对象库定义 - 编译源文件生成目标文件
# ------------------------------------------------------------------------------

# 创建Shell对象库（编译源文件但不链接）
add_library(shell 
    OBJECT 
        ${shell_Src}        # Shell源文件列表
)

# 设置Shell对象库的头文件包含路径
target_include_directories(shell 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接接口库，使得Shell源文件可以找到其公共头文件
target_link_libraries(shell 
    PRIVATE
        shell_Interface
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        shell
)
