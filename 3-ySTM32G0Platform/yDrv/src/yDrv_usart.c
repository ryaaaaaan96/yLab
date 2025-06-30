/**
 * @file yDrv_usart.c
 * @brief STM32G0 USART驱动程序实现
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 */

#include <string.h> // For memset

#include "yDrv_usart.h"
// ==================== 私有定义 ====================

/**
 * @brief 中断回调函数存储结构
 * @note 用于存储每个USART实例的中断回调函数
 */
typedef struct
{
    yDrvInterruptCallback_t callbacks[8]; // 对应 yDrvUsartInterrupt_t 中的8种中断类型
} yDrvUsartInterrupts_t;

/**
 * @brief 全局中断处理句柄数组
 * @note 存储每个USART实例的中断回调函数，按USART ID索引
 */
static yDrvUsartInterrupts_t usart_interrupt_handlers[YDRV_USART_MAX];

// ==================== 私有函数声明 ====================

/**
 * @brief 使能指定USART的时钟
 * @param usartId USART实例ID
 */
static void prv_EnableClock(yDrvUsartId_t usartId);

/**
 * @brief 禁用指定USART的时钟
 * @param usartId USART实例ID
 */
static void prv_DisableClock(yDrvUsartId_t usartId);

/**
 * @brief 配置USART相关的GPIO引脚
 * @param pConfig USART配置结构指针
 */
static void prv_ConfigGpio(const yDrvUsartConfig_t *pConfig);

/**
 * @brief 反初始化USART相关的GPIO引脚
 * @param husart USART句柄指针
 */
static void prv_DeInitGpio(const yDrvUsartHandle_t *husart);

/**
 * @brief 获取指定USART实例的APB时钟频率
 * @param instance USART外设实例指针
 * @return uint32_t APB时钟频率（Hz）
 */
static uint32_t prv_GetPclkFreq(USART_TypeDef *instance);

/**
 * @brief 获取指定USART的中断号
 * @param usartId USART实例ID
 * @return IRQn_Type 中断号，失败返回-1
 */
static IRQn_Type prv_GetUsartIrqn(yDrvUsartId_t usartId);

// ==================== 基础函数实现 ====================

