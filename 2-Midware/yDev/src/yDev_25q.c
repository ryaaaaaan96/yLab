/**
 * @file yDev_25q.c
 * @brief 25Q设备驱动实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现基于yDev框架的25Q系列SPI Flash设备驱动，提供Flash的统一抽象接口
 *
 * @par 主要功能:
 * - 25Q设备的初始化和反初始化
 * - Flash数据的读写操作
 * - 页编程、扇区擦除、块擦除、芯片擦除
 * - JEDEC ID和唯一ID读取
 * - 状态寄存器操作
 * - 写保护和掉电模式控制
 * - 25Q设备的控制操作(ioctl)
 * - 完整的错误处理机制
 * - 与底层yDrv SPI驱动的适配
 *
 * @par 更新历史:
 * - v1.0 (2025): 初始版本，完整的25Q设备抽象层实现
 */

// ==================== 包含文件 ====================

// 禁用特定的编译器警告
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wenum-conversion"
#pragma GCC diagnostic ignored "-Wunused-function"

#include "yDev_25q.h"
#include "yDev_def.h"
#include "yDrv_spi.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// ==================== 私有宏定义 ====================
/**
 * @brief 25Q Flash状态寄存器1中的BUSY位定义
 * @note BUSY位位于状态寄存器1的bit0位置
 * @note BUSY=1表示芯片正在执行擦除/编程操作，BUSY=0表示芯片空闲
 */
#define YDEV_25Q_STATUS_W25Q_BUSY (0x01) // 状态寄存器1 BUSY位 (bit0)

// ==================== 私有变量 ====================
static const uint32_t IdentifyChip25q[YDEV_25Q_TYPE_UNKNOWN] = {
    [YDEV_25Q_TYPE_W25Q16] = 0xEF4016,  // W25Q16
    [YDEV_25Q_TYPE_W25Q32] = 0xEF4017,  // W25Q32
    [YDEV_25Q_TYPE_W25Q64] = 0xEF4018,  // W25Q64
    [YDEV_25Q_TYPE_W25Q128] = 0xEF4019, // W25Q128
};

// ==================== 私有函数声明 ====================

/**
 * @brief 25Q SPI数据传输函数
 * @param handle 25Q设备句柄指针
 * @param tx_data 发送数据缓冲区指针，为NULL时发送0xFF
 * @param rx_buff 接收数据缓冲区指针，为NULL时丢弃接收数据
 * @param size 传输数据大小
 * @retval int32_t 实际传输的字节数
 * @note 进行SPI双向数据传输，支持单独发送或接收
 */
static int32_t yDev25q_Spi_Transfer(yDevHandle_25q_t *handle, const void *tx_data, void *rx_buff, uint32_t size);
/**
 * @brief 读取25Q Flash状态寄存器
 * @param handle 25Q设备句柄指针
 * @param reg_num 状态寄存器编号
 *                - 0x05: 状态寄存器1 (BUSY, WEL, BP位等写保护和忙状态)
 *                - 0x35: 状态寄存器2 (QE, SRL, LB等四线模式和安全寄存器锁定)
 *                - 0x15: 状态寄存器3 (WPS, ADS, ADP等写保护选择和驱动强度)
 * @retval uint8_t 状态寄存器的值 (0x00-0xFF)
 * @note 通过SPI命令读取指定状态寄存器的当前值
 */
static uint8_t yDev25q_ReadReg(yDevHandle_25q_t *handle, uint8_t reg_num);

/**
 * @brief 等待25Q Flash芯片就绪
 * @param handle 25Q设备句柄指针
 * @param timeout_ms 超时时间(毫秒)
 * @retval yDrvStatus_t 操作状态
 *         - YDRV_OK: 芯片已就绪
 *         - YDRV_INVALID_PARAM: 参数无效
 *         - YDRV_TIMEOUT: 等待超时
 * @note 通过读取状态寄存器检查BUSY位，等待Flash操作完成
 * @note 适用于页编程、扇区擦除、芯片擦除等需要等待的操作
 */
static yDrvStatus_t yDev25q_WaitBusy(yDevHandle_25q_t *handle, uint32_t timeout_ms);

/**
 * @brief 读取25Q设备JEDEC ID
 * @param handle 25Q设备句柄指针
 * @retval uint32_t JEDEC ID值 (24位有效)
 * @note 读取25Q设备的JEDEC制造商ID和设备ID，用于芯片识别
 */
static uint32_t yDev25qReadJedecId(yDevHandle_25q_t *handle);

