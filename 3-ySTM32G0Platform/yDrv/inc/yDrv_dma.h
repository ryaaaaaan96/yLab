/**
 * @file yDrv_dma.h
 * @brief STM32G0 DMA驱动程序头文件
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供STM32G0系列MCU的DMA通用驱动接口，支持多种传输模式和配置选项
 *
 * @par 主要特性:
 * - 支持DMA1的所有通道
 * - 支持内存到内存、内存到外设、外设到内存传输
 * - 支持正常模式和循环模式
 * - 完整的中断管理系统
 * - 内联优化高频操作函数
 * - 统一的错误处理和状态查询
 */

#ifndef YDRV_DMA_H
#define YDRV_DMA_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_dmamux.h"
#include "stm32g0xx_ll_bus.h"
#include "yDrv_basic.h"

    // ==================== DMA配置枚举 ====================

    /**
     * @brief DMA请求信号枚举
     * @note 基于STM32G0系列的DMAMUX请求信号定义
     */
    typedef enum
    {
        // 基础请求信号
        YDRV_DMA_REQ_MEM2MEM = LL_DMAMUX_REQ_MEM2MEM,       /*!< 内存到内存传输 */
        YDRV_DMA_REQ_GENERATOR0 = LL_DMAMUX_REQ_GENERATOR0, /*!< DMAMUX请求生成器0 */
        YDRV_DMA_REQ_GENERATOR1 = LL_DMAMUX_REQ_GENERATOR1, /*!< DMAMUX请求生成器1 */
        YDRV_DMA_REQ_GENERATOR2 = LL_DMAMUX_REQ_GENERATOR2, /*!< DMAMUX请求生成器2 */
        YDRV_DMA_REQ_GENERATOR3 = LL_DMAMUX_REQ_GENERATOR3, /*!< DMAMUX请求生成器3 */

        // ADC请求信号
        YDRV_DMA_REQ_ADC1 = LL_DMAMUX_REQ_ADC1, /*!< ADC1请求 */

    // AES请求信号（如果支持）
#if defined(AES)
        YDRV_DMA_REQ_AES_IN = LL_DMAMUX_REQ_AES_IN,   /*!< AES输入请求 */
        YDRV_DMA_REQ_AES_OUT = LL_DMAMUX_REQ_AES_OUT, /*!< AES输出请求 */
#endif                                                /* AES */

    // DAC请求信号（如果支持）
#if defined(DAC1)
        YDRV_DMA_REQ_DAC1_CH1 = LL_DMAMUX_REQ_DAC1_CH1, /*!< DAC1通道1请求 */
        YDRV_DMA_REQ_DAC1_CH2 = LL_DMAMUX_REQ_DAC1_CH2, /*!< DAC1通道2请求 */
#endif                                                  /* DAC1 */

        // I2C请求信号
        YDRV_DMA_REQ_I2C1_RX = LL_DMAMUX_REQ_I2C1_RX, /*!< I2C1接收请求 */
        YDRV_DMA_REQ_I2C1_TX = LL_DMAMUX_REQ_I2C1_TX, /*!< I2C1发送请求 */
        YDRV_DMA_REQ_I2C2_RX = LL_DMAMUX_REQ_I2C2_RX, /*!< I2C2接收请求 */
        YDRV_DMA_REQ_I2C2_TX = LL_DMAMUX_REQ_I2C2_TX, /*!< I2C2发送请求 */
#if defined(I2C3)
        YDRV_DMA_REQ_I2C3_RX = LL_DMAMUX_REQ_I2C3_RX, /*!< I2C3接收请求 */
        YDRV_DMA_REQ_I2C3_TX = LL_DMAMUX_REQ_I2C3_TX, /*!< I2C3发送请求 */
#endif                                                /* I2C3 */

    // LPUART请求信号（如果支持）
#if defined(LPUART1)
        YDRV_DMA_REQ_LPUART1_RX = LL_DMAMUX_REQ_LPUART1_RX, /*!< LPUART1接收请求 */
        YDRV_DMA_REQ_LPUART1_TX = LL_DMAMUX_REQ_LPUART1_TX, /*!< LPUART1发送请求 */
#endif                                                      /* LPUART1 */
#if defined(LPUART2)
        YDRV_DMA_REQ_LPUART2_RX = LL_DMAMUX_REQ_LPUART2_RX, /*!< LPUART2接收请求 */
        YDRV_DMA_REQ_LPUART2_TX = LL_DMAMUX_REQ_LPUART2_TX, /*!< LPUART2发送请求 */
#endif                                                      /* LPUART2 */

        // SPI请求信号
        YDRV_DMA_REQ_SPI1_RX = LL_DMAMUX_REQ_SPI1_RX, /*!< SPI1接收请求 */
        YDRV_DMA_REQ_SPI1_TX = LL_DMAMUX_REQ_SPI1_TX, /*!< SPI1发送请求 */
        YDRV_DMA_REQ_SPI2_RX = LL_DMAMUX_REQ_SPI2_RX, /*!< SPI2接收请求 */
        YDRV_DMA_REQ_SPI2_TX = LL_DMAMUX_REQ_SPI2_TX, /*!< SPI2发送请求 */
#if defined(SPI3)
        YDRV_DMA_REQ_SPI3_RX = LL_DMAMUX_REQ_SPI3_RX, /*!< SPI3接收请求 */
        YDRV_DMA_REQ_SPI3_TX = LL_DMAMUX_REQ_SPI3_TX, /*!< SPI3发送请求 */
#endif                                                /* SPI3 */

        // TIM1请求信号
        YDRV_DMA_REQ_TIM1_CH1 = LL_DMAMUX_REQ_TIM1_CH1,           /*!< TIM1通道1请求 */
        YDRV_DMA_REQ_TIM1_CH2 = LL_DMAMUX_REQ_TIM1_CH2,           /*!< TIM1通道2请求 */
        YDRV_DMA_REQ_TIM1_CH3 = LL_DMAMUX_REQ_TIM1_CH3,           /*!< TIM1通道3请求 */
        YDRV_DMA_REQ_TIM1_CH4 = LL_DMAMUX_REQ_TIM1_CH4,           /*!< TIM1通道4请求 */
        YDRV_DMA_REQ_TIM1_TRIG_COM = LL_DMAMUX_REQ_TIM1_TRIG_COM, /*!< TIM1触发和COM请求 */
        YDRV_DMA_REQ_TIM1_UP = LL_DMAMUX_REQ_TIM1_UP,             /*!< TIM1更新请求 */

    // TIM2请求信号（如果支持）
#if defined(TIM2)
        YDRV_DMA_REQ_TIM2_CH1 = LL_DMAMUX_REQ_TIM2_CH1,   /*!< TIM2通道1请求 */
        YDRV_DMA_REQ_TIM2_CH2 = LL_DMAMUX_REQ_TIM2_CH2,   /*!< TIM2通道2请求 */
        YDRV_DMA_REQ_TIM2_CH3 = LL_DMAMUX_REQ_TIM2_CH3,   /*!< TIM2通道3请求 */
        YDRV_DMA_REQ_TIM2_CH4 = LL_DMAMUX_REQ_TIM2_CH4,   /*!< TIM2通道4请求 */
        YDRV_DMA_REQ_TIM2_TRIG = LL_DMAMUX_REQ_TIM2_TRIG, /*!< TIM2触发请求 */
        YDRV_DMA_REQ_TIM2_UP = LL_DMAMUX_REQ_TIM2_UP,     /*!< TIM2更新请求 */
#endif                                                    /* TIM2 */

        // TIM3请求信号
        YDRV_DMA_REQ_TIM3_CH1 = LL_DMAMUX_REQ_TIM3_CH1,   /*!< TIM3通道1请求 */
        YDRV_DMA_REQ_TIM3_CH2 = LL_DMAMUX_REQ_TIM3_CH2,   /*!< TIM3通道2请求 */
        YDRV_DMA_REQ_TIM3_CH3 = LL_DMAMUX_REQ_TIM3_CH3,   /*!< TIM3通道3请求 */
        YDRV_DMA_REQ_TIM3_CH4 = LL_DMAMUX_REQ_TIM3_CH4,   /*!< TIM3通道4请求 */
        YDRV_DMA_REQ_TIM3_TRIG = LL_DMAMUX_REQ_TIM3_TRIG, /*!< TIM3触发请求 */
        YDRV_DMA_REQ_TIM3_UP = LL_DMAMUX_REQ_TIM3_UP,     /*!< TIM3更新请求 */

    // TIM4请求信号（如果支持）
#if defined(TIM4)
        YDRV_DMA_REQ_TIM4_CH1 = LL_DMAMUX_REQ_TIM4_CH1,   /*!< TIM4通道1请求 */
        YDRV_DMA_REQ_TIM4_CH2 = LL_DMAMUX_REQ_TIM4_CH2,   /*!< TIM4通道2请求 */
        YDRV_DMA_REQ_TIM4_CH3 = LL_DMAMUX_REQ_TIM4_CH3,   /*!< TIM4通道3请求 */
        YDRV_DMA_REQ_TIM4_CH4 = LL_DMAMUX_REQ_TIM4_CH4,   /*!< TIM4通道4请求 */
        YDRV_DMA_REQ_TIM4_TRIG = LL_DMAMUX_REQ_TIM4_TRIG, /*!< TIM4触发请求 */
        YDRV_DMA_REQ_TIM4_UP = LL_DMAMUX_REQ_TIM4_UP,     /*!< TIM4更新请求 */
#endif                                                    /* TIM4 */

    // TIM6请求信号（如果支持）
#if defined(TIM6)
        YDRV_DMA_REQ_TIM6_UP = LL_DMAMUX_REQ_TIM6_UP, /*!< TIM6更新请求 */
#endif                                                /* TIM6 */

    // TIM7请求信号（如果支持）
#if defined(TIM7)
        YDRV_DMA_REQ_TIM7_UP = LL_DMAMUX_REQ_TIM7_UP, /*!< TIM7更新请求 */
#endif                                                /* TIM7 */

    // TIM15请求信号（如果支持）
#if defined(TIM15)
        YDRV_DMA_REQ_TIM15_CH1 = LL_DMAMUX_REQ_TIM15_CH1,           /*!< TIM15通道1请求 */
        YDRV_DMA_REQ_TIM15_CH2 = LL_DMAMUX_REQ_TIM15_CH2,           /*!< TIM15通道2请求 */
        YDRV_DMA_REQ_TIM15_TRIG_COM = LL_DMAMUX_REQ_TIM15_TRIG_COM, /*!< TIM15触发和COM请求 */
        YDRV_DMA_REQ_TIM15_UP = LL_DMAMUX_REQ_TIM15_UP,             /*!< TIM15更新请求 */
#endif                                                              /* TIM15 */

        // TIM16请求信号
        YDRV_DMA_REQ_TIM16_CH1 = LL_DMAMUX_REQ_TIM16_CH1, /*!< TIM16通道1请求 */
        YDRV_DMA_REQ_TIM16_COM = LL_DMAMUX_REQ_TIM16_COM, /*!< TIM16 COM请求 */
        YDRV_DMA_REQ_TIM16_UP = LL_DMAMUX_REQ_TIM16_UP,   /*!< TIM16更新请求 */

        // TIM17请求信号
        YDRV_DMA_REQ_TIM17_CH1 = LL_DMAMUX_REQ_TIM17_CH1, /*!< TIM17通道1请求 */
        YDRV_DMA_REQ_TIM17_COM = LL_DMAMUX_REQ_TIM17_COM, /*!< TIM17 COM请求 */
        YDRV_DMA_REQ_TIM17_UP = LL_DMAMUX_REQ_TIM17_UP,   /*!< TIM17更新请求 */

        // USART请求信号
        YDRV_DMA_REQ_USART1_RX = LL_DMAMUX_REQ_USART1_RX, /*!< USART1接收请求 */
        YDRV_DMA_REQ_USART1_TX = LL_DMAMUX_REQ_USART1_TX, /*!< USART1发送请求 */
        YDRV_DMA_REQ_USART2_RX = LL_DMAMUX_REQ_USART2_RX, /*!< USART2接收请求 */
        YDRV_DMA_REQ_USART2_TX = LL_DMAMUX_REQ_USART2_TX, /*!< USART2发送请求 */
#if defined(USART3)
        YDRV_DMA_REQ_USART3_RX = LL_DMAMUX_REQ_USART3_RX, /*!< USART3接收请求 */
        YDRV_DMA_REQ_USART3_TX = LL_DMAMUX_REQ_USART3_TX, /*!< USART3发送请求 */
#endif                                                    /* USART3 */
#if defined(USART4)
        YDRV_DMA_REQ_USART4_RX = LL_DMAMUX_REQ_USART4_RX, /*!< USART4接收请求 */
        YDRV_DMA_REQ_USART4_TX = LL_DMAMUX_REQ_USART4_TX, /*!< USART4发送请求 */
#endif                                                    /* USART4 */
#if defined(USART5)
        YDRV_DMA_REQ_USART5_RX = LL_DMAMUX_REQ_USART5_RX, /*!< USART5接收请求 */
        YDRV_DMA_REQ_USART5_TX = LL_DMAMUX_REQ_USART5_TX, /*!< USART5发送请求 */
#endif                                                    /* USART5 */
#if defined(USART6)
        YDRV_DMA_REQ_USART6_RX = LL_DMAMUX_REQ_USART6_RX, /*!< USART6接收请求 */
        YDRV_DMA_REQ_USART6_TX = LL_DMAMUX_REQ_USART6_TX, /*!< USART6发送请求 */
#endif                                                    /* USART6 */

    // UCPD请求信号（如果支持）
#if defined(UCPD1)
        YDRV_DMA_REQ_UCPD1_RX = LL_DMAMUX_REQ_UCPD1_RX, /*!< UCPD1接收请求 */
        YDRV_DMA_REQ_UCPD1_TX = LL_DMAMUX_REQ_UCPD1_TX, /*!< UCPD1发送请求 */
#endif                                                  /* UCPD1 */
#if defined(UCPD2)
        YDRV_DMA_REQ_UCPD2_RX = LL_DMAMUX_REQ_UCPD2_RX, /*!< UCPD2接收请求 */
        YDRV_DMA_REQ_UCPD2_TX = LL_DMAMUX_REQ_UCPD2_TX, /*!< UCPD2发送请求 */
#endif                                                  /* UCPD2 */

    // 最大请求值（根据不同型号确定）
#if defined(STM32G0C1xx) || defined(STM32G0B1xx)
        YDRV_DMA_REQ_MAX = LL_DMAMUX_REQ_USART6_TX
#elif defined(STM32G0B0xx)
    YDRV_DMA_REQ_MAX = LL_DMAMUX_REQ_USART4_TX
#elif defined(STM32G081xx) || defined(STM32G071xx)
    YDRV_DMA_REQ_MAX = LL_DMAMUX_REQ_UCPD2_TX
#elif defined(STM32G070xx)
    YDRV_DMA_REQ_MAX = LL_DMAMUX_REQ_USART4_TX
#else
    YDRV_DMA_REQ_MAX = LL_DMAMUX_REQ_USART2_TX
#endif /* STM32G0C1xx || STM32G0B1xx */

    } yDrvDmaRequest_t;

    /**
     * @brief DMA传输方向枚举
     */
    typedef enum
    {
        YDRV_DMA_DIR_P2M = LL_DMA_DIRECTION_PERIPH_TO_MEMORY,
        YDRV_DMA_DIR_M2P = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
        YDRV_DMA_DIR_M2M = LL_DMA_DIRECTION_MEMORY_TO_MEMORY
    } yDrvDmaDirection_t;

    /**
     * @brief DMA传输模式枚举
     */
    typedef enum
    {
        YDRV_DMA_MODE_NORMAL = LL_DMA_MODE_NORMAL,
        YDRV_DMA_MODE_CIRCULAR = LL_DMA_MODE_CIRCULAR
    } yDrvDmaMode_t;

    /**
     * @brief DMA优先级枚举
     */
    typedef enum
    {
        YDRV_DMA_PRIORITY_LOW = LL_DMA_PRIORITY_LOW,
        YDRV_DMA_PRIORITY_MEDIUM = LL_DMA_PRIORITY_MEDIUM,
        YDRV_DMA_PRIORITY_HIGH = LL_DMA_PRIORITY_HIGH,
        YDRV_DMA_PRIORITY_VERY_HIGH = LL_DMA_PRIORITY_VERYHIGH
    } yDrvDmaPriority_t;

    /**
     * @brief DMA数据位宽枚举
     */
    typedef enum
    {
        YDRV_DMA_WIDTH_8BIT = LL_DMA_PDATAALIGN_BYTE,
        YDRV_DMA_WIDTH_16BIT = LL_DMA_PDATAALIGN_HALFWORD,
        YDRV_DMA_WIDTH_32BIT = LL_DMA_PDATAALIGN_WORD
    } yDrvDmaDataWidth_t;

    /**
     * @brief DMA地址递增模式枚举
     */
    typedef enum
    {
        YDRV_DMA_INC_DISABLE = 0,
        YDRV_DMA_INC_ENABLE
    } yDrvDmaIncrement_t;

    // ==================== DMA配置结构体 ====================

    /**
     * @brief DMA配置结构体
     */
    typedef struct
    {
        yDrvDmaChannel_t channel;     // DMA通道
        yDrvDmaRequest_t request;     // DMA请求信号（DMAMUX）
        yDrvDmaPriority_t priority;   // 优先级
        yDrvDmaMode_t mode;           // 传输模式
        yDrvDmaDataWidth_t src_width; // 源数据位宽
        yDrvDmaDataWidth_t dst_width; // 目标数据位宽
        yDrvDmaIncrement_t src_inc;   // 源地址递增模式
        yDrvDmaIncrement_t dst_inc;   // 目标地址递增模式
        void *src_buffer;             // 源缓冲区地址
        void *dst_buffer;             // 目标缓冲区地址
        uint32_t trans_len;           // 缓冲区传输长度
    } yDrvDmaConfig_t;

    // ==================== DMA句柄结构体 ====================

    /**
     * @brief DMA句柄结构体
     */
    typedef struct
    {
        yDrvDmaInfo_t DmaInfo; // DMA控制器信息
        IRQn_Type IRQ;         // 中断号
    } yDrvDmaHandle_t;

