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
        yDevHandle_t base;            /**< yDev基础句柄 */
        yDrvUsartHandle_t drv_handle; /**< USART驱动句柄 */
    } yDevHandle_Usart_t;

// ==================== yDev USART配置初始化宏 ====================

/**
 * @brief yDev USART配置结构体默认初始化宏
 * @note 提供最常用的默认配置，适用于标准UART通信
 */
#define YDEV_USART_CONFIG_DEFAULT()                \
    ((yDevConfig_Usart_t)                         \
    {                                              \
        .base = YDEV_CONFIG_DEFAULT(),             \
        .drv_config = YDRV_USART_CONFIG_DEFAULT(), \
    })

/**
 * @brief yDev USART句柄结构体默认初始化宏
 * @note 提供安全的默认初始化值
 */
#define YDEV_USART_HANDLE_DEFAULT()                \
    ((yDevHandle_Usart_t)                         \
    {                                              \
        .base = YDEV_HANDLE_DEFAULT(),             \
        .drv_handle = YDRV_USART_HANDLE_DEFAULT(), \
    })

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

// ==================== USART设备IOCTL命令定义 ====================

/**
 * @brief USART设备IOCTL命令
 */
#define YDEV_USART_IOCTL_BASE (YDEV_IOCTL_BASE + 0x100)

#define YDEV_USART_IOCTL_SET_BAUDRATE (YDEV_USART_IOCTL_BASE + 1)     /**< 设置波特率 */
#define YDEV_USART_IOCTL_GET_BAUDRATE (YDEV_USART_IOCTL_BASE + 2)     /**< 获取波特率 */
#define YDEV_USART_IOCTL_FLUSH_RX (YDEV_USART_IOCTL_BASE + 3)         /**< 清空接收缓冲区 */
#define YDEV_USART_IOCTL_FLUSH_TX (YDEV_USART_IOCTL_BASE + 4)         /**< 清空发送缓冲区 */
#define YDEV_USART_IOCTL_GET_RX_COUNT (YDEV_USART_IOCTL_BASE + 5)     /**< 获取接收缓冲区数据量 */
#define YDEV_USART_IOCTL_GET_TX_COUNT (YDEV_USART_IOCTL_BASE + 6)     /**< 获取发送缓冲区数据量 */
#define YDEV_USART_IOCTL_SET_RX_CALLBACK (YDEV_USART_IOCTL_BASE + 7)  /**< 设置接收回调函数 */
#define YDEV_USART_IOCTL_SET_TX_CALLBACK (YDEV_USART_IOCTL_BASE + 8)  /**< 设置发送完成回调函数 */
#define YDEV_USART_IOCTL_SET_ERR_CALLBACK (YDEV_USART_IOCTL_BASE + 9) /**< 设置错误回调函数 */
#define YDEV_USART_IOCTL_ENABLE_IRQ (YDEV_USART_IOCTL_BASE + 10)      /**< 使能中断 */
#define YDEV_USART_IOCTL_DISABLE_IRQ (YDEV_USART_IOCTL_BASE + 11)     /**< 禁用中断 */
#define YDEV_USART_IOCTL_GET_STATUS (YDEV_USART_IOCTL_BASE + 12)      /**< 获取状态 */

#ifdef __cplusplus
}
#endif

#endif /* YDEV_USART_H */
