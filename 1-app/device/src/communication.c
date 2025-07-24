/**
 * @file communication.c
 * @brief 通信设备模块实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供基于USART的通信功能，支持DMA接收和中断处理
 *
 * @par 主要特性:
 * - 基于yDev设备抽象层的USART通信
 * - DMA循环接收缓冲区管理
 * - 空闲中断检测数据接收完成
 * - 环形缓冲区数据处理
 * - 溢出检测和错误处理
 */
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "communication.h"

// ==================== 宏定义 ====================

#define UART_RX_BUF_SIZE (1024) /*!< 串口接收缓冲区大小 */

// ==================== 静态变量定义 ====================

/**
 * @brief USART设备配置结构体
 * @note 配置USART3，波特率115200，8N1格式，收发双向
 */
static yDevConfig_Usart_t usart_config =
    {
        .base = {
            .type = YDEV_TYPE_USART, /*!< 设备类型：USART */
        },
        .drv_config = {
            .usartId = YDRV_USART_3,              /*!< 使用USART3 */
            .txPin = YDRV_PIND8,                  /*!< 发送引脚：PD8 */
            .rxPin = YDRV_PIND9,                  /*!< 接收引脚：PD9 */
            .rtsPin = YDRV_PINNULL,               /*!< RTS引脚：未使用 */
            .ctsPin = YDRV_PINNULL,               /*!< CTS引脚：未使用 */
            .baudRate = 115200,                   /*!< 波特率：115200 */
            .dataBits = YDRV_USART_DATA_8BIT,     /*!< 数据位：8位 */
            .stopBits = YDRV_USART_STOP_1BIT,     /*!< 停止位：1位 */
            .parity = YDRV_USART_PARITY_NONE,     /*!< 校验位：无 */
            .direction = YDRV_USART_DIR_TX_RX,    /*!< 方向：收发双向 */
            .flowControl = YDRV_USART_FLOW_NONE,  /*!< 流控：无 */
            .mode = YDRV_USART_MODE_ASYNCHRONOUS, /*!< 模式：异步 */
        },
};

/**
 * @brief USART设备句柄
 * @note 用于操作USART设备的句柄
 */
static yDevHandle_Usart_t usart_handle;

/**
 * @brief USART接收中断回调函数声明
 * @param arg 回调函数参数（未使用）
 */
static void usart_rx_callback(void *arg);

/**
 * @brief USART接收缓冲区
 * @note DMA循环接收缓冲区，大小为UART_RX_BUF_SIZE
 */
static uint8_t usart_rx_buffer[UART_RX_BUF_SIZE];

/**
 * @brief 接收缓冲区头部索引
 * @note 指向已处理数据的下一个位置
 */
static uint32_t buffer_write;

/**
 * @brief 接收缓冲区尾部索引
 * @note 指向DMA写入数据的当前位置
 */
static uint32_t buffer_read;

/**
 * @brief 接收缓冲区溢出标志
 * @note 1表示发生溢出，0表示正常
 */
static uint32_t buffer_overflow;

/**
 * @brief DMA接收配置结构体
 * @note 配置USART3的DMA循环接收
 */
static yDrvDmaConfig_t dma_config = {
    .channel = YDRV_DMA1_CHANNEL1,      /*!< DMA通道：DMA1通道1 */
    .request = YDRV_DMA_REQ_USART3_RX,  /*!< DMA请求：USART3接收 */
    .priority = YDRV_DMA_PRIORITY_HIGH, /*!< DMA优先级：高 */
    .mode = YDRV_DMA_MODE_CIRCULAR,     /*!< DMA模式：循环模式 */
    .src_width = YDRV_DMA_WIDTH_8BIT,   /*!< 源数据位宽：8位 */
    .dst_width = YDRV_DMA_WIDTH_8BIT,   /*!< 目标数据位宽：8位 */
    .src_inc = YDRV_DMA_INC_DISABLE,    /*!< 源地址递增：禁用（外设地址固定） */
    .dst_inc = YDRV_DMA_INC_ENABLE,     /*!< 目标地址递增：使能（内存缓冲区） */
    .src_buffer = usart_rx_buffer,      /*!< 源缓冲区：将设置为USART数据寄存器 */
    .dst_buffer = usart_rx_buffer,      /*!< 目标缓冲区：接收缓冲区 */
    .trans_len = UART_RX_BUF_SIZE,      /*!< 传输长度：缓冲区大小 */
};

/**
 * @brief USART中断配置结构体
 * @note 配置USART空闲中断，用于检测数据接收完成
 */