// ==================== DMA初始化宏定义 ====================

/**
 * @brief DMA配置结构体默认初始化宏
 * @note 提供一个安全的默认配置，适用于大多数应用场景
 */
#define YDRV_DMA_CONFIG_DEFAULT()                                    \
    ((yDrvDmaConfig_t){                                              \
        .channel = YDRV_DMA_CHANNEL_1,     /* 默认使用DMA1通道1 */   \
        .request = YDRV_DMA_REQ_MEM2MEM,   /* 默认内存到内存传输 */  \
        .priority = YDRV_DMA_PRIORITY_LOW, /* 默认低优先级 */        \
        .mode = YDRV_DMA_MODE_NORMAL,      /* 默认正常模式 */        \
        .src_width = YDRV_DMA_WIDTH_8BIT,  /* 默认8位源数据位宽 */   \
        .dst_width = YDRV_DMA_WIDTH_8BIT,  /* 默认8位目标数据位宽 */ \
        .src_inc = YDRV_DMA_INC_ENABLE,    /* 默认源地址递增 */      \
        .dst_inc = YDRV_DMA_INC_ENABLE,    /* 默认目标地址递增 */    \
        .src_buffer = NULL,                /* 源缓冲区初始为空 */    \
        .dst_buffer = NULL,                /* 目标缓冲区初始为空 */  \
    })

