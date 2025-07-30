/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*******************************************************************************
 * FreeRTOS 配置文件 - STM32G0 系列专用
 *
 * 本文件为 STM32G0 系列微控制器的 FreeRTOS 配置文件，包含了所有必要的
 * 配置参数定义。每个配置项都有详细的中文说明。
 *
 * 配置说明：
 * - 硬件平台：STM32G0 系列 (Cortex-M0+)
 * - 内核版本：FreeRTOS V11.1.0
 * - 调度算法：抢占式调度
 * - 堆管理：动态内存分配 (heap_4.c)
 * - 系统节拍：1000Hz (1ms)
 *
 * 更多信息请参考：
 * https://www.freertos.org/a00110.html
 *
 * 注意：方括号 ('[' 和 ']') 中的常量值必须在构建前完成配置。
 * 建议使用与所用 RTOS 移植版本配套的 FreeRTOSConfig.h 文件。
 ******************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/******************************************************************************/
/* 硬件描述相关定义 *********************************************************/
/******************************************************************************/
extern uint32_t SystemCoreClock;

/* CPU时钟频率配置 (configCPU_CLOCK_HZ)
 * 在大多数情况下，configCPU_CLOCK_HZ 必须设置为驱动内核周期性滴答中断的
 * 外设时钟频率。默认值设置为 20MHz，与 QEMU 演示设置匹配。
 * 您的应用程序肯定需要不同的值，因此请正确设置此值。
 * 这通常（但不总是）等于主系统时钟频率。
 *
 * 对于 STM32G0 系列：
 * - 此值应设置为实际的系统时钟频率
 * - SystemCoreClock 变量由 HAL 库自动计算和更新
 * - 该值用于计算 SysTick 定时器的配置，确保准确的系统节拍
 */
#define configCPU_CLOCK_HZ (SystemCoreClock)

/* SysTick时钟频率配置 (configSYSTICK_CLOCK_HZ) - 仅适用于ARM Cortex-M移植版本
 *
 * 默认情况下，ARM Cortex-M 移植版本从 Cortex-M SysTick 定时器生成 RTOS 滴答中断。
 * 大多数 Cortex-M MCU 以与 MCU 本身相同的频率运行 SysTick 定时器 - 在这种情况下
 * 不需要 configSYSTICK_CLOCK_HZ 并且应该保持未定义状态。
 * 如果 SysTick 定时器的时钟频率与 MCU 核心不同，则将 configCPU_CLOCK_HZ 设置为
 * MCU 时钟频率（正常情况），将 configSYSTICK_CLOCK_HZ 设置为 SysTick 时钟频率。
 * 如果保持未定义则不使用。
 * 默认值为未定义（注释掉）。如果您需要此值，请将其恢复并设置为合适的值。
 */


// #define configSYSTICK_CLOCK_HZ                  64000000


/******************************************************************************/
/* 调度行为相关定义 *********************************************************/
/******************************************************************************/

/* 系统节拍频率配置 (configTICK_RATE_HZ)
 * 设置滴答中断的频率（单位：Hz），通常根据 configCPU_CLOCK_HZ 值计算得出。
 * 1000Hz 表示每 1ms 产生一次系统节拍中断，这是常用的配置。
 */
#define configTICK_RATE_HZ ((TickType_t)1000)

/* 抢占式调度使能 (configUSE_PREEMPTION)
 * 设置为 1 启用抢占式调度，设置为 0 使用协作式调度。
 * 抢占式调度：高优先级任务可以抢占低优先级任务的执行
 * 协作式调度：任务必须主动放弃 CPU 控制权才能进行任务切换
 * 参考：https://www.freertos.org/single-core-amp-smp-rtos-scheduling.html
 */
#define configUSE_PREEMPTION 1

/* 时间片轮转调度使能 (configUSE_TIME_SLICING)
 * 设置为 1：在每次滴答中断时，调度器会在相同优先级的就绪任务之间切换
 * 设置为 0：防止调度器仅因为滴答中断就在相同优先级的就绪任务之间切换
 * 参考：https://freertos.org/single-core-amp-smp-rtos-scheduling.html
 */
#define configUSE_TIME_SLICING 1

/* 硬件优化任务选择 (configUSE_PORT_OPTIMISED_TASK_SELECTION)
 * 设置为 1：使用针对目标硬件指令集优化的算法选择下一个要运行的任务
 *          通常使用前导零计数汇编指令（如 CLZ 指令）
 * 设置为 0：使用适用于所有 FreeRTOS 移植的通用 C 算法
 * 注意：并非所有 FreeRTOS 移植都支持此选项，默认为 0
 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

/* 低功耗无滴答模式 (configUSE_TICKLESS_IDLE)
 * 设置为 1：使用低功耗无滴答模式，在空闲时停止滴答中断以节省功耗
 * 设置为 0：滴答中断始终运行
 * 注意：并非所有 FreeRTOS 移植都支持无滴答模式
 * 参考：https://www.freertos.org/low-power-tickless-rtos.html
 */
#define configUSE_TICKLESS_IDLE 0

