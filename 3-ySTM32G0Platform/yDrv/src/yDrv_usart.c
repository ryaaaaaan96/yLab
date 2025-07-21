/**
 * @file yDrv_usart.c
 * @brief STM32G0 USART驱动程序实现
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现STM32G0系列MCU的USART/UART驱动程序，提供完整的串口通信功能
 *
 * @par 主要功能:
 * - USART硬件初始化和配置
 * - 数据发送和接收功能
 * - 中断管理和回调处理
 * - GPIO引脚复用功能配置
 * - 错误处理和状态查询
 */

#include <string.h> // For memset
#include "yDrv_usart.h"

// ==================== 私有定义 ====================

/**
 * @brief USART中断回调函数存储结构
 * @note 用于存储每个USART实例的中断回调函数和标志
 */
struct
{
    yDrvInterruptCallback_t callback[YDRV_USART_EXTI_MAX]; /*!< 中断回调函数数组 */
    uint8_t flags[YDRV_USART_EXTI_MAX];                    /*!< 中断标志数组 */
} exit_callback[YDRV_USART_MAX];

// ==================== 私有函数声明 ====================

/**
 * @brief 使能指定USART的时钟
 * @param usartId USART实例ID
 * @retval 无
 * @note 根据USART实例启用对应的外设时钟
 */
static void prv_EnableClock(yDrvUsartId_t usartId);

/**
 * @brief 禁用指定USART的时钟
 * @param usartId USART实例ID
 * @retval 无
 * @note 根据USART实例禁用对应的外设时钟
 */
static void prv_DisableClock(yDrvUsartId_t usartId);

/**
 * @brief 获取USART实例信息
 * @param usartId USART实例ID
 * @param handle USART句柄指针
 * @retval 无
 * @note 根据实例ID填充句柄中的硬件信息
 */
static void yDrvUsartGetInstance(yDrvUsartId_t usartId,
                                 yDrvUsartHandle_t *handle);

/**
 * @brief 配置USART相关的GPIO引脚
 * @param config USART配置结构指针
 * @param handle USART句柄指针
 * @retval yDrvStatus_t 操作状态
 * @note 配置TX、RX、RTS、CTS引脚的复用功能
 */
static yDrvStatus_t prv_ConfigGpio(const yDrvUsartConfig_t *config, yDrvUsartHandle_t *handle);

/**
 * @brief 反初始化USART相关的GPIO引脚
 * @param handle USART句柄指针
 */
static void prv_DeInitGpio(yDrvUsartHandle_t *handle);

// ==================== 基础函数实现 ====================

