/**
 * @file yDev.c
 * @brief yDev设备抽象层核心实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现yDev设备抽象层的核心功能，包括设备注册、操作分发和资源管理
 *
 * @par 主要特性:
 * - 设备操作表管理和查找
 * - 统一的设备接口实现
 * - 设备生命周期管理
 * - 错误处理和状态管理
 */

// ==================== 包含文件 ====================
#include <string.h>
#include <stdlib.h>
#include "yDev.h"
#include "yLib_def.h"
#include "yDrv_basic.h"
#include "yDev_def.h"

// ==================== 设备操作表边界标记 ====================

/**
 * @brief 设备操作表结束标记
 * @note 用于标记设备操作表的结束位置，便于运行时遍历操作表
 */
const yDevOps_t ydev_end_ops YLIB_SECTION(".ydev_ops_end") =
    {
        .type = YDEV_TYPE_MAX, /*!< 设备类型标记为最大值 */
        .init = NULL,          /*!< 初始化函数指针为空 */
        .deinit = NULL,        /*!< 反初始化函数指针为空 */
        .read = NULL,          /*!< 读取函数指针为空 */
        .write = NULL,         /*!< 写入函数指针为空 */
        .ioctl = NULL,         /*!< 控制函数指针为空 */
};

/**
 * @brief 设备操作表起始标记
 * @note 用于标记设备操作表的起始位置，便于运行时遍历操作表
 */
const yDevOps_t ydev_start_ops YLIB_SECTION(".ydev_ops_start") =
    {
        .type = YDEV_TYPE_START,
        .init = NULL,
        .deinit = NULL, /*!< 反初始化函数指针为空 */
        .read = NULL,   /*!< 读取函数指针为空 */
        .write = NULL,  /*!< 写入函数指针为空 */
        .ioctl = NULL,  /*!< 控制函数指针为空 */
};

// ==================== 局部变量定义 ====================

/**
 * @brief 系统时间计数器
 * @note 用于记录系统运行时间，单位为毫秒
 */
static size_t ydev_time_ms;

// ==================== 核心API实现 ====================

/**
 * @brief yLab系统初始化
 * @retval yDevStatus_t 初始化状态
 * @retval YDEV_OK 初始化成功
 * @note 初始化yLab系统，包括底层yDrv驱动初始化
 */
yDevStatus_t yLabInit(void)
{
    // 初始化底层驱动
    yDrvInit();

    return YDEV_OK;
}

/**
 * @brief 静态初始化设备
 * @param config 设备配置参数指针
 * @param handle 设备句柄指针
 * @retval yDevStatus_t 初始化状态
 * @retval YDEV_OK 初始化成功
 * @retval YDEV_INVALID_PARAM 参数无效
 * @retval YDEV_NOT_SUPPORTED 设备类型不支持
 * @note 根据配置参数初始化指定类型的设备，查找对应的设备操作表并调用初始化函数
 */
yDevStatus_t yDevInitStatic(void *config, void *handle)
{
    yDevConfig_t *dev_config;
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    // 参数有效性检查
    if ((config == NULL) || (handle == NULL))
    {
        return YDEV_INVALID_PARAM;
    }

    dev_config = (yDevConfig_t *)config;
    dev_handle = (yDevHandle_t *)handle;
    dev_ops = (&ydev_start_ops) + 1;

    // 遍历设备操作表，查找匹配的设备类型
    for (int idx = 0; dev_ops < &ydev_end_ops; dev_ops++, idx++)
    {
        if (dev_ops->type == dev_config->type)
        {
            dev_handle->index = idx;
            dev_handle->timeOutMs = 10;
            dev_handle->errno = YDEV_ERRNO_NO_ERROR;
            if (dev_ops->init != NULL)
            {
                return dev_ops->init(config, handle);
            }
            else
            {
                return YDEV_NOT_SUPPORTED;
            }
        }
    }
    dev_handle->errno = YDEV_ERRNO_NOT_FOUND;
    return YDEV_ERROR; // 未找到匹配的设备类型
}

/**
 * @brief 反初始化设备
 * @param handle 设备句柄
 * @return yDevStatus_t 操作状态
 *
 * @par 功能描述:
 * 反初始化指定的设备，释放相关资源
 */
yDevStatus_t yDevDeinitStatic(void *handle)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    dev_handle = (yDevHandle_t *)handle;
    dev_ops = &ydev_start_ops + 1 + dev_handle->index; // 直接定位到对应的操作表

    if (dev_ops->deinit != NULL)
    {
        return dev_ops->deinit(handle);
    }
    else
    {
        return YDEV_NOT_SUPPORTED;
    }
}

/**
 * @brief 从设备读取数据
 * @param handle 设备句柄
 * @param buffer 读取缓冲区
 * @param size 期望读取的字节数
 * @return size_t 实际读取的字节数，0表示未读取到数据
 *
 * @par 功能描述:
 * 从指定设备读取数据到缓冲区，返回实际读取的字节数
 */
size_t yDevRead(void *handle, void *buffer, size_t size)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size == 0))
    {
        return 0; // 成功操作了0个数据
    }

    dev_handle = (yDevHandle_t *)handle;
    dev_ops = &ydev_start_ops + 1 + dev_handle->index; // 直接定位到对应的操作表

    if (dev_ops->read != NULL)
    {
        return dev_ops->read(handle, buffer, size);
    }
    else
    {
        return 0; // 不支持读操作
    }
}

/**
 * @brief 向设备写入数据
 * @param handle 设备句柄
 * @param buffer 写入缓冲区
 * @param size 期望写入的字节数
 * @return size_t 实际写入的字节数，0表示未写入数据
 *
 * @par 功能描述:
 * 将缓冲区的数据写入到指定设备，返回实际写入的字节数
 */
size_t yDevWrite(void *handle, const void *buffer, size_t size)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size == 0))
    {
        return 0; // 成功操作了0个数据
    }

    dev_handle = (yDevHandle_t *)handle;
    dev_ops = &ydev_start_ops + 1 + dev_handle->index; // 直接定位到对应的操作表

    if (dev_ops->write != NULL)
    {
        return dev_ops->write(handle, buffer, size);
    }
    else
    {
        return 0; // 不支持写操作
    }
}

/**
 * @brief 设备控制操作
 * @param handle 设备句柄
 * @param cmd 控制命令
 * @param arg 命令参数
 * @return yDevStatus_t 操作状态
 *
 * @par 功能描述:
 * 执行设备特定的控制操作，如配置参数、状态查询等
 */
yDevStatus_t yDevIoctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    dev_handle = (yDevHandle_t *)handle;
    dev_ops = &ydev_start_ops + 1 + dev_handle->index; // 直接定位到对应的操作表

    if (dev_ops->ioctl != NULL)
    {
        return dev_ops->ioctl(handle, cmd, arg);
    }
    else
    {
        return YDEV_NOT_SUPPORTED; // 不支持控制操作
    }
}

/**
 * @brief 设备控制操作
 * @param handle 设备句柄
 * @param cmd 控制命令
 * @param arg 命令参数
 * @return yDevStatus_t 操作状态
 *
 * @par 功能描述:
 * 执行设备特定的控制操作，如配置参数、状态查询等
 */
size_t yDevGetTimeMS(void)
{
    return ydev_time_ms;
}

// ==================== yDev结构体基础初始化函数实现 ====================

void yDevConfigStructInit(yDevConfig_t *config)
{
    if (config == NULL)
    {
        return;
    }

    config->type = YDEV_TYPE_MAX;
}

void yDevHandleStructInit(yDevHandle_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    handle->index = 0;
    handle->timeOutMs = 0;
}
