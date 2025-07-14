/**
 * @file yDev.h
 * @brief yDev设备抽象层核心头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 提供类似Linux设备驱动的统一接口，支持各种外设的抽象操作
 *
 * @par 设计理念:
 * - 仿照Linux设备驱动模型设计
 * - 提供统一的init/deinit/read/write/ioctl接口
 * - 支持多种设备类型：字符设备、块设备等
 * - 简单易用，接口清晰
 *
 * @par 主要特性:
 * - 统一的设备操作接口
 * - 设备句柄管理
 * - 错误处理和状态查询
 * - 支持同步和异步操作
 */

#ifndef YDEV_CORE_H
#define YDEV_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

    // ==================== 基础类型定义 ====================

    /**
     * @brief 设备操作状态枚举
     */
    typedef enum
    {
        YDEV_OK = 0,           // 操作成功
        YDEV_ERROR,            // 通用错误
        YDEV_BUSY,             // 设备忙
        YDEV_TIMEOUT,          // 操作超时
        YDEV_INVALID_PARAM,    // 参数无效
        YDEV_NOT_INITIALIZED,  // 设备未初始化
        YDEV_NOT_SUPPORTED,    // 操作不支持
        YDEV_NO_MEMORY,        // 内存不足
        YDEV_DEVICE_NOT_FOUND, // 设备不存在
        YDEV_PERMISSION_DENIED // 权限不足
    } yDevStatus_t;

    /**
     * @brief 设备类型枚举
     */
    typedef enum
    {
        YDEV_TYPE_START = 0, // GPIO设备
        YDEV_TYPE_GPIO,      // GPIO设备
        YDEV_TYPE_USART,     // 串口设备
        YDEV_TYPE_SPI,       // SPI设备
        YDEV_TYPE_IIC,       // IIC设备
        YDEV_TYPE_MAX        // 定时器设备
    } yDevType_t;

    /**
     * @brief 设备状态枚举
     */
    typedef enum
    {
        YDEV_STATE_UNINITIALIZED = 0, // 未初始化
        YDEV_STATE_INITIALIZED,       // 已初始化
        YDEV_STATE_OPENED,            // 已打开
        YDEV_STATE_BUSY,              // 忙碌状态
        YDEV_STATE_ERROR              // 错误状态
    } yDevState_t;

    /**
     * @brief 设备访问模式枚举
     */
    typedef enum
    {
        YDEV_MODE_RDONLY = 0x01,  // 只读
        YDEV_MODE_WRONLY = 0x02,  // 只写
        YDEV_MODE_RDWR = 0x03,    // 读写
        YDEV_MODE_NONBLOCK = 0x04 // 非阻塞
    } yDevMode_t;

    // ==================== 设备操作函数指针 ====================

    /**
     * @brief 设备初始化函数指针
     * @param config 配置参数
     * @param handle 设备句柄
     * @retval 设备状态
     */
    typedef yDevStatus_t (*yDevInitFunc_t)(void *config, void *handle);

    /**
     * @brief 设备去初始化函数指针
     * @param handle 设备句柄
     * @retval 设备状态
     */
    typedef yDevStatus_t (*yDevDeinitFunc_t)(void *handle);

    /**
     * @brief 设备读取函数指针
     * @param handle 设备句柄
     * @param buffer 读取缓冲区
     * @param size 读取大小
     * @retval 实际读取的字节数，负数表示错误
     */
    typedef int32_t (*yDevReadFunc_t)(void *handle, void *buffer, uint16_t size);

    /**
     * @brief 设备写入函数指针
     * @param handle 设备句柄
     * @param buffer 写入缓冲区
     * @param size 写入大小
     * @retval 实际写入的字节数，负数表示错误
     */
    typedef int32_t (*yDevWriteFunc_t)(void *handle, const void *buffer, uint16_t size);

    /**
     * @brief 设备控制函数指针
     * @param handle 设备句柄
     * @param cmd 控制命令
     * @param arg 命令参数
     * @retval 设备状态
     */
    typedef yDevStatus_t (*yDevIoctlFunc_t)(void *handle, uint32_t cmd, void *arg);

    // ==================== 设备操作结构体 ====================

    /**
     * @brief 设备操作函数表
     */
    typedef struct
    {
        yDevType_t type;
        yDevInitFunc_t init;     // 初始化函数
        yDevDeinitFunc_t deinit; // 去初始化函数
        yDevReadFunc_t read;     // 读取函数
        yDevWriteFunc_t write;   // 写入函数
        yDevIoctlFunc_t ioctl;   // 控制函数// 操作函数表
    } yDevOps_t;

    // ==================== 设备句柄结构体 ====================

    /**
     * @brief 设备句柄结构体
     */
    typedef struct
    {
        yDevType_t type; // 设备类型
        uint32_t timeOutMs;
        // uint32_t use_mutex; // 是否使用锁
    } yDevConfig_t;

    /**
     * @brief 设备句柄结构体
     */
    typedef struct
    {
        uint32_t index;
        uint32_t timeOutMs;

        // void *mutex; // 互斥锁（可选）
    } yDevHandle_t;

// ==================== yDev基础配置初始化宏 ====================

/**
 * @brief yDev配置结构体默认初始化宏
 */
#define YDEV_CONFIG_DEFAULT()  \
    ((yDevConfig_t){           \
        .type = YDEV_TYPE_MAX, \
    })

/**
 * @brief yDev句柄结构体默认初始化宏
 */
#define YDEV_HANDLE_DEFAULT() \
    ((yDevHandle_t){          \
        .index = 0,           \
        .timeOutMs = 0,       \
    })

// ==================== IOCTL命令定义 ====================

/**
 * @brief 通用IOCTL命令基础值
 */
#define YDEV_IOCTL_BASE 0x8000
#define YDEV_IOCTL_GET_STATUS (YDEV_IOCTL_BASE + 0)
#define YDEV_IOCTL_RESET (YDEV_IOCTL_BASE + 1)

    // ==================== 核心API函数 ====================

    /**
     * @brief 初始化设备
     * @param config 配置参数
     * @param handle 设备句柄
     * @retval 设备状态
     */
    yDevStatus_t yDevInitStatic(void *config, void *handle);

    /**
     * @brief 反初始化设备
     * @param handle 设备句柄
     * @retval 设备状态
     */
    yDevStatus_t yDevDeinitStatic(void *handle);

    /**
     * @brief 读取设备数据
     * @param handle 设备句柄
     * @param buffer 读取缓冲区
     * @param size 读取大小
     * @retval 实际读取的字节数，负数表示错误
     */
    size_t yDevRead(void *handle, void *buffer, size_t size);

    /**
     * @brief 写入设备数据
     * @param handle 设备句柄
     * @param buffer 写入缓冲区
     * @param size 写入大小
     * @retval 实际写入的字节数，负数表示错误
     */
    size_t yDevWrite(void *handle, const void *buffer, size_t size);

    /**
     * @brief 设备控制
     * @param handle 设备句柄
     * @param cmd 控制命令
     * @param arg 命令参数
     * @retval 设备状态
     */
    yDevStatus_t yDevIoctl(void *handle, uint32_t cmd, void *arg);

    yDevStatus_t yLabInit(void);

    size_t yDevGetTimeMS(void);

#ifdef __cplusplus
}
#endif

#endif /* YDEV_CORE_H */
