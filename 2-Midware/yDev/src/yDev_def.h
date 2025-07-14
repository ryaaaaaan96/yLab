/**
 * @file yDev_def.h
 * @brief yDev设备抽象层定义和宏
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供yDev设备抽象层的宏定义、导出机制和工具函数
 *
 * @par 主要特性:
 * - 设备操作表导出宏
 * - 编译器兼容性宏
 * - 设备注册和发现机制
 */

#ifndef YDEV_DEF_H
#define YDEV_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yLib_def.h"
#include "yDev.h"

// ==================== 设备操作表导出宏 ====================

/**
 * @brief 设备操作表导出宏
 * @param _type 设备类型名称
 * @param _init 初始化函数名
 * @param _deinit 反初始化函数名
 * @param _read 读取函数名
 * @param _write 写入函数名
 * @param _ioctl 控制函数名
 * @note 该宏会自动生成设备操作表并将其放入特定段中，供运行时发现
 *
 * @par 使用示例:
 * @code
 * // 在GPIO设备驱动文件中使用
 * YDEV_OPS_EXPORT_EX(GPIO, yDevGPIOInit, yDevGPIODeinit,
 *                     yDevGPIORead, yDevGPIOWrite, yDevGPIOIoctl);
 * @endcode
 */
#define YDEV_OPS_EXPORT_EX(_type, _init, _deinit, _read, _write, _ioctl)     \
    YLIB_USED const yDevOps_t ydev_##_type##_ops YLIB_SECTION(".ydev_ops") = \
        {                                                                    \
            .type = _type,                                                   \
            .init = _init,                                                   \
            .deinit = _deinit,                                               \
            .read = _read,                                                   \
            .write = _write,                                                 \
            .ioctl = _ioctl};

    // extern size_t ydev_time_ms;

    /**
     * @brief 初始化yDev配置结构体为默认值
     * @param config 配置结构体指针
     * @retval 无
     */
    void yDevConfigStructInit(yDevConfig_t *config);

    /**
     * @brief 初始化yDev句柄结构体为默认值
     * @param handle 句柄结构体指针
     * @retval 无
     */
    void yDevHandleStructInit(yDevHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* YDEV_DEF_H */
