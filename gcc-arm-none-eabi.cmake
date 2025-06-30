# ============================================================================
# 系统平台配置
# ============================================================================
set(CMAKE_SYSTEM_NAME               Generic)        # 设置目标系统为通用嵌入式系统
set(CMAKE_SYSTEM_PROCESSOR          arm)            # 设置处理器架构为ARM

# ============================================================================
# 编译器标识配置
# ============================================================================
set(CMAKE_C_COMPILER_ID GNU)                        # 指定C编译器为GNU GCC
set(CMAKE_CXX_COMPILER_ID GNU)                      # 指定C++编译器为GNU GCC

# ============================================================================
# 工具链前缀配置
# ============================================================================
# 注意：arm-none-eabi- 工具链必须在系统PATH环境变量中
set(TOOLCHAIN_PREFIX                arm-none-eabi-)

# ============================================================================
# 编译器和工具设置
# ============================================================================
set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)      # C编译器
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})        # 汇编编译器（使用GCC）
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)     # C++编译器
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}g++)     # 链接器
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy) # 目标文件转换工具
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)    # 文件大小分析工具

# ============================================================================
# 可执行文件后缀配置
# ============================================================================
set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")         # 汇编生成的可执行文件后缀
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")         # C代码生成的可执行文件后缀
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")         # C++代码生成的可执行文件后缀

# ============================================================================
# 编译测试配置
# ============================================================================
# 设置为静态库模式，避免交叉编译时的链接测试问题
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ============================================================================
# MCU特定编译标志
# ============================================================================
# Cortex-M0+ 处理器特定参数
set(TARGET_FLAGS "-mcpu=cortex-m0plus ")

# ============================================================================
# C语言编译标志配置
# ============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")                    # 应用MCU标志
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")   # 汇编标志：
                                                                          # -x assembler-with-cpp: 启用C预处理器
                                                                          # -MMD -MP: 生成依赖文件
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")
# -Wall -Wextra -Wpedantic: 启用详细警告
# -fdata-sections -ffunction-sections: 将函数和数据放入单独的段，便于链接器优化

# ============================================================================
# 调试和发布模式编译标志
# ============================================================================
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")          # 调试模式：无优化，最详细调试信息
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")        # 发布模式：大小优化，无调试信息
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")        # C++ 调试模式
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")      # C++ 发布模式

# ============================================================================
# C++特定编译标志
# ============================================================================
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")
# -fno-rtti: 禁用运行时类型信息（减少代码大小）
# -fno-exceptions: 禁用异常处理（嵌入式系统常用）
# -fno-threadsafe-statics: 禁用线程安全的静态变量初始化

# ============================================================================
# 链接标志配置
# ============================================================================
set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")                                           # 应用MCU标志
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32G070XX_FLASH.ld\"") # 指定链接脚本
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --specs=nano.specs")                 # 使用newlib-nano（减小库大小）
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections") 
# -Wl,-Map=xxx.map: 生成内存映射文件
# -Wl,--gc-sections: 移除未使用的段

set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group")
# --start-group/--end-group: 解决库之间的循环依赖
# -lc: 链接C标准库
# -lm: 链接数学库

set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")           # 打印内存使用情况

# ============================================================================
# C++链接标志配置
# ============================================================================
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lstdc++ -lsupc++ -Wl,--end-group")
# -lstdc++: 链接C++标准库
# -lsupc++: 链接C++支持库（异常处理、RTTI等，即使禁用也需要基础支持）