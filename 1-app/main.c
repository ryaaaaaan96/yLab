/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

#include "blink.h"
#include "yDev.h"
/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

static void Startup(void *pvParameters)
{
    // 抑制未使用参数警告
    (void)pvParameters;


    BlinkTaskInit();
    // FreeRTOS任务应该有无限循环或者删除自己
    // 如果初始化完成后不需要继续运行，删除任务
    vTaskDelete(NULL);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    yLabInit();
    xTaskCreate(Startup, "Startup", 1024, NULL, 31, NULL);
    // 启动调度器
    vTaskStartScheduler();

    return 0;
}
