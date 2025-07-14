/**
 * @file yDrv_usart.h
 * @brief STM32F4 USART驱动程序头文件
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供STM32F4系列MCU的USART/UART通用驱动接口，支持多种工作模式和配置选项
 *
 * @par 主要特性:
 * - 支持USART1-6和UART4-5
 * - 支持多种工作模式：UART、同步、智能卡、单线、IrDA、LIN
 * - 支持硬件流控制（RTS/CTS）
 * - 完整的中断管理系统
 * - 内联优化高频操作函数
 * - 统一的错误处理和状态查询
 * - 非阻塞式数据传输接口
 *
 * @par 更新历史:
 * - v2.0 (2025): 添加完整的中断管理支持，优化API接口一致性
 * - v1.0 (2024): 初始版本，支持基本的轮询模式操作
 */

#ifndef YDRV_USART_H
#define YDRV_USART_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_bus.h"

#include "yDrv_basic.h"
    // ==================== USART配置枚举 ====================

    // USART实例枚举
    typedef enum
    {
        YDRV_USART_1 = 0,
        YDRV_USART_2,
        YDRV_USART_3,
        YDRV_USART_4,
#ifdef USART5
        YDRV_USART_5,
#endif
#ifdef USART6
        YDRV_USART_6,
#endif
        YDRV_USART_MAX
    } yDrvUsartId_t;

    // 数据位枚举
    typedef enum
    {
        YDRV_USART_DATA_7BIT = LL_USART_DATAWIDTH_7B,
        YDRV_USART_DATA_8BIT = LL_USART_DATAWIDTH_8B,
        YDRV_USART_DATA_9BIT = LL_USART_DATAWIDTH_9B,
    } yDrvUsartDataBits_t;

    // 停止位枚举
    typedef enum
    {
        YDRV_USART_STOP_0_5BIT = LL_USART_STOPBITS_0_5,
        YDRV_USART_STOP_1BIT = LL_USART_STOPBITS_1,
        YDRV_USART_STOP_1_5BIT = LL_USART_STOPBITS_1_5,
        YDRV_USART_STOP_2BIT = LL_USART_STOPBITS_2,
    } yDrvUsartStopBits_t;

    // 校验位枚举
    typedef enum
    {
        YDRV_USART_PARITY_NONE = LL_USART_PARITY_NONE,
        YDRV_USART_PARITY_EVEN = LL_USART_PARITY_EVEN,
        YDRV_USART_PARITY_ODD = LL_USART_PARITY_ODD
    } yDrvUsartParity_t;

    // 传输方向枚举
    typedef enum
    {
        YDRV_USART_DIR_TX = LL_USART_DIRECTION_TX,
        YDRV_USART_DIR_RX = LL_USART_DIRECTION_RX,
        YDRV_USART_DIR_TX_RX = LL_USART_DIRECTION_TX_RX
    } yDrvUsartDirection_t;

    // 硬件流控枚举
    typedef enum
    {
        YDRV_USART_FLOW_NONE = LL_USART_HWCONTROL_NONE,
        YDRV_USART_FLOW_RTS = LL_USART_HWCONTROL_RTS,
        YDRV_USART_FLOW_CTS = LL_USART_HWCONTROL_CTS,
        YDRV_USART_FLOW_RTS_CTS = LL_USART_HWCONTROL_RTS_CTS
    } yDrvUsartFlowControl_t;

    // USART工作模式枚举
    typedef enum
    {
        YDRV_USART_MODE_ASYNCHRONOUS = 0, // 标准UART模式
        YDRV_USART_MODE_SYNCHRONOUS,      // 同步模式（带时钟输出）
        YDRV_USART_MODE_SMARTCARD,        // 智能卡模式
        YDRV_USART_MODE_SINGLE_WIRE,      // 单线半双工模式
        YDRV_USART_MODE_IRDA,             // IrDA红外模式
        YDRV_USART_MODE_LIN               // LIN总线模式
    } yDrvUsartMode_t;

    // ==================== USART配置结构体 ====================

    // USART初始化配置结构体
    typedef struct
    {
        yDrvUsartId_t usartId;              // USART实例名称
        yDrvGpioPin_t txPin;                // TX引脚配置
        yDrvGpioPin_t rxPin;                // RX引脚配置
        yDrvGpioPin_t rtsPin;               // RTS引脚配置
        yDrvGpioPin_t ctsPin;               // CTS引脚配置
        uint32_t baudRate;                  // 波特率
        yDrvUsartDataBits_t dataBits;       // 数据位
        yDrvUsartStopBits_t stopBits;       // 停止位
        yDrvUsartParity_t parity;           // 校验位
        yDrvUsartDirection_t direction;     // 传输方向
        yDrvUsartFlowControl_t flowControl; // 硬件流控
        yDrvUsartMode_t mode;               // 工作模式
        uint8_t txAF;                       // 复用引脚
        uint8_t rxAF;                       // 复用引脚
        uint8_t ctsAF;                      // 复用引脚
        uint8_t rtsAF;                      // 复用引脚
    } yDrvUsartConfig_t;

