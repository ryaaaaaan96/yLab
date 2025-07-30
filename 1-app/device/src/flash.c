/**
 * @file flash.c
 * @brief Flash存储设备模块实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的Flash存储设备驱动实现，提供Flash读写、擦除等统一控制接口
 * 支持25Q系列SPI Flash芯片的操作和管理
 *
 * @par 更新历史:
 * - v1.0 (2025): 初始版本，基于yDev_25q驱动的Flash应用层接口
 */

// ==================== 包含文件 ====================
#include "flash.h"
#include "yDev_25q.h"
#include <string.h>
#include <stdio.h>

// ==================== 私有变量 ====================
static yDevHandle_25q_t g_flash_handle;
static yDevConfig_25q_t g_flash_config = {
    .base.type = YDEV_TYPE_25Q,
    .base.timeOutMs = 5000, // 默认超时时间5秒
    .spiId = YDRV_SPI_1,
    .dataBits = 8,
    .crc = 0,
    .csMode = YDRV_SPI_CS_HARD_OUTPUT,
    .speed = YDRV_SPI_SPEED_LEVEL0,
    .sckPin = YDRV_PINA1,
    .misoPin = YDRV_PINA6,
    .mosiPin = YDRV_PINA2,
    .csPin = YDRV_PINA4,
    .sckAF = 0,
    .misoAF = 0,
    .mosiAF = 0,
    .csAF = 0};

static int32_t data[1024];  // 用于测试写入数据的缓冲区
static int32_t data2[1024]; // 用于测试写入数据的缓冲区
// ==================== 私有函数声明 ====================

// ==================== 公共函数实现 ====================

/**
 * @brief 初始化Flash模块
 * @retval true 初始化成功
 * @retval false 初始化失败
 * @par 功能描述:
 * 初始化25Q Flash设备，配置SPI接口参数并检测芯片
 */
int32_t FlashInit(void)
{
    size_t len;
    // 初始化配置结构体
    yDev25qHandleStructInit(&g_flash_handle);

    // 设置超时时间
    g_flash_config.base.timeOutMs = 5000; // 5秒超时

    // 初始化25Q Flash设备
    if (yDevInitStatic(&g_flash_config, &g_flash_handle) != YDEV_OK)
    {
        return -1;
    }

    for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
    {
        data[i] = i;
        data2[i] = 11;
    }

    // 全片擦除
    yDevIoctl(&g_flash_handle, YDEV_25Q_IOCTL_CHIP_ERASE, NULL);

    // 读取数据
    len = 0;
    while (len < sizeof(data2) / sizeof(data2[0]))
    {
        len += yDevRead(&g_flash_handle, &data2[len], sizeof(data2) - len);
    }

    // 写数据进去
    len = 0;
    while (len < sizeof(data) / sizeof(data[0]))
    {
        len += yDevWrite(&g_flash_handle, &data[len], sizeof(data) - len);
    }

    return 0;
}
