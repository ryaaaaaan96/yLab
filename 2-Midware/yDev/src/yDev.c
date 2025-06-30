/**
 * @file yDev_core.c
 * @brief yDev设备抽象层核心实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 */

#include <string.h>
#include <stdlib.h>

#include "yDev.h"
#include "yLib_def.h"
#include "yDrv_basic.h"

// 设备的起始和结束
const yDevOps_t ydev_end_ops YLIB_SECTION(".ydev_ops_end") =
    {
        .type = YDEV_TYPE_MAX,
        .init = NULL,
        .deinit = NULL,
        .read = NULL,
        .write = NULL,
        .ioctl = NULL,
};
const yDevOps_t ydev_start_ops YLIB_SECTION(".ydev_ops_start") =
    {
        .type = YDEV_TYPE_START,
        .init = NULL,
        .deinit = NULL,
        .read = NULL,
        .write = NULL,
        .ioctl = NULL,
};

// ==================== 核心API实现 ====================

/**
 * @brief 创建设备句柄
 */
yDevStatus_t yLabInit(void)
{
    yDrvInit();

    return YDEV_OK;
}

/**
 * @brief 创建设备句柄
 */
yDevStatus_t yDevInitStatic(void *config, void *handle)
{
    yDevConfig_t *dev_config;
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    if ((config == NULL) || (handle == NULL))
    {
        return YDEV_INVALID_PARAM;
    }

    dev_config = (yDevConfig_t *)config;
    dev_handle = (yDevHandle_t *)handle;
    dev_ops = (&ydev_start_ops) + 1;

    for (int idx = 0; dev_ops < &ydev_end_ops; dev_ops++, idx++)
    {
        if (dev_ops->type == dev_config->type)
        {
            dev_handle->index = idx;
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
    return YDEV_ERROR; // 未找到匹配的设备类型
}

/**
 * @brief 销毁设备句柄
 */
yDevStatus_t yDevDeinitStatic(void *handle)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    dev_handle = (yDevHandle_t *)handle;
    dev_ops = &ydev_start_ops + 1 + dev_handle->index; // 跳过起始标记

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
 */
size_t yDevRead(void *handle, void *buffer, size_t size)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

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
 */
size_t yDevWrite(void *handle, const void *buffer, size_t size)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

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
 */
yDevStatus_t yDevIoctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_t *dev_handle;
    const yDevOps_t *dev_ops;

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
