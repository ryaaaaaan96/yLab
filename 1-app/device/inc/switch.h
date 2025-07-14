/**
 * @file switch.h
 * @brief 开关设备模块头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的开关设备驱动，提供LED和按键的统一控制接口
 * 支持LED控制、按键状态读取和按键中断处理
 */

#ifndef SWITCH_H
#define SWITCH_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDev_gpio.h"
#include <stdint.h>
#include <stdbool.h>

    // ==================== 开关类型枚举 ====================

    /**
     * @brief 开关类型枚举
     */
    typedef enum
    {
        SWITCH_TYPE_LED = 0, /**< LED开关类型 */
        SWITCH_TYPE_BUTTON,  /**< 按键开关类型 */

        SWITCH_TYPE_MAX, /**< 开关类型最大值 */
    } usrSwitchType_t;

    // ==================== 公共函数声明 ====================

    /**
     * @brief 初始化开关模块
     *
     * @par 功能描述:
     * 初始化LED和按键的GPIO配置，注册按键中断处理函数
     */
    void SwitchInit(void);

    /**
     * @brief 开关控制函数
     * @param type 开关类型
     * @param st 开关状态 (0=关闭, 1=开启, 其他=翻转)
     *
     * @par 功能描述:
     * 控制指定类型的开关状态
     */
    void SwitchCtrl(usrSwitchType_t type, uint32_t st);

    /**
     * @brief 开关状态读取函数
     * @param type 开关类型
     * @return uint32_t 开关状态 (0=关闭, 非0=开启)
     *
     * @par 功能描述:
     * 读取指定类型开关的当前状态
     */
    uint32_t SwitchRead(usrSwitchType_t type);

    /**
     * @brief 获取按键按下日志计数
     * @return uint32_t 按键连续按下次数
     *
     * @par 功能描述:
     * 获取按键在1.5秒内的连续按下次数，用于实现不同的控制模式
     */
    uint32_t SwitchGetLog(void);

#ifdef __cplusplus
}
#endif

#endif /* SWITCH_H */
