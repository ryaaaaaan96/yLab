/**
 * @file blink.h
 * @brief LED闪烁任务模块头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * LED闪烁控制任务的头文件，提供LED闪烁任务的初始化和控制接口
 * 支持根据按键状态调整LED闪烁模式
 */

#ifndef TASK_BLINK_H
#define TASK_BLINK_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include <stdint.h>

    // ==================== 公共函数声明 ====================

    /**
     * @brief LED闪烁任务处理函数
     * @param pvParameters 任务参数（未使用）
     *
     * @par 功能描述:
     * LED闪烁任务的主处理函数，根据按键状态控制LED闪烁模式
     * - 按键未按下：LED常亮
     * - 按键按下0次：正常闪烁（500ms间隔）
     * - 按键按下1次：慢闪烁（1000ms间隔）
     * - 按键按下2次：超慢闪烁（5000ms间隔）
     * - 按键按下3次及以上：LED关闭
     */
    void BlinkTaskProcess(void *pvParameters);

    /**
     * @brief 初始化LED闪烁任务
     *
     * @par 功能描述:
     * 创建LED闪烁任务，设置任务优先级和堆栈大小
     */
    void BlinkTaskInit(void);

#ifdef __cplusplus
}
#endif

#endif /* TASK_BLINK_H */