yDrvStatus_t yDrvUsartInitStatic(const yDrvUsartConfig_t *config, yDrvUsartHandle_t *handle)
{
    LL_USART_InitTypeDef usart_init;

    // 参数有效性检查
    if (handle == NULL || config == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    yDrvUsartHandleStructInit(handle);

    // 1. 获取USART实例并使能时钟
    yDrvUsartGetInstance(config->usartId, handle);
    if (handle->instance == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    prv_EnableClock(handle->usartId);

    // 2. 配置相关GPIO引脚（TX、RX、RTS、CTS）
    if (prv_ConfigGpio(config, handle) != YDRV_OK)
    {
        prv_DisableClock(config->usartId);
        return YDRV_INVALID_PARAM;
    }

    // 3. 配置USART基本参数
    LL_USART_StructInit(&usart_init);                     // 初始化为默认值
    usart_init.PrescalerValue = LL_USART_PRESCALER_DIV1;  // 分频
    usart_init.BaudRate = config->baudRate;               // 设置波特率
    usart_init.DataWidth = config->dataBits;              // 设置数据位长度
    usart_init.StopBits = config->stopBits;               // 设置停止位
    usart_init.Parity = config->parity;                   // 设置校验位
    usart_init.TransferDirection = config->direction;     // 设置传输方向
    usart_init.HardwareFlowControl = config->flowControl; // 设置硬件流控制
    usart_init.OverSampling = LL_USART_OVERSAMPLING_16;   // 过采样

    // 应用USART配置
    if (LL_USART_Init(handle->instance, &usart_init) != SUCCESS)
    {
        return YDRV_ERROR;
    }

    // 5. 配置特定工作模式
    switch (config->mode)
    {
    case YDRV_USART_MODE_ASYNCHRONOUS:
        LL_USART_ConfigAsyncMode(handle->instance);
        break;
    case YDRV_USART_MODE_SYNCHRONOUS:
        // 同步模式：使能时钟输出，配置时钟相位和极性
        LL_USART_ConfigClock(handle->instance,
                             LL_USART_PRESCALER_DIV1, // 预分频器
                             LL_USART_PHASE_2EDGE,    // 时钟相位
                             LL_USART_POLARITY_LOW);  // 时钟极性
        break;
    case YDRV_USART_MODE_SMARTCARD:
        // 智能卡模式
        LL_USART_EnableSmartcard(handle->instance);
        break;
    case YDRV_USART_MODE_SINGLE_WIRE:
        // 单线半双工模式
        LL_USART_EnableHalfDuplex(handle->instance);
        break;
    case YDRV_USART_MODE_IRDA:
        // IrDA红外模式（正常功率）
        LL_USART_EnableIrda(handle->instance);
        LL_USART_SetIrdaPowerMode(handle->instance, LL_USART_IRDA_POWER_NORMAL);
        break;
    case YDRV_USART_MODE_LIN:
        // LIN总线模式
        LL_USART_EnableLIN(handle->instance);
        break;
    default: // YDRV_USART_MODE_ASYNCHRONOUS
        // 标准异步UART模式
        LL_USART_ConfigAsyncMode(handle->instance);
        break;
    }

    // 6. 使能USART外设
    LL_USART_Enable(handle->instance);

    // 7. 初始化句柄结构

    handle->flagBtyeSend = ((config->parity == YDRV_USART_PARITY_NONE) &&
                            (config->dataBits == YDRV_USART_DATA_9BIT))
                               ? 1
                               : 0;

    return YDRV_OK;
}

yDrvStatus_t yDrvUsartDeInitStatic(yDrvUsartHandle_t *handle)
{
    // 参数有效性检查
    if (handle == NULL || handle->instance == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 1. 禁用USART外设
    LL_USART_Disable(handle->instance);

    // 2. 反初始化相关GPIO引脚，恢复为默认状态
    prv_DeInitGpio(handle);

    // 3. 禁用USART外设时钟以节省功耗
    prv_DisableClock(handle->usartId);

    return YDRV_OK;
}

void yDrvUsartConfigStructInit(yDrvUsartConfig_t *config)
{
    if (config == NULL)
    {
        return;
    }

    config->usartId = YDRV_USART_MAX;
    config->txPin = YDRV_PINNULL;
    config->rxPin = YDRV_PINNULL;
    config->rtsPin = YDRV_PINNULL;
    config->ctsPin = YDRV_PINNULL;
    config->baudRate = 115200;
    config->dataBits = YDRV_USART_DATA_8BIT;
    config->stopBits = YDRV_USART_STOP_1BIT;
    config->parity = YDRV_USART_PARITY_NONE;
    config->direction = YDRV_USART_DIR_TX_RX;
    config->flowControl = YDRV_USART_FLOW_NONE;
    config->mode = YDRV_USART_MODE_ASYNCHRONOUS;
    config->txAF = 0;
    config->rxAF = 0;
    config->ctsAF = 0;
    config->rtsAF = 0;
}

void yDrvUsartHandleStructInit(yDrvUsartHandle_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    handle->instance = NULL;
    handle->IRQ = (IRQn_Type)0;
    handle->usartId = YDRV_USART_MAX;

    handle->txPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
    handle->rxPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
    handle->rtsPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};
    handle->ctsPinInfo = (yDrvGpioInfo_t){NULL, 0, 0, 0};

    handle->flagBtyeSend = 0;
}

void yDrvUsartExtiConfigStructInit(yDrvUsartExtiConfig_t *extiConfig)
{
    if (extiConfig == NULL)
    {
        return;
    }

    extiConfig->trigger = YDRV_USART_EXTI_MAX;
    extiConfig->prio = 0;
    extiConfig->function = NULL;
    extiConfig->arg = NULL;
    extiConfig->enable = 0;
}

// ==================== 工具函数实现 ====================

static void yDrvUsartGetInstance(yDrvUsartId_t usartId,
                                 yDrvUsartHandle_t *handle)
{
    // 根据USART ID返回对应的外设实例指针
    handle->usartId = usartId;
    switch (handle->usartId)
    {
    case YDRV_USART_1:
        handle->instance = USART1;
        handle->IRQ = USART1_IRQn;
        break;
    case YDRV_USART_2:
        handle->instance = USART2;
        handle->IRQ = USART2_IRQn;
        break;
    case YDRV_USART_3:
        handle->instance = USART3;
        handle->IRQ = USART3_4_IRQn;
        break;
    case YDRV_USART_4:
        handle->instance = USART4;
        handle->IRQ = USART3_4_IRQn;
        break;
#ifdef USART5
        handle->instance = USART5;
        handle->IRQ = USART5_6_IRQn;
        break;
#endif
#ifdef USART5
    case YDRV_USART_6:
        handle->instance = USART6;
        handle->IRQ = USART5_6_IRQn;
        break;
#endif
    default:
        handle->instance = NULL;
        break;
    }
}

// ==================== 专用模式函数实现 ====================

/**
 * @brief 设置智能卡模式的保护时间
 * @param handle USART句柄指针
 * @param guardTime 保护时间值（0-255）
 * @note 仅在智能卡模式下有效
 */
void yDrvUsartSetSmartcardGuardTime(yDrvUsartHandle_t *handle, uint8_t guardTime)
{
    (void)guardTime;
    if (handle && handle->instance)
    {
        // LL_USART_SetGuardTime(handle->instance, guardTime);
    }
}

/**
 * @brief 设置IrDA模式的功耗模式
 * @param handle USART句柄指针
 * @param powerMode true=低功耗模式，false=正常功率模式
 * @note 仅在IrDA模式下有效
 */
void yDrvUsartSetIrdaPowerMode(yDrvUsartHandle_t *handle, bool powerMode)
{
    if (handle && handle->instance)
    {
        uint32_t mode = powerMode ? LL_USART_IRDA_POWER_LOW : LL_USART_IRDA_POWER_NORMAL;
        LL_USART_SetIrdaPowerMode(handle->instance, mode);
    }
}

/**
 * @brief 设置LIN模式的中断检测长度
 * @param handle USART句柄指针
 * @param breakLength true=11位中断检测，false=10位中断检测
 * @note 仅在LIN模式下有效
 */
void yDrvUsartSetLinBreakLength(yDrvUsartHandle_t *handle, bool breakLength)
{
    if (handle && handle->instance)
    {
        uint32_t length = breakLength ? LL_USART_LINBREAK_DETECT_11B : LL_USART_LINBREAK_DETECT_10B;
        LL_USART_SetLINBrkDetectionLen(handle->instance, length);
    }
}

/**
 * @brief 发送LIN中断信号
 * @param handle USART句柄指针
 * @note 仅在LIN模式下有效
 */
void yDrvUsartSendLinBreak(yDrvUsartHandle_t *handle)
{
    if (handle && handle->instance)
    {
        LL_USART_RequestBreakSending(handle->instance);
    }
}

// ==================== 中断管理函数实现 ====================

yDrvStatus_t yDrvUsartRegisterCallback(yDrvUsartHandle_t *handle,
                                       yDrvUsartExtiConfig_t *exti)
{
    // 参数有效性检查
    if (handle == NULL || handle->instance == NULL || exti == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 检查USART ID是否有效
    if (handle->usartId >= YDRV_USART_MAX)
    {
        return YDRV_INVALID_PARAM;
    }

    // 检查中断类型是否有效
    if (exti->trigger >= YDRV_USART_EXTI_MAX)
    {
        return YDRV_INVALID_PARAM;
    }

    // 存储回调函数和参数
    exit_callback[handle->usartId].callback[exti->trigger].function = exti->function;
    exit_callback[handle->usartId].callback[exti->trigger].arg = exti->arg;
    exit_callback[handle->usartId].flags[exti->trigger] = exti->enable;

    // 根据中断类型使能对应的硬件中断
    switch (exti->trigger)
    {
    case YDRV_USART_EXTI_TXE:
        LL_USART_EnableIT_TXE(handle->instance);
        break;

    case YDRV_USART_EXTI_RXNE:
        LL_USART_EnableIT_RXNE(handle->instance);
        break;

    case YDRV_USART_EXTI_TC:
        LL_USART_EnableIT_TC(handle->instance);
        break;

    case YDRV_USART_EXTI_IDLE:
        LL_USART_EnableIT_IDLE(handle->instance);
        break;

    case YDRV_USART_EXTI_PE:
        LL_USART_EnableIT_PE(handle->instance);
        break;

    case YDRV_USART_EXTI_ERR:
        LL_USART_EnableIT_ERROR(handle->instance);
        break;

    case YDRV_USART_EXTI_LBD:
        LL_USART_EnableIT_LBD(handle->instance);
        break;

    case YDRV_USART_EXTI_CTS:
        LL_USART_EnableIT_CTS(handle->instance);
        break;

    default:
        return YDRV_INVALID_PARAM;
    }

    // 设置中断优先级
    NVIC_SetPriority(handle->IRQ, exti->prio);

    if (exti->enable)
    { // 使能NVIC中断
        NVIC_EnableIRQ(handle->IRQ);
    }
    else
    {
        NVIC_DisableIRQ(handle->IRQ);
    }

    return YDRV_OK;
}

yDrvStatus_t yDrvUsartUnregisterCallback(yDrvUsartHandle_t *handle, yDrvUsartExti_t type)

{
    // 参数有效性检查
    // 禁用对应的硬件中断
    switch (type)
    {
    case YDRV_USART_EXTI_TXE:
        LL_USART_DisableIT_TXE(handle->instance);
        break;

    case YDRV_USART_EXTI_RXNE:
        LL_USART_DisableIT_RXNE(handle->instance);
        break;

    case YDRV_USART_EXTI_TC:
        LL_USART_DisableIT_TC(handle->instance);
        break;

    case YDRV_USART_EXTI_IDLE:
        LL_USART_DisableIT_IDLE(handle->instance);
        break;

    case YDRV_USART_EXTI_PE:
        LL_USART_DisableIT_PE(handle->instance);
        break;

    case YDRV_USART_EXTI_ERR:
        LL_USART_DisableIT_ERROR(handle->instance);
        break;

    case YDRV_USART_EXTI_LBD:
        LL_USART_DisableIT_LBD(handle->instance);
        break;

    case YDRV_USART_EXTI_CTS:
        LL_USART_DisableIT_CTS(handle->instance);
        break;

    default:
        return YDRV_INVALID_PARAM;
    }

    // 清除回调函数
    exit_callback[handle->usartId].callback[type].function = NULL;
    exit_callback[handle->usartId].callback[type].arg = NULL;

    return YDRV_OK;
}

// // ==================== 私有函数实现 ====================

/**
 * @brief 使能指定USART的时钟
 * @param usartId USART实例ID
 * @note STM32G0系列USART1在APB2总线上，USART2-4在APB1总线上
 */
static void prv_EnableClock(yDrvUsartId_t usartId)
{
    switch (usartId)
    {
    case YDRV_USART_1:
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
        break;
    case YDRV_USART_2:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
        break;
    case YDRV_USART_3:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
        break;
#ifdef USART4 // STM32G0部分型号有USART4
    case YDRV_USART_4:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART4);
        break;
#endif
#ifdef UART5
    case YDRV_USART_5:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
        break;
#endif
#ifdef USART6
    case YDRV_USART_6:
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
        break;
#endif
    default:
        break;
    }
}

/**
 * @brief 禁用指定USART的时钟
 * @param usartId USART实例ID
 * @note 禁用时钟以节省功耗
 */
static void prv_DisableClock(yDrvUsartId_t usartId)
{
    switch (usartId)
    {
    case YDRV_USART_1:
        LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
        break;
    case YDRV_USART_2:
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
        break;
    case YDRV_USART_3:
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART3);
        break;
    case YDRV_USART_4:
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART4);
        break;
#ifdef UART5
    case YDRV_USART_5:
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART5);
        break;
#endif
#ifdef USART6
    case YDRV_USART_6:
        LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART6);
        break;