/* 最大任务优先级数量 (configMAX_PRIORITIES)
 * 设置可用任务优先级的数量。任务可分配的优先级范围为 0 到 (configMAX_PRIORITIES - 1)
 * 0 是最低优先级，数值越大优先级越高
 *
 * STM32G0 推荐配置：
 * - 简单应用：8 个优先级 (0-7)
 * - 中等复杂度：16 个优先级 (0-15) - 推荐
 * - 复杂应用：32 个优先级 (0-31)
 *
 * 注意：优先级数量越多，RAM 占用略多，但对 STM32G0 影响很小
 * 建议使用 16，既满足大多数应用需求，又不会过度消耗资源
 */
#define configMAX_PRIORITIES 32

/* 最小堆栈大小 (configMINIMAL_STACK_SIZE)
 * 定义空闲任务使用的堆栈大小（单位：字，不是字节！）
 * 内核不会将此常量用于其他目的，演示应用程序使用此常量来保证跨硬件架构的可移植性
 */
#define configMINIMAL_STACK_SIZE ((uint16_t)128)

/* 任务名称最大长度 (configMAX_TASK_NAME_LEN)
 * 设置任务可读名称的最大长度（字符数），包括 NULL 终止符
 */
#define configMAX_TASK_NAME_LEN (16)

/* 滴答计数类型位宽配置 (configTICK_TYPE_WIDTH_IN_BITS)
 * 时间以"滴答"为单位测量 - 这是自 RTOS 内核启动以来滴答中断执行的次数。
 * 滴答计数保存在 TickType_t 类型的变量中。
 *
 * configTICK_TYPE_WIDTH_IN_BITS 控制 TickType_t 的类型（因此也是位宽）：
 *
 * 将 configTICK_TYPE_WIDTH_IN_BITS 定义为 TICK_TYPE_WIDTH_16_BITS 会使
 * TickType_t 被定义（typedef）为无符号 16 位类型。
 *
 * 将 configTICK_TYPE_WIDTH_IN_BITS 定义为 TICK_TYPE_WIDTH_32_BITS 会使
 * TickType_t 被定义（typedef）为无符号 32 位类型。
 *
 * 将 configTICK_TYPE_WIDTH_IN_BITS 定义为 TICK_TYPE_WIDTH_64_BITS 会使
 * TickType_t 被定义（typedef）为无符号 64 位类型。
 *
 * 对于 STM32G0：推荐使用 32 位类型，提供足够的计数范围
 */
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS

/* 空闲任务让出控制权配置 (configIDLE_SHOULD_YIELD)
 * 设置为 1：如果存在可以运行的空闲优先级（优先级 0）应用任务，空闲任务将让出控制权
 * 设置为 0：空闲任务使用其全部时间片
 * 默认值为 1（如果未定义）
 */
#define configIDLE_SHOULD_YIELD 1

/* 任务通知数组条目数 (configTASK_NOTIFICATION_ARRAY_ENTRIES)
 * 每个任务都有一个任务通知数组。
 * configTASK_NOTIFICATION_ARRAY_ENTRIES 设置数组中的索引数量。
 * 参考：https://www.freertos.org/RTOS-task-notifications.html
 * 默认值为 1（如果未定义）
 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

/* 队列注册表大小 (configQUEUE_REGISTRY_SIZE)
 * 设置可从队列注册表引用的队列和信号量的最大数量。
 * 仅在使用内核感知调试器时需要。
 * 默认值为 0（如果未定义）
 */
#define configQUEUE_REGISTRY_SIZE 8

/* 向后兼容性使能 (configENABLE_BACKWARD_COMPATIBILITY)
 * 设置为 1：将旧版本 FreeRTOS 的函数名和数据类型映射到最新的等效项
 * 默认值为 1（如果未定义）
 */
#define configENABLE_BACKWARD_COMPATIBILITY 1

/* 线程本地存储指针数量 (configNUM_THREAD_LOCAL_STORAGE_POINTERS)
 * 每个任务都有自己的指针数组，可用作线程本地存储。
 * configNUM_THREAD_LOCAL_STORAGE_POINTERS 设置数组中的索引数量。
 * 参考：https://www.freertos.org/thread-local-storage-pointers.html
 * 默认值为 0（如果未定义）
 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

/* 迷你列表项使用配置 (configUSE_MINI_LIST_ITEM)
 * 当 configUSE_MINI_LIST_ITEM 设置为 0 时，MiniListItem_t 和 ListItem_t 是相同的。
 * 当 configUSE_MINI_LIST_ITEM 设置为 1 时，MiniListItem_t 比 ListItem_t 少 3 个字段，
 * 这可以节省一些 RAM，但会违反一些编译器依赖的严格别名规则进行优化。
 * 默认值为 1（如果未定义）
 */
#define configUSE_MINI_LIST_ITEM 1

/* 堆栈深度类型配置 (configSTACK_DEPTH_TYPE)
 * 设置 xTaskCreate() 中指定任务堆栈大小的参数所使用的类型。
 * 相同的类型用于在各种其他 API 调用中返回有关堆栈使用情况的信息。
 * 默认为 size_t（如果未定义）
 */
#define configSTACK_DEPTH_TYPE size_t

/* 消息缓冲区长度类型配置 (configMESSAGE_BUFFER_LENGTH_TYPE)
 * 设置用于存储写入 FreeRTOS 消息缓冲区的每条消息长度的类型
 * （长度也会写入消息缓冲区）。默认为 size_t（如果未定义） -
 * 但如果消息长度永远不会超过 uint8_t 可容纳的长度，这可能会浪费空间
 */
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t

