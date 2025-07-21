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

#define UART_RX_BUF_SIZE (1024) // 串口接收缓冲区大小

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

static yDevHandle_Usart_t usart_handle;

static void usart_rx_callback(void *arg);

static uint8_t usart_rx_buffer[UART_RX_BUF_SIZE]; // 接收缓冲区
static uint32_t buffer_head;                      // 接收缓冲区头部索引
static uint32_t buffer_tail;                      // 接收缓冲区尾部索引
static uint32_t buffer_overflow;                  // 接收缓冲区溢出标志

static yDrvDmaConfig_t dma_config = {
    .channel = YDRV_DMA1_CHANNEL1,      // DMA通道
    .request = YDRV_DMA_REQ_USART3_RX,  // DMA请求信号（DMAMUX）
    .priority = YDRV_DMA_PRIORITY_HIGH, // DMA优先级
    .mode = YDRV_DMA_MODE_CIRCULAR,     // DMA传输模式（循环模式）
    .src_width = YDRV_DMA_WIDTH_8BIT,   // 源数据位宽（8位）
    .dst_width = YDRV_DMA_WIDTH_8BIT,   // 目标数据位宽（8位）
    .src_inc = YDRV_DMA_INC_DISABLE,    // 源地址不递增（外设地址固定）
    .dst_inc = YDRV_DMA_INC_ENABLE,     // 目标地址递增（内存缓冲区）
    .src_buffer = usart_rx_buffer,      // 源缓冲区地址（稍后设置为USART数据寄存器）
    .dst_buffer = usart_rx_buffer,      // 目标缓冲区地址（稍后设置为接收缓冲区）
    .trans_len = UART_RX_BUF_SIZE,      // 缓冲区传输长度
};

static yDrvUsartExtiConfig_t usart_exti_config = {
    .trigger = YDRV_USART_EXTI_IDLE,
    .prio = 2,
    .function = usart_rx_callback,
    .arg = NULL,
    .enable = 1,
};

// ==================== 公共API实现 ====================

/**
 * @brief 初始化开关模块
 */
void CommunicationInit(void)
{
    buffer_head = 0;
    buffer_tail = 0;
    buffer_overflow = 0;

    // 初始化开关模块变量
    yDevInitStatic(&usart_config, &usart_handle);
    yDevIoctl(&usart_handle,
              YDEV_USART_IOCTL_SET_RECEIVE_DMA,
              &dma_config);
    yDevIoctl(&usart_handle,
              YDEV_USART_IOCTL_ENABLE_RECEIVE_DMA,
              NULL);
    yDevIoctl(&usart_handle,
              YDEV_USART_IOCTL_SET_INTERRUPT,
              &usart_exti_config);
}

/**
 * @brief 开关控制函数
 * @param type 开关类型
 * @param st 开关状态 (0=关闭, 1=开启, 其他=翻转)
 */
void MessageWrite(void *msg, uint32_t len)
{
    yDevWrite(&usart_handle, (uint8_t *)msg, len);
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

/**
 * @brief 开关状态读取函数
 * @param type 开关类型
 * @return 开关状态 (0=关闭, 非0=开启)
 */
int MessageLoop(void)
{
    if (buffer_head > buffer_tail)
    {
        buffer_head += yDevWrite(&usart_handle,
                                 &usart_rx_buffer[buffer_head],
                                 UART_RX_BUF_SIZE - buffer_head);
        if (buffer_head >= UART_RX_BUF_SIZE)
        {
            buffer_head -= UART_RX_BUF_SIZE;
        }
    }
    if (buffer_head < buffer_tail)
    {
        buffer_head += yDevWrite(&usart_handle,
                                 &usart_rx_buffer[buffer_head],
                                 buffer_tail - buffer_head);
    }
    return buffer_overflow;
}

/**
 * @brief 开关状态读取函数
 * @param type 开关类型
 * @return 开关状态 (0=关闭, 非0=开启)
 */
void MessageLoopReset(void)
{
    buffer_overflow = 0;
}

static void usart_rx_callback(void *arg)
{
    (void)arg; // 避免未使用参数警告

    // 计算DMA当前写入位置
    uint32_t index = UART_RX_BUF_SIZE - yDevUsartDmaRxLenGet(&usart_handle);

    if (index < buffer_tail && index >= buffer_head)
    {
        buffer_overflow = 1;     // 溢出标志
        index = buffer_head - 1; // 重置索引
    }
    buffer_tail = index;
}
