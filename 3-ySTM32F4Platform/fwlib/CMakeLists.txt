# ==============================================================================
# STM32F4 HAL库的编译文件
# ==============================================================================
# STM32F4系列微控制器HAL/LL驱动库的CMake构建配置
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# STM32F4 HAL库的头文件包含路径
# ------------------------------------------------------------------------------

# 包含了项目所需的所有头文件目录
set(Hal_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}                         # STM32F4 HAL驱动头文件
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc                     # STM32F4 HAL驱动头文件
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/Legacy              # HAL驱动兼容性头文件
)

# ------------------------------------------------------------------------------
# STM32F4 HAL/LL 驱动源文件列表
# ------------------------------------------------------------------------------

# STM32F4 HAL/LL驱动源文件
# 根据yPlatform配置自动选择需要的驱动模块
set(Hal_Src
    # 定时器相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_tim.c       # 定时器HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_tim_ex.c    # 定时器扩展HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_tim.c        # 定时器LL驱动

    # GPIO和外部中断相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_gpio.c       # GPIO LL驱动（低层驱动）
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_exti.c       # 外部中断LL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_gpio.c      # GPIO HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_exti.c      # 外部中断HAL驱动

    # 串口通信相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_usart.c      # USART LL驱动（低层驱动）
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_usart.c       # UART LL驱动（低层驱动）
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_uart.c      # UART HAL驱动
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_usart.c     # USART HAL驱动

    # SPI通讯相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_spi.c        # SPI LL驱动（低层驱动）
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_spi.c       # SPI HAL驱动

    # 时钟控制相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_rcc.c        # 时钟控制LL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_utils.c      # LL工具函数
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_rcc.c       # 时钟控制HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_rcc_ex.c    # 时钟控制扩展HAL驱动

    # Flash存储器相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_flash.c     # Flash HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_flash_ex.c  # Flash扩展HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_flash_ramfunc.c # Flash RAM函数

    # DMA相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_dma.c       # DMA HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_dma_ex.c    # DMA扩展HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_dma.c        # DMA LL驱动

    # 电源管理相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_pwr.c       # 电源管理HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_pwr_ex.c    # 电源管理扩展HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_pwr.c        # 电源管理LL驱动

    # 内核相关驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_cortex.c    # Cortex-M4内核HAL驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal.c           # HAL核心驱动（初始化等）

    # STM32F4特有的驱动
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_ll_fmc.c        # FMC LL驱动（外部存储器控制器）
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_fmc.c       # FMC HAL驱动
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_sdram.c     # SDRAM HAL驱动
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_nor.c       # NOR Flash HAL驱动
    # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_sram.c      # SRAM HAL驱动

)

# ------------------------------------------------------------------------------
# 静态库目标定义和配置
# ------------------------------------------------------------------------------

# 创建HAL静态库
add_library(Fwlib_Interface INTERFACE)

# 设置头文件包含路径
target_include_directories(Fwlib_Interface 
    INTERFACE 
        ${Hal_Include_Dirs}
)

# 设置编译定义
target_compile_definitions(Fwlib_Interface 
    INTERFACE
        STM32F407xx                # STM32F407芯片定义
        USE_HAL_DRIVER             # 启用HAL库
        USE_FULL_LL_DRIVER         # 启用完整LL库
)

# 设置编译定义
target_link_libraries(Fwlib_Interface 
    INTERFACE
        Core_Interface
)


# 创建HAL静态库
add_library(Fwlib 
    STATIC
        ${Hal_Src}
)

# 设置编译定义
target_compile_definitions(Fwlib 
    PRIVATE
        STM32F407xx                # STM32F407芯片定义
        USE_HAL_DRIVER             # 启用HAL库
        USE_FULL_LL_DRIVER         # 启用完整LL库
)

# 设置编译定义
target_link_libraries(Fwlib 
    PRIVATE
        Fwlib_Interface
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        Fwlib
)


