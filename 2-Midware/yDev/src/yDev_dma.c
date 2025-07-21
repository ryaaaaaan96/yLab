/**
 * @file yDev_dma.c
 * @brief yDev DMA设备驱动实现
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 */

#include "yDev_dma.h"
#include <string.h>

// ==================== 基础函数实现 ====================

static yDevStatus_t yDevDmaInitStatic(void *config, void *handle)
{
    yDrvStatus_t drv_status;
    yDevConfig_Dma_t *dma_config;
    yDevHandle_Dma_t *dma_handle;

    // 参数有效性检查
    if (config == NULL || handle == NULL)
    {
        return YDEV_ERROR;
    }

    dma_config = (yDevConfig_Dma_t *)config;
    dma_handle = (yDevHandle_Dma_t *)handle;

    // 初始化底层DMA驱动
    drv_status = yDrvDmaInitStatic(&dma_config->drv_config, &dma_handle->drv_handle);
    if (drv_status != YDRV_OK)
    {
        dma_handle->base.errno = YDEV_ERRNO_NOT_INIT;
        return YDEV_ERROR;
    }

    return YDEV_OK;
}

static yDevStatus_t yDevDmaDeInitStatic(void *handle)
{
    yDevHandle_Dma_t *dma_handle;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    dma_handle = (yDevHandle_Dma_t *)handle;

    // 调用底层yDrv反初始化DMA
    if (yDrvDmaDeInitStatic(&dma_handle->drv_handle) != YDRV_OK)
    {
        dma_handle->base.errno = YDEV_ERRNO_NOT_DEINIT;
        return YDEV_ERROR;
    }

    return YDEV_OK;
}

// ==================== yDev DMA初始化函数实现 ====================

void yDevDmaConfigStructInit(yDevConfig_Dma_t *config)
{
    if (config == NULL)
    {
        return;
    }

    // 初始化基础配置
    yDevConfigStructInit(&config->base);

    // 初始化驱动配置
    yDrvDmaConfigStructInit(&config->drv_config);
}

void yDevDmaHandleStructInit(yDevHandle_Dma_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    // 初始化基础句柄
    yDevHandleStructInit(&handle->base);

    // 初始化驱动句柄
    yDrvDmaHandleStructInit(&handle->drv_handle);
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
static yDevStatus_t yDevDmaWrite(void *handle, void *buffer, uint16_t size)
{
    yDevHandle_Dma_t *dma_handle;
    uint32_t *data_buffer;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size != sizeof(uint32_t)))
    {
        return -1; // 返回错误
    }

    dma_handle = (yDevHandle_Dma_t *)handle;
    data_buffer = (uint32_t *)buffer;

    // 调用底层yDrv读取DMA数据
    *data_buffer = (uint32_t)
        yDrvDmaRead(&dma_handle->drv_handle);

    return sizeof(uint32_t); // 返回实际读取的字节数
}

static yDevStatus_t yDevDmaRead(void *handle, const void *buffer, uint16_t size)
{
    yDevHandle_Dma_t *dma_handle = (yDevHandle_Dma_t *)handle;

    // 参数有效性检查
    if (dma_handle == NULL || buffer == NULL || size == 0)
    {
        return YDEV_ERROR;
    }

    // 检查设备状态
    if (dma_handle->base.state != YDEV_STATE_READY)
    {
        dma_handle->base.errno = YDEV_DMA_ERRNO_NOT_INIT;
        return YDEV_ERROR;
    }

    // TODO: 实现DMA读取操作
    // 这里应该配置DMA传输，从源地址读取数据到buffer

    return YDEV_OK;
}

static yDevStatus_t yDevDmaIoctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_Dma_t *dma_handle = (yDevHandle_Dma_t *)handle;

    // 参数有效性检查
    if (dma_handle == NULL)
    {
        return YDEV_ERROR;
    }

    switch (cmd)
    {
    case YDEV_DMA_IOCTL_START:
        return yDevDmaStartTransfer(dma_handle, (const yDevDmaTransferConfig_t *)arg);

    case YDEV_DMA_IOCTL_STOP:
        return yDevDmaStopTransfer(dma_handle);

    case YDEV_DMA_IOCTL_PAUSE:
        return yDevDmaPauseTransfer(dma_handle);

    case YDEV_DMA_IOCTL_RESUME:
        return yDevDmaResumeTransfer(dma_handle);

    case YDEV_DMA_IOCTL_GET_STATUS:
        return yDevDmaGetStatus(dma_handle, (yDevDmaStatus_t *)arg);

    case YDEV_DMA_IOCTL_GET_REMAINING:
        if (arg != NULL)
        {
            *(uint32_t *)arg = yDevDmaGetRemainingCount(dma_handle);
            return YDEV_OK;
        }
        return YDEV_ERROR;

    case YDEV_DMA_IOCTL_SET_CALLBACK:
        return yDevDmaSetCallback(dma_handle, (const yDevDmaCallbackConfig_t *)arg);

    case YDEV_DMA_IOCTL_CLEAR_ERRORS:
        return yDevDmaClearErrors(dma_handle, *(uint32_t *)arg);

    case YDEV_DMA_IOCTL_GET_ERRORS:
        if (arg != NULL)
        {
            *(uint32_t *)arg = yDevDmaGetErrors(dma_handle);
            return YDEV_OK;
        }
        return YDEV_ERROR;

    default:
        dma_handle->base.errno = YDEV_DMA_ERRNO_INVALID_PARAM;
        return YDEV_ERROR;
    }
}

