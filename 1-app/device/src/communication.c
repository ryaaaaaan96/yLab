/**
 * @file switch.c
 * @brief 开关设备模块实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 */
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "communication.h"

// ==================== 静态变量 ====================

static yDevConfig_Usart_t usart_config =
    {
        .base = {
            .type = YDEV_TYPE_USART,
        },
        .drv_config = {
            .usartId = YDRV_USART_3,
            .txPin = YDRV_PIND8,
            .rxPin = YDRV_PIND9,
            .rtsPin = YDRV_PINNULL,
            .ctsPin = YDRV_PINNULL,
            .baudRate = 115200,
            .dataBits = YDRV_USART_DATA_8BIT,
            .stopBits = YDRV_USART_STOP_1BIT,
            .parity = YDRV_USART_PARITY_NONE,
            .direction = YDRV_USART_DIR_TX_RX,
            .flowControl = YDRV_USART_FLOW_NONE,
            .mode = YDRV_USART_MODE_ASYNCHRONOUS,
        },
};

static yDevHandle_Usart_t usart_handle =
    {
        .base = {0},
        .drv_handle = {0},
};
// ==================== 公共API实现 ====================

/**
 * @brief 初始化开关模块
 */
void CommunicationInit(void)
{
    // 初始化开关模块变量
    yDevInitStatic(&usart_config, &usart_handle);
}

/**
 * @brief 开关控制函数
 * @param type 开关类型
 * @param st 开关状态 (0=关闭, 1=开启, 其他=翻转)
 */
void MessageWrite(void *msg, uint32_t len)
{
    yDevWrite(&usart_handle, msg, len);
}

/**
 * @brief 开关状态读取函数
 * @param type 开关类型
 * @return 开关状态 (0=关闭, 非0=开启)
 */
uint32_t MessageRead(void *msg, uint32_t len)
{
    uint32_t size;
    size = yDevRead(&usart_handle, msg, len);

    return size;
}