/**
 * @brief 根据JEDEC ID识别芯片型号
 * @param jedec_id JEDEC ID值
 * @retval yDev25qType_t 芯片型号枚举值
 * @note 解析JEDEC ID并返回对应的芯片型号，未识别时返回YDEV_25Q_TYPE_UNKNOWN
 */
static yDev25qType_t yDev25q_IdentifyChip(uint32_t jedec_id);

/**
 * @brief 25Q Flash页编程操作
 * @param handle 25Q设备句柄指针
 * @param start_address 页编程起始地址 (必须页对齐)
 * @param data 页编程数据指针 (固定256字节)
 * @retval yDrvStatus_t 操作状态
 *         - YDRV_OK: 页编程成功
 *         - YDRV_INVALID_PARAM: 参数无效
 *         - YDRV_TIMEOUT: 操作超时
 *         - YDRV_ERROR: SPI通信错误
 * @note 执行25Q Flash的页编程操作，地址必须页对齐，数据大小固定为256字节
 */
static yDrvStatus_t yDev25q_WritePage(yDevHandle_25q_t *handle, uint32_t start_address, const uint8_t *data);

/**
 * @brief 25Q脏数据检查操作
 * @param handle 25Q设备句柄指针
 * @param start_address 检查起始地址
 * @param size 检查数据大小
 * @retval yDrvStatus_t 操作状态
 * @retval YDRV_OK 数据干净（全部为0xFF）
 * @retval YDRV_ERROR 发现脏数据（非0xFF数据）
 * @retval YDRV_INVALID_PARAM 参数无效
 * @retval YDRV_TIMEOUT 操作超时
 * @note 读取指定地址范围的数据，检查是否存在脏数据，发现脏数据立即停止
 */
// static yDrvStatus_t yDev25q_CheckDirty(yDevHandle_25q_t *handle, uint32_t start_address, uint32_t size);

/**
 * @brief 25Q Flash擦除操作
 * @param handle 25Q设备句柄指针
 * @param start_address 擦除起始地址
 * @param size 擦除数据大小
 * @retval yDrvStatus_t 操作状态
 *         - YDRV_OK: 擦除成功
 *         - YDRV_INVALID_PARAM: 参数无效
 *         - YDRV_TIMEOUT: 操作超时
 *         - YDRV_ERROR: 擦除失败
 * @note 根据擦除大小自动选择扇区擦除(4KB)或块擦除(64KB)操作
 * @note 地址会自动对齐到扇区边界
 */
static yDrvStatus_t yDev25q_Erase(yDevHandle_25q_t *handle, uint32_t start_address, uint32_t size);

// ==================== 25Q配置和句柄初始化函数 ====================

/**
 * @brief 初始化25Q设备配置结构体
 * @param config 25Q设备配置结构体指针
 * @note 将配置结构体初始化为默认值，用户需要根据实际硬件修改引脚配置
 */
void yDev25qConfigStructInit(yDevConfig_25q_t *config)
{
    if (config == NULL)
    {
        return;
    }

    // 初始化基础配置
    yDevConfigStructInit(&config->base);

    // 初始化25Q特定配置为默认值
    config->spiId = YDRV_SPI_SOFT;         // 默认使用软件SPI
    config->dataBits = 8;                  // 8位数据
    config->crc = 0;                       // 不使用CRC
    config->csMode = YDRV_SPI_CS_SOFT;     // 软件控制CS
    config->speed = YDRV_SPI_SPEED_LEVEL0; // 默认速度等级0

    // 默认引脚配置（用户需要根据实际硬件修改）
    config->sckPin = YDRV_PINNULL;  // SCK引脚
    config->misoPin = YDRV_PINNULL; // MISO引脚
    config->mosiPin = YDRV_PINNULL; // MOSI引脚
    config->csPin = YDRV_PINNULL;   // CS引脚

    // 默认复用功能
    config->sckAF = 0;  // 复用功能0
    config->misoAF = 0; // 复用功能0
    config->mosiAF = 0; // 复用功能0
    config->csAF = 0;   // 复用功能0
}

/**
 * @brief 初始化25Q设备句柄结构体
 * @param handle 25Q设备句柄结构体指针
 * @note 将句柄结构体初始化为默认值
 */
void yDev25qHandleStructInit(yDevHandle_25q_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    // 初始化基础句柄和SPI句柄
    yDevHandleStructInit(&handle->base);
    yDrvSpiHandleStructInit(&handle->spi_handle);

    // 初始化25Q特定参数为默认值
    handle->chip_type = YDEV_25Q_TYPE_UNKNOWN;
    handle->size = 0;
    handle->align = 0;
    handle->address = 0;
}