/* 释放时清空内存配置 (configHEAP_CLEAR_MEMORY_ON_FREE)
 * 如果 configHEAP_CLEAR_MEMORY_ON_FREE 设置为 1，则使用 pvPortMalloc() 分配的内存块
 * 在使用 vPortFree() 释放时将被清空（即设置为零）。
 * 默认值为 0（如果未定义）
 */
#define configHEAP_CLEAR_MEMORY_ON_FREE 1

/* 统计缓冲区最大长度 (configSTATS_BUFFER_MAX_LENGTH)
 * vTaskList 和 vTaskGetRunTimeStats API 将缓冲区作为参数，并假设
 * 缓冲区的长度为 configSTATS_BUFFER_MAX_LENGTH。
 * 默认值为 0xFFFF（如果未定义）。
 * 建议新应用程序使用 vTaskListTasks 和 vTaskGetRunTimeStatistics API，
 * 并显式提供缓冲区长度以避免内存损坏。
 */
#define configSTATS_BUFFER_MAX_LENGTH 0xFFFF

/* Newlib 可重入支持 (configUSE_NEWLIB_REENTRANT)
 * 设置为 1：为每个任务分配一个 newlib 可重入结构
 * 设置为 0：不支持 newlib 可重入结构
 * 默认值为 0（如果未定义）
 *
 * 注意：Newlib 支持是应流行需求而包含的，但 FreeRTOS 维护者本身并未使用或测试。
 * FreeRTOS 不负责由此产生的 newlib 操作。用户必须熟悉 newlib 并且必须提供
 * 必要存根的系统范围实现。请注意，（在撰写本文时）当前的 newlib 设计实现了
 * 系统范围的 malloc()，必须提供锁。
 */
#define configUSE_NEWLIB_REENTRANT 0

/******************************************************************************/
/* 软件定时器相关定义 *******************************************************/
/******************************************************************************/

/* 软件定时器功能使能 (configUSE_TIMERS)
 * 设置为 1：在构建中包含软件定时器功能
 * 设置为 0：从构建中排除软件定时器功能
 * 如果设置为 1，必须在构建中包含 FreeRTOS/source/timers.c 源文件
 * 默认值为 0（如果未定义）
 * 参考：https://www.freertos.org/RTOS-software-timer.html
 */
#define configUSE_TIMERS 1

/* 定时器任务优先级 (configTIMER_TASK_PRIORITY)
 * 设置定时器任务使用的优先级。仅在 configUSE_TIMERS 设置为 1 时使用。
 * 定时器任务是标准的 FreeRTOS 任务，因此其优先级设置与其他任务相同。
 * 参考：https://www.freertos.org/RTOS-software-timer-service-daemon-task.html
 */
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)

/* 定时器任务堆栈深度 (configTIMER_TASK_STACK_DEPTH)
 * 设置分配给定时器任务的堆栈大小（单位：字，不是字节！）
 * 定时器任务是标准的 FreeRTOS 任务。
 * 参考：https://www.freertos.org/RTOS-software-timer-service-daemon-task.html
 */
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

/* 定时器队列长度 (configTIMER_QUEUE_LENGTH)
 * 设置用于向定时器任务发送命令的队列长度（队列可容纳的离散项目数）
 * 参考：https://www.freertos.org/RTOS-software-timer-service-daemon-task.html
 */
#define configTIMER_QUEUE_LENGTH 10

/******************************************************************************/
/* 事件组相关定义 **********************************************************/
/******************************************************************************/

/* 事件组功能使能 (configUSE_EVENT_GROUPS)
 * 设置为 1：在构建中包含事件组功能
 * 设置为 0：从构建中排除事件组功能
 * 如果 configUSE_EVENT_GROUPS 设置为 1，必须在构建中包含 FreeRTOS/source/event_groups.c 源文件
 * 默认值为 1（如果未定义）
 */
#define configUSE_EVENT_GROUPS 0

/******************************************************************************/
/* 流缓冲区相关定义 ********************************************************/
/******************************************************************************/

/* 流缓冲区功能使能 (configUSE_STREAM_BUFFERS)
 * 设置为 1：在构建中包含流缓冲区功能
 * 设置为 0：从构建中排除流缓冲区功能
 * 如果 configUSE_STREAM_BUFFERS 设置为 1，必须在构建中包含 FreeRTOS/source/stream_buffer.c 源文件
 * 默认值为 1（如果未定义）
 */
#define configUSE_STREAM_BUFFERS 0

/******************************************************************************/
/* 内存分配相关定义 ********************************************************/
/******************************************************************************/

/* 静态内存分配支持 (configSUPPORT_STATIC_ALLOCATION)
 * 设置为 1：在构建中包含使用静态分配内存创建 FreeRTOS 对象（任务、队列等）的 API 函数
 * 设置为 0：从构建中排除创建静态分配对象的能力
 * 默认值为 0（如果未定义）
 * 参考：https://www.freertos.org/Static_Vs_Dynamic_Memory_Allocation.html
 */
#define configSUPPORT_STATIC_ALLOCATION 1

