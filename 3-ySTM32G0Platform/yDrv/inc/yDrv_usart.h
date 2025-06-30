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
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

#include "yDrv_basic.h"
    // ==================== USART配置枚举 ====================

    // USART实例枚举
    typedef enum
    {
        YDRV_USART_1 = 0,
        YDRV_USART_2,
        YDRV_USART_3,
        YDRV_USART_4,
        YDRV_USART_5,
        YDRV_USART_6,
        YDRV_USART_MAX
    } yDrvUsartId_t;

    // 数据位枚举
    typedef enum
    {
        YDRV_USART_DATA_8BIT = 8,
        YDRV_USART_DATA_9BIT = 9
    } yDrvUsartDataBits_t;

    // 停止位枚举
    typedef enum
    {
        YDRV_USART_STOP_1BIT = 1,
        YDRV_USART_STOP_2BIT = 2
    } yDrvUsartStopBits_t;

    // 校验位枚举
    typedef enum
    {
        YDRV_USART_PARITY_NONE = 0,
        YDRV_USART_PARITY_EVEN,
        YDRV_USART_PARITY_ODD
    } yDrvUsartParity_t;

    // 传输方向枚举
    typedef enum
    {
        YDRV_USART_DIR_TX = 1,
        YDRV_USART_DIR_RX = 2,
        YDRV_USART_DIR_TX_RX = 3
    } yDrvUsartDirection_t;

    // 硬件流控枚举
    typedef enum
    {
        YDRV_USART_FLOW_NONE = 0,
        YDRV_USART_FLOW_RTS,
        YDRV_USART_FLOW_CTS,
        YDRV_USART_FLOW_RTS_CTS
    } yDrvUsartFlowControl_t;

    // USART工作模式枚举
    typedef enum
    {
        YDRV_USART_MODE_UART = 0,    // 标准UART模式
        YDRV_USART_MODE_SYNCHRONOUS, // 同步模式（带时钟输出）
        YDRV_USART_MODE_SMARTCARD,   // 智能卡模式
        YDRV_USART_MODE_SINGLE_WIRE, // 单线半双工模式
        YDRV_USART_MODE_IRDA,        // IrDA红外模式
        YDRV_USART_MODE_LIN          // LIN总线模式
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
    } yDrvUsartConfig_t;

    // ==================== USART句柄结构体 ====================

    // USART句柄结构体（极简版）
    typedef struct
    {
        USART_TypeDef *instance; // USART硬件实例
        yDrvUsartId_t usartId;   // USART ID
        yDrvGpioPin_t txPin;     // TX引脚（用于反初始化时复位GPIO）
        yDrvGpioPin_t rxPin;     // RX引脚（用于反初始化时复位GPIO）
        yDrvGpioPin_t rtsPin;    // RTS引脚（用于反初始化时复位GPIO）
        yDrvGpioPin_t ctsPin;    // CTS引脚（用于反初始化时复位GPIO）
    } yDrvUsartHandle_t;

    // ==================== USART中断管理 ====================

    // USART中断类型枚举 - 按位掩码设计，支持组合操作
    typedef enum
    {
        YDRV_USART_IT_TXE = 0x01,  // 发送寄存器空中断
        YDRV_USART_IT_RXNE = 0x02, // 接收寄存器非空中断
        YDRV_USART_IT_TC = 0x04,   // 传输完成中断
        YDRV_USART_IT_IDLE = 0x08, // 空闲线路检测中断
        YDRV_USART_IT_PE = 0x10,   // 奇偶校验错误中断
        YDRV_USART_IT_ERR = 0x20,  // 错误中断（帧错误、噪声错误、溢出错误）
        YDRV_USART_IT_LBD = 0x40,  // LIN断开检测中断
        YDRV_USART_IT_CTS = 0x80,  // CTS状态变化中断
        YDRV_USART_IT_ALL = 0xFF   // 所有中断
    } yDrvUsartInterrupt_t;

    // ==================== USART基础函数 ====================

    /**
     * @brief 初始化USART
     * @param husart USART句柄指针
     * @param pConfig 配置参数指针（包含USART ID和引脚配置）
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartInit(yDrvUsartHandle_t *husart,
                               const yDrvUsartConfig_t *pConfig);

    /**
     * @brief 反初始化USART
     * @param husart USART句柄指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartDeInit(yDrvUsartHandle_t *husart);

    // ==================== USART数据传输函数 ====================

    /**
     * @brief 发送单个字节（非阻塞，内联优化）
     * @param husart USART句柄指针
     * @param data 要发送的字节
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartWriteByte(yDrvUsartHandle_t *husart, uint8_t data)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        if (!LL_USART_IsActiveFlag_TXE(husart->instance))
        {
            return YDRV_BUSY;
        }

        LL_USART_TransmitData8(husart->instance, data);
        return YDRV_OK;
    }

    /**
     * @brief 接收单个字节（非阻塞，内联优化）
     * @param husart USART句柄指针
     * @param pData 接收数据指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartReadByte(yDrvUsartHandle_t *husart, uint8_t *pData)
    {
        if (husart == NULL || husart->instance == NULL || pData == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        if (!LL_USART_IsActiveFlag_RXNE(husart->instance))
        {
            return YDRV_BUSY;
        }

        *pData = LL_USART_ReceiveData8(husart->instance);
        return YDRV_OK;
    }

    /**
     * @brief 发送数据缓冲区（非阻塞）
     * @param husart USART句柄指针
     * @param pData 发送数据指针
     * @param size 数据长度
     * @param pSentSize 实际发送长度指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartWrite(yDrvUsartHandle_t *husart, const uint8_t *pData, uint16_t size, uint16_t *pSentSize);

    /**
     * @brief 接收数据到缓冲区（非阻塞）
     * @param husart USART句柄指针
     * @param pData 接收缓冲区指针
     * @param size 期望接收长度
     * @param pReceivedSize 实际接收长度指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartRead(yDrvUsartHandle_t *husart, uint8_t *pData, uint16_t size, uint16_t *pReceivedSize);

    // ==================== USART状态查询函数（内联优化） ====================

    /**
     * @brief 检查发送器是否空闲（内联优化）
     * @param husart USART句柄指针
     * @retval YDRV_OK=空闲, YDRV_BUSY=忙碌, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsTxEmpty(yDrvUsartHandle_t *husart)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }
        return LL_USART_IsActiveFlag_TXE(husart->instance) ? YDRV_OK : YDRV_BUSY;
    }

    /**
     * @brief 检查接收器是否有数据（内联优化）
     * @param husart USART句柄指针
     * @retval YDRV_OK=无数据, YDRV_BUSY=有数据 , YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsRxEmpty(yDrvUsartHandle_t *husart)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        return LL_USART_IsActiveFlag_RXNE(husart->instance) ? YDRV_BUSY : YDRV_OK;
    }

    /**
     * @brief 检查传输是否完成（内联优化）
     * @param husart USART句柄指针
     * @retval YDRV_OK=传输完成, YDRV_BUSY=传输未完成, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsTransmitComplete(yDrvUsartHandle_t *husart)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        return LL_USART_IsActiveFlag_TC(husart->instance) ? YDRV_OK : YDRV_BUSY;
    }

    // ==================== USART控制函数 ====================

    /**
     * @brief 使能USART（内联优化）
     * @param husart USART句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartEnable(yDrvUsartHandle_t *husart)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        LL_USART_Enable(husart->instance);
        return YDRV_OK;
    }

    /**
     * @brief 禁用USART（内联优化）
     * @param husart USART句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartDisable(yDrvUsartHandle_t *husart)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        LL_USART_Disable(husart->instance);
        return YDRV_OK;
    }

    /**
     * @brief 设置波特率
     * @param husart USART句柄指针
     * @param baudRate 新的波特率
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartSetBaudRate(yDrvUsartHandle_t *husart, uint32_t baudRate);

    // ==================== USART工具函数 ====================

    /**
     * @brief 检查USART ID是否有效（内联优化）
     * @param usartId USART ID
     * @retval YDRV_OK=有效, YDRV_INVALID_PARAM=无效
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsIdValid(yDrvUsartId_t usartId)
    {
        return (usartId < YDRV_USART_MAX) ? YDRV_OK : YDRV_INVALID_PARAM;
    }

    /**
     * @brief 获取USART实例指针
     * @param usartId USART ID
     * @retval USART实例指针，NULL表示无效
     */
    USART_TypeDef *yDrvUsartGetInstance(yDrvUsartId_t usartId);

    // ==================== USART专用模式函数 ====================

    /**
     * @brief 设置智能卡保护时间
     * @param husart USART句柄指针
     * @param guardTime 保护时间（0-255）
     * @note 仅在有效句柄时生效，无效句柄会被忽略
     */
    void yDrvUsartSetSmartcardGuardTime(yDrvUsartHandle_t *husart, uint8_t guardTime);

    /**
     * @brief 设置IrDA功率模式
     * @param husart USART句柄指针
     * @param powerMode 功率模式（true=低功率，false=正常功率）
     * @note 仅在有效句柄时生效，无效句柄会被忽略
     */
    void yDrvUsartSetIrdaPowerMode(yDrvUsartHandle_t *husart, bool powerMode);

    /**
     * @brief 设置LIN断开检测长度
     * @param husart USART句柄指针
     * @param breakLength 断开检测长度（true=11位，false=10位）
     * @note 仅在有效句柄时生效，无效句柄会被忽略
     */
    void yDrvUsartSetLinBreakLength(yDrvUsartHandle_t *husart, bool breakLength);

    /**
     * @brief 发送LIN断开信号
     * @param husart USART句柄指针
     * @note 仅在有效句柄时生效，无效句柄会被忽略
     */
    void yDrvUsartSendLinBreak(yDrvUsartHandle_t *husart);

    // ==================== USART中断管理函数 ====================
    /**
     * @brief 使能指定中断
     * @param husart USART句柄指针
     * @param prio 优先级
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartEnableInterrupt(yDrvUsartHandle_t *husart, uint32_t prio);

    /**
     * @brief 禁用指定中断
     * @param husart USART句柄指针
     * @param interrupts 中断类型位掩码
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartDisableInterrupt(yDrvUsartHandle_t *husart);

    /**
     * @brief 注册中断回调函数
     * @param husart USART句柄指针
     * @param interruptType 中断类型
     * @param callback 回调函数指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartRegisterCallback(yDrvUsartHandle_t *husart,
                                           uint32_t interruptType,
                                           yDrvInterruptCallback_t callback);

    /**
     * @brief 反注册中断回调函数
     * @param husart USART句柄指针
     * @param interruptType 中断类型
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvUsartUnregisterCallback(yDrvUsartHandle_t *husart,
                                             uint32_t interruptType);

    /**
     * @brief 获取中断使能状态
     * @param husart USART句柄指针
     * @retval 已使能的中断位掩码
     */
    uint8_t yDrvUsartGetEnabledInterrupts(yDrvUsartHandle_t *husart);

    /**
     * @brief 获取中断标志状态（内联优化）
     * @param husart USART句柄指针
     * @retval 中断标志位掩码
     */
    YLIB_INLINE uint8_t yDrvUsartGetInterruptFlags(yDrvUsartHandle_t *husart)
    {
        if (husart == NULL || husart->instance == NULL)
            return 0;

        uint8_t flags = 0;
        if (LL_USART_IsActiveFlag_TXE(husart->instance))
            flags |= YDRV_USART_IT_TXE;
        if (LL_USART_IsActiveFlag_RXNE(husart->instance))
            flags |= YDRV_USART_IT_RXNE;
        if (LL_USART_IsActiveFlag_TC(husart->instance))
            flags |= YDRV_USART_IT_TC;
        if (LL_USART_IsActiveFlag_IDLE(husart->instance))
            flags |= YDRV_USART_IT_IDLE;
        if (LL_USART_IsActiveFlag_PE(husart->instance))
            flags |= YDRV_USART_IT_PE;
        if (LL_USART_IsActiveFlag_FE(husart->instance) ||
            LL_USART_IsActiveFlag_NE(husart->instance) ||
            LL_USART_IsActiveFlag_ORE(husart->instance))
            flags |= YDRV_USART_IT_ERR;
        if (LL_USART_IsActiveFlag_LBD(husart->instance))
            flags |= YDRV_USART_IT_LBD;
        if (LL_USART_IsActiveFlag_nCTS(husart->instance))
            flags |= YDRV_USART_IT_CTS;

        return flags;
    }

    /**
     * @brief 清除中断标志（内联优化）
     * @param husart USART句柄指针
     * @param interrupts 要清除的中断位掩码
     */
    YLIB_INLINE void yDrvUsartClearInterruptFlags(yDrvUsartHandle_t *husart, uint8_t interrupts)
    {
        if (husart == NULL || husart->instance == NULL)
            return;

        if (interrupts & YDRV_USART_IT_TC)
            LL_USART_ClearFlag_TC(husart->instance);
        if (interrupts & YDRV_USART_IT_IDLE)
            LL_USART_ClearFlag_IDLE(husart->instance);
        if (interrupts & YDRV_USART_IT_PE)
            LL_USART_ClearFlag_PE(husart->instance);
        if (interrupts & YDRV_USART_IT_ERR)
        {
            LL_USART_ClearFlag_FE(husart->instance);
            LL_USART_ClearFlag_NE(husart->instance);
            LL_USART_ClearFlag_ORE(husart->instance);
        }
        if (interrupts & YDRV_USART_IT_LBD)
            LL_USART_ClearFlag_LBD(husart->instance);
        if (interrupts & YDRV_USART_IT_CTS)
            LL_USART_ClearFlag_nCTS(husart->instance);
    }

    /**
     * @brief 检查中断是否使能（内联优化）
     * @param husart USART句柄指针
     * @param interruptType 中断类型
     * @retval YDRV_OK=已使能, YDRV_BUSY=未使能, YDRV_INVALID_PARAM=参数错误
     */
    YLIB_INLINE yDrvStatus_t yDrvUsartIsInterruptEnabled(yDrvUsartHandle_t *husart,
                                                         yDrvUsartInterrupt_t interruptType)
    {
        if (husart == NULL || husart->instance == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        bool isEnabled = false;
        switch (interruptType)
        {
        case YDRV_USART_IT_TXE:
            isEnabled = LL_USART_IsEnabledIT_TXE(husart->instance);
            break;
        case YDRV_USART_IT_RXNE:
            isEnabled = LL_USART_IsEnabledIT_RXNE(husart->instance);
            break;
        case YDRV_USART_IT_TC:
            isEnabled = LL_USART_IsEnabledIT_TC(husart->instance);
            break;
        case YDRV_USART_IT_IDLE:
            isEnabled = LL_USART_IsEnabledIT_IDLE(husart->instance);
            break;
        case YDRV_USART_IT_PE:
            isEnabled = LL_USART_IsEnabledIT_PE(husart->instance);
            break;
        case YDRV_USART_IT_ERR:
            isEnabled = LL_USART_IsEnabledIT_ERROR(husart->instance);
            break;
        case YDRV_USART_IT_LBD:
            isEnabled = LL_USART_IsEnabledIT_LBD(husart->instance);
            break;
        case YDRV_USART_IT_CTS:
            isEnabled = LL_USART_IsEnabledIT_CTS(husart->instance);
            break;
        default:
            return YDRV_INVALID_PARAM;
        }

        return isEnabled ? YDRV_OK : YDRV_BUSY;
    }

#ifdef __cplusplus
}
#endif

#endif /* YDRV_USART_H */