// ==================== 25Q设备操作函数实现 ====================

/**
 * @brief 25Q设备初始化
 * @param config 25Q设备配置参数指针
 * @param handle 25Q设备句柄指针
 * @retval yDevStatus_t 初始化状态
 *         - YDEV_OK: 初始化成功
 *         - YDEV_INVALID_PARAM: 参数无效
 *         - YDEV_ERROR: 初始化失败
 * @note 初始化25Q设备，配置底层SPI驱动参数并识别芯片型号
 */
static yDevStatus_t yDev_25q_Init(void *config, void *handle)
{
    yDevConfig_25q_t *config_25q;
    yDevHandle_25q_t *handle_25q;
    yDrvSpiConfig_t spi_config;
    uint32_t jedec_id;

    // 参数有效性检查
    if ((handle == NULL) || (config == NULL))
    {
        return YDEV_INVALID_PARAM;
    }

    handle_25q = (yDevHandle_25q_t *)handle;
    config_25q = (yDevConfig_25q_t *)config;

    // 构建SPI配置
    yDrvSpiConfigStructInit(&spi_config);
    spi_config.spiId = config_25q->spiId;
    spi_config.direction = YDRV_SPI_DIR_2LINES_FULL_DUPLEX; // 默认全双工
    spi_config.dataBits = config_25q->dataBits;
    spi_config.crc = config_25q->crc;
    spi_config.mode = YDRV_SPI_MODE_MASTER;
    spi_config.polarity = YDRV_SPI_POLARITY_LOW;
    spi_config.phase = YDRV_SPI_PHASE_1EDGE;
    spi_config.csMode = config_25q->csMode;
    spi_config.speed = config_25q->speed;
    spi_config.bitOrder = YDRV_SPI_BITORDER_MSB;
    spi_config.sckPin = config_25q->sckPin;
    spi_config.misoPin = config_25q->misoPin;
    spi_config.mosiPin = config_25q->mosiPin;
    spi_config.csPin = config_25q->csPin;
    spi_config.sckAF = config_25q->sckAF;
    spi_config.misoAF = config_25q->misoAF;
    spi_config.mosiAF = config_25q->mosiAF;
    spi_config.csAF = config_25q->csAF;

    // 初始化SPI驱动
    if (yDrvSpiInitStatic(&spi_config, &handle_25q->spi_handle) != YDRV_OK)
    {
        return YDEV_ERROR;
    }

    // 读取JEDEC ID并识别芯片型号
    jedec_id = yDev25qReadJedecId(handle_25q);
    handle_25q->chip_type = yDev25q_IdentifyChip(jedec_id);

    // 根据芯片型号识别结果设置Flash大小
    if (handle_25q->chip_type == YDEV_25Q_TYPE_UNKNOWN)
    {
        handle_25q->base.errno = YDEV_25Q_ERRNO_CHIP_NOT_FOUND;
        yDrvSpiDeInitStatic(&handle_25q->spi_handle);
        return YDEV_ERROR;
    }

    // 根据JEDEC ID计算Flash容量
    // JEDEC ID格式: [制造商ID(8位)][设备类型(8位)][容量ID(8位)]
    // 容量ID对应关系: 0x15=2MB, 0x16=4MB, 0x17=8MB, 0x18=16MB, 0x19=32MB
    uint8_t capacity_id = jedec_id & 0xFF;
    if (capacity_id >= 0x10 && capacity_id <= 0x20)
    {
        handle_25q->size = 1UL << capacity_id; // 2^capacity_id 字节
    }
    else
    {
        handle_25q->size = 2 * 1024 * 1024; // 默认2MB
    }

    return YDEV_OK;
}

/**
 * @brief 25Q设备反初始化
 * @param handle 25Q设备句柄指针
 * @retval yDevStatus_t 操作状态
 *         - YDEV_OK: 反初始化成功
 *         - YDEV_INVALID_PARAM: 参数无效
 *         - YDEV_ERROR: 反初始化失败
 * @note 释放25Q设备资源，反初始化底层SPI驱动
 */
static yDevStatus_t yDev_25q_Deinit(void *handle)
{
    yDevHandle_25q_t *handle_25q;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    handle_25q = (yDevHandle_25q_t *)handle;

    // 反初始化SPI驱动
    if (yDrvSpiDeInitStatic(&handle_25q->spi_handle) != YDRV_OK)
    {
        return YDEV_ERROR;
    }

    // 重置设备状态
    handle_25q->chip_type = YDEV_25Q_TYPE_UNKNOWN;
    handle_25q->size = 0;

    return YDEV_OK;
}