yDrvStatus_t yDrvUsartInit(yDrvUsartHandle_t *husart, const yDrvUsartConfig_t *pConfig)
{
    // 参数有效性检查
    if (husart == NULL || pConfig == NULL || yDrvUsartIsIdValid(pConfig->usartId) != YDRV_OK)
    {
        return YDRV_INVALID_PARAM;
    }

    // 1. 获取USART外设实例指针
    USART_TypeDef *instance = yDrvUsartGetInstance(pConfig->usartId);
    if (instance == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 2. 使能USART外设时钟
    prv_EnableClock(pConfig->usartId);

    // 3. 配置相关GPIO引脚（TX、RX、RTS、CTS）
    prv_ConfigGpio(pConfig);

    // 4. 配置USART基本参数
    LL_USART_InitTypeDef usart_init;
    LL_USART_StructInit(&usart_init); // 初始化为默认值

    // 设置波特率
    usart_init.BaudRate = pConfig->baudRate;

    // 设置数据位长度
    usart_init.DataWidth = (pConfig->dataBits == YDRV_USART_DATA_9BIT) ? LL_USART_DATAWIDTH_9B : LL_USART_DATAWIDTH_8B;

    // 设置停止位
    usart_init.StopBits = (pConfig->stopBits == YDRV_USART_STOP_2BIT) ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1;

    // 设置校验位
    switch (pConfig->parity)
    {
    case YDRV_USART_PARITY_EVEN:
        usart_init.Parity = LL_USART_PARITY_EVEN;
        break;
    case YDRV_USART_PARITY_ODD:
        usart_init.Parity = LL_USART_PARITY_ODD;
        break;
    default:
        usart_init.Parity = LL_USART_PARITY_NONE;
        break;
    }

    // 设置传输方向
    switch (pConfig->direction)
    {
    case YDRV_USART_DIR_TX:
        usart_init.TransferDirection = LL_USART_DIRECTION_TX;
        break;
    case YDRV_USART_DIR_RX:
        usart_init.TransferDirection = LL_USART_DIRECTION_RX;
        break;
    default:
        usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
        break;
    }

    // 设置硬件流控制
    switch (pConfig->flowControl)
    {
    case YDRV_USART_FLOW_RTS:
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_RTS;
        break;
    case YDRV_USART_FLOW_CTS:
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_CTS;
        break;
    case YDRV_USART_FLOW_RTS_CTS:
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_RTS_CTS;
        break;
    default:
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
        break;
    }

    // 应用USART配置
    if (LL_USART_Init(instance, &usart_init) != SUCCESS)
    {
        return YDRV_ERROR;
    }

    // 5. 配置特定工作模式
    switch (pConfig->mode)
    {
    case YDRV_USART_MODE_SYNCHRONOUS:
        // 同步模式：使能时钟输出，配置时钟相位和极性
        LL_USART_ConfigClock(instance, LL_USART_CLOCK_ENABLE | LL_USART_CLOCKPHASE_2EDGE | LL_USART_CLOCKPOLARITY_LOW);
        break;
    case YDRV_USART_MODE_SMARTCARD:
        // 智能卡模式
        LL_USART_EnableSmartcard(instance);
        break;
    case YDRV_USART_MODE_SINGLE_WIRE:
        // 单线半双工模式
        LL_USART_EnableHalfDuplex(instance);
        break;
    case YDRV_USART_MODE_IRDA:
        // IrDA红外模式（正常功率）
        LL_USART_EnableIrda(instance, LL_USART_IRDA_POWER_NORMAL);
        break;
    case YDRV_USART_MODE_LIN:
        // LIN总线模式
        LL_USART_EnableLIN(instance);
        break;
    default: // YDRV_USART_MODE_UART
        // 标准异步UART模式
        LL_USART_ConfigAsyncMode(instance);
        break;
    }

    // 6. 使能USART外设
    LL_USART_Enable(instance);

    // 7. 初始化句柄结构
    husart->instance = instance;
    husart->usartId = pConfig->usartId;
    husart->txPin = pConfig->txPin;
    husart->rxPin = pConfig->rxPin;
    husart->rtsPin = pConfig->rtsPin;
    husart->ctsPin = pConfig->ctsPin;

    // 8. 保存句柄指针到全局数组，供中断处理使用
    usart_handles[pConfig->usartId] = husart;

    return YDRV_OK;
}

yDrvStatus_t yDrvUsartDeInit(yDrvUsartHandle_t *husart)
{
    // 参数有效性检查
    if (husart == NULL || husart->instance == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 1. 禁用USART外设
    LL_USART_Disable(husart->instance);

    // 2. 反初始化相关GPIO引脚，恢复为默认状态
    prv_DeInitGpio(husart);

    // 3. 禁用USART外设时钟以节省功耗
    prv_DisableClock(husart->usartId);

    // 4. 清理全局句柄指针并重置句柄结构
    usart_handles[husart->usartId] = NULL;
    memset(husart, 0, sizeof(yDrvUsartHandle_t));

    return YDRV_OK;
}

// ==================== 数据传输函数实现 ====================

yDrvStatus_t yDrvUsartWrite(yDrvUsartHandle_t *husart, const uint8_t *pData, uint16_t size, uint16_t *pSentSize)
{
    // 参数有效性检查
    if (husart == NULL || husart->instance == NULL || pData == NULL || size == 0)
    {
        return YDRV_INVALID_PARAM;
    }

    uint16_t sent = 0;

    // 循环发送所有数据，直到发送完成或发送缓冲区满
    while (sent < size)
    {
        if (yDrvUsartWriteByte(husart, pData[sent]) == YDRV_OK)
        {
            sent++; // 成功发送一个字节
        }
        else
        {
            break; // 发送缓冲区已满，停止发送
        }
    }

    // 返回实际发送的字节数
    if (pSentSize != NULL)
    {
        *pSentSize = sent;
    }

    return (sent > 0) ? YDRV_OK : YDRV_BUSY;
}

yDrvStatus_t yDrvUsartRead(yDrvUsartHandle_t *husart, uint8_t *pData, uint16_t size, uint16_t *pReceivedSize)
{
    // 参数有效性检查
    if (husart == NULL || husart->instance == NULL || pData == NULL || size == 0)
    {
        return YDRV_INVALID_PARAM;
    }

    uint16_t received = 0;

    // 循环接收数据，直到接收完成或接收缓冲区空
    while (received < size)
    {
        if (yDrvUsartReadByte(husart, &pData[received]) == YDRV_OK)
        {
            received++; // 成功接收一个字节
        }
        else
        {
            break; // 接收缓冲区已空，停止接收
        }
    }

    // 返回实际接收的字节数
    if (pReceivedSize != NULL)
    {
        *pReceivedSize = received;
    }

    return (received > 0) ? YDRV_OK : YDRV_BUSY;
}

// ==================== 控制函数实现 ====================

yDrvStatus_t yDrvUsartSetBaudRate(yDrvUsartHandle_t *husart, uint32_t baudRate)
{
    // 参数有效性检查
    if (husart == NULL || husart->instance == NULL || baudRate == 0)
    {
        return YDRV_INVALID_PARAM;
    }

    // 获取USART对应的APB时钟频率
    uint32_t pclk = prv_GetPclkFreq(husart->instance);

    // 使用16倍过采样率设置波特率
    LL_USART_SetBaudRate(husart->instance, pclk, LL_USART_OVERSAMPLING_16, baudRate);

    return YDRV_OK;
}

// ==================== 工具函数实现 ====================

USART_TypeDef *yDrvUsartGetInstance(yDrvUsartId_t usartId)
{
    // 根据USART ID返回对应的外设实例指针
    switch (usartId)
    {
    case YDRV_USART_1:
        return USART1;
    case YDRV_USART_2:
        return USART2;
    case YDRV_USART_3:
        return USART3;
    case YDRV_USART_4:
        return UART4; // 注意：UART4不是USART4
    case YDRV_USART_5:
        return UART5; // 注意：UART5不是USART5
    case YDRV_USART_6:
        return USART6;
    default:
        return NULL; // 无效的USART ID
    }
}

// ==================== 专用模式函数实现 ====================

/**
 * @brief 设置智能卡模式的保护时间
 * @param husart USART句柄指针
 * @param guardTime 保护时间值（0-255）
 * @note 仅在智能卡模式下有效
 */
void yDrvUsartSetSmartcardGuardTime(yDrvUsartHandle_t *husart, uint8_t guardTime)
{
    if (husart && husart->instance)
    {
        LL_USART_SetGuardTime(husart->instance, guardTime);
    }
}

/**
 * @brief 设置IrDA模式的功耗模式
 * @param husart USART句柄指针
 * @param powerMode true=低功耗模式，false=正常功率模式
 * @note 仅在IrDA模式下有效
 */
void yDrvUsartSetIrdaPowerMode(yDrvUsartHandle_t *husart, bool powerMode)
{
    if (husart && husart->instance)
    {
        uint32_t mode = powerMode ? LL_USART_IRDA_POWER_LOW : LL_USART_IRDA_POWER_NORMAL;
        LL_USART_SetIrdaPowerMode(husart->instance, mode);
    }
}

/**
 * @brief 设置LIN模式的中断检测长度
 * @param husart USART句柄指针
 * @param breakLength true=11位中断检测，false=10位中断检测
 * @note 仅在LIN模式下有效
 */
void yDrvUsartSetLinBreakLength(yDrvUsartHandle_t *husart, bool breakLength)
{
    if (husart && husart->instance)
    {
        uint32_t length = breakLength ? LL_USART_LINBREAK_DETECT_11B : LL_USART_LINBREAK_DETECT_10B;
        LL_USART_SetLINBrkDetectionLen(husart->instance, length);
    }
}

/**
 * @brief 发送LIN中断信号
 * @param husart USART句柄指针
 * @note 仅在LIN模式下有效
 */
void yDrvUsartSendLinBreak(yDrvUsartHandle_t *husart)
{
    if (husart && husart->instance)
    {
        LL_USART_RequestBreakSending(husart->instance);
    }
}

// ==================== 中断管理函数实现 ====================

yDrvStatus_t yDrvUsartEnableInterrupt(yDrvUsartHandle_t *husart, uint32_t prio)
{
    // 参数有效性检查
    if (husart == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 获取对应的中断号
    IRQn_Type irqn = prv_GetUsartIrqn(husart->usartId);
    if (irqn < 0)
    {
        return YDRV_NOT_SUPPORTED; // 不支持的USART实例
    }

    // 设置中断优先级并使能中断
    NVIC_SetPriority(irqn, prio);
    NVIC_EnableIRQ(irqn);

    return YDRV_OK;
}

yDrvStatus_t yDrvUsartDisableInterrupt(yDrvUsartHandle_t *husart)
{
    // 参数有效性检查
    if (husart == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 获取对应的中断号
    IRQn_Type irqn = prv_GetUsartIrqn(husart->usartId);
    if (irqn < 0)
    {
        return YDRV_NOT_SUPPORTED; // 不支持的USART实例
    }

    // 禁用中断
    NVIC_DisableIRQ(irqn);
    return YDRV_OK;
}

yDrvStatus_t yDrvUsartRegisterCallback(yDrvUsartHandle_t *husart, uint32_t interruptType, yDrvInterruptCallback_t callback)
{
    // 参数有效性检查
    if (husart == NULL || callback == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 遍历所有可能的中断类型位
    for (uint32_t i = 0; i < 8; i++)
    {
        uint32_t current_it = 1 << i; // 计算当前位对应的中断类型

        // 如果当前中断类型被设置
        if (interruptType & current_it)
        {
            // 注册回调函数
            usart_interrupt_handlers[husart->usartId].callbacks[i] = callback;

            // 在外设中使能对应的中断
            switch ((yDrvUsartInterrupt_t)current_it)
            {
            case YDRV_USART_IT_TXE: // 发送数据寄存器空中断
                LL_USART_EnableIT_TXE(husart->instance);
                break;
            case YDRV_USART_IT_RXNE: // 接收数据寄存器非空中断
                LL_USART_EnableIT_RXNE(husart->instance);
                break;
            case YDRV_USART_IT_TC: // 发送完成中断
                LL_USART_EnableIT_TC(husart->instance);
                break;
            case YDRV_USART_IT_IDLE: // 空闲线路检测中断
                LL_USART_EnableIT_IDLE(husart->instance);
                break;
            case YDRV_USART_IT_PE: // 奇偶校验错误中断
                LL_USART_EnableIT_PE(husart->instance);
                break;
            case YDRV_USART_IT_ERR: // 错误中断（帧错误、噪声错误、过载错误）
                LL_USART_EnableIT_ERROR(husart->instance);
                break;
            case YDRV_USART_IT_LBD: // LIN中断检测中断
                LL_USART_EnableIT_LBD(husart->instance);
                break;
            case YDRV_USART_IT_CTS: // CTS中断
                LL_USART_EnableIT_CTS(husart->instance);
                break;
            default:
                break;
            }
        }
    }
    return YDRV_OK;
}

yDrvStatus_t yDrvUsartUnregisterCallback(yDrvUsartHandle_t *husart, uint32_t interruptType)
{
    // 参数有效性检查
    if (husart == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 遍历所有可能的中断类型位
    for (uint32_t i = 0; i < 8; i++)
    {
        uint32_t current_it = 1 << i; // 计算当前位对应的中断类型

        // 如果当前中断类型被设置
        if (interruptType & current_it)
        {
            // 清除回调函数
            usart_interrupt_handlers[husart->usartId].callbacks[i] = NULL;

            // 在外设中禁用对应的中断
            switch ((yDrvUsartInterrupt_t)current_it)
            {
            case YDRV_USART_IT_TXE: // 发送数据寄存器空中断
                LL_USART_DisableIT_TXE(husart->instance);
                break;
            case YDRV_USART_IT_RXNE: // 接收数据寄存器非空中断
                LL_USART_DisableIT_RXNE(husart->instance);
                break;
            case YDRV_USART_IT_TC: // 发送完成中断
                LL_USART_DisableIT_TC(husart->instance);
                break;
            case YDRV_USART_IT_IDLE: // 空闲线路检测中断
                LL_USART_DisableIT_IDLE(husart->instance);
                break;
            case YDRV_USART_IT_PE: // 奇偶校验错误中断
                LL_USART_DisableIT_PE(husart->instance);
                break;
            case YDRV_USART_IT_ERR: // 错误中断
                LL_USART_DisableIT_ERROR(husart->instance);
                break;
            case YDRV_USART_IT_LBD: // LIN中断检测中断
                LL_USART_DisableIT_LBD(husart->instance);
                break;
            case YDRV_USART_IT_CTS: // CTS中断
                LL_USART_DisableIT_CTS(husart->instance);
                break;
            default:
                break;
            }
        }
    }
    return YDRV_OK;
}

uint8_t yDrvUsartGetEnabledInterrupts(yDrvUsartHandle_t *husart)
{
    // 参数有效性检查
    if (husart == NULL || husart->instance == NULL)
        return 0;

    uint8_t enabled_its = 0;

    // 检查各种中断是否已使能，构建返回值
    if (LL_USART_IsEnabledIT_TXE(husart->instance))
        enabled_its |= YDRV_USART_IT_TXE; // 发送数据寄存器空中断
    if (LL_USART_IsEnabledIT_RXNE(husart->instance))
        enabled_its |= YDRV_USART_IT_RXNE; // 接收数据寄存器非空中断
    if (LL_USART_IsEnabledIT_TC(husart->instance))
        enabled_its |= YDRV_USART_IT_TC; // 发送完成中断
    if (LL_USART_IsEnabledIT_IDLE(husart->instance))
        enabled_its |= YDRV_USART_IT_IDLE; // 空闲线路检测中断
    if (LL_USART_IsEnabledIT_PE(husart->instance))
        enabled_its |= YDRV_USART_IT_PE; // 奇偶校验错误中断
    if (LL_USART_IsEnabledIT_ERROR(husart->instance))
        enabled_its |= YDRV_USART_IT_ERR; // 错误中断
    if (LL_USART_IsEnabledIT_LBD(husart->instance))
        enabled_its |= YDRV_USART_IT_LBD; // LIN中断检测中断
    if (LL_USART_IsEnabledIT_CTS(husart->instance))
        enabled_its |= YDRV_USART_IT_CTS; // CTS中断

    return enabled_its;
}

// ==================== 私有函数实现 ====================

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
#ifdef USART3 // STM32G0部分型号有USART3
    case YDRV_USART_3:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
        break;
#endif
#ifdef USART4 // STM32G0部分型号有USART4
    case YDRV_USART_4:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART4);
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
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART4);
        break;
    case YDRV_USART_5:
        LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART5);
        break;
    case YDRV_USART_6:
        LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART6);
        break;
    default:
        break;
    }
}

