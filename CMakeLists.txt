# ==============================================================================
# STM32G0 TempTest 项目 CMake 配置文件
# ==============================================================================
# 此文件定义了STM32G0项目的构建配置模板
# 使用STM32CubeMX生成的代码结构，支持HAL和LL库
# ==============================================================================

# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# 项目基本配置
# ------------------------------------------------------------------------------

# 设置编译器标准
set(CMAKE_C_STANDARD 11)                    # 使用C11标准
set(CMAKE_C_STANDARD_REQUIRED ON)           # 严格要求C11标准
set(CMAKE_C_EXTENSIONS ON)                  # 允许编译器扩展

# 定义构建类型（如果未指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")           # 默认为调试模式
endif()

# 包含ARM交叉编译工具链文件
# 此文件配置了arm-none-eabi-gcc等ARM工具链
include("gcc-arm-none-eabi.cmake")

# 启用编译命令导出，便于代码索引工具（如clangd）使用
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# ------------------------------------------------------------------------------
# 项目定义
# ------------------------------------------------------------------------------

# 核心项目设置
project(YLab_STM32G0_Template)
message("Build type: " ${CMAKE_BUILD_TYPE})

# 启用CMake对汇编和C语言的支持
# STM32项目需要编译启动文件（.s文件）
enable_language(C ASM)

# 创建可执行文件目标
# 这将生成最终的.elf文件
add_executable(${CMAKE_PROJECT_NAME})

# 添加STM32CubeMX生成的源码和配置
# 增加几个子目录
add_subdirectory(3-ySTM32F4Platform)
add_subdirectory(2-Midware)
add_subdirectory(1-app)

# 输出构建信息
message(STATUS "项目: ${PROJECT_NAME}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")

# 将map文件添加到清理目标中
# 执行'make clean'或'ninja clean'时会自动删除.map文件
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)