/**
 * @brief USART配置结构体默认初始化宏
 * @note 提供最常用的默认配置，适用于标准UART通信
 */
#define YDRV_USART_CONFIG_DEFAULT()           \
    ((yDrvUsartConfig_t){                     \
        .usartId = YDRV_USART_MAX,            \
        .txPin = YDRV_PIN_NULL,               \
        .rxPin = YDRV_PIN_NULL,               \
        .rtsPin = YDRV_PIN_NULL,              \
        .ctsPin = YDRV_PIN_NULL,              \
        .baudRate = 115200,                   \
        .dataBits = YDRV_USART_DATA_8BIT,     \
        .stopBits = YDRV_USART_STOP_1BIT,     \
        .parity = YDRV_USART_PARITY_NONE,     \
        .direction = YDRV_USART_DIR_TX_RX,    \
        .flowControl = YDRV_USART_FLOW_NONE,  \
        .mode = YDRV_USART_MODE_ASYNCHRONOUS, \
        .txAF = 0,                            \
        .rxAF = 0,                            \
        .ctsAF = 0,                           \
        .rtsAF = 0,                           \
    })

    // ==================== USART句柄结构体 ====================

    // USART句柄结构体（极简版）
    typedef struct
    {
        USART_TypeDef *instance; // USART硬件实例
        IRQn_Type IRQ;
        yDrvUsartId_t usartId;     // USART ID
        yDrvGpioInfo_t txPinInfo;  // TX引脚（用于反初始化时复位GPIO）
        yDrvGpioInfo_t rxPinInfo;  // RX引脚（用于反初始化时复位GPIO）
        yDrvGpioInfo_t rtsPinInfo; // RTS引脚（用于反初始化时复位GPIO）
        yDrvGpioInfo_t ctsPinInfo; // CTS引脚（用于反初始化时复位GPIO）
        uint32_t flagBtyeSend;     // 是否使用uint16做数据
    } yDrvUsartHandle_t;

/**
 * @brief USART句柄结构体默认初始化宏
 * @note 提供安全的默认初始化值
 */
#define YDRV_USART_HANDLE_DEFAULT() \
    ((yDrvUsartHandle_t){           \
        .instance = NULL,           \
        .IRQ = (IRQn_Type)0,        \
        .usartId = YDRV_USART_MAX,  \
        .txPinInfo = {NULL, 0},     \
        .rxPinInfo = {NULL, 0},     \
        .rtsPinInfo = {NULL, 0},    \
        .ctsPinInfo = {NULL, 0},    \
        .flagBtyeSend = 0,          \
    })

    // ==================== USART中断管理 ====================

    // USART中断类型枚举 - 按位掩码设计，支持组合操作
    typedef enum
    {
        YDRV_USART_EXTI_TXE = 0, // 发送寄存器空中断
        YDRV_USART_EXTI_RXNE,    // 接收寄存器非空中断
        YDRV_USART_EXTI_TC,      // 传输完成中断
        YDRV_USART_EXTI_IDLE,    // 空闲线路检测中断
        YDRV_USART_EXTI_PE,      // 奇偶校验错误中断
        YDRV_USART_EXTI_ERR,     // 错误中断（帧错误、噪声错误、溢出错误）
        YDRV_USART_EXTI_LBD,     // LIN断开检测中断
        YDRV_USART_EXTI_CTS,     // CTS状态变化中断
        YDRV_USART_EXTI_MAX      // 所有中断
    } yDrvUsartExti_t;

    /**
     * @brief EXIT配置句柄结构体
     */
    typedef struct
    {
        yDrvUsartExti_t trigger;
        uint32_t prio;
        void (*function)(void *para); // 回调函数指针
        void *arg;                    // 回调函数参数
        uint32_t enable;
    } yDrvUsartExtiConfig_t;