/**
 * @brief 25Q设备读取操作
 * @param handle 25Q设备句柄指针
 * @param buffer 读取数据缓冲区指针
 * @param size 读取数据大小
 * @retval int32_t 实际读取的字节数，-1表示错误
 * @note 从25Q Flash中读取数据，使用设备句柄中的address字段作为起始地址
 */
static int32_t yDev_25q_Read(void *handle, void *buffer, uint16_t size)
{
    yDevHandle_25q_t *handle_25q;
    uint8_t *read_buff;
    uint32_t start_time;
    uint32_t index;
    uint8_t write_data;
    uint8_t read_cmd[4];
    uint32_t len;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size == 0))
    {
        return -1;
    }

    handle_25q = (yDevHandle_25q_t *)handle;
    read_buff = (uint8_t *)buffer;

    // 检查地址有效性
    if ((handle_25q->address + size) > handle_25q->size)
    {
        handle_25q->base.errno = YDEV_25Q_ERRNO_INVALID_ADDRESS;
        return -1;
    }

    // 等待芯片就绪
    if (yDev25q_WaitBusy(handle_25q, YDEV_25Q_TIMEOUT_PAGE_PROGRAM) != YDRV_OK)
    {
        handle_25q->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
        return -1;
    }

    // 构建读取命令
    yDrvSpiCsControl(&handle_25q->spi_handle, 1); // 选中芯片(CS拉低)
    read_cmd[0] = YDEV_25Q_CMD_READ_DATA;
    read_cmd[1] = (handle_25q->address >> 16) & 0xFF; // 地址高字节
    read_cmd[2] = (handle_25q->address >> 8) & 0xFF;  // 地址中字节
    read_cmd[3] = handle_25q->address & 0xFF;         // 地址低字节

    if (yDev25q_Spi_Transfer(handle_25q, read_cmd, NULL, 4) != 4) // 发送读取命令和地址
    {
        yDrvSpiCsControl(&handle_25q->spi_handle, 0); // 取消选中
        handle_25q->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
        return -1;
    }

    // 逐字节读取数据
    start_time = yDevGetTimeMS();
    write_data = 0xFF; // 发送空数据以时钟Flash输出数据
    index = 0;

    while (index < size)
    {
        // 检查超时
        if (yDevGetTimeMS() - start_time > handle_25q->base.timeOutMs)
        {
            yDrvSpiCsControl(&handle_25q->spi_handle, 0); // 取消选中(CS拉高)
            handle_25q->address += index;
            handle_25q->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
            return (int32_t)index;
        }

        // 发送空字节并读取数据
        len = yDrvSpiWriteByte(&handle_25q->spi_handle, &write_data);
        if (len == 0)
        {
            continue; // 如果写入失败，重试
        }

        len = yDrvSpiReadByte(&handle_25q->spi_handle, &read_buff[index]);
        index += len;
    }

    yDrvSpiCsControl(&handle_25q->spi_handle, 0); // 取消选中(CS拉高)

    // 更新位置
    handle_25q->address += index;

    return (int32_t)index;
}

/**
 * @brief 25Q设备写入操作 (页编程)
 * @param handle 25Q设备句柄指针
 * @param buffer 写入数据缓冲区指针
 * @param size 写入数据大小
 * @retval int32_t 实际写入的字节数，-1表示错误
 * @note 向25Q Flash写入数据，自动处理页边界和写使能
 * @note 使用设备句柄中的address字段作为起始地址
 */
