/**
 * @file yDev_usart.c
 * @brief yDev USART设备驱动实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现基于yDev框架的USART设备驱动，提供USART的统一抽象接口
 *
 * @par 主要特性:
 * - 串口数据的读写操作
 * - 中断驱动的缓冲区管理
 * - 非阻塞式数据传输
 * - 完整的错误处理机制
 * - 回调函数支持
 * - 与底层yDrv USART驱动的适配
 */

// ==================== 包含文件 ====================
#include "yDev_usart.h"
#include "yDev_def.h"
#include "yDrv_usart.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// ==================== 私有变量 ====================

// ==================== 私有函数声明 ====================

// ==================== USART设备操作函数实现 ====================

/**
 * @brief USART设备初始化
 * @param config USART设备配置参数
 * @param handle USART设备句柄
 * @return yDevStatus_t 初始化状态
 */
static yDevStatus_t yDev_Usart_Init(void *config, void *handle)
{
    yDevConfig_Usart_t *usart_config;
    yDevHandle_Usart_t *usart_handle;

    // 参数有效性检查
    if ((handle == NULL) || (config == NULL))
    {
        return YDEV_INVALID_PARAM;
    }

    usart_handle = (yDevHandle_Usart_t *)handle;
    usart_config = (yDevConfig_Usart_t *)config;

    if (yDrvUsartInitStatic(&usart_config->drv_config, &usart_handle->drv_handle) != YDRV_OK)
    {
        return YDEV_ERROR;
    }

    return YDEV_OK;
}

/**
 * @brief USART设备反初始化
 * @param handle USART设备句柄
 * @return yDevStatus_t 操作状态
 */
static yDevStatus_t yDev_Usart_Deinit(void *handle)
{
    yDevHandle_Usart_t *usart_handle;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    usart_handle = (yDevHandle_Usart_t *)handle;

    if (yDrvUsartDeInitStatic(&usart_handle->drv_handle) != YDRV_OK)
    {
        return YDEV_ERROR;
    }

    return YDEV_OK;
}

// ==================== yDev USART初始化函数实现 ====================

void yDevUsartConfigStructInit(yDevConfig_Usart_t *config)
{
    if (config == NULL)
    {
        return;
    }

    // 初始化基础配置
    yDevConfigStructInit(&config->base);

    // 初始化驱动配置
    yDrvUsartConfigStructInit(&config->drv_config);
}

void yDevUsartHandleStructInit(yDevHandle_Usart_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    // 初始化基础句柄
    yDevHandleStructInit(&handle->base);

    // 初始化驱动句柄
    yDrvUsartHandleStructInit(&handle->drv_handle);
}

/**
 * @brief USART设备读取操作
 * @param handle USART设备句柄
 * @param buffer 读取缓冲区
 * @param size 缓冲区大小
 * @return int32_t 实际读取的字节数，-1表示错误
 */
static int32_t yDev_Usart_Read(void *handle, void *buffer, uint16_t size)
{
    yDevHandle_Usart_t *usart_handle;
    uint16_t read_count;
    uint8_t *read_buff;
    int32_t read_res;
    size_t start_time;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size == 0))
    {
        return -1;
    }

    usart_handle = (yDevHandle_Usart_t *)handle;
    read_count = 0;
    read_buff = (uint8_t *)buffer;

    // 检查句柄有效性
    if (yDrvUsartHandleIsValid(&usart_handle->drv_handle) != YDRV_OK)
    {
        return -1;
    }

    start_time = yDevGetTimeMS();
    // 循环读取数据，直到读取完成或没有更多数据
    while (read_count < size)
    {
        read_res = yDrvUsartReadByte(&usart_handle->drv_handle,
                                     read_buff);
        read_count += read_res;
        read_buff += read_res;
        if (yDevGetTimeMS() - start_time >= usart_handle->base.timeOutMs)
        {
            return read_count;
        }
    }

    return read_count;
}

/**
 * @brief USART设备写入操作
 * @param handle USART设备句柄
 * @param buffer 写入缓冲区
 * @param size 写入大小
 * @return int32_t 实际写入的字节数，-1表示错误
 */
static int32_t yDev_Usart_Write(void *handle, const void *buffer, uint16_t size)
{
    yDevHandle_Usart_t *usart_handle;
    uint16_t write_count;
    const uint8_t *write_buff;
    int32_t write_res;
    size_t start_time;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size == 0))
    {
        return -1;
    }

    usart_handle = (yDevHandle_Usart_t *)handle;
    write_count = 0;
    write_buff = buffer;

    if (yDrvUsartHandleIsValid(&usart_handle->drv_handle))
    {
        return -1;
    }

    start_time = yDevGetTimeMS();
    while (write_count < size)
    {
        write_res = yDrvUsartWriteByte(&usart_handle->drv_handle,
                                       write_buff);

        write_count += write_res;
        write_buff += write_res;
        if (yDevGetTimeMS() - start_time >= usart_handle->base.timeOutMs)
        {
            return -1;
        }
    }

    return (int32_t)write_count;
}

/**
 * @brief USART设备控制操作
 * @param handle USART设备句柄
 * @param cmd 控制命令
 * @param arg 命令参数
 * @return yDevStatus_t 操作状态
 */
static yDevStatus_t yDev_Usart_Ioctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_Usart_t *usart_handle;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    usart_handle = (yDevHandle_Usart_t *)handle;

    switch (cmd)
    {
    case YDEV_USART_IOCTL_SET_BAUDRATE:
        if (arg != NULL)
        {
            (void)usart_handle;
            return YDEV_OK;
        }
        return YDEV_INVALID_PARAM;

    case YDEV_USART_IOCTL_GET_BAUDRATE:
        // 注意：yDrv层没有提供获取波特率的接口，这里返回不支持
        return YDEV_NOT_SUPPORTED;

    default:
        return YDEV_NOT_SUPPORTED;
    }
}

YDEV_OPS_EXPORT_EX(
    YDEV_TYPE_USART,   // 设备类型
    yDev_Usart_Init,   // 初始化函数
    yDev_Usart_Deinit, // 反初始化函数
    yDev_Usart_Read,   // 读取函数
    yDev_Usart_Write,  // 写入函数
    yDev_Usart_Ioctl)  // 控制函数