/**
 * @brief USART中断配置结构体默认初始化宏
 * @note 提供安全的默认中断配置
 */
#define YDRV_USART_EXTI_CONFIG_DEFAULT() \
    ((yDrvUsartExtiConfig_t){            \
        .trigger = YDRV_USART_EXTI_MAX,  \
        .prio = 0,                       \
        .function = NULL,                \
        .arg = NULL,                     \
        .enable = 0,                     \
    })

    // ==================== USART基础函数 ====================

    /**
     * @brief 初始化USART
     * @param handle USART句柄指针
     * @param config 配置参数指针（包含USART ID和引脚配置）
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartInitStatic(const yDrvUsartConfig_t *config,
                                     yDrvUsartHandle_t *handle);
    /**
     * @brief 反初始化USART
     * @param handle USART句柄指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartDeInitStatic(yDrvUsartHandle_t *handle);

    /**
     * @brief 初始化USART配置结构体为默认值（内联优化）
     * @param config 配置结构体指针
     * @retval 无
     */
    void yDrvUsartConfigStructInit(yDrvUsartConfig_t *config);

    /**
     * @brief 初始化USART句柄结构体为默认值（内联优化）
     * @param handle 句柄结构体指针
     * @retval 无
     */
    void yDrvUsartHandleStructInit(yDrvUsartHandle_t *handle);

    /**
     * @brief 初始化USART中断配置结构体为默认值
     * @param extiConfig 中断配置结构体指针
     * @retval 无
     */
    void yDrvUsartExtiConfigStructInit(yDrvUsartExtiConfig_t *extiConfig);

    // ==================== USART数据传输函数 ====================

    /**
     * @brief 发送单个字节（非阻塞，内联优化）
     * @param handle USART句柄指针
     * @param data 要发送的字节
     * @retval yDrv状态
     */
    YLIB_INLINE int32_t yDrvUsartWriteByte(yDrvUsartHandle_t *handle, const void *data)
    {
        if (!LL_USART_IsActiveFlag_TXE(handle->instance))
        {
            return 0;
        }
        LL_USART_TransmitData9(handle->instance,
                               (handle->flagBtyeSend == 0) ? ((uint16_t)*((const uint8_t *)data)) : *((const uint16_t *)data));
        return (handle->flagBtyeSend == 0) ? 1 : 2;
    }

    /**
     * @brief 接收单个字节（非阻塞，内联优化）
     * @param handle USART句柄指针
     * @param pData 接收数据指针
     * @retval yDrv状态
     */
    YLIB_INLINE int32_t yDrvUsartReadByte(yDrvUsartHandle_t *handle, void *pData)
    {
        if (pData == NULL)
        {
            return 0;
        }

        if (!LL_USART_IsActiveFlag_RXNE(handle->instance))
        {
            return 0;
        }

        if (handle->flagBtyeSend == 0)
        {
            *((uint8_t *)pData) = LL_USART_ReceiveData8(handle->instance);
            return 1;
        }
        else
        {
            *((uint16_t *)pData) = LL_USART_ReceiveData9(handle->instance);
            return 2;
        }
    }

    // ==================== USART状态查询函数（内联优化） ====================

    /**
     * @brief 检查发送器是否空闲（内联优化）
     * @param handle USART句柄指针
     * @retval YDRV_OK=空闲, YDRV_BUSY=忙碌, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsTxEmpty(yDrvUsartHandle_t *handle)
    {
        return LL_USART_IsActiveFlag_TXE(handle->instance) ? YDRV_OK : YDRV_BUSY;
    }

    /**
     * @brief 检查接收器是否有数据（内联优化）
     * @param handle USART句柄指针
     * @retval YDRV_OK=无数据, YDRV_BUSY=有数据 , YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsRxEmpty(yDrvUsartHandle_t *handle)
    {
        return LL_USART_IsActiveFlag_RXNE(handle->instance) ? YDRV_BUSY : YDRV_OK;
    }

    /**
     * @brief 检查传输是否完成（内联优化）
     * @param handle USART句柄指针
     * @retval YDRV_OK=传输完成, YDRV_BUSY=传输未完成, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsTransmitComplete(yDrvUsartHandle_t *handle)
    {
        return LL_USART_IsActiveFlag_TC(handle->instance) ? YDRV_OK : YDRV_BUSY;
    }

    // ==================== USART控制函数 ====================

    /**
     * @brief 使能USART（内联优化）
     * @param handle USART句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartEnable(yDrvUsartHandle_t *handle)
    {
        LL_USART_Enable(handle->instance);
        return YDRV_OK;
    }

    /**
     * @brief 禁用USART（内联优化）
     * @param handle USART句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartDisable(yDrvUsartHandle_t *handle)
    {
        LL_USART_Disable(handle->instance);
        return YDRV_OK;
    }

    /**
     * @brief 检查USART句柄是否有效（内联优化）
     * @param handle USART句柄指针
     * @retval YDRV_OK=句柄有效, YDRV_INVALID_PARAM=句柄无效
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartHandleIsValid(yDrvUsartHandle_t *handle)
    {
        if (handle == NULL || handle->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        return YDRV_OK;
    }

    // ==================== USART专用模式函数 ====================

    // /**
    //  * @brief 设置智能卡保护时间
    //  * @param handle USART句柄指针
    //  * @param guardTime 保护时间（0-255）
    //  * @note 仅在有效句柄时生效，无效句柄会被忽略
    //  */
    // void yDrvUsartSetSmartcardGuardTime(yDrvUsartHandle_t *handle, uint8_t guardTime);

    // /**
    //  * @brief 设置IrDA功率模式
    //  * @param handle USART句柄指针
    //  * @param powerMode 功率模式（true=低功率，false=正常功率）
    //  * @note 仅在有效句柄时生效，无效句柄会被忽略
    //  */
    // void yDrvUsartSetIrdaPowerMode(yDrvUsartHandle_t *handle, bool powerMode);

    // /**
    //  * @brief 设置LIN断开检测长度
    //  * @param handle USART句柄指针
    //  * @param breakLength 断开检测长度（true=11位，false=10位）
    //  * @note 仅在有效句柄时生效，无效句柄会被忽略
    //  */
    // void yDrvUsartSetLinBreakLength(yDrvUsartHandle_t *handle, bool breakLength);

    // /**
    //  * @brief 发送LIN断开信号
    //  * @param handle USART句柄指针
    //  * @note 仅在有效句柄时生效，无效句柄会被忽略
    //  */
    // void yDrvUsartSendLinBreak(yDrvUsartHandle_t *handle);

    // ==================== USART中断管理函数 ====================
    /**
     * @brief 使能指定中断
     * @param handle USART句柄指针
     * @param prio 优先级
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartEnableInterrupt(yDrvUsartHandle_t *handle)
    {
        // 参数有效性检查
        if (handle == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        // 设置中断优先级并使能中断
        NVIC_EnableIRQ(handle->IRQ);

        return YDRV_OK;
    }

    /**
     * @brief 禁用指定中断
     * @param handle USART句柄指针
     * @param interrupts 中断类型位掩码
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartDisableInterrupt(yDrvUsartHandle_t *handle)
    {
        // 参数有效性检查
        if (handle == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        // 设置中断优先级并使能中断
        NVIC_DisableIRQ(handle->IRQ);

        return YDRV_OK;
    }

    /**
     * @brief 注册中断回调函数
     * @param handle USART句柄指针
     * @param interruptType 中断类型
     * @param callback 回调函数指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartRegisterCallback(yDrvUsartHandle_t *handle,
                                           yDrvUsartExtiConfig_t exit);

    /**
     * @brief 反注册中断回调函数
     * @param handle USART句柄指针
     * @param interruptType 中断类型
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartUnregisterCallback(yDrvUsartHandle_t *handle,
                                             yDrvUsartExti_t type);

    /**
     * @brief 获取USART溢出错误标志状态（内联优化）
     * @param handle USART句柄指针
     * @retval 1 溢出错误标志已设置
     * @retval 0 溢出错误标志未设置
     * @note 溢出错误发生在接收器无法及时处理接收到的数据时
     */
    YLIB_INLINE uint32_t yDrvUsartGetFlagORE(yDrvUsartHandle_t *handle)
    {
        return LL_USART_IsActiveFlag_ORE(handle->instance);
    }

    /**
     * @brief 获取USART溢出错误标志状态（内联优化）
     * @param handle USART句柄指针
     * @retval 1 溢出错误标志已设置
     * @retval 0 溢出错误标志未设置
     * @note 溢出错误发生在接收器无法及时处理接收到的数据时
     */
    YLIB_INLINE void yDrvUsartResetFlagORE(yDrvUsartHandle_t *handle)
    {
        LL_USART_ClearFlag_ORE(handle->instance);
    }

#ifdef __cplusplus
}
#endif

#endif /* YDRV_USART_H */