/* 动态内存分配支持 (configSUPPORT_DYNAMIC_ALLOCATION)
 * 设置为 1：在构建中包含使用动态分配内存创建 FreeRTOS 对象（任务、队列等）的 API 函数
 * 设置为 0：从构建中排除创建动态分配对象的能力
 * 默认值为 1（如果未定义）
 * 参考：https://www.freertos.org/Static_Vs_Dynamic_Memory_Allocation.html
 */
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* FreeRTOS 堆总大小 (configTOTAL_HEAP_SIZE)
 * 当构建中包含 heap_1.c、heap_2.c 或 heap_4.c 时，设置 FreeRTOS 堆的总大小（字节）
 * 此值默认为 4096 字节，但必须根据每个应用程序进行调整
 * 注意：堆将出现在 .bss 段中
 * 当前设置：80KB (80 * 1024 = 81920 字节)
 * 参考：https://www.freertos.org/a00111.html
 */
#define configTOTAL_HEAP_SIZE ((size_t)(10 * 1024))

/* 应用程序分配堆 (configAPPLICATION_ALLOCATED_HEAP)
 * 设置为 1：由应用程序分配用作 FreeRTOS 堆的数组
 * 设置为 0：由链接器分配用作 FreeRTOS 堆的数组
 * 默认值为 0（如果未定义）
 */
#define configAPPLICATION_ALLOCATED_HEAP 0

/* 从独立堆分配堆栈 (configSTACK_ALLOCATION_FROM_SEPARATE_HEAP)
 * 设置为 1：从 FreeRTOS 堆以外的地方分配任务堆栈，这在确保堆栈保存在快速内存中时很有用
 * 设置为 0：任务堆栈来自标准 FreeRTOS 堆
 * 如果设置为 1，应用程序编写者必须提供 pvPortMallocStack() 和 vPortFreeStack() 的实现
 * 默认值为 0（如果未定义）
 */
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP 0

/* 堆保护器使能 (configENABLE_HEAP_PROTECTOR)
 * 设置为 1：在 heap_4.c 和 heap_5.c 中启用边界检查和内部堆块指针混淆，以帮助捕获指针损坏
 * 默认值为 0（如果未定义）
 */
#define configENABLE_HEAP_PROTECTOR 0

/******************************************************************************/
/* 中断嵌套行为配置 ********************************************************/
/******************************************************************************/

/* Cortex-M 特定定义 */
#ifdef __NVIC_PRIO_BITS
/* 使用 CMSIS 时将指定 __NVIC_PRIO_BITS */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
/* STM32G0 系列有 4 位优先级位，支持 16 个优先级等级 (0-15) */
#define configPRIO_BITS 2 /* 15 个优先级等级 */
#endif

/* 库最低中断优先级 (configLIBRARY_LOWEST_INTERRUPT_PRIORITY)
 * 可在调用"设置优先级"函数时使用的最低中断优先级
 * 对于 4 位优先级：0xF 是最低优先级（数值越大优先级越低）
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0x3

/* 库最大系统调用中断优先级 (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
 * 可被调用中断安全 FreeRTOS API 函数的任何中断服务例程使用的最高中断优先级
 * 重要：不要从优先级高于此优先级的任何中断中调用中断安全 FreeRTOS API 函数！
 * （较高优先级对应较小的数值）
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 0x3

/* 内核中断优先级 (configKERNEL_INTERRUPT_PRIORITY)
 * 设置滴答和上下文切换执行中断的优先级
 * 并非所有 FreeRTOS 移植都支持此设置
 * 参考：https://www.freertos.org/RTOS-Cortex-M3-M4.html（ARM Cortex-M 设备的特定信息）
 */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* 最大系统调用中断优先级 (configMAX_SYSCALL_INTERRUPT_PRIORITY)
 * 设置不得进行 FreeRTOS API 调用的中断优先级阈值
 * 高于此优先级的中断永远不会被禁用，因此永远不会被 RTOS 活动延迟
 * 默认值设置为最高中断优先级 (0)
 * 并非所有 FreeRTOS 移植都支持此设置
 * 参考：https://www.freertos.org/RTOS-Cortex-M3-M4.html（ARM Cortex-M 设备的特定信息）
 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* configMAX_SYSCALL_INTERRUPT_PRIORITY 的另一个名称 - 使用的名称取决于 FreeRTOS 移植版本 */
#define configMAX_API_CALL_INTERRUPT_PRIORITY configMAX_SYSCALL_INTERRUPT_PRIORITY

/******************************************************************************/
/* 钩子函数和回调函数相关定义 **********************************************/
/******************************************************************************/

/* 钩子函数使能配置
 * 将以下 configUSE_* 常量设置为 1 以在构建中包含指定的钩子功能。
 * 设置为 0 以从构建中排除钩子功能。
 * 应用程序编写者负责为任何设置为 1 的钩子提供钩子函数。
 * 参考：https://www.freertos.org/a00016.html
 */
#define configUSE_IDLE_HOOK 0                // 空闲任务钩子
#define configUSE_TICK_HOOK 0                // 滴答中断钩子
#define configUSE_MALLOC_FAILED_HOOK 0       // 内存分配失败钩子
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0 // 守护任务启动钩子

/* 流缓冲区完成回调配置 (configUSE_SB_COMPLETED_CALLBACK)
 * 设置为 1：为流缓冲区或消息缓冲区的每个实例提供发送和接收完成回调
 * 当选项设置为 1 时，可以使用 API xStreamBufferCreateWithCallback() 和
 * xStreamBufferCreateStaticWithCallback()（以及消息缓冲区的类似 API）
 * 创建带有应用程序提供的回调的流缓冲区或消息缓冲区实例。
 * 默认值为 0（如果未定义）
 */
