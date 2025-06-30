/**
 * @file yDev_gpio.c
 * @brief yDev GPIO设备驱动实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 */

#include "yDev_gpio.h"
#include "yDev_def.h"
#include "yDrv_gpio.h"
#include <stdlib.h>
#include <string.h>

// ==================== GPIO设备操作函数 ====================

/**
 * @brief GPIO设备初始化
 */
static yDevStatus_t yDev_Gpio_Init(void *config, void *handle)
{
    yDevConfig_Gpio_t *gpio_config;
    yDevHandle_Gpio_t *gpio_handle;

    if ((handle == NULL) || (config == NULL))
    {
        return YDEV_INVALID_PARAM;
    }
    gpio_handle = handle;
    gpio_config = config;
    // 调用底层yDrv初始化GPIO
    yDrvGpioInitStatic(&gpio_config->drv_config, &gpio_handle->drv_handle);

    return YDEV_OK;
}

/**
 * @brief GPIO设备去初始化
 */
static yDevStatus_t yDev_Gpio_Deinit(void *handle)
{
    yDevHandle_Gpio_t *gpio_handle;

    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    gpio_handle = handle;
    // 调用底层yDrv反初始化GPIO
    yDrvGpioDeInitStatic(&gpio_handle->drv_handle);

    return YDEV_OK;
}

/**
 * @brief GPIO设备读取（读取引脚状态）
 */
static int32_t yDev_Gpio_Read(void *handle, void *buffer, size_t size)
{
    yDevHandle_Gpio_t *gpio_handle;
    yDrvPinState_t pin_state;
    uint32_t *state_buffer;

    if ((handle == NULL) || (buffer == NULL) || (size < sizeof(uint32_t)))
    {
        return -1; // 返回错误
    }

    gpio_handle = (yDevHandle_Gpio_t *)handle;
    state_buffer = (uint32_t *)buffer;

    // 调用底层yDrv读取GPIO引脚状态
    pin_state = yDrvGpioRead(&gpio_handle->drv_handle);

    // 将状态写入缓冲区
    *state_buffer = (uint32_t)pin_state;

    return sizeof(uint32_t); // 返回实际读取的字节数
}

/**
 * @brief GPIO设备写入（设置引脚状态）
 */
static int32_t yDev_Gpio_Write(void *handle, const void *buffer, size_t size)
{
    yDevHandle_Gpio_t *gpio_handle;
    const uint32_t *state_buffer;
    yDrvPinState_t pin_state;
    yDrvStatus_t result;

    if ((handle == NULL) || (buffer == NULL) || (size < sizeof(uint32_t)))
    {
        return -1; // 返回错误
    }

    gpio_handle = (yDevHandle_Gpio_t *)handle;
    state_buffer = (const uint32_t *)buffer;

    // 转换状态值
    pin_state = (*state_buffer != 0) ? YDRV_PIN_SET : YDRV_PIN_RESET;

    // 调用底层yDrv设置GPIO引脚状态
    result = yDrvGpioWrite(&gpio_handle->drv_handle, pin_state);

    if (result != YDRV_OK)
    {
        return -1; // 返回错误
    }

    return sizeof(uint32_t); // 返回实际写入的字节数
}

/**
 * @brief GPIO设备控制
 */
static yDevStatus_t yDev_Gpio_Ioctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_Gpio_t *gpio_handle;
    yDrvStatus_t result;
    yDrvPinState_t pin_state;

    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    gpio_handle = (yDevHandle_Gpio_t *)handle;

    switch (cmd)
    {
    case YDEV_GPIO_SET_PIN:
        // 设置GPIO引脚为高电平
        result = yDrvGpioSet(&gpio_handle->drv_handle);
        return (result == YDRV_OK) ? YDEV_OK : YDEV_ERROR;

    case YDEV_GPIO_CLEAR_PIN:
        // 设置GPIO引脚为低电平
        result = yDrvGpioReset(&gpio_handle->drv_handle);
        return (result == YDRV_OK) ? YDEV_OK : YDEV_ERROR;

    case YDEV_GPIO_TOGGLE_PIN:
        // 翻转GPIO引脚状态
        result = yDrvGpioToggle(&gpio_handle->drv_handle);
        return (result == YDRV_OK) ? YDEV_OK : YDEV_ERROR;

    case YDEV_GPIO_GET_PIN:
        // 获取GPIO引脚状态
        if (arg != NULL)
        {
            pin_state = yDrvGpioRead(&gpio_handle->drv_handle);
            *(uint32_t *)arg = (uint32_t)pin_state;
            return YDEV_OK;
        }
        return YDEV_INVALID_PARAM;

    case YDEV_IOCTL_GET_STATUS:
        // 返回GPIO设备状态
        if (arg != NULL)
        {
            *(yDevStatus_t *)arg = YDEV_OK;
            return YDEV_OK;
        }
        return YDEV_INVALID_PARAM;

    case YDEV_IOCTL_RESET:
        // 重置GPIO设备 (可以重新初始化或设置为默认状态)
        return YDEV_OK;

    default:
        return YDEV_NOT_SUPPORTED;
    }
}

// ==================== GPIO设备操作表 ====================

/**
 * @brief 导出GPIO设备操作表
 * 使用YDEV_OPS_EXPORT_EX宏将GPIO设备操作表导出到指定段中
 */
YDEV_OPS_EXPORT_EX(
    YDEV_TYPE_GPIO,
    yDev_Gpio_Init,
    yDev_Gpio_Deinit,
    yDev_Gpio_Read,
    yDev_Gpio_Write,
    yDev_Gpio_Ioctl)

// const yDevOps_t ydev_GPIO_ops YLIB_SECTION(".ydev_ops") =
//     {
//         .type = YDEV_TYPE_GPIO,
//         .init = yDev_Gpio_Init,
//         .deinit = yDev_Gpio_Deinit,
//         .read = yDev_Gpio_Read,
//         .write = yDev_Gpio_Write,
//         .ioctl = yDev_Gpio_Ioctl};
