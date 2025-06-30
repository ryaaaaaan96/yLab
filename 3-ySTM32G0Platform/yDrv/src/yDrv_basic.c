#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_bus.h"
#include "yDrv_basic.h"



// ==================== GPIO端口映射表 ====================
static GPIO_TypeDef *const gpioPortMap[] = {
    GPIOA, // Port A
    GPIOB, // Port B
    GPIOC, // Port C
    GPIOD, // Port D
#ifdef GPIOE
    GPIOE, // Port E (部分STM32G0型号可能没有)
#else
    NULL,
#endif
#ifdef GPIOF
    GPIOF, // Port F (部分STM32G0型号可能没有)
#else
    NULL,
#endif
};

/**
 * @brief 使能GPIO端口时钟
 * @param port GPIO端口指针
 */
static void prv_EnableGpioClock(GPIO_TypeDef *port);

/**
 * @brief 使能GPIO端口时钟
 * @param port GPIO端口指针
 * @note 根据不同的GPIO端口使能对应的AHB1时钟
 */
static void prv_EnableGpioClock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    }
    else if (port == GPIOB)
    {
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    }
    else if (port == GPIOC)
    {
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
    }
    else if (port == GPIOD)
    {
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOD);
    }
#ifdef GPIOE
    else if (port == GPIOE)
    {
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOE);
    }
#endif
#ifdef GPIOF
    else if (port == GPIOF)
    {
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOF);
    }
#endif
}

// ==================== GPIO解析函数实现 ====================

yDrvStatus_t yDrvParseGpio(yDrvGpioPin_t index, yDrvGpioInfo_t *pGpioInfo)
{
    if (pGpioInfo == NULL || index >= YDRV_PINMAX)
    {
        return YDRV_INVALID_PARAM;
    }

    // 计算端口索引和引脚编号
    uint8_t portIndex = index / 16; // 每个端口16个引脚
    uint8_t pinNumber = index % 16; // 引脚编号0-15

    // 检查端口是否存在
    if (portIndex >= sizeof(gpioPortMap) / sizeof(gpioPortMap[0]) ||
        gpioPortMap[portIndex] == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 填充GPIO信息
    pGpioInfo->port = gpioPortMap[portIndex];
    pGpioInfo->pin = (1U << pinNumber);
    pGpioInfo->pinNumber = pinNumber;

    // 使能GPIO端口时钟
    prv_EnableGpioClock(pGpioInfo->port);

    return YDRV_OK;
}

int32_t yDrvIsGpioValid(yDrvGpioPin_t index)
{
    if (index >= YDRV_PINMAX)
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
