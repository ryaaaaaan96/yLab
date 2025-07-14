/**
 * @file yDev_gpio.c
 * @brief yDev GPIO设备驱动实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现基于yDev框架的GPIO设备驱动，提供GPIO的统一抽象接口
 *
 * @par 主要特性:
 * - GPIO引脚的读写操作
 * - GPIO状态控制和查询
 * - 外部中断注册和处理
 * - 与底层yDrv GPIO驱动的适配
 */

// ==================== 包含文件 ====================
#include "yDev_gpio.h"
#include "yDev_def.h"
#include "yDrv_gpio.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// ==================== GPIO设备操作函数 ====================

/**
 * @brief GPIO设备初始化
 * @param config GPIO设备配置参数
 * @param handle GPIO设备句柄
 * @return yDevStatus_t 初始化状态
 *
 * @par 功能描述:
 * 初始化GPIO设备，配置引脚模式、上下拉、速度等参数
 */
static yDevStatus_t yDev_Gpio_Init(void *config, void *handle)
{
    yDevConfig_Gpio_t *gpio_config;
    yDevHandle_Gpio_t *gpio_handle;

    // 参数有效性检查
    if ((handle == NULL) || (config == NULL))
    {
        return YDEV_INVALID_PARAM;
    }

    gpio_handle = (yDevHandle_Gpio_t *)handle;
    gpio_config = (yDevConfig_Gpio_t *)config;

    // 调用底层yDrv初始化GPIO
    if (yDrvGpioInitStatic(&gpio_config->drv_config, &gpio_handle->drv_handle) != YDRV_OK)
    {
        return YDEV_ERROR;
    }

    return YDEV_OK;
}

/**
 * @brief GPIO设备反初始化
 * @param handle GPIO设备句柄
 * @return yDevStatus_t 操作状态
 *
 * @par 功能描述:
 * 反初始化GPIO设备，释放相关资源
 */
static yDevStatus_t yDev_Gpio_Deinit(void *handle)
{
    yDevHandle_Gpio_t *gpio_handle;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    gpio_handle = (yDevHandle_Gpio_t *)handle;

    // 调用底层yDrv反初始化GPIO
    if (yDrvGpioDeInitStatic(&gpio_handle->drv_handle) != YDRV_OK)
    {
        return YDEV_ERROR;
    }

    return YDEV_OK;
}

// ==================== yDev GPIO初始化函数实现 ====================

void yDevGpioConfigStructInit(yDevConfig_Gpio_t *config)
{
    if (config == NULL)
    {
        return;
    }

    // 初始化基础配置
    yDevConfigStructInit(&config->base);

    // 初始化驱动配置
    yDrvGpioConfigStructInit(&config->drv_config);
}

void yDevGpioHandleStructInit(yDevHandle_Gpio_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    // 初始化基础句柄
    yDevHandleStructInit(&handle->base);

    // 初始化驱动句柄
    yDrvGpioHandleStructInit(&handle->drv_handle);
}

/**
 * @brief GPIO设备读取操作
 * @param handle GPIO设备句柄
 * @param buffer 读取缓冲区
 * @param size 缓冲区大小
 * @return int32_t 实际读取的字节数，-1表示错误
 *
 * @par 功能描述:
 * 读取GPIO引脚的当前状态，将状态值写入缓冲区
 * 状态值：0表示低电平，非0表示高电平
 */
static int32_t yDev_Gpio_Read(void *handle, void *buffer, uint16_t size)
{
    yDevHandle_Gpio_t *gpio_handle;
    uint32_t *state_buffer;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size != sizeof(uint32_t)))
    {
        return -1; // 返回错误
    }

    gpio_handle = (yDevHandle_Gpio_t *)handle;
    state_buffer = (uint32_t *)buffer;

    // 调用底层yDrv读取GPIO引脚状态
    *state_buffer = (uint32_t)yDrvGpioRead(&gpio_handle->drv_handle);

    return sizeof(uint32_t); // 返回实际读取的字节数
}

/**
 * @brief GPIO设备写入操作
 * @param handle GPIO设备句柄
 * @param buffer 写入缓冲区
 * @param size 缓冲区大小
 * @return int32_t 实际写入的字节数，-1表示错误
 *
 * @par 功能描述:
 * 设置GPIO引脚的输出状态
 * 写入值：0设置为低电平，非0设置为高电平
 */
static int32_t yDev_Gpio_Write(void *handle, const void *buffer, uint16_t size)
{
    yDevHandle_Gpio_t *gpio_handle;
    const uint32_t *state_buffer;
    yDrvPinState_t pin_state;
    yDrvStatus_t result;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size != sizeof(uint32_t)))
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
 * @brief GPIO设备控制操作
 * @param handle GPIO设备句柄
 * @param cmd 控制命令
 * @param arg 命令参数
 * @return yDevStatus_t 操作状态
 *
 * @par 功能描述:
 * 执行GPIO设备的特定控制操作，包括设置、清除、翻转引脚状态，
 * 获取引脚状态，注册外部中断等
 *
 * @par 支持的命令:
 * - YDEV_GPIO_SET_PIN: 设置引脚为高电平
 * - YDEV_GPIO_CLEAR_PIN: 设置引脚为低电平
 * - YDEV_GPIO_TOGGLE_PIN: 翻转引脚状态
 * - YDEV_GPIO_GET_PIN: 获取引脚状态
 * - YDEV_GPIO_REGISTER_EXIT: 注册外部中断回调
 * - YDEV_IOCTL_GET_STATUS: 获取设备状态
 * - YDEV_IOCTL_RESET: 重置设备
 */
static yDevStatus_t yDev_Gpio_Ioctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_Gpio_t *gpio_handle;
    yDrvStatus_t result;
    yDrvPinState_t pin_state;

    // 参数有效性检查
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
        // 重置GPIO设备（可以重新初始化或设置为默认状态）
        return YDEV_OK;

    case YDEV_GPIO_REGISTER_EXIT:
        // 注册GPIO外部中断回调
        if (yDrvGpioRegisterCallback(&gpio_handle->drv_handle, arg) == YDRV_OK)
        {
            return YDEV_OK;
        }
        return YDEV_ERROR;

    default:
        return YDEV_NOT_SUPPORTED;
    }
}

// ==================== GPIO设备操作表导出 ====================

/**
 * @brief 导出GPIO设备操作表
 *
 * @par 功能描述:
 * 使用YDEV_OPS_EXPORT_EX宏将GPIO设备操作表导出到指定段中，
 * 供yDev核心模块在运行时发现和使用
 */
YDEV_OPS_EXPORT_EX(
    YDEV_TYPE_GPIO,   // 设备类型
    yDev_Gpio_Init,   // 初始化函数
    yDev_Gpio_Deinit, // 反初始化函数
    yDev_Gpio_Read,   // 读取函数
    yDev_Gpio_Write,  // 写入函数
    yDev_Gpio_Ioctl)  // 控制函数
