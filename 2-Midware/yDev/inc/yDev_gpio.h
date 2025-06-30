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

    typedef struct
    {
        yDevConfig_t base;
        yDrvGpioConfig_t drv_config;
    } yDevConfig_Gpio_t;

    /**
     * @brief GPIOhandle
     */
    typedef struct
    {
        yDevHandle_t base;
        yDrvGpioHandle_t drv_handle;
    } yDevHandle_Gpio_t;


#ifdef __cplusplus
}
#endif

#endif /* YDEV_GPIO_H */
