/**
 * @file yDev_def.h
 * @brief yDev设备抽象层内部定义和宏
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供yDev设备抽象层的内部宏定义、导出机制和工具函数
 *
 * @par 主要功能:
 * - 设备操作表导出宏定义
 * - 编译器兼容性宏定义
 * - 设备注册和发现机制支持
 * - 内部工具函数声明
 *
 * @par 设计说明:
 * - 该文件为yDev层内部使用，不对外公开
 * - 提供设备驱动的自动注册机制
 * - 支持运行时设备操作表的动态发现
 *
 * @note 此文件仅供yDev层内部使用，应用层不应直接包含
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
     * @brief 设备操作表导出宏(完整版本)
     * @param _type 设备类型名称标识
     * @param _init 设备初始化函数名
     * @param _deinit 设备反初始化函数名
     * @param _read 设备读取函数名
     * @param _write 设备写入函数名
     * @param _ioctl 设备控制函数名
     * @note 该宏会自动生成设备操作表并将其放入特定段中，供运行时自动发现和注册
     *
     * @par 使用示例:
     * @code
     * // 在GPIO设备驱动文件中使用
     * YDEV_OPS_EXPORT_EX(GPIO, yDev_Gpio_Init, yDev_Gpio_Deinit,
     *                     yDev_Gpio_Read, yDev_Gpio_Write, yDev_Gpio_Ioctl);
     * @endcode
     *
     * @par 实现原理:
     * - 使用编译器段属性将操作表放入.ydev_ops段
     * - 运行时通过遍历该段来发现所有已注册的设备
     * - 提供设备驱动的自动注册机制
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
