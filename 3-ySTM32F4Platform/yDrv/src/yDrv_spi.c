/**
 ******************************************************************************
 * @file    yDrv_spi.c
 * @author  yLab2.0
 * @brief   SPI驱动实现文件
 * @details 基于STM32G0平台LL库实现的SPI驱动程序，提供完整的SPI通信功能
 *          - 支持SPI主从模式配置
 *          - 支持数据帧格式和传输参数配置
 *          - 提供DMA和非DMA传输模式
 *          - 提供完整的错误处理和状态管理
 ******************************************************************************
 * @attention
 * 该驱动基于STM32G0系列芯片的SPI外设，使用前需要正确配置相关的GPIO引脚
 * 和时钟源。驱动提供了完整的初始化和反初始化流程，支持多种工作模式。
 ******************************************************************************
 */

/* 包含的头文件 ----------------------------------------------------------------*/
#include <string.h> // For memset
#include "yDrv_spi.h"

/* 私有宏定义 ------------------------------------------------------------------*/

/**
 * @brief SPI中断回调函数存储结构
 * @note 用于存储每个SPI实例的中断回调函数和标志
 */
// struct
// {
//     yDrvInterruptCallback_t callback[YDRV_SPI_EXTI_MAX]; /*!< 中断回调函数数组 */
//     uint8_t flags[YDRV_SPI_EXTI_MAX];                    /*!< 中断标志数组 */
// } exit_callback[YDRV_SPI_MAX];

static const uint32_t spi_data_width[] = {
    LL_SPI_DATAWIDTH_4BIT,  /*!< 4位数据宽度 */
    LL_SPI_DATAWIDTH_5BIT,  /*!< 5位数据宽度 */
    LL_SPI_DATAWIDTH_6BIT,  /*!< 6位数据宽度 */
    LL_SPI_DATAWIDTH_7BIT,  /*!< 7位数据宽度 */
    LL_SPI_DATAWIDTH_8BIT,  /*!< 8位数据宽度 */
    LL_SPI_DATAWIDTH_9BIT,  /*!< 9位数据宽度 */
    LL_SPI_DATAWIDTH_10BIT, /*!< 10位数据宽度 */
    LL_SPI_DATAWIDTH_11BIT, /*!< 11位数据宽度 */
    LL_SPI_DATAWIDTH_12BIT, /*!< 12位数据宽度 */
    LL_SPI_DATAWIDTH_13BIT, /*!< 13位数据宽度 */
    LL_SPI_DATAWIDTH_14BIT, /*!< 14位数据宽度 */
    LL_SPI_DATAWIDTH_15BIT, /*!< 15位数据宽度 */
    LL_SPI_DATAWIDTH_16BIT, /*!< 16位数据宽度 */
};

/* 私有函数声明 ----------------------------------------------------------------*/
/**
 * @brief 使能指定SPI的时钟
 * @param spiId SPI实例ID
 * @retval 无
 * @note 根据SPI实例启用对应的外设时钟
 */
static void
prv_EnableClock(yDrvSpiId_t spiId);

/**
 * @brief 禁用指定SPI的时钟
 * @param spiId SPI实例ID
 * @retval 无
 * @note 根据SPI实例禁用对应的外设时钟
 */
static void prv_DisableClock(yDrvSpiId_t spiId);

/**
 * @brief 获取SPI实例信息
 * @param spiId SPI实例ID
 * @param handle SPI句柄指针
 * @retval 无
 * @note 根据实例ID填充句柄中的硬件信息
 */
static void yDrvSpiGetInstance(yDrvSpiId_t spiId,
                               yDrvSpiHandle_t *handle);

/**
 * @brief 配置SPI相关的GPIO引脚
 * @param config SPI配置结构指针
 * @param handle SPI句柄指针
 * @retval yDrvStatus_t 操作状态
 * @note 配置CS引脚的复用功能
 */