#endif
    default:
        break;
    }
}

/**
 * @brief 配置USART相关的GPIO引脚
 * @param config USART配置结构指针
 * @note 配置TX、RX引脚为复用功能，根据流控设置配置RTS、CTS引脚
 */
static yDrvStatus_t prv_ConfigGpio(const yDrvUsartConfig_t *config, yDrvUsartHandle_t *handle)
{
    yDrvStatus_t status;
    LL_GPIO_InitTypeDef gpio_init;

    // 配置TX引脚：复用功能、推挽输出
    if ((config->direction == YDRV_USART_DIR_TX) ||
        (config->direction == YDRV_USART_DIR_TX_RX))
    {
        status = yDrvParseGpio(config->txPin, &handle->txPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->txPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->txAF;

        // 配置TX引脚为复用功能模式
        LL_GPIO_Init(handle->txPinInfo.port, &gpio_init);
        handle->txPinInfo.flag = 1;
    }

    // 配置RX引脚：复用功能
    if ((config->direction == YDRV_USART_DIR_RX) ||
        (config->direction == YDRV_USART_DIR_TX_RX))
    {
        status = yDrvParseGpio(config->rxPin, &handle->txPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->txPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->rxAF;

        // 配置RX引脚为复用功能模式
        LL_GPIO_Init(handle->txPinInfo.port, &gpio_init);
        handle->rxPinInfo.flag = 1;
    }

    // 如果使能RTS流控，配置RTS引脚
    if ((config->flowControl == YDRV_USART_FLOW_RTS) ||
        (config->flowControl == YDRV_USART_FLOW_RTS_CTS))
    {
        status = yDrvParseGpio(config->rtsPin, &handle->rtsPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->rtsPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->rtsAF;

        // 配置RTS引脚为复用功能模式
        LL_GPIO_Init(handle->rtsPinInfo.port, &gpio_init);
        handle->rtsPinInfo.flag = 1;
    }

    // 如果使能CTS流控，配置CTS引脚
    if ((config->flowControl == YDRV_USART_FLOW_CTS) ||
        (config->flowControl == YDRV_USART_FLOW_RTS_CTS))
    {
        status = yDrvParseGpio(config->ctsPin, &handle->ctsPinInfo);
        if (status != YDRV_OK)
        {
            return YDRV_INVALID_PARAM;
        }

        gpio_init.Pin = handle->ctsPinInfo.pinMask;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_NO;
        gpio_init.Alternate = config->ctsAF;

        // 配置CTS引脚为复用功能模式
        LL_GPIO_Init(handle->ctsPinInfo.port, &gpio_init);
        handle->ctsPinInfo.flag = 1;
    }

    return YDRV_OK;
}

/**
 * @brief 反初始化USART相关的GPIO引脚
 * @param handle USART句柄指针
 * @note 将所有使用的GPIO引脚恢复为默认状态
 */
static void prv_DeInitGpio(yDrvUsartHandle_t *handle)
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

    if (handle->txPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->txPinInfo.pinMask;
        LL_GPIO_Init(handle->txPinInfo.port, &gpio_init);
        handle->txPinInfo.flag = 0;
    }

    if (handle->rxPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->rxPinInfo.pinMask;
        LL_GPIO_Init(handle->rxPinInfo.port, &gpio_init);
        handle->rxPinInfo.flag = 0;
    }

    if (handle->rtsPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->rtsPinInfo.pinMask;
        LL_GPIO_Init(handle->rtsPinInfo.port, &gpio_init);
        handle->rtsPinInfo.flag = 0;
    }

    if (handle->ctsPinInfo.flag == 1)
    {
        gpio_init.Pin = handle->ctsPinInfo.pinMask;
        LL_GPIO_Init(handle->ctsPinInfo.port, &gpio_init);
        handle->ctsPinInfo.flag = 0;
    }
}

yDrvStatus_t yDrvUsartDmaWrite(yDrvUsartHandle_t *handle, yDrvDmaChannel_t *channel)
{
    yDrvDmaInfo_t dma_info;

    // 参数有效性检查
    if (handle == NULL || channel == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    yDrvParseDma(*channel, &dma_info);
    LL_DMA_SetPeriphAddress(DMA1,
                            LL_DMA_CHANNEL_5,
                            LL_USART_DMA_GetRegAddr(handle->instance, LL_USART_DMA_REG_DATA_TRANSMIT));
    LL_DMA_SetPeriphSize(dma_info.dma,
                         dma_info.channel,
                         LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetPeriphIncMode(dma_info.dma,
                            dma_info.channel, LL_DMA_PERIPH_NOINCREMENT); // 配置优先级
    switch (handle->usartId)
    {
    case YDRV_USART_1:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_USART1_TX); // 配置DMAMUX请求信号
        return YDRV_OK;
    case YDRV_USART_2:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_USART2_TX); // 配置DMAMUX请求信号
        return YDRV_OK;
    case YDRV_USART_3:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_USART3_TX); // 配置DMAMUX请求信号
        return YDRV_OK;
    default:
        return YDRV_ERROR;
    }
    return YDRV_INVALID_PARAM;
}

yDrvStatus_t yDrvUsartDmaRead(yDrvUsartHandle_t *handle, yDrvDmaChannel_t *channel)
{
    yDrvDmaInfo_t dma_info;

    if (yDrvParseDma(*channel, &dma_info) != YDRV_OK)
    {
        return YDRV_INVALID_PARAM;
    }
    LL_DMA_SetPeriphAddress(dma_info.dma,
                            dma_info.channel,
                            LL_USART_DMA_GetRegAddr(handle->instance, LL_USART_DMA_REG_DATA_RECEIVE));
    LL_DMA_SetPeriphSize(dma_info.dma,
                         dma_info.channel,
                         LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetPeriphIncMode(dma_info.dma,
                            dma_info.channel,
                            LL_DMA_PERIPH_NOINCREMENT); // 配置优先级
    // 启用USART的DMA接收
    LL_USART_EnableDMAReq_RX(handle->instance);
    switch (handle->usartId)
    {
    case YDRV_USART_1:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_USART1_RX); // 配置DMAMUX请求信号
        return YDRV_OK;
    case YDRV_USART_2:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_USART2_RX); // 配置DMAMUX请求信号
        return YDRV_OK;
    case YDRV_USART_3:
        LL_DMA_SetPeriphRequest(dma_info.dma,
                                dma_info.channel,
                                LL_DMAMUX_REQ_USART3_RX); // 配置DMAMUX请求信号
        return YDRV_OK;
    default:
        return YDRV_ERROR;
    }
    return YDRV_OK;
}

