/**
 * @file yDrv_basic.c
 * @brief STM32F4 yDrv层基础功能实现
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 *
 * @par 功能描述:
 * 实现yDrv层的基础功能，包括GPIO解析、DMA解析、系统信息查询等核心功能
 *
 * @par 主要功能:
 * - GPIO引脚信息解析和映射
 * - DMA通道信息解析和映射
 * - 系统信息查询（芯片ID、修订版本、Flash大小、唯一ID等）
 * - GPIO时钟使能控制
 * - 硬件抽象层基础服务
 */

#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_bus.h"
#include "yDrv_basic.h"

// ==================== GPIO端口映射表 ====================

/**
 * @brief GPIO端口映射表
 * @note 提供从端口索引到GPIO寄存器基地址的映射
 */
static GPIO_TypeDef *const gpioPortMap[] = {
    GPIOA, /*!< GPIOA端口寄存器基地址 */
    GPIOB, /*!< GPIOB端口寄存器基地址 */
    GPIOC, /*!< GPIOC端口寄存器基地址 */
    GPIOD, /*!< GPIOD端口寄存器基地址 */
#ifdef GPIOE
    GPIOE, /*!< GPIOE端口寄存器基地址 */
#else
    NULL,
#endif
#ifdef GPIOF
    GPIOF, /*!< GPIOF端口寄存器基地址 */
#else
    NULL,
#endif
#ifdef GPIOG
    GPIOG, /*!< GPIOG端口寄存器基地址 (STM32F4系列) */
#else
    NULL,
#endif
#ifdef GPIOH
    GPIOH, /*!< GPIOH端口寄存器基地址 (STM32F4系列) */
#else
    NULL,
#endif
#ifdef GPIOI
    GPIOI, /*!< GPIOI端口寄存器基地址 (STM32F4系列) */
#else
    NULL,
#endif
#ifdef GPIOJ
    GPIOJ, /*!< GPIOJ端口寄存器基地址 (STM32F4系列) */
#else
    NULL,
#endif
#ifdef GPIOK
    GPIOK, /*!< GPIOK端口寄存器基地址 (STM32F4系列) */
#else
    NULL,
#endif
};

// ==================== 私有函数声明 ====================

/**
 * @brief 使能GPIO端口时钟
 * @param port GPIO端口寄存器指针
 * @retval 无
 * @note 根据不同的GPIO端口使能对应的IOP时钟
 */
static void prv_EnableGpioClock(GPIO_TypeDef *port);

// ==================== 私有函数实现 ====================

/**
 * @brief 使能GPIO端口时钟
 * @param port GPIO端口寄存器指针
 * @retval 无
 * @note 根据不同的GPIO端口使能对应的AHB1时钟 (STM32F4系列)
 */
static void prv_EnableGpioClock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    }
    else if (port == GPIOB)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    }
    else if (port == GPIOC)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    }
    else if (port == GPIOD)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
    }
#ifdef GPIOE
    else if (port == GPIOE)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    }
#endif
#ifdef GPIOF
    else if (port == GPIOF)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
    }
#endif
#ifdef GPIOG
    else if (port == GPIOG)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
    }
#endif
#ifdef GPIOH
    else if (port == GPIOH)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
    }
#endif
#ifdef GPIOI
    else if (port == GPIOI)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOI);
    }
#endif
#ifdef GPIOJ
    else if (port == GPIOJ)
    {
        LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_GPIOJ);
    }
#endif
#ifdef GPIOK
    else if (port == GPIOK)
    {
        LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_GPIOK);
    }
#endif
}

// ==================== GPIO解析函数实现 ====================

