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
     * @brief yDev设备操作状态枚举
     * @note 统一的设备操作返回状态定义，用于所有yDev层函数的返回值
     * @brief yDev设备操作状态枚举
     * @note 统一的设备操作返回状态定义，用于所有yDev层函数的返回值
     */
    typedef enum
    {
        YDEV_OK = 0,           /*!< 操作成功完成 */
        YDEV_ERROR,            /*!< 通用错误 */
        YDEV_BUSY,             /*!< 设备忙，无法执行操作 */
        YDEV_TIMEOUT,          /*!< 操作超时 */
        YDEV_INVALID_PARAM,    /*!< 参数无效或超出范围 */
        YDEV_NOT_INITIALIZED,  /*!< 设备未初始化 */
        YDEV_NOT_SUPPORTED,    /*!< 操作不支持或未实现 */
        YDEV_NO_MEMORY,        /*!< 内存不足 */
        YDEV_DEVICE_NOT_FOUND, /*!< 设备不存在 */
        YDEV_PERMISSION_DENIED /*!< 权限不足 */
            YDEV_OK = 0,       /*!< 操作成功完成 */
        YDEV_ERROR,            /*!< 通用错误 */
        YDEV_BUSY,             /*!< 设备忙，无法执行操作 */
        YDEV_TIMEOUT,          /*!< 操作超时 */
        YDEV_INVALID_PARAM,    /*!< 参数无效或超出范围 */
        YDEV_NOT_INITIALIZED,  /*!< 设备未初始化 */
        YDEV_NOT_SUPPORTED,    /*!< 操作不支持或未实现 */
        YDEV_NO_MEMORY,        /*!< 内存不足 */
        YDEV_DEVICE_NOT_FOUND, /*!< 设备不存在 */
        YDEV_PERMISSION_DENIED /*!< 权限不足 */
    } yDevStatus_t;

    /**
     * @brief yDev设备类型枚举
     * @note 定义yDev层支持的各种设备类型
     * @brief yDev设备类型枚举
     * @note 定义yDev层支持的各种设备类型
     */
    typedef enum
    {
        YDEV_TYPE_START = 0, /*!< 设备类型起始值 */
        YDEV_TYPE_GPIO,      /*!< GPIO通用输入输出设备 */
        YDEV_TYPE_USART,     /*!< USART串口通信设备 */
        YDEV_TYPE_SPI,       /*!< SPI串行外设接口设备 */
        YDEV_TYPE_25Q,       /*!< 25Q闪存设备 */

        YDEV_TYPE_IIC, /*!< IIC总线接口设备 */
        YDEV_TYPE_DMA, /*!< DMA直接内存访问设备 */
        YDEV_TYPE_MAX  /*!< 设备类型最大值 */
    } yDevType_t;

    /**
     * @brief yDev设备状态枚举
     * @note 定义设备的运行状态
     * @brief yDev设备状态枚举
     * @note 定义设备的运行状态
     */
    typedef enum
    {
        YDEV_STATE_UNINITIALIZED = 0,     /*!< 设备未初始化状态 */
        YDEV_STATE_INITIALIZED,           /*!< 设备已初始化状态 */
        YDEV_STATE_OPENED,                /*!< 设备已打开状态 */
        YDEV_STATE_BUSY,                  /*!< 设备忙碌状态 */
        YDEV_STATE_ERROR                  /*!< 设备错误状态 */
            YDEV_STATE_UNINITIALIZED = 0, /*!< 设备未初始化状态 */
        YDEV_STATE_INITIALIZED,           /*!< 设备已初始化状态 */
        YDEV_STATE_OPENED,                /*!< 设备已打开状态 */
        YDEV_STATE_BUSY,                  /*!< 设备忙碌状态 */
        YDEV_STATE_ERROR                  /*!< 设备错误状态 */
    } yDevState_t;

    /**
     * @brief yDev设备访问模式枚举
     * @note 定义设备的访问权限和模式
     * @brief yDev设备访问模式枚举
     * @note 定义设备的访问权限和模式
     */
    typedef enum
    {
        YDEV_MODE_RDONLY = 0x01,  /*!< 只读模式 */
        YDEV_MODE_WRONLY = 0x02,  /*!< 只写模式 */
        YDEV_MODE_RDWR = 0x03,    /*!< 读写模式 */
        YDEV_MODE_NONBLOCK = 0x04 /*!< 非阻塞模式 */
        YDEV_MODE_RDONLY = 0x01,  /*!< 只读模式 */
        YDEV_MODE_WRONLY = 0x02,  /*!< 只写模式 */
        YDEV_MODE_RDWR = 0x03,    /*!< 读写模式 */
        YDEV_MODE_NONBLOCK = 0x04 /*!< 非阻塞模式 */
    } yDevMode_t;

    // ==================== 设备操作函数指针 ====================

    /**
     * @brief 设备初始化函数指针类型
     * @param config 设备配置参数指针
     * @param handle 设备句柄指针
     * @retval yDevStatus_t 设备操作状态
     * @note 用于设备的初始化操作，配置硬件参数
     * @brief 设备初始化函数指针类型
     * @param config 设备配置参数指针
     * @param handle 设备句柄指针
     * @retval yDevStatus_t 设备操作状态
     * @note 用于设备的初始化操作，配置硬件参数
     */
    typedef yDevStatus_t (*yDevInitFunc_t)(void *config, void *handle);

    /**
     * @brief 设备反初始化函数指针类型
     * @param handle 设备句柄指针
     * @retval yDevStatus_t 设备操作状态
     * @note 用于设备的反初始化操作，释放资源
     * @brief 设备反初始化函数指针类型
     * @param handle 设备句柄指针
     * @retval yDevStatus_t 设备操作状态
     * @note 用于设备的反初始化操作，释放资源
     */
    typedef yDevStatus_t (*yDevDeinitFunc_t)(void *handle);

    /**
     * @brief 设备读取函数指针类型
     * @param handle 设备句柄指针
     * @param buffer 读取数据缓冲区指针
     * @param size 期望读取的字节数
     * @retval int32_t 实际读取的字节数，负数表示错误
     * @note 用于从设备读取数据
     * @brief 设备读取函数指针类型
     * @param handle 设备句柄指针
     * @param buffer 读取数据缓冲区指针
     * @param size 期望读取的字节数
     * @retval int32_t 实际读取的字节数，负数表示错误
     * @note 用于从设备读取数据
     */
    typedef int32_t (*yDevReadFunc_t)(void *handle, void *buffer, uint16_t size);

    /**
     * @brief 设备写入函数指针类型
     * @param handle 设备句柄指针
     * @param buffer 写入数据缓冲区指针
     * @param size 期望写入的字节数
     * @retval int32_t 实际写入的字节数，负数表示错误
     * @note 用于向设备写入数据
     * @brief 设备写入函数指针类型
     * @param handle 设备句柄指针
     * @param buffer 写入数据缓冲区指针
     * @param size 期望写入的字节数
     * @retval int32_t 实际写入的字节数，负数表示错误
     * @note 用于向设备写入数据
     */
    typedef int32_t (*yDevWriteFunc_t)(void *handle, const void *buffer, uint16_t size);

    /**
     * @brief 设备控制函数指针类型
     * @param handle 设备句柄指针
     * @param cmd 控制命令码
     * @param arg 命令参数指针
     * @retval yDevStatus_t 设备操作状态
     * @note 用于设备的特殊控制操作和配置
     * @brief 设备控制函数指针类型
     * @param handle 设备句柄指针
     * @param cmd 控制命令码
     * @param arg 命令参数指针
     * @retval yDevStatus_t 设备操作状态
     * @note 用于设备的特殊控制操作和配置
     */
    typedef yDevStatus_t (*yDevIoctlFunc_t)(void *handle, uint32_t cmd, void *arg);

    // ==================== 设备操作结构体 ====================

    /**
     * @brief 设备操作函数表结构体
     * @note 定义设备的所有操作函数指针，实现设备驱动的多态性
     * @brief 设备操作函数表结构体
     * @note 定义设备的所有操作函数指针，实现设备驱动的多态性
     */
    typedef struct
    {
        yDevType_t type;         /*!< 设备类型标识 */
        yDevInitFunc_t init;     /*!< 设备初始化函数指针 */
        yDevDeinitFunc_t deinit; /*!< 设备反初始化函数指针 */
        yDevType_t type;         /*!< 设备类型标识 */
        yDevInitFunc_t init;     /*!< 设备初始化函数指针 */
        yDevDeinitFunc_t deinit; /*!< 设备反初始化函数指针 */
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
        uint32_t errno;
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

#define YDEV_ERRNO_NO_ERROR (0)            // 设备未初始化错误
#define YDEV_ERRNO_NOT_FOUND (1UL << 0UL)  // 设备未找到错误
#define YDEV_ERRNO_NOT_INIT (1UL << 1UL)   // 设备未初始化错误
#define YDEV_ERRNO_NOT_DEINIT (1UL << 2UL) // 设备未反初始化错误

#ifdef __cplusplus
}
#endif

#endif /* YDEV_CORE_H */