/**
 * @brief 配置USART相关的GPIO引脚
 * @param pConfig USART配置结构指针
 * @note 配置TX、RX引脚为复用功能，根据流控设置配置RTS、CTS引脚
 */
static void prv_ConfigGpio(const yDrvUsartConfig_t *pConfig)
{
    // 配置TX引脚：复用功能、高速、上拉、推挽输出
    yDrvGpioInit(pConfig->txPin, YDRV_GPIO_MODE_AF, YDRV_GPIO_SPEED_HIGH, YDRV_GPIO_PUPD_PULLUP, YDRV_GPIO_TYPE_PUSHPULL);

    // 配置RX引脚：复用功能、高速、上拉、推挽输出
    yDrvGpioInit(pConfig->rxPin, YDRV_GPIO_MODE_AF, YDRV_GPIO_SPEED_HIGH, YDRV_GPIO_PUPD_PULLUP, YDRV_GPIO_TYPE_PUSHPULL);

    // 如果使能RTS流控，配置RTS引脚
    if (pConfig->flowControl == YDRV_USART_FLOW_RTS || pConfig->flowControl == YDRV_USART_FLOW_RTS_CTS)
    {
        yDrvGpioInit(pConfig->rtsPin, YDRV_GPIO_MODE_AF, YDRV_GPIO_SPEED_HIGH, YDRV_GPIO_PUPD_NONE, YDRV_GPIO_TYPE_PUSHPULL);
    }

    // 如果使能CTS流控，配置CTS引脚
    if (pConfig->flowControl == YDRV_USART_FLOW_CTS || pConfig->flowControl == YDRV_USART_FLOW_RTS_CTS)
    {
        yDrvGpioInit(pConfig->ctsPin, YDRV_GPIO_MODE_AF, YDRV_GPIO_SPEED_HIGH, YDRV_GPIO_PUPD_NONE, YDRV_GPIO_TYPE_PUSHPULL);
    }
}

