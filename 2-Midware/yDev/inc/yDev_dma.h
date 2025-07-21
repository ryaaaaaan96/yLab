/**
 * @file yDev_dma.h
 * @brief yDev DMA设备驱动头文件
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供基于yDrv的DMA设备抽象层接口，支持多种传输模式和配置选项
 *
 * @par 主要特性:
 * - 支持内存到内存、内存到外设、外设到内存传输
 * - 支持正常模式和循环模式
 * - 完整的中断管理和回调机制
 * - 统一的错误处理和状态查询
 * - 支持多通道并发操作
 */

#ifndef YDEV_DMA_H
#define YDEV_DMA_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev_basic.h"
#include "yDrv_dma.h"
#include "yDev.h"

    // ==================== DMA传输配置结构体 ====================

    /**
     * @brief DMA传输配置结构体
     */
    typedef struct
    {
        yDevConfig_t base;
        yDrvDmaConfig_t drv_config;
    } yDevConfig_Dma_t;

    // ==================== DMA设备句柄结构体 ====================

    /**
     * @brief DMA设备句柄结构体
     */
    typedef struct
    {
        yDevHandle_t base;          // yDev基础句柄
        yDrvDmaHandle_t drv_handle; // yDrv的DMA驱动句柄
    } yDevHandle_Dma_t;

    // ==================== DMA设备状态枚举类型 ====================

// ==================== DMA设备IOCTL命令定义 ====================

/**
 * @brief DMA设备IOCTL命令
 */
#define YDEV_DMA_IOCTL_BASE (YDEV_IOCTL_BASE + 0x200)
#define YDEV_DMA_IOCTL_START (YDEV_DMA_IOCTL_BASE + 1)         /**< 启动传输 */
#define YDEV_DMA_IOCTL_STOP (YDEV_DMA_IOCTL_BASE + 2)          /**< 停止传输 */
#define YDEV_DMA_IOCTL_PAUSE (YDEV_DMA_IOCTL_BASE + 3)         /**< 暂停传输 */
#define YDEV_DMA_IOCTL_RESUME (YDEV_DMA_IOCTL_BASE + 4)        /**< 恢复传输 */
#define YDEV_DMA_IOCTL_GET_STATUS (YDEV_DMA_IOCTL_BASE + 5)    /**< 获取状态 */
#define YDEV_DMA_IOCTL_GET_REMAINING (YDEV_DMA_IOCTL_BASE + 6) /**< 获取剩余传输量 */
#define YDEV_DMA_IOCTL_SET_CALLBACK (YDEV_DMA_IOCTL_BASE + 7)  /**< 设置回调函数 */
#define YDEV_DMA_IOCTL_CLEAR_ERRORS (YDEV_DMA_IOCTL_BASE + 8)  /**< 清除错误标志 */
#define YDEV_DMA_IOCTL_GET_ERRORS (YDEV_DMA_IOCTL_BASE + 9)    /**< 获取错误标志 */

// ==================== DMA错误代码定义 ====================

/**
 * @brief DMA错误代码定义
 * @note 错误代码采用位掩码方式定义，可以进行位运算组合
 */
#define YDEV_DMA_ERRNO_NONE (0UL)                // 无错误
#define YDEV_DMA_ERRNO_TRANSFER_ERROR (1UL << 0) // 传输错误
#define YDEV_DMA_ERRNO_HALF_TRANSFER (1UL << 1)  // 半传输完成
#define YDEV_DMA_ERRNO_FULL_TRANSFER (1UL << 2)  // 全传输完成
#define YDEV_DMA_ERRNO_TIMEOUT (1UL << 3)        // 超时错误
#define YDEV_DMA_ERRNO_INVALID_PARAM (1UL << 4)  // 无效参数错误

#define YDEV_DMA_ERRNO_BUSY (1UL << 6)  // 设备忙错误
#define YDEV_DMA_ERRNO_ABORT (1UL << 7) // 传输中止错误

// 错误掩码定义
#define YDEV_DMA_ERRNO_HW_MASK (YDEV_DMA_ERRNO_TRANSFER_ERROR)                                                                                                // 硬件错误掩码
#define YDEV_DMA_ERRNO_SW_MASK (YDEV_DMA_ERRNO_TIMEOUT | YDEV_DMA_ERRNO_INVALID_PARAM | YDEV_DMA_ERRNO_NOT_INIT | YDEV_DMA_ERRNO_BUSY | YDEV_DMA_ERRNO_ABORT) // 软件错误掩码
#define YDEV_DMA_ERRNO_ALL_MASK (YDEV_DMA_ERRNO_HW_MASK | YDEV_DMA_ERRNO_SW_MASK | YDEV_DMA_ERRNO_HALF_TRANSFER | YDEV_DMA_ERRNO_FULL_TRANSFER)               // 所有错误掩码

// 错误检查宏
#define YDEV_DMA_IS_HW_ERROR(errno) (((errno) & YDEV_DMA_ERRNO_HW_MASK) != 0) // 检查是否有硬件错误
#define YDEV_DMA_IS_SW_ERROR(errno) (((errno) & YDEV_DMA_ERRNO_SW_MASK) != 0) // 检查是否有软件错误
#define YDEV_DMA_HAS_ERROR(errno, error) (((errno) & (error)) != 0)           // 检查指定错误是否发生

#ifdef __cplusplus
}
#endif

#endif /* YDEV_DMA_H */