/**
 * @file yDrv_gpio.h
 * @brief STM32G0 GPIO驱动程序头文件
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供STM32G0系列MCU的GPIO通用驱动接口，支持多种工作模式和高效操作
 *
 * @par 主要特性:
 * - 支持所有GPIO端口(GPIOA-GPIOF)
 * - 支持多种工作模式：输入、输出、复用功能、模拟
 * - 支持EXTI外部中断管理
 * - 内联优化高频操作函数
 * - 统一的错误处理和状态查询
 * - 高效的位操作和端口操作
 *
 * @par 更新历史:
 * - v2.0 (2025): 完整的GPIO驱动实现，支持中断管理和内联优化
 */

#ifndef YDRV_GPIO_H
#define YDRV_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"

#include "yDrv_basic.h"

    // ==================== GPIO配置枚举 ====================

    /**
     * @brief GPIO工作模式枚举
     */
    typedef enum
    {
        YDRV_GPIO_MODE_INPUT = 0, // 输入模式
        YDRV_GPIO_MODE_OUTPUT_PP, // 输出模式
        YDRV_GPIO_MODE_OUTPUT_OD, // 输出模式
        YDRV_GPIO_MODE_ANALOG     // 模拟模式
    } yDrvGpioMode_t;

    /**
     * @brief GPIO输出速度枚举
     */
    typedef enum
    {
        YDRV_GPIO_SPEED_LOW = 0,  // 低速 (2MHz)
        YDRV_GPIO_SPEED_MEDIUM,   // 中速 (25MHz)
        YDRV_GPIO_SPEED_HIGH,     // 高速 (50MHz)
        YDRV_GPIO_SPEED_VERY_HIGH // 极高速 (100MHz)
    } yDrvGpioSpeed_t;

    /**
     * @brief GPIO上下拉枚举
     */
    typedef enum
    {
        YDRV_GPIO_PUPD_NONE = 0, // 无上下拉
        YDRV_GPIO_PUPD_PULLUP,   // 上拉
        YDRV_GPIO_PUPD_PULLDOWN  // 下拉
    } yDrvGpioPuPd_t;

    /**
     * @brief GPIO引脚状态枚举
     */
    typedef enum
    {
        YDRV_PIN_RESET = 0, // 引脚低电平
        YDRV_PIN_SET = 1    // 引脚高电平
    } yDrvPinState_t;

    /**
     * @brief EXTI触发模式枚举
     */
    typedef enum
    {
        YDRV_EXTI_TRIGGER_RISING = 1,        // 上升沿触发
        YDRV_EXTI_TRIGGER_FALLING = 2,       // 下降沿触发
        YDRV_EXTI_TRIGGER_RISING_FALLING = 3 // 双沿触发
    } yDrvExtiTrigger_t;

    // ==================== GPIO配置结构体 ====================

    /**
     * @brief GPIO配置结构体
     */
    typedef struct
    {
        yDrvGpioPin_t pin;     // GPIO引脚编号
        yDrvGpioMode_t mode;   // 工作模式
        yDrvGpioSpeed_t speed; // 输出速度
        yDrvGpioPuPd_t pupd;   // 上下拉配置
    } yDrvGpioConfig_t;

    /**
     * @brief GPIO句柄结构体
     */
    typedef struct
    {
        GPIO_TypeDef *port; // GPIO端口指针
        uint32_t pin;       // GPIO引脚位掩码
        IRQn_Type IRQ;      //
        uint8_t pinNumber;  // GPIO引脚编号(0-15)
    } yDrvGpioHandle_t;

    /**
     * @brief EXIT句柄结构体
     */
    typedef struct
    {
        yDrvExtiTrigger_t trigger;
        uint32_t prio;
        void (*function)(void *para); // 回调函数指针
        void *arg;                    // 回调函数参数
        uint32_t enable;
    } yDrvGpioExit_t;

    // ==================== GPIO基础函数 ====================

    /**
     * @brief 初始化GPIO引脚
     * @param config 配置文件
     * @param handle 工作模式
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvGpioInitStatic(yDrvGpioConfig_t *config, yDrvGpioHandle_t *handle);

    /**
     * @brief 反初始化GPIO引脚
     * @param pin GPIO引脚编号
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvGpioDeInitStatic(yDrvGpioHandle_t *handle);

    // ==================== GPIO高频操作函数（内联优化） ====================

    /**
     * @brief 写入GPIO引脚状态（内联优化）
     * @param handle GPIO句柄指针
     * @param state 引脚状态
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvGpioWrite(yDrvGpioHandle_t *handle, yDrvPinState_t state)
    {
        if (handle == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        if (state == YDRV_PIN_SET)
        {
            LL_GPIO_SetOutputPin(handle->port, handle->pin);
        }
        else
        {
            LL_GPIO_ResetOutputPin(handle->port, handle->pin);
        }

        return YDRV_OK;
    }

    /**
     * @brief 读取GPIO引脚状态（内联优化）
     * @param handle GPIO句柄指针
     * @retval 引脚状态 (YDRV_PIN_SET/YDRV_PIN_RESET)，错误时返回YDRV_PIN_RESET
     */
    YLIB_INLINE yDrvPinState_t yDrvGpioRead(yDrvGpioHandle_t *handle)
    {
        if (handle == NULL)
        {
            return YDRV_PIN_RESET;
        }

        return (yDrvPinState_t)LL_GPIO_IsInputPinSet(handle->port, handle->pin);
    }

    /**
     * @brief 翻转GPIO引脚状态（内联优化）
     * @param handle GPIO句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvGpioToggle(yDrvGpioHandle_t *handle)
    {
        if (handle == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        LL_GPIO_TogglePin(handle->port, handle->pin);
        return YDRV_OK;
    }

    /**
     * @brief 设置GPIO引脚为高电平（内联优化）
     * @param handle GPIO句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvGpioSet(yDrvGpioHandle_t *handle)
    {
        return yDrvGpioWrite(handle, YDRV_PIN_SET);
    }

    /**
     * @brief 设置GPIO引脚为低电平（内联优化）
     * @param handle GPIO句柄指针
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvGpioReset(yDrvGpioHandle_t *handle)
    {
        return yDrvGpioWrite(handle, YDRV_PIN_RESET);
    }

    // ==================== EXTI中断管理函数 ====================

    /**
     * @brief 使能GPIO外部中断
     * @param pin GPIO引脚编号
     * @param trigger 触发模式
     * @param priority 中断优先级
     * @retval yDrv状态
     */

    YLIB_INLINE yDrvStatus_t yDrvGpioEnableInterrupt(yDrvGpioHandle_t *handle)
    {
        // 参数有效性检查
        if (handle == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        NVIC_EnableIRQ(handle->IRQ);

        return YDRV_OK;
    }

    /**
     * @brief 禁用GPIO外部中断
     * @param pin GPIO引脚编号
     * @retval yDrv状态
     */
    YLIB_INLINE yDrvStatus_t yDrvGpioDisableInterrupt(yDrvGpioHandle_t *handle)
    {
        // 参数有效性检查
        if (handle == NULL)
        {
            return YDRV_INVALID_PARAM;
        }

        NVIC_DisableIRQ(handle->IRQ);

        return YDRV_OK;
    }

    /**
     * @brief 注册GPIO中断回调函数
     * @param pin GPIO引脚编号
     * @param callback 回调函数
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvGpioRegisterCallback(yDrvGpioHandle_t *handle,
                                          yDrvGpioExit_t *exit);

    /**
     * @brief 注销GPIO中断回调函数
     * @param pin GPIO引脚编号
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvGpioUnregisterCallback(yDrvGpioHandle_t *handle, yDrvExtiTrigger_t trigger);

    /**
     * @brief 清除GPIO外部中断标志
     * @param pin GPIO引脚编号
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvGpioClearInterruptFlag(yDrvGpioHandle_t *handle);

    /**
     * @brief 检查GPIO外部中断标志
     * @param pin GPIO引脚编号
     * @retval true=中断标志置位, false=中断标志未置位
     */
    uint8_t yDrvGpioGetInterruptFlag(yDrvGpioHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* YDRV_GPIO_H */