/**
 * @brief 反初始化USART相关的GPIO引脚
 * @param husart USART句柄指针
 * @note 将所有使用的GPIO引脚恢复为默认状态
 */
static void prv_DeInitGpio(const yDrvUsartHandle_t *husart)
{
    yDrvGpioDeInit(husart->txPin);  // 反初始化TX引脚
    yDrvGpioDeInit(husart->rxPin);  // 反初始化RX引脚
    yDrvGpioDeInit(husart->rtsPin); // 反初始化RTS引脚
    yDrvGpioDeInit(husart->ctsPin); // 反初始化CTS引脚
}

/**
 * @brief 获取指定USART实例的APB时钟频率
 * @param instance USART外设实例指针
 * @return uint32_t APB时钟频率（Hz）
 * @note USART1和USART6使用APB2时钟，其他使用APB1时钟
 */
static uint32_t prv_GetPclkFreq(USART_TypeDef *instance)
{
    if (instance == USART1 || instance == USART6)
    {
        // USART1和USART6在APB2总线上
        return LL_RCC_GetAPB2CLKFreq(LL_RCC_GetSysClkFreq());
    }
    else
    {
        // USART2、USART3、UART4、UART5在APB1总线上
        return LL_RCC_GetAPB1CLKFreq(LL_RCC_GetSysClkFreq());
    }
}