// ==================== 私有函数实现 ====================

static void prv_DmaCompleteCallback(void *arg)
{
    yDevHandle_Dma_t *handle = (yDevHandle_Dma_t *)arg;

    if (handle == NULL)
    {
        return;
    }

    // 设置完成标志（这是正常的传输完成，不是错误）
    dma_handle->errors |= YDEV_DMA_ERRNO_FULL_TRANSFER;
    dma_handle->base.errno |= YDEV_DMA_ERRNO_FULL_TRANSFER;

    // 设置设备状态为就绪
    dma_handle->base.state = YDEV_STATE_READY;

    // 调用用户回调
    if (dma_handle->callbacks.complete_callback != NULL)
    {
        dma_handle->callbacks.complete_callback(dma_handle->callbacks.callback_arg);
    }
}

static void prv_DmaHalfCompleteCallback(void *arg)
{
    yDevHandle_Dma_t *handle = (yDevHandle_Dma_t *)arg;

    if (handle == NULL)
    {
        return;
    }

    // 设置半完成标志（这是正常的半传输完成，不是错误）
    dma_handle->errors |= YDEV_DMA_ERRNO_HALF_TRANSFER;
    dma_handle->base.errno |= YDEV_DMA_ERRNO_HALF_TRANSFER;

    // 调用用户回调
    if (dma_handle->callbacks.half_complete_callback != NULL)
    {
        dma_handle->callbacks.half_complete_callback(dma_handle->callbacks.callback_arg);
    }
}

static void prv_DmaErrorCallback(void *arg)
{
    yDevHandle_Dma_t *handle = (yDevHandle_Dma_t *)arg;

    if (handle == NULL)
    {
        return;
    }

    // 设置错误标志
    dma_handle->errors |= YDEV_DMA_ERRNO_TRANSFER_ERROR;
    dma_handle->base.errno |= YDEV_DMA_ERRNO_TRANSFER_ERROR;
    dma_handle->error_count++;

    // 设置设备状态为错误
    dma_handle->base.state = YDEV_STATE_ERROR;

    // 调用用户回调
    if (dma_handle->callbacks.error_callback != NULL)
    {
        dma_handle->callbacks.error_callback(dma_handle->callbacks.callback_arg);
    }
}

static yDevStatus_t prv_ValidateTransferConfig(const yDevDmaTransferConfig_t *config)
{
    // 检查地址是否为空
    if (config->src_addr == 0 || config->dst_addr == 0)
    {
        return YDEV_ERROR;
    }

    // 检查数据长度
    if (config->data_length == 0)
    {
        return YDEV_ERROR;
    }

    // 检查地址对齐
    switch (config->src_width)
    {
    case YDRV_DMA_WIDTH_16BIT:
        if (config->src_addr % 2 != 0)
            return YDEV_ERROR;
        break;
    case YDRV_DMA_WIDTH_32BIT:
        if (config->src_addr % 4 != 0)
            return YDEV_ERROR;
        break;
    default:
        break;
    }

    switch (config->dst_width)
    {
    case YDRV_DMA_WIDTH_16BIT:
        if (config->dst_addr % 2 != 0)
            return YDEV_ERROR;
        break;
    case YDRV_DMA_WIDTH_32BIT:
        if (config->dst_addr % 4 != 0)
            return YDEV_ERROR;
        break;
    default:
        break;
    }

    return YDEV_OK;
}

// ==================== 扩展功能函数实现 ====================

/**
 * @brief 启动DMA传输
 * @param handle DMA设备句柄指针
 * @param config 传输配置指针
 * @retval yDev状态
 */
