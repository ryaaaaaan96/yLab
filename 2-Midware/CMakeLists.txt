# ==============================================================================
# 中间件 CMake 配置文件
# ==============================================================================
# 此文件定义了第三方中间件和库的配置
# 如FreeRTOS、TCP/IP栈、文件系统等
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# yDev设备抽象层
# ------------------------------------------------------------------------------
add_subdirectory(yDev)

add_subdirectory(freeRTOS)

# ------------------------------------------------------------------------------
# 中间件库目标（示例）
# ------------------------------------------------------------------------------

# 创建中间件接口库（当前为空，可根据需要添加）
add_library(middleware INTERFACE)

# 中间件包含路径（根据需要添加）
target_include_directories(middleware INTERFACE
    # ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# 中间件编译定义（根据需要添加）
target_compile_definitions(middleware INTERFACE
    # MIDDLEWARE_ENABLED=1
)

# 链接yDev设备抽象层
target_link_libraries(middleware 
    INTERFACE
        yDev_Interface
        freeRTOS_Interface
)