/**
 * @brief 获取指定USART的中断号
 * @param usartId USART实例ID
 * @return IRQn_Type 中断号，失败返回-1
 */
static IRQn_Type prv_GetUsartIrqn(yDrvUsartId_t usartId)
{
    switch (usartId)
    {
    case YDRV_USART_1:
        return USART1_IRQn;
    case YDRV_USART_2:
        return USART2_IRQn;
    case YDRV_USART_3:
        return USART3_IRQn;
    case YDRV_USART_4:
        return UART4_IRQn;
    case YDRV_USART_5:
        return UART5_IRQn;
    case YDRV_USART_6:
        return USART6_IRQn;
    default:
        return (IRQn_Type)-1; // 无效的USART ID
    }
}

/**
 * @brief USART中断统一处理函数
 * @param usartId USART实例ID
 * @note 此函数被各个USART的中断处理函数调用，统一处理中断逻辑
 */
/**
 * @brief 获取指定USART的中断号
 * @param usartId USART实例ID
 * @return IRQn_Type 中断号，失败返回-1
 */
static IRQn_Type prv_GetUsartIrqn(yDrvUsartId_t usartId)
{
    switch (usartId)
    {
    case YDRV_USART_1:
        return USART1_IRQn;
    case YDRV_USART_2:
        return USART2_IRQn;
    case YDRV_USART_3:
        return USART3_IRQn;
    case YDRV_USART_4:
        return UART4_IRQn;
    case YDRV_USART_5:
        return UART5_IRQn;
    case YDRV_USART_6:
        return USART6_IRQn;
    default:
        return (IRQn_Type)-1; // 无效的USART ID
    }
}

