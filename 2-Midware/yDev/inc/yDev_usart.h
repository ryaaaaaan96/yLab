/**
 * @file yDev_usart.h
 * @brief yDev USART设备驱动头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的USART设备驱动，提供统一的串口通信接口
 *
 * @par 主要特性:
 * - 统一的串口读写接口
 * - 支持阻塞和非阻塞模式
 * - 完整的USART配置参数管理
 * - 中断回调机制
 * - 错误处理和状态查询
 * - 与底层yDrv USART驱动的适配
 */

#ifndef YDEV_USART_H
#define YDEV_USART_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDrv_usart.h"
#include "yDrv_dma.h"
    // ==================== USART设备特定定义 ====================

    /**
     * @brief USART设备配置结构体
     *
     * @par 功能描述:
     * 包含yDev基础配置和USART特定的驱动配置参数
     */
    typedef struct
    {
        yDevConfig_t base;            /**< yDev基础配置 */
        yDrvUsartConfig_t drv_config; /**< USART驱动配置 */
    } yDevConfig_Usart_t;

    /**
     * @brief USART设备句柄结构体
     *
     * @par 功能描述:
     * 包含yDev基础句柄和USART特定的驱动句柄及缓冲区管理
     */
    typedef struct
    {
        yDevHandle_t base;             /**< yDev基础句柄 */
        yDrvUsartHandle_t drv_handle;  /**< USART驱动句柄 */
        yDrvDmaHandle_t rx_dma_handle; /**< 接收缓冲区指针 */
        yDrvDmaHandle_t tx_dma_handle; /**< 发送缓冲区指针 */
    } yDevHandle_Usart_t;

// ==================== yDev USART配置初始化宏 ====================

/**
 * @brief yDev USART配置结构体默认初始化宏
 * @note 提供最常用的默认配置，适用于标准UART通信
 */
#define YDEV_USART_CONFIG_DEFAULT()                \
    ((yDevConfig_Usart_t){                         \
        .base = YDEV_CONFIG_DEFAULT(),             \
        .drv_config = YDRV_USART_CONFIG_DEFAULT(), \
    })

/**
 * @brief yDev USART句柄结构体默认初始化宏
 * @note 提供安全的默认初始化值
 */
#define YDEV_USART_HANDLE_DEFAULT()                 \
    ((yDevHandle_Usart_t){                          \
        .base = YDEV_HANDLE_DEFAULT(),              \
        .drv_handle = YDRV_USART_HANDLE_DEFAULT(),  \
        .rx_dma_handle = YDRV_DMA_HANDLE_DEFAULT(), \
        .tx_dma_handle = YDRV_DMA_HANDLE_DEFAULT()})

    // ==================== yDev USART初始化函数 ====================

    /**
     * @brief 初始化yDev USART配置结构体为默认值
     * @param config 配置结构体指针
     * @retval 无
     */
    void yDevUsartConfigStructInit(yDevConfig_Usart_t *config);

    /**
     * @brief 初始化yDev USART句柄结构体为默认值
     * @param handle 句柄结构体指针
     * @retval 无
     */
    void yDevUsartHandleStructInit(yDevHandle_Usart_t *handle);

    // ==================== yDev USART中断DMA调用串口函数 ====================

    YLIB_INLINE uint32_t yDevUsartDmaRxLenGet(yDevHandle_Usart_t *handle)
    {
        return yDrvDmaCurLenGet(&handle->rx_dma_handle);
    }

// ==================== USART设备IOCTL命令定义 ====================

/**
 * @brief USART设备IOCTL命令
 */
#define YDEV_USART_IOCTL_BASE (YDEV_IOCTL_BASE + 0x100)

#define YDEV_USART_IOCTL_SET_INTERRUPT (YDEV_USART_IOCTL_BASE + 1)     /**< 设置中断 */
#define YDEV_USART_IOCTL_RESET_INTERRUPT (YDEV_USART_IOCTL_BASE + 2)   /**< 重置中断 */
#define YDEV_USART_IOCTL_ENABLE_INTERRUPT (YDEV_USART_IOCTL_BASE + 3)  /**< 打开中断状态 */
#define YDEV_USART_IOCTL_DISABLE_INTERRUPT (YDEV_USART_IOCTL_BASE + 4) /**< 关闭中断状态 */

#define YDEV_USART_IOCTL_SET_SEND_DMA (YDEV_USART_IOCTL_BASE + 14)        /**< 设置发送DMA */
#define YDEV_USART_IOCTL_SET_RECEIVE_DMA (YDEV_USART_IOCTL_BASE + 15)     /**< 设置接收DMA */
#define YDEV_USART_IOCTL_ENABLE_SEND_DMA (YDEV_USART_IOCTL_BASE + 16)     /**< 使能发送DMA */
#define YDEV_USART_IOCTL_ENABLE_RECEIVE_DMA (YDEV_USART_IOCTL_BASE + 17)  /**< 使能接收DMA */
#define YDEV_USART_IOCTL_DISABLE_SEND_DMA (YDEV_USART_IOCTL_BASE + 18)    /**< 禁用发送DMA */
#define YDEV_USART_IOCTL_DISABLE_RECEIVE_DMA (YDEV_USART_IOCTL_BASE + 19) /**< 禁用接收DMA */

    // ==================== USART错误代码定义 ====================

    /**
     * @brief USART错误代码定义
     * @note 这些错误代码用于标识USART通信过程中可能出现的各种错误情况
     * @note 错误代码采用位掩码方式定义，可以进行位运算组合
     */

#define YDEV_USART_ERRNO_NONE (0UL)                 // 无错误
#define YDEV_USART_ERRNO_ORE (1UL << (0))           // 溢出错误 (Overrun Error)
#define YDEV_USART_ERRNO_PE (1UL << (1))            // 奇偶校验错误 (Parity Error)
#define YDEV_USART_ERRNO_FE (1UL << (2))            // 帧错误 (Frame Error)
#define YDEV_USART_ERRNO_NE (1UL << (3))            // 噪声错误 (Noise Error)
#define YDEV_USART_ERRNO_IDLE (1UL << (4))          // 空闲线检测 (IDLE Line Detection)
#define YDEV_USART_ERRNO_BREAK (1UL << (5))         // 断路检测错误 (Break Detection)
#define YDEV_USART_ERRNO_DMA (1UL << (6))           // DMA传输错误
#define YDEV_USART_ERRNO_TIMEOUT (1UL << (7))       // 超时错误
#define YDEV_USART_ERRNO_BUFFER_FULL (1UL << (8))   // 缓冲区满错误
#define YDEV_USART_ERRNO_INVALID_PARAM (1UL << (9)) // 无效参数错误
#define YDEV_USART_ERRNO_NOT_INIT (1UL << (10))     // 设备未初始化错误
#define YDEV_USART_ERRNO_BUSY (1UL << (11))         // 设备忙错误

#ifdef __cplusplus
}
#endif

#endif /* YDEV_USART_H */