static yDrvStatus_t prv_ConfigGpio(const yDrvSpiConfig_t *config, yDrvSpiHandle_t *handle);

/**
 * @brief 反初始化SPI相关的GPIO引脚
 * @param handle SPI句柄指针
 */
static void prv_DeInitGpio(yDrvSpiHandle_t *handle);

/* 公共函数实现 ----------------------------------------------------------------*/

/**
 * @brief 初始化SPI外设（静态配置方式）
 * @param config SPI配置参数指针
 * @param handle SPI句柄指针
 * @retval yDrvStatus_t 初始化状态
 * @note 该函数执行完整的SPI初始化流程：
 *       1. 参数有效性检查
 *       2. 使能SPI外设时钟
 *       3. 配置相关GPIO引脚
 *       4. 配置SPI基本参数
 *       5. 使能SPI外设
 */
yDrvStatus_t yDrvSpiInitStatic(yDrvSpiConfig_t *config, yDrvSpiHandle_t *handle)
{
    LL_SPI_InitTypeDef spi_init;

    if (config == NULL || handle == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    yDrvSpiHandleStructInit(handle);

    // 1. 获取SPI实例
    yDrvSpiGetInstance(config->spiId, handle);
    if (handle->instance == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 2. 使能时钟和配置GPIO
    prv_EnableClock(config->spiId);
    if (prv_ConfigGpio(config, handle) != YDRV_OK)
    {
        prv_DisableClock(config->spiId);
        return YDRV_ERROR;
    }

    // 3. 配置SPI参数
    if (((size_t)(config->dataBits - 4)) >= (sizeof(spi_data_width) / sizeof(spi_data_width[0])))
    {
        return YDRV_INVALID_PARAM;
    }

    LL_SPI_StructInit(&spi_init);
    spi_init.TransferDirection = config->direction;
    spi_init.Mode = config->mode;
    spi_init.DataWidth = spi_data_width[config->dataBits - 4];
    spi_init.ClockPolarity = config->polarity;
    spi_init.ClockPhase = config->phase;
    spi_init.NSS = config->csMode;
    spi_init.BaudRate = config->speed;
    spi_init.BitOrder = config->bitOrder;
    if (config->crc != 0)
    {
        spi_init.CRCCalculation = LL_SPI_CRCCALCULATION_ENABLE;
        spi_init.CRCPoly = config->crc;
    }
    else
    {
        spi_init.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
        spi_init.CRCPoly = 7; // 默认CRC多项式
    }
    if (LL_SPI_Init(handle->instance, &spi_init) != SUCCESS)
    {
        return YDRV_ERROR;
    }
    handle->flagBtyeSend = (config->dataBits > 8) ? 1 : 0;
    handle->flagCsControl = (config->csMode == YDRV_SPI_CS_SOFT) ? 1 : 0;
    LL_SPI_SetStandard(handle->instance, LL_SPI_PROTOCOL_MOTOROLA);
    // LL_SPI_EnableNSSPulseMgt(handle->instance);

    // 4. 使能SPI
    LL_SPI_Enable(handle->instance);

    return YDRV_OK;
}

/**
 * @brief 反初始化SPI外设（静态配置方式）
 * @param handle SPI句柄指针
 * @retval yDrvStatus_t 反初始化状态
 * @note 该函数执行完整的SPI反初始化流程：
 *       1. 禁用SPI外设
 *       2. 反初始化相关GPIO引脚
 *       3. 禁用SPI外设时钟
 */
yDrvStatus_t yDrvSpiDeInitStatic(yDrvSpiHandle_t *handle)
{
    // 参数有效性检查
    if (handle == NULL || handle->instance == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 1. 禁用SPI外设
    LL_SPI_Disable(handle->instance);

    // 2. 反初始化GPIO引脚
    prv_DeInitGpio(handle);

    // 3. 禁用SPI外设时钟
    prv_DisableClock(handle->spiId);
    return YDRV_OK;
}

/**
 * @brief 初始化SPI配置结构体为默认值
 * @param config SPI配置结构体指针
 * @note 默认配置：
 *       - SPI1外设
 *       - 主模式
 *       - 全双工
 *       - 8位数据宽度
 *       - 软件NSS管理
 *       - MSB先传输
 */
void yDrvSpiConfigStructInit(yDrvSpiConfig_t *config)
{
    if (config == NULL)
    {
        return;
    }

    config->spiId = YDRV_SPI_MAX;
    config->sckPin = YDRV_PINNULL;
    config->misoPin = YDRV_PINNULL;
    config->mosiPin = YDRV_PINNULL;
    config->csPin = YDRV_PINNULL;
    config->sckAF = 0;
    config->misoAF = 0;
    config->mosiAF = 0;
    config->csAF = 0;
    config->mode = LL_SPI_MODE_MASTER;
    config->direction = LL_SPI_FULL_DUPLEX;
    config->polarity = LL_SPI_POLARITY_LOW;
    config->phase = LL_SPI_PHASE_1EDGE;
    config->csMode = LL_SPI_NSS_SOFT;
    config->speed = LL_SPI_BAUDRATEPRESCALER_DIV256;
    config->bitOrder = LL_SPI_MSB_FIRST;
}

/**
 * @brief 初始化SPI句柄结构体为默认值
 * @param handle SPI句柄结构体指针
 */
void yDrvSpiHandleStructInit(yDrvSpiHandle_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    handle->instance = NULL;
    handle->spiId = YDRV_SPI_MAX;

    handle->sckPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
    handle->misoPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
    handle->mosiPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
    handle->csPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
}

/**
 * @brief 配置SPI DMA发送
 * @param handle SPI句柄指针
 * @param channel DMA通道指针
 * @retval yDrvStatus_t 配置状态
 * @note 配置SPI的DMA发送功能：
 *       1. 设置DMA外设地址为SPI数据寄存器
 *       2. 配置DMA传输参数
 *       3. 设置对应的DMAMUX请求信号
 */
yDrvStatus_t yDrvSpiDmaWrite(yDrvSpiHandle_t *handle, yDrvDmaChannel_t *channel)
{
    yDrvDmaInfo_t dma_info;

    // 参数有效性检查
    if (handle == NULL || channel == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 解析DMA通道信息
    if (yDrvParseDma(*channel, &dma_info) != YDRV_OK)
    {
        return YDRV_INVALID_PARAM;
    }

    // 设置DMA外设地址为SPI数据寄存器
    LL_DMA_SetPeriphAddress(dma_info.dma,
                            dma_info.channel,
                            LL_SPI_DMA_GetRegAddr(handle->instance));

    // 配置DMA传输参数
    LL_DMA_SetPeriphSize(dma_info.dma,
                         dma_info.channel,
                         LL_DMA_PDATAALIGN_BYTE);

    LL_DMA_SetPeriphIncMode(dma_info.dma,
                            dma_info.channel,
                            LL_DMA_PERIPH_NOINCREMENT);

    // 根据SPI实例配置DMAMUX请求信号
    switch (handle->spiId)
    {
    case YDRV_SPI_1:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_SPI1_TX);
        break;
    case YDRV_SPI_2:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_SPI2_TX);
        break;
    default:
        return YDRV_ERROR;
    }

    // 启用SPI的DMA发送
    LL_SPI_EnableDMAReq_TX(handle->instance);

    return YDRV_OK;
}

/**
 * @brief 配置SPI DMA接收
 * @param handle SPI句柄指针
 * @param channel DMA通道指针
 * @retval yDrvStatus_t 配置状态
 * @note 配置SPI的DMA接收功能：
 *       1. 设置DMA外设地址为SPI数据寄存器
 *       2. 配置DMA传输参数
 *       3. 设置对应的DMAMUX请求信号
 *       4. 启用SPI DMA接收请求
 */
yDrvStatus_t yDrvSpiDmaRead(yDrvSpiHandle_t *handle, yDrvDmaChannel_t *channel)
{
    yDrvDmaInfo_t dma_info;

    // 参数有效性检查
    if (handle == NULL || channel == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 解析DMA通道信息
    if (yDrvParseDma(*channel, &dma_info) != YDRV_OK)
    {
        return YDRV_INVALID_PARAM;
    }

    // 设置DMA外设地址为SPI数据寄存器
    LL_DMA_SetPeriphAddress(dma_info.dma,
                            dma_info.channel,
                            LL_SPI_DMA_GetRegAddr(handle->instance));

    // 配置DMA传输参数
    LL_DMA_SetPeriphSize(dma_info.dma,
                         dma_info.channel,
                         LL_DMA_PDATAALIGN_BYTE);

    LL_DMA_SetPeriphIncMode(dma_info.dma,
                            dma_info.channel,
                            LL_DMA_PERIPH_NOINCREMENT);

    // 根据SPI实例配置DMAMUX请求信号
    switch (handle->spiId)
    {
    case YDRV_SPI_1:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_SPI1_RX);
        break;
    case YDRV_SPI_2:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_SPI2_RX);
        break;
    default:
        return YDRV_ERROR;
    }

    // 启用SPI的DMA接收
    LL_SPI_EnableDMAReq_RX(handle->instance);

    return YDRV_OK;
}

/* 私有函数实现 ----------------------------------------------------------------*/

/**
 * @brief 获取SPI实例信息
 * @param spiId SPI实例ID
 * @param handle SPI句柄指针
 * @retval 无
 * @note 根据SPI ID填充句柄中的硬件实例信息
 *       - 设置SPI外设基地址指针
 *       - 配置对应的SPI实例ID
 *       - 支持STM32G0系列的SPI1和SPI2外设
 */
static void yDrvSpiGetInstance(yDrvSpiId_t spiId,
                               yDrvSpiHandle_t *handle)
{
    // 根据SPI ID返回对应的外设实例指针
    handle->spiId = spiId;
    switch (handle->spiId)
    {
    case YDRV_SPI_1:
        handle->instance = SPI1;
        handle->IRQ = SPI1_IRQn; // 设置SPI1的中断号
        break;
    case YDRV_SPI_2:
        handle->instance = SPI2;
        handle->IRQ = SPI2_IRQn; // 设置SPI2的中断号
        break;
    default:
        handle->instance = NULL;
        break;
    }
}

/**
 * @brief 使能SPI外设时钟
 * @param spiId SPI外设ID
 * @note 根据SPI ID使能对应的RCC时钟
 */
static void prv_EnableClock(yDrvSpiId_t spiId)
{
    switch (spiId)
    {
    case YDRV_SPI_1:
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
        break;
    case YDRV_SPI_2:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
        break;
    default:
        break;
    }
}

/**
 * @brief 禁用SPI外设时钟
 * @param spiId SPI外设ID
 * @note 根据SPI ID禁用对应的RCC时钟以节省功耗
 */
static void prv_DisableClock(yDrvSpiId_t spiId)
{
    switch (spiId)
    {
    case YDRV_SPI_1:
        LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
        break;
    case YDRV_SPI_2:
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI2);
        break;
    default:
        break;
    }
}