// ==================== 中断处理函数（优化版本） ====================

/**
 * @brief USART1中断处理函数（优化版本）
 * @note 此函数由NVIC调用，使用内联优化提高性能
 */
void USART1_IRQHandler(void)
{
    yDrvUsartHandle_t *husart = usart_handles[YDRV_USART_1];
    if (husart == NULL)
        return;

    uint32_t sr = USART1->SR;
    uint32_t cr1 = USART1->CR1;
    uint32_t cr2 = USART1->CR2;
    uint32_t cr3 = USART1->CR3;
    yDrvUsartInterrupts_t *cb = &usart_interrupt_handlers[YDRV_USART_1];

    // 按优先级处理中断
    if ((sr & USART_SR_RXNE) && (cr1 & USART_CR1_RXNEIE) && cb->callbacks[1])
        cb->callbacks[1](husart);
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE) && cb->callbacks[0])
        cb->callbacks[0](husart);
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE) && cb->callbacks[2])
        cb->callbacks[2](husart);
    if ((sr & USART_SR_IDLE) && (cr1 & USART_CR1_IDLEIE) && cb->callbacks[3])
        cb->callbacks[3](husart);
    if ((sr & USART_SR_PE) && (cr1 & USART_CR1_PEIE) && cb->callbacks[4])
        cb->callbacks[4](husart);
    if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) && (cr3 & USART_CR3_EIE) && cb->callbacks[5])
        cb->callbacks[5](husart);
    if ((sr & USART_SR_LBD) && (cr2 & USART_CR2_LBDIE) && cb->callbacks[6])
        cb->callbacks[6](husart);
    if ((sr & USART_SR_CTS) && (cr3 & USART_CR3_CTSIE) && cb->callbacks[7])
        cb->callbacks[7](husart);
}

