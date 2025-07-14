/**
 * @file blink.c
 * @brief LED闪烁任务模块实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现LED闪烁任务，根据按键按下次数控制不同的LED闪烁模式
 * 支持多种闪烁频率和常亮/关闭状态控制
 */

// ==================== 包含文件 ====================
#include "FreeRTOS.h"
#include "task.h"
#include "switch.h"
#include "communication.h"

// ==================== 私有宏定义 ====================
#define LED_TASK_PRIO 6  /**< LED任务优先级 */
#define LED_STK_SIZE 512 /**< LED任务堆栈大小 */

// ==================== 私有变量 ====================
TaskHandle_t LedTask_Handler; /**< LED任务句柄 */

static uint8_t data_log[100];

// ==================== 公共函数实现 ====================

/**
 * @brief LED闪烁任务处理函数
 * @param pvParameters 任务参数（未使用）
 *
 * @par 功能描述:
 * LED任务主循环，根据按键状态和按下次数控制LED闪烁模式：
 * - 按键未按下：LED常亮
 * - 按键按下0次：正常闪烁（500ms间隔）
 * - 按键按下1次：慢闪烁（1000ms间隔）
 * - 按键按下2次：超慢闪烁（5000ms间隔）
 * - 按键按下3次及以上：LED关闭
 */
void BlinkTaskProcess(void *pvParameters)
{
    // 抑制未使用参数警告
    (void)pvParameters;
    int32_t len;
    uint32_t button_press_count;

    // 初始化开关模块
    SwitchInit();
    CommunicationInit();

    // 任务主循环
    for (;;)
    {
        len = MessageRead(&data_log, 10);
        if (len > 0)
        {
            MessageWrite(&data_log, len);
        }

        // 检查按键状态
        if (SwitchRead(SWITCH_TYPE_BUTTON) != 0)
        {
            // 按键未按下，获取按键按下计数
            button_press_count = SwitchGetLog();
        }
        else
        {
            // 按键按下，LED常亮
            SwitchCtrl(SWITCH_TYPE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // 根据按键按下次数控制LED闪烁模式
        switch (button_press_count)
        {
        case 0:
            // 正常闪烁模式：500ms间隔翻转
            SwitchCtrl(SWITCH_TYPE_LED, 2);
            vTaskDelay(pdMS_TO_TICKS(500));
            break;
        case 1:
            // 慢闪烁模式：1000ms间隔翻转
            SwitchCtrl(SWITCH_TYPE_LED, 2);
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        case 2:
            // 超慢闪烁模式：5000ms间隔翻转
            SwitchCtrl(SWITCH_TYPE_LED, 2);
            vTaskDelay(pdMS_TO_TICKS(5000));
            break;
        default:
            // 按下3次及以上：LED关闭
            SwitchCtrl(SWITCH_TYPE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        }
    }
}
/**
 * @brief 初始化LED闪烁任务
 *
 * @par 功能描述:
 * 创建LED闪烁任务，设置任务名称、堆栈大小和优先级
 */
void BlinkTaskInit(void)
{
    xTaskCreate(BlinkTaskProcess, // 任务函数
                "LedTask",        // 任务名称
                LED_STK_SIZE,     // 堆栈大小
                NULL,             // 任务参数
                LED_TASK_PRIO,    // 任务优先级
                NULL);            // 任务句柄
}