// // ==================== 中断处理函数（优化版本） ====================

#define USART_HANDLE_EXIT_IRQ(instance, index)                            \
    do                                                                    \
    {                                                                     \
        /* 1. 接收数据寄存器非空中断 (RXNE) - 最高优先级 */               \
        if (LL_USART_IsActiveFlag_RXNE(instance) &&                       \
            LL_USART_IsEnabledIT_RXNE(instance) &&                        \
            exit_callback[index].callback[YDRV_USART_EXTI_RXNE].function) \
        {                                                                 \
            exit_callback[index].callback[YDRV_USART_EXTI_RXNE].function( \
                exit_callback[index].callback[YDRV_USART_EXTI_RXNE].arg); \
        }                                                                 \
                                                                          \
        /* 2. 发送数据寄存器空中断 (TXE) */                               \
        if (LL_USART_IsActiveFlag_TXE(instance) &&                        \
            LL_USART_IsEnabledIT_TXE(instance) &&                         \
            exit_callback[index].callback[YDRV_USART_EXTI_TXE].function)  \
        {                                                                 \
            exit_callback[index].callback[YDRV_USART_EXTI_TXE].function(  \
                exit_callback[index].callback[YDRV_USART_EXTI_TXE].arg);  \
        }                                                                 \
                                                                          \
        /* 3. 传输完成中断 (TC) */                                        \
        if (LL_USART_IsActiveFlag_TC(instance) &&                         \
            LL_USART_IsEnabledIT_TC(instance) &&                          \
            exit_callback[index].callback[YDRV_USART_EXTI_TC].function)   \
        {                                                                 \
            exit_callback[index].callback[YDRV_USART_EXTI_TC].function(   \
                exit_callback[index].callback[YDRV_USART_EXTI_TC].arg);   \
        }                                                                 \
                                                                          \
        /* 4. 空闲线路检测中断 (IDLE) */                                  \
        if (LL_USART_IsActiveFlag_IDLE(instance) &&                       \
            LL_USART_IsEnabledIT_IDLE(instance) &&                        \
            exit_callback[index].callback[YDRV_USART_EXTI_IDLE].function) \
        {                                                                 \
            LL_USART_ClearFlag_IDLE(instance);                            \
            exit_callback[index].callback[YDRV_USART_EXTI_IDLE].function( \
                exit_callback[index].callback[YDRV_USART_EXTI_IDLE].arg); \
        }                                                                 \
                                                                          \
        /* 5. 奇偶校验错误中断 (PE) */                                    \
        if (LL_USART_IsActiveFlag_PE(instance) &&                         \
            LL_USART_IsEnabledIT_PE(instance) &&                          \
            exit_callback[index].callback[YDRV_USART_EXTI_PE].function)   \
        {                                                                 \
            LL_USART_ClearFlag_PE(instance);                              \
            exit_callback[index].callback[YDRV_USART_EXTI_PE].function(   \
                exit_callback[index].callback[YDRV_USART_EXTI_PE].arg);   \
        }                                                                 \
                                                                          \
        /* 6. 错误中断 (FE, NE, ORE) */                                   \
        if ((LL_USART_IsActiveFlag_FE(instance) ||                        \
             LL_USART_IsActiveFlag_NE(instance) ||                        \
             LL_USART_IsActiveFlag_ORE(instance)) &&                      \
            LL_USART_IsEnabledIT_ERROR(instance) &&                       \
            exit_callback[index].callback[YDRV_USART_EXTI_ERR].function)  \
        {                                                                 \
            if (LL_USART_IsActiveFlag_FE(instance))                       \
                LL_USART_ClearFlag_FE(instance);                          \
            if (LL_USART_IsActiveFlag_NE(instance))                       \
                LL_USART_ClearFlag_NE(instance);                          \
            if (LL_USART_IsActiveFlag_ORE(instance))                      \
                LL_USART_ClearFlag_ORE(instance);                         \
            exit_callback[index].callback[YDRV_USART_EXTI_ERR].function(  \
                exit_callback[index].callback[YDRV_USART_EXTI_ERR].arg);  \
        }                                                                 \
                                                                          \
        /* 7. LIN断开检测中断 (LBD) */                                    \
        if (LL_USART_IsActiveFlag_LBD(instance) &&                        \
            LL_USART_IsEnabledIT_LBD(instance) &&                         \
            exit_callback[index].callback[YDRV_USART_EXTI_LBD].function)  \
        {                                                                 \
            LL_USART_ClearFlag_LBD(instance);                             \
            exit_callback[index].callback[YDRV_USART_EXTI_LBD].function(  \
                exit_callback[index].callback[YDRV_USART_EXTI_LBD].arg);  \
        }                                                                 \
                                                                          \
        /* 8. CTS状态变化中断 (CTS) */                                    \
        if (LL_USART_IsActiveFlag_nCTS(instance) &&                       \
            LL_USART_IsEnabledIT_CTS(instance) &&                         \
            exit_callback[index].callback[YDRV_USART_EXTI_CTS].function)  \
        {                                                                 \
            LL_USART_ClearFlag_nCTS(instance);                            \
            exit_callback[index].callback[YDRV_USART_EXTI_CTS].function(  \
                exit_callback[index].callback[YDRV_USART_EXTI_CTS].arg);  \
        }                                                                 \
    } while (0)

