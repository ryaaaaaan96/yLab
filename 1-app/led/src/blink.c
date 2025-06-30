/**
 ******************************************************************************
 * @file
 * @brief
 * @author Moon
 * @version V1.0.0
 * @date 2022/02/08
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

#include "switch.h"
/* Private defines -----------------------------------------------------------*/
#define LED_TASK_PRIO 6  // 任务优先级
#define LED_STK_SIZE 100 // 任务堆栈大小

TaskHandle_t LedTask_Handler; // 任务句柄
/* Private Struct  -----------------------------------------------------------*/

/* Private enum    -----------------------------------------------------------*/

/* Private Variable  ---------------------------------------------------------*/

// Pointer

// Array

// Const

/***********************************************************************************
 * @brief 监控进程
 * ex:
 * @par
 * None
 * @retval
 **********************************************************************************/
void BlinkTaskProcess(void *pvParameters)
{
    (void)pvParameters;
    SwitchInit();

    for (;;)
    {
        SwitchCtrl(SWITCH_TYPE_LED, 3);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
/***********************************************************************************
 * @brief 监控进程
 * ex:
 * @par
 * None
 * @retval
 **********************************************************************************/
void BlinkTaskInit(void)
{
    xTaskCreate(BlinkTaskProcess, "LedTask",
                LED_STK_SIZE,
                NULL,
                LED_TASK_PRIO,
                NULL);
}