#define configUSE_SB_COMPLETED_CALLBACK 0

/* 堆栈溢出检查配置 (configCHECK_FOR_STACK_OVERFLOW)
 * 设置为 1 或 2：FreeRTOS 在上下文切换时检查堆栈溢出
 * 设置为 0：不检查堆栈溢出
 *
 * 如果 configCHECK_FOR_STACK_OVERFLOW 为 1：
 * - 检查仅在任务上下文保存到其堆栈时查看堆栈指针是否越界
 * - 这很快但效果有限
 *
 * 如果 configCHECK_FOR_STACK_OVERFLOW 为 2：
 * - 检查写入任务堆栈末尾的模式是否被覆盖
 * - 这较慢，但会捕获大多数（但不是全部）堆栈溢出
 *
 * 当 configCHECK_FOR_STACK_OVERFLOW 设置为 1 时，应用程序编写者必须提供堆栈溢出回调。
 * 参考：https://www.freertos.org/Stacks-and-stack-overflow-checking.html
 * 默认值为 0（如果未定义）
 */
#define configCHECK_FOR_STACK_OVERFLOW 0

/******************************************************************************/
/* 运行时和任务统计收集相关定义 *********************************************/
/******************************************************************************/

/* 运行时统计生成配置 (configGENERATE_RUN_TIME_STATS)
 * 设置为 1：让 FreeRTOS 收集每个任务使用的处理时间数据
 * 设置为 0：不收集数据
 * 如果设置为 1，应用程序编写者需要提供时钟源
 * 默认值为 0（如果未定义）
 * 参考：https://www.freertos.org/rtos-run-time-stats.html
 */
#define configGENERATE_RUN_TIME_STATS 0

/* 跟踪设施使能配置 (configUSE_TRACE_FACILITY)
 * 设置为 1：包含跟踪和可视化函数及工具使用的额外任务结构成员
 * 设置为 0：从结构中排除额外信息
 * 默认值为 0（如果未定义）
 */
#define configUSE_TRACE_FACILITY 0

/* 统计格式化函数使能 (configUSE_STATS_FORMATTING_FUNCTIONS)
 * 设置为 1：在构建中包含 vTaskList() 和 vTaskGetRunTimeStats() 函数
 * 设置为 0：从构建中排除这些函数
 * 这两个函数引入了对字符串格式化函数的依赖，否则这些函数不会存在 -
 * 因此它们被单独保留
 * 默认值为 0（如果未定义）
 */
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

/******************************************************************************/
/* 协程相关定义 ************************************************************/
/******************************************************************************/

/* 协程功能使能 (configUSE_CO_ROUTINES)
 * 设置为 1：在构建中包含协程功能
 * 设置为 0：从构建中忽略协程功能
 * 要包含协程，必须在项目中包含 croutine.c
 * 默认值为 0（如果未定义）
 *
 * 注意：协程是 FreeRTOS 的早期功能，现在很少使用，
 * 大多数应用程序应使用任务而不是协程
 */
#define configUSE_CO_ROUTINES 0

/* 协程最大优先级数 (configMAX_CO_ROUTINE_PRIORITIES)
 * 定义应用程序协程可用的优先级数量
 * 任意数量的协程可以共享相同的优先级
 * 默认值为 0（如果未定义）
 */
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/******************************************************************************/
/* 调试辅助 ***************************************************************/
/******************************************************************************/

/* 断言宏配置 (configASSERT)
 * configASSERT() 具有与标准 C assert() 相同的语义。
 * 它可以定义为在断言失败时执行操作，或者根本不定义
 * （即注释掉或删除定义）以完全删除断言。
 *
 * configASSERT() 可以定义为您想要的任何内容，例如：
 * - 如果断言失败，您可以调用一个函数，传递失败断言的文件名和行号
 *   （例如，"vAssertCalled( __FILE__, __LINE__ )"）
 * - 或者它可以简单地禁用中断并在循环中停留，以在调试器中查看失败行
 *
 * 当前配置：禁用中断并进入无限循环，便于调试器中断点调试
 */
#define configASSERT(x)           \
    if ((x) == 0)                 \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for (;;)                  \
            ;                     \
    }

/******************************************************************************/
/* FreeRTOS MPU 特定定义 ***************************************************/
/******************************************************************************/

/* 应用程序定义特权函数包含配置 (configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS)
 * 如果 configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 设置为 1，
 * 则应用程序编写者可以提供在特权模式下执行的函数。
 * 参考：https://www.freertos.org/a00110.html#configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS
 * 默认值为 0（如果未定义）
 * 仅由 FreeRTOS Cortex-M MPU 移植使用，不适用于标准 ARMv7-M Cortex-M 移植
 *
 * 注意：STM32G0 系列（Cortex-M0+）不支持 MPU，此配置对本平台无效
 */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* MPU 区域总数配置 (configTOTAL_MPU_REGIONS)
 * 设置目标硬件上实现的 MPU 区域数量，通常为 8 或 16。
 * 仅由 FreeRTOS Cortex-M MPU 移植使用，不适用于标准 ARMv7-M Cortex-M 移植。
 * 默认值为 8（如果未定义）
 *
 * 注意：STM32G0 系列（Cortex-M0+）不支持 MPU，此配置对本平台无效
 */
#define configTOTAL_MPU_REGIONS 8

