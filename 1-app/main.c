/**
 * @file main.c
 * @brief 主程序入口文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 程序主入口，负责系统初始化和FreeRTOS调度器启动
 * 创建启动任务，完成各模块初始化后启动任务调度
 */
// ==================== 包含文件 ====================
#include "FreeRTOS.h"
#include "task.h"
#include "blink.h"
#include "yDev.h"
#include "serialshell.h"
#include "flash.h"
// ==================== 私有函数声明 ====================

/**
 * @brief 系统启动任务
 * @param pvParameters 任务参数（未使用）
 *
 * @par 功能描述:
 * 负责系统各模块的初始化，完成后删除自身
 */
static void Startup(void *pvParameters)
{
    // 抑制未使用参数警告
    (void)pvParameters;
    // 初始化LED闪烁任务
    FlashInit();
    BlinkTaskInit();
    ShellTaskInit();

    // 初始化完成后删除启动任务
    vTaskDelete(NULL);
}

/**
 * @brief 程序主入口函数
 * @return int 返回值（正常情况下不会返回）
 *
 * @par 功能描述:
 * 程序入口点，完成系统底层初始化，创建启动任务，启动FreeRTOS调度器
 */
int main(void)
{
    // 系统底层初始化
    yLabInit();

    // 创建启动任务
    xTaskCreate(Startup, "Startup", 1024, NULL, 31, NULL);

    // 启动FreeRTOS调度器
    vTaskStartScheduler();

    // 正常情况下不会执行到这里
    return 0;
}
