/**
 * @file yDev_gpio.h
 * @brief yDev GPIO设备驱动头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的GPIO设备驱动，提供统一的GPIO操作接口
 *
 * @par 主要特性:
 * - 统一的GPIO读写接口
 * - 支持输入输出模式配置
 * - 完整的GPIO配置参数管理
 * - 中断回调机制支持
 * - 错误处理和状态查询
 * - 与底层yDrv GPIO驱动的适配
 */

#ifndef YDEV_GPIO_H
#define YDEV_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDrv_gpio.h"
    // ==================== GPIO设备特定定义 ====================

    /**
     * @brief yDev GPIO设备配置结构体
     * @note 包含yDev基础配置和GPIO特定的驱动配置参数
     */
    typedef struct
    {
        yDevConfig_t base;           /*!< yDev基础配置结构体 */
        yDrvGpioConfig_t drv_config; /*!< GPIO底层驱动配置结构体 */
    } yDevConfig_Gpio_t;

    /**
     * @brief yDev GPIO设备句柄结构体
     * @note 包含yDev基础句柄和GPIO特定的驱动句柄
     */
    typedef struct
    {
        yDevHandle_t base;           /*!< yDev基础句柄结构体 */
        yDrvGpioHandle_t drv_handle; /*!< GPIO底层驱动句柄 */
    } yDevHandle_Gpio_t;

// ==================== yDev GPIO配置初始化宏 ====================

/**
 * @brief yDev GPIO配置结构体默认初始化宏
 * @note 提供最常用的默认配置，适用于输入模式
 */
#define YDEV_GPIO_CONFIG_DEFAULT()                \
    ((yDevConfig_Gpio_t){                         \
        .base = YDEV_CONFIG_DEFAULT(),            \
        .drv_config = YDRV_GPIO_CONFIG_DEFAULT(), \
    })

/**
 * @brief yDev GPIO句柄结构体默认初始化宏
 * @note 提供安全的默认初始化值
 */
#define YDEV_GPIO_HANDLE_DEFAULT()                \
    ((yDevHandle_Gpio_t){                         \
        .base = YDEV_HANDLE_DEFAULT(),            \
        .drv_handle = YDRV_GPIO_HANDLE_DEFAULT(), \
    })

    // ==================== yDev GPIO初始化函数 ====================

    /**
     * @brief 初始化yDev GPIO配置结构体为默认值
     * @param config 配置结构体指针
     * @retval 无
     */
    void yDevGpioConfigStructInit(yDevConfig_Gpio_t *config);

    /**
     * @brief 初始化yDev GPIO句柄结构体为默认值
     * @param handle 句柄结构体指针
     * @retval 无
     */
    void yDevGpioHandleStructInit(yDevHandle_Gpio_t *handle);

    // ==================== GPIO设备IOCTL命令定义 ====================

    /**
     * @brief GPIO设备IOCTL命令
     *
     * @par 命令说明:
     * - YDEV_GPIO_SET_PIN: 设置GPIO引脚为高电平
     * - YDEV_GPIO_CLEAR_PIN: 设置GPIO引脚为低电平
     * - YDEV_GPIO_TOGGLE_PIN: 翻转GPIO引脚状态
     * - YDEV_GPIO_GET_PIN: 获取GPIO引脚状态
     * - YDEV_GPIO_REGISTER_EXIT: 注册GPIO外部中断回调
     */
#define YDEV_GPIO_IOCTL_BASE 0x8100
#define YDEV_GPIO_SET_PIN (YDEV_GPIO_IOCTL_BASE + 0)
#define YDEV_GPIO_CLEAR_PIN (YDEV_GPIO_IOCTL_BASE + 1)
#define YDEV_GPIO_TOGGLE_PIN (YDEV_GPIO_IOCTL_BASE + 2)
#define YDEV_GPIO_GET_PIN (YDEV_GPIO_IOCTL_BASE + 3)
#define YDEV_GPIO_REGISTER_EXIT (YDEV_GPIO_IOCTL_BASE + 4)
#define YDEV_GPIO_SET_EXIT (YDEV_GPIO_IOCTL_BASE + 5)
#define YDEV_GPIO_UNREGISTER_EXIT (YDEV_GPIO_IOCTL_BASE + 6)

#ifdef __cplusplus
}
#endif

#endif /* YDEV_GPIO_H */