/**
 * @brief USART2中断处理函数（优化版本）
 * @note 此函数由NVIC调用，使用内联优化提高性能
 */
void USART2_IRQHandler(void)
{
    yDrvUsartHandle_t *husart = usart_handles[YDRV_USART_2];
    if (husart == NULL)
        return;

    uint32_t sr = USART2->SR;
    uint32_t cr1 = USART2->CR1;
    uint32_t cr2 = USART2->CR2;
    uint32_t cr3 = USART2->CR3;
    yDrvUsartInterrupts_t *cb = &usart_interrupt_handlers[YDRV_USART_2];

    // 按优先级处理中断
    if ((sr & USART_SR_RXNE) && (cr1 & USART_CR1_RXNEIE) && cb->callbacks[1])
        cb->callbacks[1](husart);
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE) && cb->callbacks[0])
        cb->callbacks[0](husart);
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE) && cb->callbacks[2])
        cb->callbacks[2](husart);
    if ((sr & USART_SR_IDLE) && (cr1 & USART_CR1_IDLEIE) && cb->callbacks[3])
        cb->callbacks[3](husart);
    if ((sr & USART_SR_PE) && (cr1 & USART_CR1_PEIE) && cb->callbacks[4])
        cb->callbacks[4](husart);
    if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) && (cr3 & USART_CR3_EIE) && cb->callbacks[5])
        cb->callbacks[5](husart);
    if ((sr & USART_SR_LBD) && (cr2 & USART_CR2_LBDIE) && cb->callbacks[6])
        cb->callbacks[6](husart);
    if ((sr & USART_SR_CTS) && (cr3 & USART_CR3_CTSIE) && cb->callbacks[7])
        cb->callbacks[7](husart);
}

/**
 * @brief USART3中断处理函数（优化版本）
 * @note 此函数由NVIC调用，使用内联优化提高性能
 */
void USART3_IRQHandler(void)
{
    yDrvUsartHandle_t *husart = usart_handles[YDRV_USART_3];
    if (husart == NULL)
        return;

    uint32_t sr = USART3->SR;
    uint32_t cr1 = USART3->CR1;
    uint32_t cr2 = USART3->CR2;
    uint32_t cr3 = USART3->CR3;
    yDrvUsartInterrupts_t *cb = &usart_interrupt_handlers[YDRV_USART_3];

    // 按优先级处理中断
    if ((sr & USART_SR_RXNE) && (cr1 & USART_CR1_RXNEIE) && cb->callbacks[1])
        cb->callbacks[1](husart);
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE) && cb->callbacks[0])
        cb->callbacks[0](husart);
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE) && cb->callbacks[2])
        cb->callbacks[2](husart);
    if ((sr & USART_SR_IDLE) && (cr1 & USART_CR1_IDLEIE) && cb->callbacks[3])
        cb->callbacks[3](husart);
    if ((sr & USART_SR_PE) && (cr1 & USART_CR1_PEIE) && cb->callbacks[4])
        cb->callbacks[4](husart);
    if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) && (cr3 & USART_CR3_EIE) && cb->callbacks[5])
        cb->callbacks[5](husart);
    if ((sr & USART_SR_LBD) && (cr2 & USART_CR2_LBDIE) && cb->callbacks[6])
        cb->callbacks[6](husart);
    if ((sr & USART_SR_CTS) && (cr3 & USART_CR3_CTSIE) && cb->callbacks[7])
        cb->callbacks[7](husart);
}

/**
 * @brief UART4中断处理函数（优化版本）
 * @note 此函数由NVIC调用，使用内联优化提高性能
 */