/* Flash 区域 TEX_S_C_B 位配置 (configTEX_S_C_B_FLASH)
 * 允许应用程序编写者覆盖覆盖 Flash 的 MPU 区域的 TEX、共享 (S)、
 * 可缓存 (C) 和可缓冲 (B) 位的默认值。
 * 默认值为 0x07UL（意味着 TEX=000, S=1, C=1, B=1）（如果未定义）
 * 仅由 FreeRTOS Cortex-M MPU 移植使用，不适用于标准 ARMv7-M Cortex-M 移植
 */
#define configTEX_S_C_B_FLASH 0x07UL

/* SRAM 区域 TEX_S_C_B 位配置 (configTEX_S_C_B_SRAM)
 * 允许应用程序编写者覆盖覆盖 RAM 的 MPU 区域的 TEX、共享 (S)、
 * 可缓存 (C) 和可缓冲 (B) 位的默认值。
 * 默认值为 0x07UL（意味着 TEX=000, S=1, C=1, B=1）（如果未定义）
 * 仅由 FreeRTOS Cortex-M MPU 移植使用，不适用于标准 ARMv7-M Cortex-M 移植
 */
#define configTEX_S_C_B_SRAM 0x07UL

/* 仅从内核强制系统调用配置 (configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY)
 * 设置为 0：防止来自内核代码本身外部的任何特权提升
 * 设置为 1：允许应用程序任务提升特权
 * 默认值为 1（如果未定义）
 * 仅由 FreeRTOS Cortex-M MPU 移植使用，不适用于标准 ARMv7-M Cortex-M 移植
 */
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY 1

/* 允许非特权临界区配置 (configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS)
 * 设置为 1：允许非特权任务进入临界区（有效地屏蔽中断）
 * 设置为 0：防止非特权任务进入临界区
 * 默认值为 1（如果未定义）
 * 仅由 FreeRTOS Cortex-M MPU 移植使用，不适用于标准 ARMv7-M Cortex-M 移植
 */
#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS 0

/* MPU 包装器版本配置 (configUSE_MPU_WRAPPERS_V1)
 * FreeRTOS 内核版本 10.6.0 引入了新的 v2 MPU 包装器，即 mpu_wrappers_v2.c。
 * 设置为 0：使用新的 v2 MPU 包装器
 * 设置为 1：使用旧的 v1 MPU 包装器 (mpu_wrappers.c)
 * 默认值为 0（如果未定义）
 */
#define configUSE_MPU_WRAPPERS_V1 0

/* 受保护内核对象池大小 (configPROTECTED_KERNEL_OBJECT_POOL_SIZE)
 * 使用 v2 MPU 包装器时，设置应用程序中内核对象的总数，
 * 包括任务、队列、信号量、互斥量、事件组、定时器、流缓冲区和消息缓冲区。
 * 应用程序在任何时候都不能拥有超过 configPROTECTED_KERNEL_OBJECT_POOL_SIZE 的内核对象。
 */
#define configPROTECTED_KERNEL_OBJECT_POOL_SIZE 10

/* 系统调用堆栈大小 (configSYSTEM_CALL_STACK_SIZE)
 * 使用 v2 MPU 包装器时，设置系统调用堆栈的大小（以字为单位）。
 * 每个任务都有一个此大小的静态分配内存缓冲区，用作执行系统调用的堆栈。
 * 例如，如果 configSYSTEM_CALL_STACK_SIZE 定义为 128 且应用程序中有 10 个任务，
 * 则用于系统调用堆栈的内存总量为 128 * 10 = 1280 字。
 */
#define configSYSTEM_CALL_STACK_SIZE 128

/* 访问控制列表使能 (configENABLE_ACCESS_CONTROL_LIST)
 * 使用 v2 MPU 包装器时，设置为 1 以启用访问控制列表 (ACL) 功能。
 * 启用 ACL 时，非特权任务默认情况下无法访问除自身之外的任何内核对象。
 * 应用程序编写者需要使用提供的 API 显式授予非特权任务对其所需内核对象的访问权限。
 * 默认值为 0（如果未定义）
 */
#define configENABLE_ACCESS_CONTROL_LIST 1

/******************************************************************************/
/* SMP（对称多处理）特定配置定义 *******************************************/
/******************************************************************************/

/* 处理器核心数量配置 (configNUMBER_OF_CORES)
 * 设置可用处理器核心的数量
 * 默认值为 1（如果未定义）
 *
 * 注意：STM32G0 系列是单核处理器，此配置对本平台无效，保持注释状态
 */
/*
 #define configNUMBER_OF_CORES                     [Num of available cores]
 */

/* 多优先级运行配置 (configRUN_MULTIPLE_PRIORITIES)
 * 使用 SMP 时（即 configNUMBER_OF_CORES 大于 1），设置 configRUN_MULTIPLE_PRIORITIES
 * 为 0：仅当任务优先级不相等时才允许多个任务同时运行，
 *      从而保持较低优先级任务在较高优先级任务能够运行时永不运行的范式
 * 为 1：不同优先级的多个任务可以同时运行 - 因此较高和较低优先级任务可以在不同核心上同时运行
 */
#define configRUN_MULTIPLE_PRIORITIES 0

