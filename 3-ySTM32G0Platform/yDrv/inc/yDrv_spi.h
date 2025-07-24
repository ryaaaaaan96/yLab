/**
 * @file yDrv_spi.h
 * @brief STM32G0 SPI驱动程序头文件
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供STM32G0系列MCU的SPI通用驱动接口，支持轮询和DMA模式
 *
 * @par 主要特性:
 * - 支持SPI1/SPI2
 * - 支持主/从模式
 * - 支持全双工、单向接收、单向发送
 * - 支持DMA传输
 * - 统一的错误处理和状态查询
 */

#ifndef YDRV_SPI_H
#define YDRV_SPI_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "stm32g0xx_ll_spi.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_bus.h"

#include "yDrv_basic.h"

    // ==================== SPI配置枚举 ====================

    /**
     * @brief SPI实例ID枚举
     */
    typedef enum
    {
        YDRV_SPI_SOFT = 0, /*!< 软件模拟SPI */
        YDRV_SPI_1,
        YDRV_SPI_2,
        YDRV_SPI_MAX
    } yDrvSpiId_t;

    /**
     * @brief SPI模式枚举
     */
    typedef enum
    {
        YDRV_SPI_MODE_SLAVE = LL_SPI_MODE_SLAVE,
        YDRV_SPI_MODE_MASTER = LL_SPI_MODE_MASTER
    } yDrvSpiMode_t;

    /**
     * @brief SPI传输方向枚举
     */
    typedef enum
    {
        YDRV_SPI_DIR_2LINES_FULL_DUPLEX = LL_SPI_FULL_DUPLEX, /*!< 全双工，2线模式 */
        YDRV_SPI_DIR_1LINES_RX_ONLY = LL_SPI_SIMPLEX_RX,      /*!< 单工接收，2线模式 */
        YDRV_SPI_DIR_1LINE_RX = LL_SPI_HALF_DUPLEX_RX,        /*!< 半双工接收，1线模式 */
        YDRV_SPI_DIR_1LINE_TX = LL_SPI_HALF_DUPLEX_TX         /*!< 半双工发送，1线模式 */
    } yDrvSpiDirection_t;

    /**
     * @brief SPI时钟极性 (CPOL)
     */
    typedef enum
    {
        YDRV_SPI_POLARITY_LOW = LL_SPI_POLARITY_LOW,
        YDRV_SPI_POLARITY_HIGH = LL_SPI_POLARITY_HIGH
    } yDrvSpiClockPolarity_t;

    /**
     * @brief SPI时钟相位 (CPHA)
     */
    typedef enum
    {
        YDRV_SPI_PHASE_1EDGE = LL_SPI_PHASE_1EDGE,
        YDRV_SPI_PHASE_2EDGE = LL_SPI_PHASE_2EDGE
    } yDrvSpiClockPhase_t;

    /**
     * @brief SPI从机选择 (NSS) 模式
     */
    typedef enum
    {
        YDRV_SPI_CS_SOFT = LL_SPI_NSS_SOFT,
        YDRV_SPI_CS_HARD_INPUT = LL_SPI_NSS_HARD_INPUT,
        YDRV_SPI_CS_HARD_OUTPUT = LL_SPI_NSS_HARD_OUTPUT
    } yDrvSpiCsMode_t;

    /**
     * @brief SPI波特率分频系数
     */
    typedef enum
    {
        YDRV_SPI_SPEED_LEVEL0 = LL_SPI_BAUDRATEPRESCALER_DIV256,
        YDRV_SPI_SPEED_LEVEL1 = LL_SPI_BAUDRATEPRESCALER_DIV128,
        YDRV_SPI_SPEED_LEVEL2 = LL_SPI_BAUDRATEPRESCALER_DIV64,
        YDRV_SPI_SPEED_LEVEL3 = LL_SPI_BAUDRATEPRESCALER_DIV32,
        YDRV_SPI_SPEED_LEVEL4 = LL_SPI_BAUDRATEPRESCALER_DIV16,
        YDRV_SPI_SPEED_LEVEL5 = LL_SPI_BAUDRATEPRESCALER_DIV8,
        YDRV_SPI_SPEED_LEVEL6 = LL_SPI_BAUDRATEPRESCALER_DIV4,
        YDRV_SPI_SPEED_LEVEL7 = LL_SPI_BAUDRATEPRESCALER_DIV2,
    } yDrvSpiSpeedLevel_t;

    /**
     * @brief SPI位传输顺序
     */
    typedef enum
    {
        YDRV_SPI_BITORDER_MSB = LL_SPI_MSB_FIRST,
        YDRV_SPI_BITORDER_LSB = LL_SPI_LSB_FIRST
    } yDrvSpiBitOrder_t;

    // ==================== SPI配置和句柄结构体 ====================

    /**
     * @brief SPI配置结构体
     */
    typedef struct
    {
        yDrvSpiId_t spiId;

        yDrvSpiMode_t mode;
        yDrvSpiDirection_t direction;
        uint16_t dataBits;
        uint16_t crc;
        yDrvSpiClockPolarity_t polarity;
        yDrvSpiClockPhase_t phase;
        yDrvSpiCsMode_t csMode;
        yDrvSpiSpeedLevel_t speed;
        yDrvSpiBitOrder_t bitOrder;

        yDrvGpioPin_t sckPin;
        yDrvGpioPin_t misoPin;
        yDrvGpioPin_t mosiPin;
        yDrvGpioPin_t csPin;

        uint32_t sckAF;
        uint32_t misoAF;
        uint32_t mosiAF;
        uint32_t csAF;
    } yDrvSpiConfig_t;

    /**
     * @brief SPI句柄结构体
     */
    typedef struct
    {
        SPI_TypeDef *instance;
        IRQn_Type IRQ;
        yDrvSpiId_t spiId;
        yDrvGpioInfo_t sckPinInfo;
        yDrvGpioInfo_t misoPinInfo;
        yDrvGpioInfo_t mosiPinInfo;
        yDrvGpioInfo_t csPinInfo;
        uint32_t flagBtyeSend;
    } yDrvSpiHandle_t;

    // ==================== 公共函数声明 ====================

    /**
     * @brief 初始化SPI
     * @param handle SPI句柄指针
     * @param config 配置参数指针（包含SPI ID和引脚配置）
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvSpiInitStatic(yDrvSpiConfig_t *config, yDrvSpiHandle_t *handle);

    /**
     * @brief 反初始化SPI
     * @param handle SPI句柄指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvSpiDeInitStatic(yDrvSpiHandle_t *handle);

    /**
     * @brief 初始化SPI配置结构体为默认值
     * @param config 配置结构体指针
     * @retval 无
     */
    void yDrvSpiConfigStructInit(yDrvSpiConfig_t *config);

    /**
     * @brief 初始化SPI句柄结构体为默认值
     * @param handle 句柄结构体指针
     * @retval 无
     */
    void yDrvSpiHandleStructInit(yDrvSpiHandle_t *handle);

    // ==================== SPI数据传输函数 ====================

    /**
     * @brief 发送单个字节（非阻塞，内联优化）
     * @param handle SPI句柄指针
     * @param data 要发送的字节
     * @retval yDrv状态
     */
    YLIB_INLINE int32_t yDrvSpiWriteByte(yDrvSpiHandle_t *handle, const void *data)
    {
        if (!LL_SPI_IsActiveFlag_TXE(handle->instance))
        {
            return 0;
        }

        // 根据字节大小发送数据
        if (handle->flagBtyeSend == 0)
        {
            LL_SPI_TransmitData8(handle->instance, *((const uint8_t *)data));
            return 1;
        }
        else
        {
            LL_SPI_TransmitData16(handle->instance, *((const uint16_t *)data));
            return 2;
        }
    }

    /**
     * @brief 接收单个字节（非阻塞，内联优化）
     * @param handle USART句柄指针
     * @param pData 接收数据指针
     * @retval yDrv状态
     */
    YLIB_INLINE int32_t yDrvSpiReadByte(yDrvSpiHandle_t *handle, void *pData)
    {
        if (!LL_SPI_IsActiveFlag_RXNE(handle->instance))
        {
            return 0;
        }

        if (handle->flagBtyeSend == 0)
        {
            *((uint8_t *)pData) = LL_SPI_ReceiveData8(handle->instance);
            return 1;
        }
        else
        {
            *((uint16_t *)pData) = LL_SPI_ReceiveData16(handle->instance);
            return 2;
        }
    }

    /**
     * @brief 接收单个字节（非阻塞，内联优化）
     * @param handle USART句柄指针
     * @param pData 接收数据指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvSpiCsControl(yDrvSpiHandle_t *handle, uint8_t state)
    {
        if (handle == NULL || handle->csPinInfo.flag == 0)
        {
            return YDRV_INVALID_PARAM;
        }

        if (state == 0)
        {
            LL_GPIO_SetOutputPin(handle->csPinInfo.port, handle->csPinInfo.pinMask);
        }
        else
        {
            LL_GPIO_ResetOutputPin(handle->csPinInfo.port, handle->csPinInfo.pinMask);
        }

        return YDRV_OK;
    }

    // ==================== SPI状态查询函数（内联优化） ====================

    /**
     * @brief 检查发送器是否为空（可以发送新数据）
     * @param handle SPI句柄指针
     * @retval YDRV_OK=为空, YDRV_BUSY=忙碌, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvSpiIsTxEmpty(yDrvSpiHandle_t *handle)
    {

        return LL_SPI_IsActiveFlag_TXE(handle->instance) ? YDRV_OK : YDRV_BUSY;
    }

    /**
     * @brief 检查接收器是否不为空（有数据可读）
     * @param handle SPI句柄指针
     * @retval YDRV_OK=有数据, YDRV_BUSY=无数据, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvSpiIsRxNotEmpty(yDrvSpiHandle_t *handle)
    {
        return LL_SPI_IsActiveFlag_RXNE(handle->instance) ? YDRV_OK : YDRV_BUSY;
    }

    /**
     * @brief 检查SPI总线是否忙碌
     * @param handle SPI句柄指针
     * @retval YDRV_OK=不忙, YDRV_BUSY=忙碌, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvSpiIsBusy(yDrvSpiHandle_t *handle)
    {
        return LL_SPI_IsActiveFlag_BSY(handle->instance) ? YDRV_BUSY : YDRV_OK;
    }

    // ==================== SPI控制函数 ====================

    /**
     * @brief 使能SPI（内联优化）
     * @param handle SPI句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE void yDrvSpiEnable(yDrvSpiHandle_t *handle)
    {
        LL_SPI_Enable(handle->instance);
    }

    /**
     * @brief 禁用SPI（内联优化）
     * @param handle SPI句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE void yDrvSpiDisable(yDrvSpiHandle_t *handle)
    {
        LL_SPI_Disable(handle->instance);
    }

    /**
     * @brief 检查SPI句柄是否有效（内联优化）
     * @param handle SPI句柄指针
     * @retval YDRV_OK=句柄有效, YDRV_INVALID_PARAM=句柄无效
     */
    YLIB_INLINE yDrvStatus_t yDrvSpiHandleIsValid(yDrvSpiHandle_t *handle)
    {
        if (handle == NULL || handle->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        return YDRV_OK;
    }

#ifdef __cplusplus
}
#endif

#endif /* YDRV_SPI_H */