void UART4_IRQHandler(void)
{
    yDrvUsartHandle_t *husart = usart_handles[YDRV_USART_4];
    if (husart == NULL)
        return;

    uint32_t sr = UART4->SR;
    uint32_t cr1 = UART4->CR1;
    uint32_t cr2 = UART4->CR2;
    uint32_t cr3 = UART4->CR3;
    yDrvUsartInterrupts_t *cb = &usart_interrupt_handlers[YDRV_USART_4];

    // 按优先级处理中断
    if ((sr & USART_SR_RXNE) && (cr1 & USART_CR1_RXNEIE) && cb->callbacks[1])
        cb->callbacks[1](husart);
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE) && cb->callbacks[0])
        cb->callbacks[0](husart);
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE) && cb->callbacks[2])
        cb->callbacks[2](husart);
    if ((sr & USART_SR_IDLE) && (cr1 & USART_CR1_IDLEIE) && cb->callbacks[3])
        cb->callbacks[3](husart);
    if ((sr & USART_SR_PE) && (cr1 & USART_CR1_PEIE) && cb->callbacks[4])
        cb->callbacks[4](husart);
    if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) && (cr3 & USART_CR3_EIE) && cb->callbacks[5])
        cb->callbacks[5](husart);
    if ((sr & USART_SR_LBD) && (cr2 & USART_CR2_LBDIE) && cb->callbacks[6])
        cb->callbacks[6](husart);
    if ((sr & USART_SR_CTS) && (cr3 & USART_CR3_CTSIE) && cb->callbacks[7])
        cb->callbacks[7](husart);
}

/**
 * @brief UART5中断处理函数（优化版本）
 * @note 此函数由NVIC调用，使用内联优化提高性能
 */
void UART5_IRQHandler(void)
{
    yDrvUsartHandle_t *husart = usart_handles[YDRV_USART_5];
    if (husart == NULL)
        return;

    uint32_t sr = UART5->SR;
    uint32_t cr1 = UART5->CR1;
    uint32_t cr2 = UART5->CR2;
    uint32_t cr3 = UART5->CR3;
    yDrvUsartInterrupts_t *cb = &usart_interrupt_handlers[YDRV_USART_5];

    // 按优先级处理中断
    if ((sr & USART_SR_RXNE) && (cr1 & USART_CR1_RXNEIE) && cb->callbacks[1])
        cb->callbacks[1](husart);
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE) && cb->callbacks[0])
        cb->callbacks[0](husart);
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE) && cb->callbacks[2])
        cb->callbacks[2](husart);
    if ((sr & USART_SR_IDLE) && (cr1 & USART_CR1_IDLEIE) && cb->callbacks[3])
        cb->callbacks[3](husart);
    if ((sr & USART_SR_PE) && (cr1 & USART_CR1_PEIE) && cb->callbacks[4])
        cb->callbacks[4](husart);
    if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) && (cr3 & USART_CR3_EIE) && cb->callbacks[5])
        cb->callbacks[5](husart);
    if ((sr & USART_SR_LBD) && (cr2 & USART_CR2_LBDIE) && cb->callbacks[6])
        cb->callbacks[6](husart);
    if ((sr & USART_SR_CTS) && (cr3 & USART_CR3_CTSIE) && cb->callbacks[7])
        cb->callbacks[7](husart);
}

/**
 * @brief USART6中断处理函数（优化版本）
 * @note 此函数由NVIC调用，使用内联优化提高性能
 */
void USART6_IRQHandler(void)
{
    yDrvUsartHandle_t *husart = usart_handles[YDRV_USART_6];
    if (husart == NULL)
        return;

    uint32_t sr = USART6->SR;
    uint32_t cr1 = USART6->CR1;
    uint32_t cr2 = USART6->CR2;
    uint32_t cr3 = USART6->CR3;
    yDrvUsartInterrupts_t *cb = &usart_interrupt_handlers[YDRV_USART_6];

    // 按优先级处理中断
    if ((sr & USART_SR_RXNE) && (cr1 & USART_CR1_RXNEIE) && cb->callbacks[1])
        cb->callbacks[1](husart);
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE) && cb->callbacks[0])
        cb->callbacks[0](husart);
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE) && cb->callbacks[2])
        cb->callbacks[2](husart);
    if ((sr & USART_SR_IDLE) && (cr1 & USART_CR1_IDLEIE) && cb->callbacks[3])
        cb->callbacks[3](husart);
    if ((sr & USART_SR_PE) && (cr1 & USART_CR1_PEIE) && cb->callbacks[4])
        cb->callbacks[4](husart);
    if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) && (cr3 & USART_CR3_EIE) && cb->callbacks[5])
        cb->callbacks[5](husart);
    if ((sr & USART_SR_LBD) && (cr2 & USART_CR2_LBDIE) && cb->callbacks[6])
        cb->callbacks[6](husart);
    if ((sr & USART_SR_CTS) && (cr3 & USART_CR3_CTSIE) && cb->callbacks[7])
        cb->callbacks[7](husart);
}