// ==================== 中断处理函数实现 ====================

/**
 * @brief USART1中断处理函数
 */
void USART1_IRQHandler(void)
{
    USART_HANDLE_EXIT_IRQ(USART1, YDRV_USART_1);
}

/**
 * @brief USART2中断处理函数
 */
void USART2_IRQHandler(void)
{
    USART_HANDLE_EXIT_IRQ(USART2, YDRV_USART_2);
}

/**
 * @brief USART3/USART4共享中断处理函数
 */
void USART3_4_IRQHandler(void)
{
    // 检查是USART3还是USART4产生的中断
    USART_HANDLE_EXIT_IRQ(USART3, YDRV_USART_3);

    USART_HANDLE_EXIT_IRQ(USART4, YDRV_USART_4);
}

#ifdef USART5
/**
 * @brief USART5/USART6共享中断处理函数
 */
void USART5_6_IRQHandler(void)
{
    // 检查是USART5还是USART6产生的中断
    if (exit_callback[YDRV_USART_5].callback != NULL)
    {
        USART_HANDLE_EXIT_IRQ(USART5, YDRV_USART_5);
    }

#ifdef USART6
    if (exit_callback[YDRV_USART_6].callback != NULL)
    {
        USART_HANDLE_EXIT_IRQ(USART6, YDRV_USART_6);
    }
#endif
}
#endif