/**
 * @brief DMA句柄结构体默认初始化宏
 * @note 初始化为安全的默认状态
 */
#define YDRV_DMA_HANDLE_DEFAULT()                     \
    ((yDrvDmaHandle_t){                               \
        .DmaInfo = {                                  \
            .dma = NULL,  /* DMA控制器指针初始为空 */ \
            .channel = 0, /* 通道初始为0 */           \
        },                                            \
        .IRQ = 0, /* 中断号初始为0 */                 \
    })
    // ==================== DMA基础函数 ====================

    /**
     * @brief 初始化DMA
     * @param config DMA配置指针
     * @param handle DMA句柄指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvDmaInitStatic(const yDrvDmaConfig_t *config, yDrvDmaHandle_t *handle, yDrvDmaDirection_t direction);

    /**
     * @brief 反初始化DMA
     * @param handle DMA句柄指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvDmaDeInitStatic(yDrvDmaHandle_t *handle);

    /**
     * @brief 启动DMA传输
     * @param handle DMA句柄指针
     * @param transfer_config 传输配置指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvDmaSrcBufferSet(yDrvDmaHandle_t *handle, void *src_buf, yDrvDmaDataWidth_t len)
    {
        LL_DMA_SetPeriphAddress(handle->DmaInfo.dma, handle->DmaInfo.channel, (uint32_t)src_buf);
        LL_DMA_SetPeriphSize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             len);
        return YDRV_OK;
    }

    YLIB_INLINE yDrvStatus_t yDrvDmaDstBufferSet(yDrvDmaHandle_t *handle, void *dst_buf, yDrvDmaDataWidth_t len)
    {
        LL_DMA_SetMemoryAddress(handle->DmaInfo.dma, handle->DmaInfo.channel, (uint32_t)dst_buf);
        LL_DMA_SetMemorySize(handle->DmaInfo.dma,
                             handle->DmaInfo.channel,
                             len);
        return YDRV_OK;
    }

    YLIB_INLINE yDrvStatus_t yDrvDmaDstBufferLen(yDrvDmaHandle_t *handle, uint32_t len)
    {
        LL_DMA_SetDataLength(handle->DmaInfo.dma, handle->DmaInfo.channel, len);
        return YDRV_OK;
    }

    /**
     * @brief 暂停DMA传输
     * @param handle DMA句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvDmaTransDisable(yDrvDmaHandle_t *handle)
    {
        LL_DMA_DisableChannel(handle->DmaInfo.dma, handle->DmaInfo.channel);
        return YDRV_OK;
    }

    /**
     * @brief 恢复DMA传输
     * @param handle DMA句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvDmaTransEnable(yDrvDmaHandle_t *handle)
    {
        LL_DMA_EnableChannel(handle->DmaInfo.dma, handle->DmaInfo.channel);
        return YDRV_OK;
    }

    /**
     * @brief 恢复DMA传输
     * @param handle DMA句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE uint32_t yDrvDmaCurLenGet(yDrvDmaHandle_t *handle)
    {
        return LL_DMA_GetDataLength(handle->DmaInfo.dma, handle->DmaInfo.channel);
    }

#ifdef __cplusplus
}
#endif

#endif /* YDRV_DMA_H */