/**
 * @brief 配置SPI相关的GPIO引脚
 * @param config SPI配置参数指针
 * @param handle SPI句柄指针
 * @retval yDrvStatus_t 配置状态
 * @note 配置SCK、MISO、MOSI和NSS引脚为复用功能模式
 */
static yDrvStatus_t prv_ConfigGpio(const yDrvSpiConfig_t *config, yDrvSpiHandle_t *handle)
{
    yDrvStatus_t status;
    LL_GPIO_InitTypeDef gpio_init;

    // 配置SCK引脚：复用功能、推挽输出、高速
    if (config->sckPin != YDRV_PINNULL)
    {
        status = yDrvParseGpio(config->sckPin, &handle->sckPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->sckPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->sckAF;

        // 配置SCK引脚为复用功能模式
        LL_GPIO_Init(handle->sckPinInfo.port, &gpio_init);
        handle->sckPinInfo.flag = 1;
    }

    // 配置MISO引脚：复用功能（仅在全双工或接收模式下配置）
    if ((config->direction == LL_SPI_FULL_DUPLEX) ||
        (config->direction == LL_SPI_SIMPLEX_RX) ||
        (config->direction == LL_SPI_HALF_DUPLEX_RX))
    {
        status = yDrvParseGpio(config->misoPin, &handle->misoPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->misoPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->misoAF;

        // 配置MISO引脚为复用功能模式
        LL_GPIO_Init(handle->misoPinInfo.port, &gpio_init);
        handle->misoPinInfo.flag = 1;
    }

    // 配置MOSI引脚：复用功能（仅在全双工或发送模式下配置）
    if ((config->direction == LL_SPI_FULL_DUPLEX) ||
        (config->direction == LL_SPI_HALF_DUPLEX_TX))
    {
        status = yDrvParseGpio(config->mosiPin, &handle->mosiPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->mosiPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->mosiAF;

        // 配置MOSI引脚为复用功能模式
        LL_GPIO_Init(handle->mosiPinInfo.port, &gpio_init);
        handle->mosiPinInfo.flag = 1;
    }

    // 配置NSS引脚：仅在硬件NSS模式下配置
    if (config->csMode != LL_SPI_NSS_SOFT)
    {
        status = yDrvParseGpio(config->csPin, &handle->csPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->csPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_UP; // NSS通常需要上拉
        gpio_init.Alternate = config->csAF;

        // 配置NSS引脚为复用功能模式
        LL_GPIO_Init(handle->csPinInfo.port, &gpio_init);
        handle->csPinInfo.flag = 1;
    }
    else
    {
        status = yDrvParseGpio(config->csPin, &handle->csPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_OK;
        }

        gpio_init.Pin = handle->csPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_UP; // CS通常需要上拉
        gpio_init.Alternate = 0;

        // 配置NSS引脚为复用功能模式
        LL_GPIO_Init(handle->csPinInfo.port, &gpio_init);
        handle->csPinInfo.flag = 1;
    }

    return YDRV_OK;
}

/**
 * @brief 反初始化SPI相关的GPIO引脚
 * @param handle SPI句柄指针
 * @note 将所有配置的GPIO引脚恢复为默认状态
 */
static void prv_DeInitGpio(yDrvSpiHandle_t *handle)
{
    LL_GPIO_InitTypeDef gpio_init;

    // 参数有效性检查
    if (handle == NULL)
    {
        return;
    }

    // 反初始化GPIO引脚为默认状态
    LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ANALOG;     // 模拟模式，最低功耗
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW; // 低速
    gpio_init.Pull = LL_GPIO_PULL_NO;         // 无上下拉
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;

    // 反初始化SCK引脚
    if (handle->sckPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->sckPinInfo.pinMask;
        LL_GPIO_Init(handle->sckPinInfo.port, &gpio_init);
        handle->sckPinInfo.flag = 0;
    }

    // 反初始化MISO引脚
    if (handle->misoPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->misoPinInfo.pinMask;
        LL_GPIO_Init(handle->misoPinInfo.port, &gpio_init);
        handle->misoPinInfo.flag = 0;
    }

    // 反初始化MOSI引脚
    if (handle->mosiPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->mosiPinInfo.pinMask;
        LL_GPIO_Init(handle->mosiPinInfo.port, &gpio_init);
        handle->mosiPinInfo.flag = 0;
    }

    // 反初始化NSS引脚
    if (handle->csPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->csPinInfo.pinMask;
        LL_GPIO_Init(handle->csPinInfo.port, &gpio_init);
        handle->csPinInfo.flag = 0;
    }
}
