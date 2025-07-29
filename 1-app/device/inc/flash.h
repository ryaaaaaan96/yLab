/**
 * @file flash.h
 * @brief Flash存储设备模块头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的Flash存储设备驱动，提供Flash读写、擦除等统一控制接口
 * 支持25Q系列SPI Flash芯片的操作和管理
 */

#ifndef APP_FLASH_H
#define APP_FLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDev_25q.h"
#include <stdint.h>
#include <stdbool.h>

    // ==================== 宏定义 ====================

    // ==================== 函数声明 ====================

    /**
     * @brief 初始化Flash模块
     * @retval true 初始化成功
     * @retval false 初始化失败
     * @par 功能描述:
     * 初始化25Q Flash设备，配置SPI接口参数并检测芯片
     */
    int32_t FlashInit(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FLASH_H */