static yDrvUsartExtiConfig_t usart_exti_config = {
    .trigger = YDRV_USART_EXTI_IDLE, /*!< 触发类型：空闲中断 */
    .prio = 2,                       /*!< 中断优先级：2 */
    .function = usart_rx_callback,   /*!< 回调函数：接收处理函数 */
    .arg = NULL,                     /*!< 回调参数：无 */
    .enable = 1,                     /*!< 中断使能：开启 */
};

// ==================== 公共API实现 ====================

/**
 * @brief 通信模块初始化
 * @retval 无
 * @note 初始化USART设备、DMA接收和中断配置
 */
void CommunicationInit(void)
{
    // 初始化缓冲区索引和标志
    buffer_write = 0;
    buffer_read = 0;
    buffer_overflow = 0;

    // 初始化USART设备
    yDevInitStatic(&usart_config, &usart_handle);

    // 配置DMA接收
    yDevIoctl(&usart_handle,
              YDEV_USART_IOCTL_SET_RECEIVE_DMA,
              &dma_config);

    // 使能DMA接收
    yDevIoctl(&usart_handle,
              YDEV_USART_IOCTL_ENABLE_RECEIVE_DMA,
              NULL);

    // 配置空闲中断
    yDevIoctl(&usart_handle,
              YDEV_USART_IOCTL_SET_INTERRUPT,
              &usart_exti_config);
}

/**
 * @brief 消息写入函数
 * @param msg 要发送的消息数据指针
 * @param len 要发送的消息长度
 * @retval 无
 * @note 通过USART发送指定长度的数据
 */
int32_t MessageWrite(const void *msg, uint16_t len)
{
    return yDevWrite(&usart_handle, (const uint8_t *)msg, len);
}

/**
 * @brief 从通信缓冲区读取消息
 * @param buff 用于存放读取数据的缓冲区指针
 * @param len  要读取的最大数据长度
 * @retval int32_t 返回实际读取到的数据长度，如果缓冲区为空则返回0
 * @note 该函数从环形缓冲区中读取数据，并更新读取指针。
 */
int32_t MessageRead(void *buff, uint16_t len)
{
    uint16_t bytes_to_read = 0;
    uint8_t *p_buff = (uint8_t *)buff;

    // 检查缓冲区是否为空
    if (buffer_write == buffer_read)
    {
        return 0;
    }

    // 计算可读取的数据长度
    if (buffer_write > buffer_read)
    {
        bytes_to_read = buffer_write - buffer_read;
    }
    else // 数据已回绕
    {
        bytes_to_read = UART_RX_BUF_SIZE + buffer_write - buffer_read;
    }

    // 取请求长度和可读长度中的较小值
    if (len < bytes_to_read)
    {
        bytes_to_read = len;
    }

    // 从环形缓冲区复制数据
    for (uint16_t i = 0; i < bytes_to_read; i++)
    {
        // 如果之前发生过溢出，清空标志并重置缓冲区
        if (buffer_overflow)
        {
            buffer_read = buffer_write + 1; // 重置尾部索引到头部下一个位置
            buffer_overflow = 0;
            return i; // 返回i表示曾发生溢出，数据可能不连续
        }
        p_buff[i] = usart_rx_buffer[buffer_read];
        buffer_read = (buffer_read + 1) % UART_RX_BUF_SIZE;
    }

    return bytes_to_read;
}

/**
 * @brief 消息循环处理函数
 * @retval int 缓冲区溢出状态 (0=正常, 非0=溢出)
 * @note 处理环形缓冲区中的数据，将接收到的数据通过USART发送出去
 *       这是一个回环测试函数，实现数据的原样返回
 */
int MessageLoop(void)
{
    return buffer_overflow;
}

// ==================== 私有函数实现 ====================

/**
 * @brief USART接收中断回调函数
 * @param arg 回调函数参数（未使用）
 * @retval 无
 * @note 在USART空闲中断时调用，更新缓冲区尾部索引并检测溢出
 */
static void usart_rx_callback(void *arg)
{
    (void)arg; // 避免未使用参数警告

    // 计算DMA当前写入位置（剩余长度转换为已接收位置）
    uint32_t index = UART_RX_BUF_SIZE - yDevUsartDmaRxLenGet(&usart_handle);

    // 检测缓冲区溢出：新数据覆盖了未处理的数据
    if (index < buffer_write && index >= buffer_read)
    {
        buffer_overflow = 1; // 设置溢出标志
    }

    // 更新缓冲区尾部索引
    buffer_write = index;
}