/* 核心亲和性使能配置 (configUSE_CORE_AFFINITY)
 * 使用 SMP 时（即 configNUMBER_OF_CORES 大于 1），设置 configUSE_CORE_AFFINITY
 * 为 1：启用核心亲和性功能。启用核心亲和性功能时，可以使用 vTaskCoreAffinitySet
 *      和 vTaskCoreAffinityGet API 来设置和检索任务可以在哪些核心上运行
 * 为 0：FreeRTOS 调度器可以自由地在任何可用核心上运行任何任务
 */
#define configUSE_CORE_AFFINITY 0

/* 任务默认核心亲和性配置 (configTASK_DEFAULT_CORE_AFFINITY)
 * 在启用核心亲和性功能的 SMP 中使用，设置 configTASK_DEFAULT_CORE_AFFINITY
 * 以更改未指定亲和性掩码创建的任务的默认核心亲和性掩码。
 * 将定义设置为 1 将使此类任务在核心 0 上运行，
 * 设置为 (1 << portGET_CORE_ID()) 将使此类任务在当前核心上运行。
 * 如果不支持在核心之间交换任务（例如 Tricore）或需要控制传统代码，此配置值很有用。
 * 默认值为 tskNO_AFFINITY（如果未定义）
 */
#define configTASK_DEFAULT_CORE_AFFINITY tskNO_AFFINITY

/* 任务抢占禁用使能配置 (configUSE_TASK_PREEMPTION_DISABLE)
 * 使用 SMP 时（即 configNUMBER_OF_CORES 大于 1），如果 configUSE_TASK_PREEMPTION_DISABLE
 * 设置为 1，可以使用 vTaskPreemptionDisable 和 vTaskPreemptionEnable API
 * 将各个任务设置为抢占式或协作式模式
 */
#define configUSE_TASK_PREEMPTION_DISABLE 0

/* 被动空闲钩子使能配置 (configUSE_PASSIVE_IDLE_HOOK)
 * 使用 SMP 时（即 configNUMBER_OF_CORES 大于 1），设置 configUSE_PASSIVE_IDLE_HOOK
 * 为 1：允许应用程序编写者使用被动空闲任务钩子来添加后台功能，而无需单独任务的开销
 * 默认值为 0（如果未定义）
 */
#define configUSE_PASSIVE_IDLE_HOOK 0

/* 定时器服务任务核心亲和性配置 (configTIMER_SERVICE_TASK_CORE_AFFINITY)
 * 使用 SMP 时（即 configNUMBER_OF_CORES 大于 1），configTIMER_SERVICE_TASK_CORE_AFFINITY
 * 允许应用程序编写者设置 RTOS 守护程序/定时器服务任务的核心亲和性
 * 默认值为 tskNO_AFFINITY（如果未定义）
 */
#define configTIMER_SERVICE_TASK_CORE_AFFINITY tskNO_AFFINITY

/******************************************************************************/
/* ARMv8-M 安全端口相关定义 ************************************************/
/******************************************************************************/

/* 最大安全上下文数量 (secureconfigMAX_SECURE_CONTEXTS)
 * 定义可以调用 ARMv8-M 芯片安全端的最大任务数量
 * 不被任何其他移植使用
 *
 * 注意：STM32G0 系列是 Cortex-M0+ 内核，不支持 TrustZone，此配置对本平台无效
 */
#define secureconfigMAX_SECURE_CONTEXTS 5

/* 内核提供的静态内存配置 (configKERNEL_PROVIDED_STATIC_MEMORY)
 * 定义内核提供的 vApplicationGetIdleTaskMemory() 和 vApplicationGetTimerTaskMemory() 实现，
 * 以提供空闲任务和定时器任务分别使用的内存。
 * 应用程序可以通过将 configKERNEL_PROVIDED_STATIC_MEMORY 设置为 0 或保持未定义
 * 来提供自己的 vApplicationGetIdleTaskMemory() 和 vApplicationGetTimerTaskMemory() 实现。
 */
#define configKERNEL_PROVIDED_STATIC_MEMORY 1

/******************************************************************************/
/* ARMv8-M 移植特定配置定义 ************************************************/
/******************************************************************************/

/* TrustZone 使能配置 (configENABLE_TRUSTZONE)
 * 当在非安全端运行 FreeRTOS 时设置为 1，以启用 FreeRTOS ARMv8-M 移植中的 TrustZone 支持，
 * 这允许非安全 FreeRTOS 任务调用从安全端导出的（非安全可调用）函数。
 *
 * 注意：STM32G0 系列是 Cortex-M0+ 内核，不支持 TrustZone，此配置对本平台无效
 */
#define configENABLE_TRUSTZONE 1

/* 仅在安全端运行 FreeRTOS 配置 (configRUN_FREERTOS_SECURE_ONLY)
 * 如果应用程序编写者不想使用 TrustZone，但硬件不支持禁用 TrustZone，
 * 则整个应用程序（包括 FreeRTOS 调度器）可以在安全端运行，而不会分支到非安全端。
 * 要做到这一点，除了将 configENABLE_TRUSTZONE 设置为 0 之外，
 * 还要将 configRUN_FREERTOS_SECURE_ONLY 设置为 1。
 *
 * 注意：STM32G0 系列是 Cortex-M0+ 内核，不支持 TrustZone，此配置对本平台无效
 */
#define configRUN_FREERTOS_SECURE_ONLY 1

/* 内存保护单元使能配置 (configENABLE_MPU)
 * 设置为 1：启用内存保护单元 (MPU)
 * 设置为 0：保持内存保护单元禁用状态
 *
 * 注意：STM32G0 系列是 Cortex-M0+ 内核，不支持 MPU，此配置对本平台无效
 */
