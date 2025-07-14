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
#include "stm32g0xx_ll_gpio.h"

    // ==================== 基础类型定义 ====================

    // 返回状态枚举
    typedef enum
    {
        YDRV_OK = 0,
        YDRV_ERROR,
        YDRV_BUSY,
        YDRV_TIMEOUT,
        YDRV_INVALID_PARAM,
        YDRV_NOT_INITIALIZED,
        YDRV_NOT_SUPPORTED,
        YDRV_OVERFLOW,
        YDRV_UNDERFLOW
    } yDrvStatus_t;

    // 使能/禁用枚举
    typedef enum
    {
        YDRV_DISABLE = 0,
        YDRV_ENABLE = 1
    } yDrvState_t;

    // 功能状态枚举
    typedef enum
    {
        YDRV_RESET = 0,
        YDRV_SET = 1
    } yDrvFlagStatus_t;

    // ==================== GPIO配置结构体 ====================
    // GPIO引脚编号枚举 (适配STM32G0系列)
    typedef enum
    {
        YDRV_PINNULL = (0U),

        YDRV_PINA0,
        YDRV_PINA1,
        YDRV_PINA2,
        YDRV_PINA3,
        YDRV_PINA4,
        YDRV_PINA5,
        YDRV_PINA6,
        YDRV_PINA7,
        YDRV_PINA8,
        YDRV_PINA9,
        YDRV_PINA10,
        YDRV_PINA11,
        YDRV_PINA12,
        YDRV_PINA13,
        YDRV_PINA14,
        YDRV_PINA15,

        YDRV_PINB0,
        YDRV_PINB1,
        YDRV_PINB2,
        YDRV_PINB3,
        YDRV_PINB4,
        YDRV_PINB5,
        YDRV_PINB6,
        YDRV_PINB7,
        YDRV_PINB8,
        YDRV_PINB9,
        YDRV_PINB10,
        YDRV_PINB11,
        YDRV_PINB12,
        YDRV_PINB13,
        YDRV_PINB14,
        YDRV_PINB15,

        YDRV_PINC0,
        YDRV_PINC1,
        YDRV_PINC2,
        YDRV_PINC3,
        YDRV_PINC4,
        YDRV_PINC5,
        YDRV_PINC6,
        YDRV_PINC7,
        YDRV_PINC8,
        YDRV_PINC9,
        YDRV_PINC10,
        YDRV_PINC11,
        YDRV_PINC12,
        YDRV_PINC13,
        YDRV_PINC14,
        YDRV_PINC15,

        YDRV_PIND0,
        YDRV_PIND1,
        YDRV_PIND2,
        YDRV_PIND3,
        YDRV_PIND4,
        YDRV_PIND5,
        YDRV_PIND6,
        YDRV_PIND7,
        YDRV_PIND8,
        YDRV_PIND9,
        YDRV_PIND10,
        YDRV_PIND11,
        YDRV_PIND12,
        YDRV_PIND13,
        YDRV_PIND14,
        YDRV_PIND15,

        // STM32G0系列通常没有GPIOE，但部分型号可能有
        YDRV_PINE0,
        YDRV_PINE1,
        YDRV_PINE2,
        YDRV_PINE3,
        YDRV_PINE4,
        YDRV_PINE5,
        YDRV_PINE6,
        YDRV_PINE7,
        YDRV_PINE8,
        YDRV_PINE9,
        YDRV_PINE10,
        YDRV_PINE11,
        YDRV_PINE12,
        YDRV_PINE13,
        YDRV_PINE14,
        YDRV_PINE15,

        // STM32G0系列部分型号有GPIOF，但通常只有少数引脚
        YDRV_PINF0,
        YDRV_PINF1,
        YDRV_PINF2,
        YDRV_PINF3,
        YDRV_PINF4,
        YDRV_PINF5,
        YDRV_PINF6,
        YDRV_PINF7,
        YDRV_PINF8,
        YDRV_PINF9,
        YDRV_PINF10,
        YDRV_PINF11,
        YDRV_PINF12,
        YDRV_PINF13,
        YDRV_PINF14,
        YDRV_PINF15,

        YDRV_PINMAX,
    } yDrvGpioPin_t;

    // GPIO解析结构体
    typedef struct
    {
        GPIO_TypeDef *port; // GPIO端口寄存器指针
        uint16_t pinMask;   // GPIO引脚位掩码
        uint8_t pinIndex;   // GPIO的引脚
        uint8_t flag;       // GPIO的引脚
    } yDrvGpioInfo_t;

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

    yDrvStatus_t yDrvInit(void);
#ifdef __cplusplus
}
#endif

#endif /* YDRV_BASIC_H */