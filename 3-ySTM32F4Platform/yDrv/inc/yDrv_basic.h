/**
 * @file yDrv_basic.h
 * @brief STM32F4 yDrv层基础定义和功能头文件
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供yDrv层的基础类型定义、错误状态枚举、GPIO和DMA解析函数等核心功能
 *
 * @par 主要特性:
 * - 统一的错误状态和返回值定义
 * - 完整的GPIO引脚映射和解析功能
 * - DMA通道映射和解析功能
 * - 系统信息查询接口
 * - 全局中断控制接口
 * - 统一的中断回调函数结构体
 *
 * @par 更新历史:
 * - v2.0 (2025): 完整的基础驱动框架，支持STM32F4系列
 */

#ifndef YDRV_BASIC_H
#define YDRV_BASIC_H

#ifdef __cplusplus
extern "C"
{
#endif

    // ==================== 包含文件 ====================
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "yLib_def.h"

    // 平台相关头文件
#include "stm32f4xx_ll_gpio.h"

    // ==================== 基础类型定义 ====================

    /**
     * @brief yDrv层返回状态枚举
     * @note 统一的错误状态定义，用于所有yDrv层函数的返回值
     */
    typedef enum
    {
        YDRV_OK = 0,          /*!< 操作成功 */
        YDRV_ERROR,           /*!< 通用错误 */
        YDRV_BUSY,            /*!< 设备忙 */
        YDRV_TIMEOUT,         /*!< 操作超时 */
        YDRV_INVALID_PARAM,   /*!< 无效参数 */
        YDRV_NOT_INITIALIZED, /*!< 设备未初始化 */
        YDRV_NOT_SUPPORTED,   /*!< 功能不支持 */
        YDRV_OVERFLOW,        /*!< 缓冲区溢出 */
        YDRV_UNDERFLOW        /*!< 缓冲区下溢 */
    } yDrvStatus_t;

    /**
     * @brief 使能/禁用状态枚举
     * @note 用于控制外设的使能状态
     */
    typedef enum
    {
        YDRV_DISABLE = 0, /*!< 禁用状态 */
        YDRV_ENABLE = 1   /*!< 使能状态 */
    } yDrvState_t;

    /**
     * @brief 功能标志状态枚举
     * @note 用于表示功能标志的设置/复位状态
     */
    typedef enum
    {
        YDRV_RESET = 0, /*!< 复位状态 */
        YDRV_SET = 1    /*!< 设置状态 */
    } yDrvFlagStatus_t;

    // ==================== GPIO配置结构体 ====================

    /**
     * @brief GPIO引脚编号枚举 (适配STM32F4系列)
     * @note 提供统一的GPIO引脚映射，支持STM32F4系列的所有可用引脚
     */
    typedef enum
    {
        YDRV_PINNULL = (0U), /*!< 空引脚，用于表示未使用的引脚 */

        // GPIOA端口引脚定义
        YDRV_PINA0,  /*!< GPIOA引脚0 */
        YDRV_PINA1,  /*!< GPIOA引脚1 */
        YDRV_PINA2,  /*!< GPIOA引脚2 */
        YDRV_PINA3,  /*!< GPIOA引脚3 */
        YDRV_PINA4,  /*!< GPIOA引脚4 */
        YDRV_PINA5,  /*!< GPIOA引脚5 */
        YDRV_PINA6,  /*!< GPIOA引脚6 */
        YDRV_PINA7,  /*!< GPIOA引脚7 */
        YDRV_PINA8,  /*!< GPIOA引脚8 */
        YDRV_PINA9,  /*!< GPIOA引脚9 */
        YDRV_PINA10, /*!< GPIOA引脚10 */
        YDRV_PINA11, /*!< GPIOA引脚11 */
        YDRV_PINA12, /*!< GPIOA引脚12 */
        YDRV_PINA13, /*!< GPIOA引脚13 */
        YDRV_PINA14, /*!< GPIOA引脚14 */
        YDRV_PINA15, /*!< GPIOA引脚15 */

        // GPIOB端口引脚定义
        YDRV_PINB0,  /*!< GPIOB引脚0 */
        YDRV_PINB1,  /*!< GPIOB引脚1 */
        YDRV_PINB2,  /*!< GPIOB引脚2 */
        YDRV_PINB3,  /*!< GPIOB引脚3 */
        YDRV_PINB4,  /*!< GPIOB引脚4 */
        YDRV_PINB5,  /*!< GPIOB引脚5 */
        YDRV_PINB6,  /*!< GPIOB引脚6 */
        YDRV_PINB7,  /*!< GPIOB引脚7 */
        YDRV_PINB8,  /*!< GPIOB引脚8 */
        YDRV_PINB9,  /*!< GPIOB引脚9 */
        YDRV_PINB10, /*!< GPIOB引脚10 */
        YDRV_PINB11, /*!< GPIOB引脚11 */
        YDRV_PINB12, /*!< GPIOB引脚12 */
        YDRV_PINB13, /*!< GPIOB引脚13 */
        YDRV_PINB14, /*!< GPIOB引脚14 */
        YDRV_PINB15, /*!< GPIOB引脚15 */

        // GPIOC端口引脚定义
        YDRV_PINC0,  /*!< GPIOC引脚0 */
        YDRV_PINC1,  /*!< GPIOC引脚1 */
        YDRV_PINC2,  /*!< GPIOC引脚2 */
        YDRV_PINC3,  /*!< GPIOC引脚3 */
        YDRV_PINC4,  /*!< GPIOC引脚4 */
        YDRV_PINC5,  /*!< GPIOC引脚5 */
        YDRV_PINC6,  /*!< GPIOC引脚6 */
        YDRV_PINC7,  /*!< GPIOC引脚7 */
        YDRV_PINC8,  /*!< GPIOC引脚8 */
        YDRV_PINC9,  /*!< GPIOC引脚9 */
        YDRV_PINC10, /*!< GPIOC引脚10 */
        YDRV_PINC11, /*!< GPIOC引脚11 */
        YDRV_PINC12, /*!< GPIOC引脚12 */
        YDRV_PINC13, /*!< GPIOC引脚13 */
        YDRV_PINC14, /*!< GPIOC引脚14 */
        YDRV_PINC15, /*!< GPIOC引脚15 */

        // GPIOD端口引脚定义
        YDRV_PIND0,  /*!< GPIOD引脚0 */
        YDRV_PIND1,  /*!< GPIOD引脚1 */
        YDRV_PIND2,  /*!< GPIOD引脚2 */
        YDRV_PIND3,  /*!< GPIOD引脚3 */
        YDRV_PIND4,  /*!< GPIOD引脚4 */
        YDRV_PIND5,  /*!< GPIOD引脚5 */
        YDRV_PIND6,  /*!< GPIOD引脚6 */
        YDRV_PIND7,  /*!< GPIOD引脚7 */
        YDRV_PIND8,  /*!< GPIOD引脚8 */
        YDRV_PIND9,  /*!< GPIOD引脚9 */
        YDRV_PIND10, /*!< GPIOD引脚10 */
        YDRV_PIND11, /*!< GPIOD引脚11 */
        YDRV_PIND12, /*!< GPIOD引脚12 */
        YDRV_PIND13, /*!< GPIOD引脚13 */
        YDRV_PIND14, /*!< GPIOD引脚14 */
        YDRV_PIND15, /*!< GPIOD引脚15 */

        // GPIOE端口引脚定义
        YDRV_PINE0,  /*!< GPIOE引脚0 */
        YDRV_PINE1,  /*!< GPIOE引脚1 */
        YDRV_PINE2,  /*!< GPIOE引脚2 */
        YDRV_PINE3,  /*!< GPIOE引脚3 */
        YDRV_PINE4,  /*!< GPIOE引脚4 */
        YDRV_PINE5,  /*!< GPIOE引脚5 */
        YDRV_PINE6,  /*!< GPIOE引脚6 */
        YDRV_PINE7,  /*!< GPIOE引脚7 */
        YDRV_PINE8,  /*!< GPIOE引脚8 */
        YDRV_PINE9,  /*!< GPIOE引脚9 */
        YDRV_PINE10, /*!< GPIOE引脚10 */
        YDRV_PINE11, /*!< GPIOE引脚11 */
        YDRV_PINE12, /*!< GPIOE引脚12 */
        YDRV_PINE13, /*!< GPIOE引脚13 */
        YDRV_PINE14, /*!< GPIOE引脚14 */
        YDRV_PINE15, /*!< GPIOE引脚15 */

        // GPIOF端口引脚定义
        YDRV_PINF0,  /*!< GPIOF引脚0 */
        YDRV_PINF1,  /*!< GPIOF引脚1 */
        YDRV_PINF2,  /*!< GPIOF引脚2 */
        YDRV_PINF3,  /*!< GPIOF引脚3 */
        YDRV_PINF4,  /*!< GPIOF引脚4 */
        YDRV_PINF5,  /*!< GPIOF引脚5 */
        YDRV_PINF6,  /*!< GPIOF引脚6 */
        YDRV_PINF7,  /*!< GPIOF引脚7 */
        YDRV_PINF8,  /*!< GPIOF引脚8 */
        YDRV_PINF9,  /*!< GPIOF引脚9 */
        YDRV_PINF10, /*!< GPIOF引脚10 */
        YDRV_PINF11, /*!< GPIOF引脚11 */
        YDRV_PINF12, /*!< GPIOF引脚12 */
        YDRV_PINF13, /*!< GPIOF引脚13 */
        YDRV_PINF14, /*!< GPIOF引脚14 */
        YDRV_PINF15, /*!< GPIOF引脚15 */

        // GPIOG端口引脚定义 (STM32F4系列)
        YDRV_PING0,  /*!< GPIOG引脚0 */
        YDRV_PING1,  /*!< GPIOG引脚1 */
        YDRV_PING2,  /*!< GPIOG引脚2 */
        YDRV_PING3,  /*!< GPIOG引脚3 */
        YDRV_PING4,  /*!< GPIOG引脚4 */
        YDRV_PING5,  /*!< GPIOG引脚5 */
        YDRV_PING6,  /*!< GPIOG引脚6 */
        YDRV_PING7,  /*!< GPIOG引脚7 */
        YDRV_PING8,  /*!< GPIOG引脚8 */
        YDRV_PING9,  /*!< GPIOG引脚9 */
        YDRV_PING10, /*!< GPIOG引脚10 */
        YDRV_PING11, /*!< GPIOG引脚11 */
        YDRV_PING12, /*!< GPIOG引脚12 */
        YDRV_PING13, /*!< GPIOG引脚13 */
        YDRV_PING14, /*!< GPIOG引脚14 */
        YDRV_PING15, /*!< GPIOG引脚15 */

        // GPIOH端口引脚定义 (STM32F4系列)
        YDRV_PINH0,  /*!< GPIOH引脚0 */
        YDRV_PINH1,  /*!< GPIOH引脚1 */
        YDRV_PINH2,  /*!< GPIOH引脚2 */
        YDRV_PINH3,  /*!< GPIOH引脚3 */
        YDRV_PINH4,  /*!< GPIOH引脚4 */
        YDRV_PINH5,  /*!< GPIOH引脚5 */
        YDRV_PINH6,  /*!< GPIOH引脚6 */
        YDRV_PINH7,  /*!< GPIOH引脚7 */
        YDRV_PINH8,  /*!< GPIOH引脚8 */
        YDRV_PINH9,  /*!< GPIOH引脚9 */
        YDRV_PINH10, /*!< GPIOH引脚10 */
        YDRV_PINH11, /*!< GPIOH引脚11 */
        YDRV_PINH12, /*!< GPIOH引脚12 */
        YDRV_PINH13, /*!< GPIOH引脚13 */
        YDRV_PINH14, /*!< GPIOH引脚14 */
        YDRV_PINH15, /*!< GPIOH引脚15 */

        // GPIOI端口引脚定义 (STM32F4系列)
        YDRV_PINI0,  /*!< GPIOI引脚0 */
        YDRV_PINI1,  /*!< GPIOI引脚1 */
        YDRV_PINI2,  /*!< GPIOI引脚2 */
        YDRV_PINI3,  /*!< GPIOI引脚3 */
        YDRV_PINI4,  /*!< GPIOI引脚4 */
        YDRV_PINI5,  /*!< GPIOI引脚5 */
        YDRV_PINI6,  /*!< GPIOI引脚6 */
        YDRV_PINI7,  /*!< GPIOI引脚7 */
        YDRV_PINI8,  /*!< GPIOI引脚8 */
        YDRV_PINI9,  /*!< GPIOI引脚9 */
        YDRV_PINI10, /*!< GPIOI引脚10 */
        YDRV_PINI11, /*!< GPIOI引脚11 */
        YDRV_PINI12, /*!< GPIOI引脚12 */
        YDRV_PINI13, /*!< GPIOI引脚13 */
        YDRV_PINI14, /*!< GPIOI引脚14 */
        YDRV_PINI15, /*!< GPIOI引脚15 */

        // GPIOJ端口引脚定义 (STM32F4系列)
        YDRV_PINJ0,  /*!< GPIOJ引脚0 */
        YDRV_PINJ1,  /*!< GPIOJ引脚1 */
        YDRV_PINJ2,  /*!< GPIOJ引脚2 */
        YDRV_PINJ3,  /*!< GPIOJ引脚3 */
        YDRV_PINJ4,  /*!< GPIOJ引脚4 */
        YDRV_PINJ5,  /*!< GPIOJ引脚5 */
        YDRV_PINJ6,  /*!< GPIOJ引脚6 */
        YDRV_PINJ7,  /*!< GPIOJ引脚7 */
        YDRV_PINJ8,  /*!< GPIOJ引脚8 */
        YDRV_PINJ9,  /*!< GPIOJ引脚9 */
        YDRV_PINJ10, /*!< GPIOJ引脚10 */
        YDRV_PINJ11, /*!< GPIOJ引脚11 */
        YDRV_PINJ12, /*!< GPIOJ引脚12 */
        YDRV_PINJ13, /*!< GPIOJ引脚13 */
        YDRV_PINJ14, /*!< GPIOJ引脚14 */
        YDRV_PINJ15, /*!< GPIOJ引脚15 */

        // GPIOK端口引脚定义 (STM32F4系列)
        YDRV_PINK0,  /*!< GPIOK引脚0 */
        YDRV_PINK1,  /*!< GPIOK引脚1 */
        YDRV_PINK2,  /*!< GPIOK引脚2 */
        YDRV_PINK3,  /*!< GPIOK引脚3 */
        YDRV_PINK4,  /*!< GPIOK引脚4 */
        YDRV_PINK5,  /*!< GPIOK引脚5 */
        YDRV_PINK6,  /*!< GPIOK引脚6 */
        YDRV_PINK7,  /*!< GPIOK引脚7 */
        YDRV_PINK8,  /*!< GPIOK引脚8 */
        YDRV_PINK9,  /*!< GPIOK引脚9 */
        YDRV_PINK10, /*!< GPIOK引脚10 */
        YDRV_PINK11, /*!< GPIOK引脚11 */
        YDRV_PINK12, /*!< GPIOK引脚12 */
        YDRV_PINK13, /*!< GPIOK引脚13 */
        YDRV_PINK14, /*!< GPIOK引脚14 */
        YDRV_PINK15, /*!< GPIOK引脚15 */

        YDRV_PINMAX, /*!< 引脚数量上限，用于数组大小定义 */
    } yDrvGpioPin_t;

    /**
     * @brief GPIO解析结构体
     * @note 包含GPIO端口的完整信息，用于高效访问GPIO寄存器
     */
    typedef struct
    {
        GPIO_TypeDef *port; /*!< GPIO端口寄存器基地址指针 */
        uint16_t pinMask;   /*!< GPIO引脚位掩码，用于位操作 */
        uint8_t pinIndex;   /*!< GPIO引脚索引号(0-15) */
        uint8_t flag;       /*!< 保留标志位，用于扩展功能 */
    } yDrvGpioInfo_t;

    /**
     * @brief DMA通道枚举
     */
    typedef enum
    {
        YDRV_DMA1_CHANNEL1 = 0,
        YDRV_DMA1_CHANNEL2,
        YDRV_DMA1_CHANNEL3,
        YDRV_DMA1_CHANNEL4,
        YDRV_DMA1_CHANNEL5,
        YDRV_DMA1_CHANNEL6,
        YDRV_DMA1_CHANNEL7,
        YDRV_DMA_CHANNEL_MAX
    } yDrvDmaChannel_t;

    /**
     * @brief DMA解析结构体
     */
    typedef struct
    {
        DMA_TypeDef *dma; // DMA控制器寄存器指针
        uint32_t channel; // DMA通道号
    } yDrvDmaInfo_t;

    // ==================== 回调函数结构体 ====================

    /**
     * @brief 统一的中断回调函数结构体
     */
    typedef struct
    {
        void (*function)(void *para); // 回调函数指针
        void *arg;                    // 回调函数参数
    } yDrvInterruptCallback_t;

    // ==================== 全局中断管理函数 ====================

    /**
     * @brief 全局中断使能
     */
    YLIB_USED YLIB_INLINE void yDrvEnableIrq(void)
    {
        __enable_irq();
    }

    /**
     * @brief 全局中断禁用
     */
    YLIB_USED YLIB_INLINE void yDrvDisableIrq(void)
    {
        __disable_irq();
    }

    // ==================== GPIO解析函数 ====================

    /**
     * @brief 解析GPIO位置信息
     * @param index GPIO引脚索引枚举
     * @param pGpioInfo 输出的GPIO信息结构体指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvParseGpio(yDrvGpioPin_t index, yDrvGpioInfo_t *pGpioInfo);

    /**
     * @brief 检查GPIO配置是否有效
     * @param index GPIO引脚索引枚举
     * @retval 1=有效, 0=无效, -1=参数错误
     */
    int32_t yDrvIsGpioValid(yDrvGpioPin_t index);

    // ==================== DMA解析函数 ====================

    /**
     * @brief 解析DMA通道信息
     * @param index DMA通道索引枚举
     * @param pDmaInfo 输出的DMA信息结构体指针
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvParseDma(yDrvDmaChannel_t index, yDrvDmaInfo_t *pDmaInfo);

    /**
     * @brief 检查DMA通道配置是否有效
     * @param index DMA通道索引枚举
     * @retval 1=有效, 0=无效, -1=参数错误
     */
    int32_t yDrvIsDmaValid(yDrvDmaChannel_t index);

    // ==================== 系统信息函数 ====================

    /**
     * @brief 获取芯片ID
     * @retval 芯片ID
     */
    uint32_t yDrvGetChipId(void);

    /**
     * @brief 获取芯片修订版本
     * @retval 修订版本
     */
    uint32_t yDrvGetRevisionId(void);

    /**
     * @brief 获取Flash大小
     * @retval Flash大小(KB)
     */
    uint16_t yDrvGetFlashSize(void);

    /**
     * @brief 获取唯一设备ID
     * @param uid 输出缓冲区(12字节)
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvGetUniqueId(uint8_t uid[12]);

    /**
     * @brief yDrv初始化
     * @retval yDrv状态
     */
    yDrvStatus_t yDrvInit(void);

#ifdef __cplusplus
}
#endif

#endif /* YDRV_BASIC_H */