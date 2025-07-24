/**
 * @file yDev_25q.h
 * @brief yDev 25Q系列SPI Flash设备驱动头文件
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 基于yDev框架的25Q系列SPI Flash设备驱动，提供统一的Flash存储接口
 *
 * @par 主要特性:
 * - 支持W25Q16/32/64/128等多种型号
 * - 统一的读写擦除接口
 * - 支持页编程和扇区擦除
 * - 支持芯片擦除和块擦除
 * - 硬件写保护功能
 * - 状态寄存器读写
 * - 设备ID和JEDEC ID读取
 * - 与底层yDrv SPI驱动的适配
 */

#ifndef YDEV_25Q_H
#define YDEV_25Q_H

#ifdef __cplusplus
extern "C"
{
#endif

// ==================== 包含文件 ====================
#include "yDev.h"
#include "yDrv_spi.h"
#include "yDrv_dma.h"
    // ==================== 25Q设备特定定义 ====================

    /**
     * @brief 25Q芯片型号枚举
     */
    typedef enum
    {
        YDEV_25Q_TYPE_W25Q16 = 0, /*!< W25Q16: 2MB */
        YDEV_25Q_TYPE_W25Q32,     /*!< W25Q32: 4MB */
        YDEV_25Q_TYPE_W25Q64,     /*!< W25Q64: 8MB */
        YDEV_25Q_TYPE_W25Q128,    /*!< W25Q128: 16MB */
        YDEV_25Q_TYPE_UNKNOWN     /*!< 未知型号 */
    } yDev25qType_t;

    /**
     * @brief 25Q命令定义
     */
    typedef enum
    {
        YDEV_25Q_CMD_WRITE_ENABLE = 0x06,  /*!< 写使能 */
        YDEV_25Q_CMD_WRITE_DISABLE = 0x04, /*!< 写禁止 */
        YDEV_25Q_CMD_PAGE_PROGRAM = 0x02,  /*!< 页编程 */

        YDEV_25Q_CMD_READ_DATA = 0x03, /*!< 读数据 */

        YDEV_25Q_CMD_SECTOR_ERASE = 0x20, /*!< 扇区擦除 (4KB) */
        YDEV_25Q_CMD_BLOCK_ERASE = 0xD8,  /*!< 块擦除 (64KB) */

        YDEV_25Q_CMD_READ_STATUS_REG1 = 0x05, /*!< 读状态寄存器1 */

        YDEV_25Q_CMD_READ_MANUFACTURER_ID = 0x90 /*!< 读制造商ID */
    } yDev25qCmd_t;

    /**
     * @brief yDev 25Q设备配置结构体
     * @note 包含yDev基础配置和25Q特定的SPI配置参数
     */
    typedef struct
    {
        yDevConfig_t base; /*!< yDev基础配置结构体 */

        yDrvSpiId_t spiId;
        yDrvSpiDirection_t direction;
        uint16_t dataBits;
        uint16_t crc;
        yDrvSpiCsMode_t csMode;
        yDrvSpiSpeedLevel_t speed;

        yDrvGpioPin_t sckPin;
        yDrvGpioPin_t misoPin;
        yDrvGpioPin_t mosiPin;
        yDrvGpioPin_t csPin;

        uint32_t sckAF;
        uint32_t misoAF;
        uint32_t mosiAF;
        uint32_t csAF;
    } yDevConfig_25q_t;

    /**
     * @brief yDev 25Q设备句柄结构体
     * @note 包含yDev基础句柄和25Q特定的SPI句柄及设备信息
     */
    typedef struct
    {
        yDevHandle_t base;          /*!< yDev基础句柄结构体 */
        yDrvSpiHandle_t spi_handle; /*!< SPI底层驱动句柄 */
        uint32_t address;           /*!< 当前操作地址 */
        uint32_t align;             /*!< 写入对齐大小 */
        uint32_t size;              /*!< Flash总大小 */
        yDev25qType_t chip_type;    /*!< 芯片型号 */
        uint16_t device_id;         /*!< 设备ID */
        uint8_t manufacturer_id;    /*!< 制造商ID */
    } yDevHandle_25q_t;

    // =============== yDev 25Q配置初始化宏 ====================

    /**
     * @brief yDev 25Q配置结构体默认初始化宏
     * @note 提供最常用的默认配置，适用于标准25Q Flash芯片
     */
#define YDEV_25Q_CONFIG_DEFAULT()                     \
    ((yDevConfig_25q_t){                              \
        .base = YDEV_CONFIG_DEFAULT(),                \
        .spiId = YDRV_SPI_1,                          \
        .direction = YDRV_SPI_DIR_2LINES_FULL_DUPLEX, \
        .dataBits = 8,                                \
        .crc = 0,                                     \
        .csMode = YDRV_SPI_CS_SOFT,                   \
        .speed = YDRV_SPI_SPEED_LEVEL3,               \
        .sckPin = YDRV_PINNULL,                       \
        .misoPin = YDRV_PINNULL,                      \
        .mosiPin = YDRV_PINNULL,                      \
        .csPin = YDRV_PINNULL,                        \
        .sckAF = 0,                                   \
        .misoAF = 0,                                  \
        .mosiAF = 0,                                  \
        .csAF = 0})

    /**
     * @brief yDev 25Q句柄结构体默认初始化宏
     * @note 提供安全的默认初始化值
     */
#define YDEV_25Q_HANDLE_DEFAULT()                \
    {                                            \
        .base = YDEV_HANDLE_DEFAULT(),           \
        .spi_handle = YDRV_SPI_HANDLE_DEFAULT(), \
        .chip_type = YDEV_25Q_TYPE_UNKNOWN}

    // ==================== yDev 25Q初始化函数 ====================

    /**
     * @brief 初始化yDev 25Q配置结构体为默认值
     * @param config 25Q配置结构体指针
     * @retval 无
     * @note 将配置结构体设置为安全的默认值
     */
    void yDev25qConfigStructInit(yDevConfig_25q_t *config);

    /**
     * @brief 初始化yDev 25Q句柄结构体为默认值
     * @param handle 25Q句柄结构体指针
     * @retval 无
     * @note 将句柄结构体设置为安全的默认值
     */
    void yDev25qHandleStructInit(yDevHandle_25q_t *handle);

// ==================== 25Q设备IOCTL命令定义 ====================

/**
 * @brief 25Q设备IOCTL命令
 */
#define YDEV_25Q_IOCTL_BASE (YDEV_IOCTL_BASE + 0x200)

#define YDEV_25Q_IOCTL_CHIP_ERASE (YDEV_25Q_IOCTL_BASE + 1)        /**< 芯片擦除 */
#define YDEV_25Q_IOCTL_SECTOR_ERASE (YDEV_25Q_IOCTL_BASE + 2)      /**< 扇区擦除 */
#define YDEV_25Q_IOCTL_BLOCK_ERASE_32K (YDEV_25Q_IOCTL_BASE + 3)   /**< 32KB块擦除 */
#define YDEV_25Q_IOCTL_BLOCK_ERASE_64K (YDEV_25Q_IOCTL_BASE + 4)   /**< 64KB块擦除 */
#define YDEV_25Q_IOCTL_WRITE_ENABLE (YDEV_25Q_IOCTL_BASE + 5)      /**< 写使能 */
#define YDEV_25Q_IOCTL_WRITE_DISABLE (YDEV_25Q_IOCTL_BASE + 6)     /**< 写禁止 */
#define YDEV_25Q_IOCTL_POWER_DOWN (YDEV_25Q_IOCTL_BASE + 7)        /**< 进入掉电模式 */
#define YDEV_25Q_IOCTL_POWER_UP (YDEV_25Q_IOCTL_BASE + 8)          /**< 退出掉电模式 */
#define YDEV_25Q_IOCTL_READ_JEDEC_ID (YDEV_25Q_IOCTL_BASE + 9)     /**< 读取JEDEC ID */
#define YDEV_25Q_IOCTL_READ_UNIQUE_ID (YDEV_25Q_IOCTL_BASE + 10)   /**< 读取唯一ID */
#define YDEV_25Q_IOCTL_READ_STATUS_REG (YDEV_25Q_IOCTL_BASE + 11)  /**< 读取状态寄存器 */
#define YDEV_25Q_IOCTL_WRITE_STATUS_REG (YDEV_25Q_IOCTL_BASE + 12) /**< 写入状态寄存器 */
#define YDEV_25Q_IOCTL_SET_PROTECTION (YDEV_25Q_IOCTL_BASE + 13)   /**< 设置写保护 */
#define YDEV_25Q_IOCTL_CLEAR_PROTECTION (YDEV_25Q_IOCTL_BASE + 14) /**< 清除写保护 */

    // ==================== 25Q错误代码定义 ====================

    /**
     * @brief 25Q错误代码定义
     * @note 这些错误代码用于标识25Q操作过程中可能出现的各种错误情况
     * @note 错误代码采用位掩码方式定义，可以进行位运算组合
     */

#define YDEV_25Q_ERRNO_NONE (0UL)                    // 无错误
#define YDEV_25Q_ERRNO_BUSY (1UL << (0))             // 芯片忙错误
#define YDEV_25Q_ERRNO_WRITE_PROTECTED (1UL << (1))  // 写保护错误
#define YDEV_25Q_ERRNO_ERASE_FAIL (1UL << (2))       // 擦除失败
#define YDEV_25Q_ERRNO_PROGRAM_FAIL (1UL << (3))     // 编程失败
#define YDEV_25Q_ERRNO_INVALID_ADDRESS (1UL << (4))  // 无效地址错误
#define YDEV_25Q_ERRNO_INVALID_SIZE (1UL << (5))     // 无效大小错误
#define YDEV_25Q_ERRNO_SPI_ERROR (1UL << (6))        // SPI通信错误
#define YDEV_25Q_ERRNO_TIMEOUT (1UL << (7))          // 超时错误
#define YDEV_25Q_ERRNO_CHIP_NOT_FOUND (1UL << (8))   // 芯片未找到错误
#define YDEV_25Q_ERRNO_INVALID_PARAM (1UL << (9))    // 无效参数错误
#define YDEV_25Q_ERRNO_NOT_INIT (1UL << (10))        // 设备未初始化错误
#define YDEV_25Q_ERRNO_ALIGNMENT_ERROR (1UL << (11)) // 地址对齐错误
#define YDEV_25Q_ERRNO_VERIFY_FAIL (1UL << (12))     // 校验失败错误
#define YDEV_25Q_ERRNO_NO_MEMORY (1UL << (13))       // 内存不足错误
#define YDEV_25Q_ERRNO_WRITE_FAIL (1UL << (14))      // 写入失败错误

    // ==================== 25Q常用常量定义 ====================

    /**
     * @brief 25Q基本参数定义
     */
#define YDEV_25Q_PAGE_SIZE (256)         /*!< 标准页大小 (字节) */
#define YDEV_25Q_SECTOR_SIZE (4096)      /*!< 标准扇区大小 (字节) */
#define YDEV_25Q_HALF_BLOCK_SIZE (32768) /*!< 32KB块大小 (字节) */
#define YDEV_25Q_BLOCK_SIZE (65536)      /*!< 64KB块大小 (字节) */

    /**
     * @brief 25Q常用时间定义 (毫秒)
     */
#define YDEV_25Q_TIMEOUT_PAGE_PROGRAM (5)       /*!< 页编程超时时间 */
#define YDEV_25Q_TIMEOUT_SECTOR_ERASE (400)     /*!< 扇区擦除超时时间 */
#define YDEV_25Q_TIMEOUT_BLOCK_ERASE_32K (1600) /*!< 32KB块擦除超时时间 */
#define YDEV_25Q_TIMEOUT_BLOCK_ERASE_64K (2000) /*!< 64KB块擦除超时时间 */
#define YDEV_25Q_TIMEOUT_CHIP_ERASE (40000)     /*!< 芯片擦除超时时间 */
#define YDEV_25Q_TIMEOUT_WRITE_ENABLE (1)       /*!< 写使能超时时间 */
#define YDEV_25Q_TIMEOUT_POWER_DOWN (3)         /*!< 掉电模式超时时间 */

    /**
     * @brief 25q JEDEC ID掩码定义
     */
#define YDEV_25Q_JEDEC_MANUFACTURER_MASK (0xFF0000) /*!< 制造商ID掩码 */
#define YDEV_25Q_JEDEC_DEVICE_TYPE_MASK (0x00FF00)  /*!< 设备类型掩码 */
#define YDEV_25Q_JEDEC_CAPACITY_MASK (0x0000FF)     /*!< 容量ID掩码 */

#ifdef __cplusplus
}
#endif

#endif /* YDEV_25Q_H */