yDrvStatus_t yDrvParseGpio(yDrvGpioPin_t index, yDrvGpioInfo_t *pGpioInfo)
{
    if (pGpioInfo == NULL || index >= YDRV_PINMAX || index <= YDRV_PINNULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 计算端口索引和引脚编号
    uint8_t portIndex = ((uint32_t)index - 1) / 16; // 每个端口16个引脚
    uint8_t pinNumber = ((uint32_t)index - 1) % 16; // 引脚编号0-15

    // 检查端口是否存在
    if ((portIndex >= (sizeof(gpioPortMap) / sizeof(gpioPortMap[0]))) ||
        (gpioPortMap[portIndex] == NULL))
    {
        return YDRV_INVALID_PARAM;
    }

    // 填充GPIO信息
    pGpioInfo->port = gpioPortMap[portIndex];
    pGpioInfo->pinIndex = pinNumber;
    pGpioInfo->pinMask = (1UL << pinNumber);

    // 使能GPIO端口时钟
    prv_EnableGpioClock(pGpioInfo->port);

    return YDRV_OK;
}

int32_t yDrvIsGpioValid(yDrvGpioPin_t index)
{
    if (index <= YDRV_PINNULL || index >= YDRV_PINMAX)
    {
        return -1; // 参数错误
    }

    uint8_t portIndex = index / 16;

    // 检查端口是否存在
    if (portIndex >= sizeof(gpioPortMap) / sizeof(gpioPortMap[0]) ||
        gpioPortMap[portIndex] == NULL)
    {
        return 0; // 无效
    }

    return 1; // 有效
}

// ==================== 系统信息函数实现 ====================

uint32_t yDrvGetChipId(void)
{
    return LL_DBGMCU_GetDeviceID();
}

uint32_t yDrvGetRevisionId(void)
{
    return LL_DBGMCU_GetRevisionID();
}

uint16_t yDrvGetFlashSize(void)
{
    // STM32G0系列Flash大小存储在0x1FFF75E0地址
    return (*((uint16_t *)0x1FFF75E0));
}

yDrvStatus_t yDrvGetUniqueId(uint8_t uid[12])
{
    if (uid == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // STM32G0系列唯一ID存储在0x1FFF7590-0x1FFF759B
    uint32_t *uidBase = (uint32_t *)0x1FFF7590;

    // 复制12字节的唯一ID
    for (int i = 0; i < 3; i++)
    {
        uint32_t word = uidBase[i];
        uid[i * 4 + 0] = (word >> 0) & 0xFF;
        uid[i * 4 + 1] = (word >> 8) & 0xFF;
        uid[i * 4 + 2] = (word >> 16) & 0xFF;
        uid[i * 4 + 3] = (word >> 24) & 0xFF;
    }

    return YDRV_OK;
}

// ==================== DMA解析函数实现 ====================

/**
 * @brief DMA通道信息查找表
 * @note STM32G0只有DMA1，包含7个通道
 */
static const yDrvDmaInfo_t g_dmaInfoTable[YDRV_DMA_CHANNEL_MAX] = {
    {DMA1, LL_DMA_CHANNEL_1}, // YDRV_DMA1_CHANNEL1
    {DMA1, LL_DMA_CHANNEL_2}, // YDRV_DMA1_CHANNEL2
    {DMA1, LL_DMA_CHANNEL_3}, // YDRV_DMA1_CHANNEL3
    {DMA1, LL_DMA_CHANNEL_4}, // YDRV_DMA1_CHANNEL4
    {DMA1, LL_DMA_CHANNEL_5}, // YDRV_DMA1_CHANNEL5
    {DMA1, LL_DMA_CHANNEL_6}, // YDRV_DMA1_CHANNEL6
    {DMA1, LL_DMA_CHANNEL_7}, // YDRV_DMA1_CHANNEL7
};

yDrvStatus_t yDrvParseDma(yDrvDmaChannel_t index, yDrvDmaInfo_t *pDmaInfo)
{
    // 参数有效性检查
    if (pDmaInfo == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 检查索引范围
    if (index >= YDRV_DMA_CHANNEL_MAX)
    {
        return YDRV_INVALID_PARAM;
    }

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    // 复制DMA信息
    pDmaInfo->dma = g_dmaInfoTable[index].dma;
    pDmaInfo->channel = g_dmaInfoTable[index].channel;

    return YDRV_OK;
}

int32_t yDrvIsDmaValid(yDrvDmaChannel_t index)
{
    // 检查索引范围
    if (index >= YDRV_DMA_CHANNEL_MAX)
    {
        return -1; // 参数错误
    }

    // STM32G0系列只有DMA1，所有7个通道都有效
    // 但需要检查DMA1是否存在
    if (g_dmaInfoTable[index].dma == NULL)
    {
        return 0; // 无效
    }

    return 1; // 有效
}

// ==================== 弱函数实现 ====================

__attribute__((weak)) void yDrvErrorHandler(uint32_t errorCode)
{
    // 默认错误处理：无限循环
    // 用户可以重新定义此函数来实现自定义错误处理
    (void)errorCode;
    while (1)
    {
        // 等待调试器或看门狗复位
    }
}
