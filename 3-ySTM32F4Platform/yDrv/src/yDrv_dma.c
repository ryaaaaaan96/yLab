/**
 * @file yDrv_dma.c
 * @brief STM32G0 DMA驱动程序实现
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现STM32G0系列MCU的DMA驱动程序，提供完整的DMA传输功能
 *
 * @par 主要功能:
 * - DMA通道初始化和配置
 * - 多种传输模式支持（内存到内存、内存到外设、外设到内存）
 * - 中断管理和状态查询
 * - DMAMUX请求信号配置
 * - 传输参数动态设置
 *
 * @par 更新历史:
 * - v2.0 (2025): 完整的DMA驱动实现，支持STM32G0系列
 */

#include "yDrv_dma.h"
#include <string.h>

// ==================== 私有定义 ====================

/**
 * @brief DMA通道中断号映射表
 * @note 将DMA通道枚举映射到对应的中断号
 */
static const IRQn_Type DMA_IRQ_MAP[YDRV_DMA_CHANNEL_MAX] = {
    DMA1_Channel1_IRQn,          /*!< DMA1通道1中断 */
    DMA1_Channel2_3_IRQn,        /*!< DMA1通道2_3共享中断 */
    DMA1_Channel2_3_IRQn,        /*!< DMA1通道2_3共享中断 */
    DMA1_Ch4_7_DMAMUX1_OVR_IRQn, /*!< DMA1通道4_7共享中断 */
    DMA1_Ch4_7_DMAMUX1_OVR_IRQn, /*!< DMA1通道4_7共享中断 */
    DMA1_Ch4_7_DMAMUX1_OVR_IRQn, /*!< DMA1通道4_7共享中断 */
    DMA1_Ch4_7_DMAMUX1_OVR_IRQn, /*!< DMA1通道4_7共享中断 */
};

// ==================== 私有函数声明 ====================

// ==================== 基础函数实现 ====================

yDrvStatus_t yDrvDmaInitStatic(const yDrvDmaConfig_t *config, yDrvDmaHandle_t *handle, yDrvDmaDirection_t direction)
{
    // 参数有效性检查
    if (config == NULL || handle == NULL)
    {
        return YDRV_ERROR;
    }

    // 检查通道范围
    if (config->channel >= YDRV_DMA_CHANNEL_MAX)
    {
        return YDRV_ERROR;
    }

    // 初始化句柄结构体
    yDrvParseDma(config->channel, &handle->DmaInfo);
    handle->IRQ = DMA_IRQ_MAP[config->channel];

    // 禁用DMA通道
    LL_DMA_DisableChannel(handle->DmaInfo.dma,
                          handle->DmaInfo.channel);

    LL_DMA_SetPeriphRequest(handle->DmaInfo.dma,
                            handle->DmaInfo.channel,
                            config->request); // 配置DMAMUX请求信号
    LL_DMA_SetDataTransferDirection(handle->DmaInfo.dma,
                                    handle->DmaInfo.channel,
                                    direction); // 配置传输方向
    LL_DMA_SetChannelPriorityLevel(handle->DmaInfo.dma,
                                   handle->DmaInfo.channel,
                                   config->priority); // 配置优先级
    LL_DMA_SetDataLength(handle->DmaInfo.dma,
                         handle->DmaInfo.channel,
                         (config->trans_len > 0x0000FFFF) ? 0x0000FFFF : config->trans_len); // 设置传输长度
    LL_DMA_SetMode(handle->DmaInfo.dma,
                   handle->DmaInfo.channel,
                   config->mode); // 配置模式
    switch (direction)
    {
    case YDRV_DMA_DIR_M2P:
        LL_DMA_SetMemoryIncMode(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (config->src_inc == YDRV_DMA_INC_DISABLE) ? LL_DMA_MEMORY_NOINCREMENT : LL_DMA_MEMORY_INCREMENT); // 配置优先级
        LL_DMA_SetMemorySize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             config->src_width);

        LL_DMA_SetMemorySize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             config->src_width);
        LL_DMA_SetMemoryAddress(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (uint32_t)config->src_buffer);
        break;
    case YDRV_DMA_DIR_P2M:
        LL_DMA_SetMemoryIncMode(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (config->dst_inc == YDRV_DMA_INC_DISABLE) ? LL_DMA_MEMORY_NOINCREMENT : LL_DMA_MEMORY_INCREMENT); // 配置优先级
        LL_DMA_SetMemorySize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             config->dst_width);
        LL_DMA_SetMemoryAddress(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (uint32_t)config->dst_buffer);
        break;
    case YDRV_DMA_DIR_M2M:
        LL_DMA_SetMemoryIncMode(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (config->dst_inc == YDRV_DMA_INC_DISABLE) ? LL_DMA_MEMORY_NOINCREMENT : LL_DMA_MEMORY_INCREMENT); // 配置优先级
        LL_DMA_SetPeriphIncMode(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (config->src_inc == YDRV_DMA_INC_DISABLE) ? LL_DMA_PERIPH_NOINCREMENT : LL_DMA_PERIPH_INCREMENT); // 配置优先级
        LL_DMA_SetMemorySize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             config->dst_width);
        LL_DMA_SetPeriphSize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             config->src_width);
        LL_DMA_SetMemoryAddress(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (uint32_t)config->dst_buffer);
        LL_DMA_SetPeriphAddress(handle->DmaInfo.dma,
                                handle->DmaInfo.channel,
                                (uint32_t)config->src_buffer);
        break;
    default:
        return YDRV_ERROR;
    }

    return YDRV_OK;
}

/**
 * @brief 反初始化DMA
 * @param handle DMA句柄指针
 * @retval yDrv状态
 */
yDrvStatus_t yDrvDmaDeInitStatic(yDrvDmaHandle_t *handle)
{
    // 参数有效性检查
    if (handle == NULL)
    {
        return YDRV_ERROR;
    }

    LL_DMA_DisableChannel(handle->DmaInfo.dma,
                          handle->DmaInfo.channel);

    return YDRV_OK;
}
