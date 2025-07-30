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

#ifndef COMMUNITION_H
#define COMMUNITION_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDev_usart.h"

    // ==================== 公共函数声明 ====================

    /**
     * @brief 初始化开关模块
     *
     * @par 功能描述:
     * 初始化LED和按键的GPIO配置，注册按键中断处理函数
     */
    void CommunicationInit(void);

    /**
     * @brief 开关控制函数
     * @param type 开关类型
     * @param st 开关状态 (0=关闭, 1=开启, 其他=翻转)
     *
     * @par 功能描述:
     * 控制指定类型的开关状态
     */
    int32_t MessageWrite(const void *msg, uint16_t len);

    /**
     * @brief 开关状态读取函数
     * @param type 开关类型
     * @return uint32_t 开关状态 (0=关闭, 非0=开启)
     *
     * @par 功能描述:
     * 读取指定类型开关的当前状态
     */
    int32_t MessageRead(void *buff, uint16_t len);

    /**
     * @brief 开关状态读取函数
     * @param type 开关类型
     * @return int 开关状态 (0=关闭, 非0=开启)
     *
     * @par 功能描述:
     * 处理接收到的消息循环，更新缓冲区头部索引
     */
    int MessageLoop(void);

    void MessageLoopReset(void);

#ifdef __cplusplus
}
#endif

#endif /* SWITCH_H */
