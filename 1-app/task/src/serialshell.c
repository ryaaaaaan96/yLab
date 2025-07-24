/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief
 * @version 0.1
 * @date 2019-02-22
 *
 * @copyright (c) 2019 Letter
 *
 */

#include "FreeRTOS.h"
#include "task.h"

#include "communication.h"
#include "serialshell.h"

static Shell shell;
static char shell_buffer[512];

// static int32_t shell_read(void *buff, uint16_t len);
// static int32_t shell_write(const void *buff, uint16_t len);

// static int32_t shell_read(void *data, uint16_t len)
// {
//     return 0;
// }

// static int32_t shell_write(const void *buff, uint16_t len)
// {
//     return 0;
// }
/**
 * @brief shell处理函数
 *
 */
static void serial_shell_task(void *arg)
{
    (void)arg;
    uint8_t data;
    shell.write = MessageWrite;
    shell.read = MessageRead;
    CommunicationInit();
    shellInit(&shell, shell_buffer, 512);
    for (;;)
    {
        if (MessageRead(&data, 1) == 1)
        {
            shellHandler(&shell, data);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 延时10ms，避免过快处理
    }
}
/**
 * @brief 用户shell初始化
 *
 */
void ShellTaskInit(void)
{
    xTaskCreate(serial_shell_task, // 任务函数
                "ShellTask",       // 任务名称
                512,               // 堆栈大小
                NULL,              // 任务参数
                10,                // 任务优先级
                NULL);             // 任务句柄
}