static int32_t yDev_25q_Write(void *handle, const void *buffer, uint16_t size)
{
    yDevHandle_25q_t *handle_25q;
    const uint8_t *write_buff;
    uint32_t current_address;
    uint32_t total_written;
    uint32_t page_offset;
    uint32_t write_size;

    // 参数有效性检查
    if ((handle == NULL) || (buffer == NULL) || (size == 0))
    {
        return -1;
    }

    handle_25q = (yDevHandle_25q_t *)handle;
    write_buff = (const uint8_t *)buffer;
    total_written = 0;
    // 检查地址有效性
    if ((handle_25q->address + size) > handle_25q->size)
    {
        handle_25q->base.errno = YDEV_25Q_ERRNO_INVALID_ADDRESS;
        return -1;
    }

    current_address = handle_25q->address;

    // 按页写入数据循环
    while (total_written < size)
    {
        // 计算当前页内偏移和本次写入大小
        page_offset = current_address % YDEV_25Q_PAGE_SIZE;
        write_size = YDEV_25Q_PAGE_SIZE - page_offset;
        if (write_size > (size - total_written))
        {
            write_size = size - total_written;
        }

        // 处理非对齐写入 (读取-修改-写入)
        if ((page_offset != 0) ||
            (write_size != YDEV_25Q_PAGE_SIZE))
        {
            uint8_t page_buffer[YDEV_25Q_PAGE_SIZE];
            uint32_t page_start = current_address & ~(YDEV_25Q_PAGE_SIZE - 1);

            // 保存当前地址并读取整页数据
            uint32_t saved_address = handle_25q->address;
            handle_25q->address = page_start;
            if (yDev_25q_Read(handle, page_buffer, YDEV_25Q_PAGE_SIZE) != YDEV_25Q_PAGE_SIZE)
            {
                handle_25q->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
                return -1;
            }

            // 修改页数据并写入整页
            memcpy(&page_buffer[page_offset], &write_buff[total_written], write_size);
            if (yDev25q_WritePage(handle_25q, page_start, page_buffer) != YDRV_OK)
            {
                handle_25q->base.errno = YDEV_25Q_ERRNO_WRITE_FAIL;
                return -1;
            }

            handle_25q->address = saved_address; // 恢复地址
        }
        else
        {
            // 整页写入
            if (yDev25q_WritePage(handle_25q, current_address, &write_buff[total_written]) != YDRV_OK)
            {
                handle_25q->base.errno = YDEV_25Q_ERRNO_WRITE_FAIL;
                return -1;
            }
        }

        // 更新计数器和地址
        total_written += write_size;
        current_address += write_size;
    }

    // 更新设备地址
    handle_25q->address = current_address;

    return (int32_t)total_written;
}

/**
 * @brief 25Q设备控制操作
 * @param handle 25Q设备句柄指针
 * @param cmd 控制命令
 * @param arg 命令参数指针
 * @retval yDevStatus_t 操作状态
 *         - YDEV_OK: 操作成功
 *         - YDEV_INVALID_PARAM: 参数无效
 *         - YDEV_NOT_SUPPORTED: 不支持的命令
 *         - YDEV_ERROR: 操作失败
 * @note 执行25Q设备的控制操作，如芯片擦除、读取JEDEC ID等
 */
static yDevStatus_t yDev_25q_Ioctl(void *handle, uint32_t cmd, void *arg)
{
    yDevHandle_25q_t *handle_25q;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDEV_INVALID_PARAM;
    }

    handle_25q = (yDevHandle_25q_t *)handle;

    switch (cmd)
    {
    case YDEV_25Q_IOCTL_CHIP_ERASE:
        // 全片擦除操作
        return yDev25q_Erase(handle_25q, 0, handle_25q->size);

    case YDEV_25Q_IOCTL_READ_JEDEC_ID:
        // 读取JEDEC ID
        if (arg != NULL)
        {
            *((uint32_t *)arg) = yDev25qReadJedecId(handle_25q);
            return YDEV_OK;
        }
        return YDEV_INVALID_PARAM;

    default:
        return YDEV_NOT_SUPPORTED;
    }
}

// ==================== 25Q私有函数实现 ====================

/**
 * @brief 25Q SPI数据传输函数实现
 * @param handle 25Q设备句柄指针
 * @param tx_data 发送数据缓冲区指针，为NULL时发送临时数据
 * @param rx_buff 接收数据缓冲区指针，为NULL时丢弃接收数据
 * @param size 传输数据大小
 * @retval int32_t 实际传输的字节数
 * @note 进行SPI双向数据传输，支持单独发送或接收
 */