yDevStatus_t yDevDmaStartTransfer(yDevHandle_Dma_t *handle, const yDevDmaTransferConfig_t *config)
{
    if (handle == NULL || config == NULL)
    {
        return YDEV_ERROR;
    }

    // 验证传输配置
    if (prv_ValidateTransferConfig(config) != YDEV_OK)
    {
        handle->base.errno = YDEV_DMA_ERRNO_INVALID_PARAM;
        return YDEV_ERROR;
    }

    // 检查设备状态
    if (handle->base.state != YDEV_STATE_READY)
    {
        handle->base.errno = YDEV_DMA_ERRNO_NOT_INIT;
        return YDEV_ERROR;
    }

    // 设置设备为忙碌状态
    handle->base.state = YDEV_STATE_BUSY;

    // TODO: 调用底层驱动启动传输
    // yDrvStatus_t drv_status = yDrvDmaStartTransfer(&handle->drv_handle, config);

    return YDEV_OK;
}

/**
 * @brief 停止DMA传输
 * @param handle DMA设备句柄指针
 * @retval yDev状态
 */
yDevStatus_t yDevDmaStopTransfer(yDevHandle_Dma_t *handle)
{
    if (handle == NULL)
    {
        return YDEV_ERROR;
    }

    // TODO: 调用底层驱动停止传输
    // yDrvDmaStopTransfer(&handle->drv_handle);

    // 设置设备状态为就绪
    handle->base.state = YDEV_STATE_READY;

    return YDEV_OK;
}

/**
 * @brief 暂停DMA传输
 * @param handle DMA设备句柄指针
 * @retval yDev状态
 */
yDevStatus_t yDevDmaPauseTransfer(yDevHandle_Dma_t *handle)
{
    if (handle == NULL)
    {
        return YDEV_ERROR;
    }

    // TODO: 调用底层驱动暂停传输
    // yDrvDmaPauseTransfer(&handle->drv_handle);

    return YDEV_OK;
}

/**
 * @brief 恢复DMA传输
 * @param handle DMA设备句柄指针
 * @retval yDev状态
 */
yDevStatus_t yDevDmaResumeTransfer(yDevHandle_Dma_t *handle)
{
    if (handle == NULL)
    {
        return YDEV_ERROR;
    }

    // TODO: 调用底层驱动恢复传输
    // yDrvDmaResumeTransfer(&handle->drv_handle);

    return YDEV_OK;
}

/**
 * @brief 获取DMA传输状态
 * @param handle DMA设备句柄指针
 * @param status 状态输出指针
 * @retval yDev状态
 */
yDevStatus_t yDevDmaGetStatus(yDevHandle_Dma_t *handle, yDevDmaStatus_t *status)
{
    if (handle == NULL || status == NULL)
    {
        return YDEV_ERROR;
    }

    // TODO: 从底层驱动获取状态
    // *status = yDrvDmaGetStatus(&handle->drv_handle);

    return YDEV_OK;
}

/**
 * @brief 获取DMA剩余传输数量
 * @param handle DMA设备句柄指针
 * @retval 剩余传输数量
 */
uint32_t yDevDmaGetRemainingCount(yDevHandle_Dma_t *handle)
{
    if (handle == NULL)
    {
        return 0;
    }

    // TODO: 从底层驱动获取剩余传输数量
    // return yDrvDmaGetRemainingCount(&handle->drv_handle);

    return 0;
}

/**
 * @brief 设置DMA回调函数
 * @param handle DMA设备句柄指针
 * @param callbacks 回调函数配置指针
 * @retval yDev状态
 */
yDevStatus_t yDevDmaSetCallback(yDevHandle_Dma_t *handle, const yDevDmaCallbackConfig_t *callbacks)
{
    if (handle == NULL || callbacks == NULL)
    {
        return YDEV_ERROR;
    }

    // 复制回调函数配置
    memcpy(&handle->callbacks, callbacks, sizeof(yDevDmaCallbackConfig_t));

    return YDEV_OK;
}

/**
 * @brief 清除DMA错误标志
 * @param handle DMA设备句柄指针
 * @param error_mask 错误掩码
 * @retval yDev状态
 */
yDevStatus_t yDevDmaClearErrors(yDevHandle_Dma_t *handle, uint32_t error_mask)
{
    if (handle == NULL)
    {
        return YDEV_ERROR;
    }

    // 清除指定的错误标志
    handle->errors &= ~error_mask;
    handle->base.errno &= ~error_mask;

    // 如果没有错误了，恢复设备状态
    if (handle->errors == YDEV_DMA_ERRNO_NONE)
    {
        if (handle->base.state == YDEV_STATE_ERROR)
        {
            handle->base.state = YDEV_STATE_READY;
        }
    }

    return YDEV_OK;
}

/**
 * @brief 获取DMA错误标志
 * @param handle DMA设备句柄指针
 * @retval 错误标志
 */
uint32_t yDevDmaGetErrors(yDevHandle_Dma_t *handle)
{
    if (handle == NULL)
    {
        return YDEV_DMA_ERRNO_INVALID_PARAM;
    }

    return handle->errors;
}