/**
 * @file led.h
 * @brief 简化LED设备模块头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的简化LED设备驱动，提供基本的LED控制功能
 */

#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDev_gpio.h"
#include <stdint.h>
#include <stdbool.h>

    // ==================== LED状态枚举 ====================

    typedef enum
    {
        SWITCH_TYPE_LED = 0,
        SWITCH_TYPE_MAX,
    } usrSwitchType_t;

    void SwitchInit(void);

    void SwitchCtrl(usrSwitchType_t type, uint32_t st);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