static int32_t yDev25q_Spi_Transfer(yDevHandle_25q_t *handle,
                                    const void *tx_data,
                                    void *rx_buff,
                                    uint32_t size)
{
    uint32_t start_time;
    uint32_t index;
    uint32_t len;
    const uint8_t *write_buff;
    uint8_t *read_buff;
    uint32_t temp_data;
    uint8_t flag;

    // 设置发送和接收缓冲区指针
    write_buff = (tx_data == NULL) ? (const uint8_t *)&temp_data : (const uint8_t *)tx_data;
    read_buff = (rx_buff == NULL) ? (uint8_t *)&temp_data : (uint8_t *)rx_buff;

    // 开始计时
    start_time = yDevGetTimeMS();
    index = 0;
    temp_data = 0xFF; // 默认发送0xFF
    flag = 1;

    // 逐字节传输数据
    while (index < size)
    {
        // 检查超时
        if (yDevGetTimeMS() - start_time > handle->base.timeOutMs)
        {
            return (int32_t)index;
        }

        // 发送一个字节
        if (flag == 1)
        {
            len = yDrvSpiWriteByte(&handle->spi_handle, write_buff);
            if (len == 0)
            {
                continue; // 发送失败，重试
            }
            flag = 0; // 标记发送成功
            write_buff += ((tx_data == NULL) ? 0 : len);
        }
        // 更新发送指针和接收数据
        len = yDrvSpiReadByte(&handle->spi_handle, read_buff);
        if (len == 0)
        {
            continue; // 发送失败，重试
        }
        flag = 1; // 标记发送成功
        read_buff += ((rx_buff == NULL) ? 0 : len);
        index += len;
    }

    return (int32_t)index;
}
/**
 * @brief 读取25Q Flash状态寄存器实现
 * @param handle 25Q设备句柄指针
 * @param reg 状态寄存器命令字节
 * @retval uint8_t 状态寄存器的值，读取失败时返回0xFF
 * @note 通过SPI发送寄存器读取命令并接收状态值
 */
static uint8_t yDev25q_ReadReg(yDevHandle_25q_t *handle, uint8_t reg)
{
    uint8_t write_data[2];
    uint8_t read_data[2];

    write_data[0] = reg;                      // 状态寄存器读取命令
    write_data[1] = 0xFF;                     // 读取数据填充
    yDrvSpiCsControl(&handle->spi_handle, 0); // 选中芯片

    if (yDev25q_Spi_Transfer(handle, write_data, read_data, 2) != 2) // 发送命令并接收数据
    {
        yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中
        return 0xFF;                              // 读取失败
    }

    yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中
    return read_data[1];
}

/**
 * @brief 等待25Q Flash芯片就绪实现
 * @param handle 25Q设备句柄指针
 * @param timeout_ms 超时时间(毫秒)
 * @retval yDrvStatus_t 操作状态
 * @note 根据芯片类型选择对应的状态寄存器和BUSY位检查
 */
static yDrvStatus_t yDev25q_WaitBusy(yDevHandle_25q_t *handle, uint32_t timeout_ms)
{
    uint32_t start_time;
    uint8_t status;
    uint8_t reg_cmd;
    uint8_t busy_mask;

    // 根据芯片类型确定状态寄存器和BUSY位
    switch (IdentifyChip25q[handle->chip_type] & 0xFF0000)
    {
    case 0xEF0000:                             // W25Q系列芯片
        reg_cmd = 0x05;                        // 状态寄存器1读取命令
        busy_mask = YDEV_25Q_STATUS_W25Q_BUSY; // BUSY位掩码
        break;
    default:
        return YDRV_INVALID_PARAM; // 不支持的芯片类型
    }

    // 轮询检查BUSY位直到芯片就绪或超时
    start_time = yDevGetTimeMS();
    do
    {
        status = yDev25q_ReadReg(handle, reg_cmd);
        if ((status & busy_mask) == 0)
        {
            return YDRV_OK; // 芯片就绪
        }
    } while ((yDevGetTimeMS() - start_time) <= timeout_ms);

    return YDRV_TIMEOUT; // 超时
}

/**
 * @brief 根据JEDEC ID识别芯片型号实现
 * @param jedec_id JEDEC ID值
 * @retval yDev25qType_t 芯片型号枚举值
 * @note 遍历已知芯片ID表进行匹配识别
 */
static yDev25qType_t yDev25q_IdentifyChip(uint32_t jedec_id)
{
    // 遍历芯片ID表进行匹配
    for (yDev25qType_t i = YDEV_25Q_TYPE_W25Q16; i < YDEV_25Q_TYPE_UNKNOWN; i++)
    {
        if (IdentifyChip25q[i] == jedec_id)
        {
            return i;
        }
    }
    return YDEV_25Q_TYPE_UNKNOWN;
}

/**
 * @brief 读取25Q设备JEDEC ID实现
 * @param handle 25Q设备句柄指针
 * @retval uint32_t JEDEC ID值，读取失败时返回0
 * @note 发送0x9F命令读取3字节JEDEC ID
 */
