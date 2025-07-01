/**
 * @file yDev_gpio.h
 * @brief yDev GPIO设备驱动头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的GPIO设备驱动，提供统一的GPIO操作接口
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
     * @brief GPIO设备配置结构体
     *
     * @par 功能描述:
     * 包含yDev基础配置和GPIO特定的驱动配置参数
     */
    typedef struct
    {
        yDevConfig_t base;           /**< yDev基础配置 */
        yDrvGpioConfig_t drv_config; /**< GPIO驱动配置 */
    } yDevConfig_Gpio_t;

    /**
     * @brief GPIO设备句柄结构体
     *
     * @par 功能描述:
     * 包含yDev基础句柄和GPIO特定的驱动句柄
     */
    typedef struct
    {
        yDevHandle_t base;           /**< yDev基础句柄 */
        yDrvGpioHandle_t drv_handle; /**< GPIO驱动句柄 */
    } yDevHandle_Gpio_t;

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