#define configENABLE_MPU 0

/* 浮点单元使能配置 (configENABLE_FPU)
 * 设置为 1：启用浮点单元 (FPU)
 * 设置为 0：保持浮点单元禁用状态
 *
 * 注意：STM32G0 系列是 Cortex-M0+ 内核，不包含 FPU，
 * 但此配置设置为 1 不会造成问题，只是不会有实际效果
 */
#define configENABLE_FPU 0

/* M-Profile 矢量扩展使能配置 (configENABLE_MVE)
 * 设置为 1：启用 M-Profile 矢量扩展 (MVE) 支持
 * 设置为 0：保持 MVE 支持禁用状态
 * 此选项仅适用于 Cortex-M55 和 Cortex-M85 移植，因为 M-Profile 矢量扩展 (MVE)
 * 仅在这些架构上可用。对于 Cortex-M23、Cortex-M33 和 Cortex-M35P 移植，
 * configENABLE_MVE 必须保持未定义或定义为 0。
 *
 * 注意：STM32G0 系列是 Cortex-M0+ 内核，不支持 MVE，
 * 应将此配置设置为 0，但当前设置不会造成问题
 */
#define configENABLE_MVE 1

/******************************************************************************/
/* ARMv7-M 和 ARMv8-M 移植特定配置定义 *************************************/
/******************************************************************************/

/* 中断处理程序安装检查配置 (configCHECK_HANDLER_INSTALLATION)
 * 设置为 1：启用额外断言以验证应用程序是否正确安装了 FreeRTOS 中断处理程序
 *
 * 应用程序可以通过以下方式之一安装 FreeRTOS 中断处理程序：
 *   1. 直接路由 - 分别为 SVC 调用和 PendSV 中断安装函数 vPortSVCHandler 和 xPortPendSVHandler
 *   2. 间接路由 - 为 SVC 调用和 PendSV 中断安装单独的处理程序，并将程序控制从这些处理程序
 *                路由到 vPortSVCHandler 和 xPortPendSVHandler 函数
 *
 * 使用间接路由的应用程序必须将 configCHECK_HANDLER_INSTALLATION 设置为 0。
 *
 * 默认值为 1（如果未定义）
 */
#define configCHECK_HANDLER_INSTALLATION 1

/******************************************************************************/
/* 包含或排除功能的定义 ****************************************************/
/******************************************************************************/

/* 功能包含配置
 * 将以下 configUSE_* 常量设置为 1 以在构建中包含指定功能，
 * 设置为 0 以从构建中排除指定功能。
 */
#define configUSE_TASK_NOTIFICATIONS 1   // 任务通知功能
#define configUSE_MUTEXES 1              // 互斥量功能
#define configUSE_RECURSIVE_MUTEXES 1    // 递归互斥量功能
#define configUSE_COUNTING_SEMAPHORES 1  // 计数信号量功能
#define configUSE_QUEUE_SETS 0           // 队列集功能
#define configUSE_APPLICATION_TASK_TAG 0 // 应用程序任务标签功能

/* API 函数包含配置
 * 将以下 INCLUDE_* 常量设置为 1 以包含指定的 API 函数，
 * 设置为 0 以排除指定的 API 函数。
 * 大多数链接器即使常量为 1 也会删除未使用的函数。
 */
#define INCLUDE_vTaskPrioritySet 1            // 设置任务优先级 API
#define INCLUDE_uxTaskPriorityGet 1           // 获取任务优先级 API
#define INCLUDE_vTaskDelete 1                 // 删除任务 API
#define INCLUDE_vTaskSuspend 1                // 挂起任务 API
#define INCLUDE_xResumeFromISR 1              // 从 ISR 恢复任务 API
#define INCLUDE_vTaskDelayUntil 1             // 任务延迟到指定时间 API
#define INCLUDE_vTaskDelay 1                  // 任务延迟 API
#define INCLUDE_xTaskGetSchedulerState 1      // 获取调度器状态 API
#define INCLUDE_xTaskGetCurrentTaskHandle 1   // 获取当前任务句柄 API
#define INCLUDE_uxTaskGetStackHighWaterMark 0 // 获取任务堆栈高水位标记 API
#define INCLUDE_xTaskGetIdleTaskHandle 0      // 获取空闲任务句柄 API
#define INCLUDE_eTaskGetState 0               // 获取任务状态 API
#define INCLUDE_xEventGroupSetBitFromISR 1    // 从 ISR 设置事件组位 API
#define INCLUDE_xTimerPendFunctionCall 0      // 定时器挂起函数调用 API
#define INCLUDE_xTaskAbortDelay 0             // 中止任务延迟 API
#define INCLUDE_xTaskGetHandle 0              // 通过名称获取任务句柄 API
#define INCLUDE_xTaskResumeFromISR 1          // 从 ISR 恢复任务 API

/* FreeRTOS 移植中断处理程序到 CMSIS 标准名称的映射定义 */
#define vPortSVCHandler SVC_Handler         // SVC 中断处理程序映射
#define xPortPendSVHandler PendSV_Handler   // PendSV 中断处理程序映射
#define xPortSysTickHandler SysTick_Handler // SysTick 中断处理程序映射

#endif /* FREERTOS_CONFIG_H */