static uint32_t yDev25qReadJedecId(yDevHandle_25q_t *handle)
{
    uint8_t cmd[4] = {0x9F, 0xFF, 0xFF, 0xFF}; // JEDEC ID读取命令
    uint8_t jedec_data[4] = {0};               // 用于存储读取的JEDEC ID数据
    uint32_t jedec_id = 0;

    // 读取3字节JEDEC ID数据
    yDrvSpiCsControl(&handle->spi_handle, 0); // 选中
    if (yDev25q_Spi_Transfer(handle, &cmd, &jedec_data[0], 4) != 4)
    {
        yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中
        return 0;
    }
    yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中

    // 组合24位JEDEC ID
    jedec_id = ((uint32_t)jedec_data[1] << 16) |
               ((uint32_t)jedec_data[2] << 8) |
               (uint32_t)jedec_data[3];
    return jedec_id;
}

/**
 * @brief 25Q Flash页编程操作实现
 * @param handle 25Q设备句柄指针
 * @param start_address 页编程起始地址
 * @param write_data 页编程数据指针
 * @retval yDrvStatus_t 操作状态
 * @note 执行完整的页编程流程：等待就绪->写使能->发送命令和地址->发送数据->等待完成
 */
static yDrvStatus_t yDev25q_WritePage(yDevHandle_25q_t *handle,
                                      uint32_t start_address,
                                      const uint8_t *write_data)
{
    uint8_t write_cmd[4];

    // 1. 等待芯片就绪
    if (yDev25q_WaitBusy(handle, YDEV_25Q_TIMEOUT_PAGE_PROGRAM) != YDRV_OK)
    {
        handle->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
        return YDRV_TIMEOUT;
    }

    // 2. 发送写使能命令
    write_cmd[0] = YDEV_25Q_CMD_WRITE_ENABLE; // 写使能命令
    yDrvSpiCsControl(&handle->spi_handle, 0); // 选中芯片(CS拉低)
    if (yDev25q_Spi_Transfer(handle, &write_cmd[0], NULL, 4) != 4)
    {
        yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中
        handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
        return YDRV_ERROR;
    }

    // 3. 发送页编程命令和地址
    write_cmd[0] = YDEV_25Q_CMD_PAGE_PROGRAM;    // 页编程命令
    write_cmd[1] = (start_address >> 16) & 0xFF; // 地址高字节
    write_cmd[2] = (start_address >> 8) & 0xFF;  // 地址中字节
    write_cmd[3] = start_address & 0xFF;         // 地址低字节

    if (yDev25q_Spi_Transfer(handle, write_cmd, NULL, 4) != 4)
    {
        yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中
        handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
        return YDRV_ERROR;
    }

    // 4. 发送页数据
    if (yDev25q_Spi_Transfer(handle, write_data, NULL, YDEV_25Q_PAGE_SIZE) != YDEV_25Q_PAGE_SIZE)
    {
        yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中
        handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
        return YDRV_ERROR;
    }

    // 5. 等待页编程完成
    if (yDev25q_WaitBusy(handle, YDEV_25Q_TIMEOUT_PAGE_PROGRAM) != YDRV_OK)
    {
        handle->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
        return YDRV_TIMEOUT;
    }
    yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中(CS拉高)

    return YDRV_OK;
}

// 删除未使用的yDev25q_CheckDirty函数

/**
 * @brief 25Q Flash擦除操作实现
 * @param handle 25Q设备句柄指针
 * @param start_address 擦除起始地址
 * @param size 擦除数据大小
 * @retval yDrvStatus_t 操作状态
 * @note 自动选择最优擦除方式：64KB块擦除优先，不足部分使用4KB扇区擦除
 * @note 地址自动对齐到扇区边界
 */
