# ==============================================================================
# 应用程序 CMake 配置文件
# ==============================================================================
# 此文件定义了主应用程序的源文件和配置
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# 应用程序源文件
# ------------------------------------------------------------------------------

# 应用程序源文件列表
set(APP_DEVICE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/communication.c     # 主程序文件
    ${CMAKE_CURRENT_SOURCE_DIR}/src/switch.c     # 主程序文件

)

# ------------------------------------------------------------------------------
# 应用程序头文件路径
# ------------------------------------------------------------------------------

# 应用程序包含路径
set(APP_DEVICE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)

# ------------------------------------------------------------------------------
# 将源文件添加到主项目
# ------------------------------------------------------------------------------

# 将应用程序源文件添加到主可执行文件
add_library(APP_Device_Interface
    INTERFACE
)

# 将应用程序包含路径添加到主项目
target_include_directories(APP_Device_Interface
    INTERFACE
        ${APP_DEVICE_INCLUDE_DIRS}
)
target_link_libraries(APP_Device_Interface
    INTERFACE
        middleware                 # STM32平台接口库（包含编译选项和宏定义）
)


# 将应用程序源文件添加到主可执行文件
add_library(APP_Device
    STATIC
        ${APP_DEVICE_SOURCES}
)

target_link_libraries(APP_Device
    PRIVATE
        APP_Device_Interface                 # STM32平台接口库（包含编译选项和宏定义）
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    PRIVATE
        APP_Device                 # STM32平台接口库（包含编译选项和宏定义）
)

