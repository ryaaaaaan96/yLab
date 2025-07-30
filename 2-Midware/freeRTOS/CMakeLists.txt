# ==============================================================================
# FreeRTOS实时操作系统 CMake 配置文件
# ==============================================================================
# 此文件定义了FreeRTOS实时操作系统的编译配置
# 提供任务调度、同步原语、内存管理等RTOS功能
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# FreeRTOS源文件定义
# ------------------------------------------------------------------------------

# 头文件路径
set(FREERTOS_INC_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# FreeRTOS核心源文件
set(FREERTOS_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/croutine.c
)

# FreeRTOS移植层源文件
set(FREERTOS_PORTABLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/portasm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/heap_4.c
)

# FreeRTOS MPU相关源文件（如果需要）
set(FREERTOS_MPU_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mpu_wrappers_v2_asm.c
)

# 合并所有源文件
set(freeRTOS_Src
    ${FREERTOS_CORE_SOURCES}
    ${FREERTOS_PORTABLE_SOURCES}
    # 如果项目需要MPU支持，取消下面的注释
    # ${FREERTOS_MPU_SOURCES}
)



# ------------------------------------------------------------------------------
# FreeRTOS INTERFACE 库 - 接口库，用于头文件和编译定义
# ------------------------------------------------------------------------------

add_library(freeRTOS_Interface INTERFACE)

target_include_directories(freeRTOS_Interface 
    INTERFACE 
        ${FREERTOS_INC_DIR}
)

# ------------------------------------------------------------------------------
# FreeRTOS STATIC 库 - 静态库，用于编译源文件
# ------------------------------------------------------------------------------

# 创建 FreeRTOS 静态库
add_library(freeRTOS 
    STATIC 
        ${freeRTOS_Src}
)

# 链接接口库
target_link_libraries(freeRTOS 
    PUBLIC
        freeRTOS_Interface
)

# 链接到主项目
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        freeRTOS
)