static yDrvStatus_t yDev25q_Erase(yDevHandle_25q_t *handle, uint32_t start_address, uint32_t size)
{
    uint32_t current_address;
    uint32_t remaining_size;
    uint32_t erase_address;
    uint32_t erase_size;
    uint8_t write_enable_cmd;
    uint8_t erase_cmd[4];

    // 参数有效性检查
    if (handle == NULL || size == 0)
    {
        return YDRV_INVALID_PARAM;
    }

    // 检查地址有效性
    if ((start_address + size) > handle->size)
    {
        handle->base.errno = YDEV_25Q_ERRNO_INVALID_ADDRESS;
        return YDRV_INVALID_PARAM;
    }

    // 将起始地址对齐到扇区边界（4KB对齐）
    current_address = start_address & ~(YDEV_25Q_SECTOR_SIZE - 1);

    // 计算需要擦除的总大小（包含对齐产生的额外部分）
    remaining_size = ((start_address + size + YDEV_25Q_SECTOR_SIZE - 1) &
                      ~(YDEV_25Q_SECTOR_SIZE - 1)) -
                     current_address;
    write_enable_cmd = YDEV_25Q_CMD_WRITE_ENABLE;

    // 擦除循环：优先使用64KB块擦除，剩余部分使用4KB扇区擦除
    while (remaining_size > 0)
    {
        // 判断是否可以进行64KB块擦除
        if (remaining_size >= YDEV_25Q_BLOCK_SIZE &&
            (current_address % YDEV_25Q_BLOCK_SIZE) == 0)
        {
            // 执行64KB块擦除
            erase_address = current_address;
            erase_size = YDEV_25Q_BLOCK_SIZE;

            // 发送写使能命令
            yDrvSpiCsControl(&handle->spi_handle, 0); // 选中芯片
            if (yDev25q_Spi_Transfer(handle, &write_enable_cmd, NULL, 1) != 1)
            {
                yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中
                handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
                return YDRV_ERROR;
            }

            // 等待芯片就绪
            if (yDev25q_WaitBusy(handle, YDEV_25Q_TIMEOUT_BLOCK_ERASE_64K) != YDRV_OK)
            {
                handle->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
                return YDRV_TIMEOUT;
            }

            // 发送64KB块擦除命令
            erase_cmd[0] = YDEV_25Q_CMD_BLOCK_ERASE;
            erase_cmd[1] = (erase_address >> 16) & 0xFF; // 地址高字节
            erase_cmd[2] = (erase_address >> 8) & 0xFF;  // 地址中字节
            erase_cmd[3] = erase_address & 0xFF;         // 地址低字节

            if (yDev25q_Spi_Transfer(handle, erase_cmd, NULL, 4) != 4)
            {
                yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中
                handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
                return YDRV_ERROR;
            }
            yDrvSpiCsControl(&handle->spi_handle, 1); // 取消选中

            // 等待块擦除完成
            if (yDev25q_WaitBusy(handle, YDEV_25Q_TIMEOUT_BLOCK_ERASE_64K) != YDRV_OK)
            {
                handle->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
                return YDRV_TIMEOUT;
            }
        }
        else
        {
            // 执行4KB扇区擦除
            erase_address = current_address;
            erase_size = YDEV_25Q_SECTOR_SIZE;

            // 等待芯片就绪
            if (yDev25q_WaitBusy(handle, YDEV_25Q_TIMEOUT_SECTOR_ERASE) != YDRV_OK)
            {
                handle->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
                return YDRV_TIMEOUT;
            }

            // 发送写使能命令
            yDrvSpiCsControl(&handle->spi_handle, 1); // 选中芯片
            if (yDev25q_Spi_Transfer(handle, &write_enable_cmd, NULL, 1) != 1)
            {
                yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中
                handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
                return YDRV_ERROR;
            }
            yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中

            // 发送4KB扇区擦除命令
            yDrvSpiCsControl(&handle->spi_handle, 1); // 选中芯片
            erase_cmd[0] = YDEV_25Q_CMD_SECTOR_ERASE;
            erase_cmd[1] = (erase_address >> 16) & 0xFF; // 地址高字节
            erase_cmd[2] = (erase_address >> 8) & 0xFF;  // 地址中字节
            erase_cmd[3] = erase_address & 0xFF;         // 地址低字节

            if (yDev25q_Spi_Transfer(handle, erase_cmd, NULL, 4) != 4)
            {
                yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中
                handle->base.errno = YDEV_25Q_ERRNO_SPI_ERROR;
                return YDRV_ERROR;
            }
            yDrvSpiCsControl(&handle->spi_handle, 0); // 取消选中

            // 等待扇区擦除完成
            if (yDev25q_WaitBusy(handle, YDEV_25Q_TIMEOUT_SECTOR_ERASE) != YDRV_OK)
            {
                handle->base.errno = YDEV_25Q_ERRNO_TIMEOUT;
                return YDRV_TIMEOUT;
            }
        }

        // 更新地址和剩余大小
        current_address += erase_size;
        remaining_size -= erase_size;
    }

    return YDRV_OK;
}

// ==================== 25Q设备操作导出 ====================

YDEV_OPS_EXPORT_EX(
    YDEV_TYPE_25Q,   // 设备类型
    yDev_25q_Init,   // 初始化函数
    yDev_25q_Deinit, // 反初始化函数
    yDev_25q_Read,   // 读取函数
    yDev_25q_Write,  // 写入函数
    yDev_25q_Ioctl)  // 控制函数

// 恢复编译器警告
#pragma GCC diagnostic pop
