/**
 * @file yDrv_gpio.c
 * @brief STM32G0 GPIO驱动程序实现
 * @version 2.0
 * @date 2025
 * @author YLab Development Team
 */

#include "yDrv_gpio.h"
#include "yDrv_basic.h"
#include <string.h>

// ==================== 私有定义 ====================

/**
 * @brief 全局EXTI中断回调函数数组
 * @note 存储所有EXTI线的回调函数
 */
static struct
{
    yDrvInterruptCallback_t rising_edge_callback;
    yDrvInterruptCallback_t falling_edge_callback;
    uint8_t rising_edge_flag;
    uint8_t falling_edge_flag;

} exti_callbacks[16]; // EXTI0-EXTI15对应的回调函数

// ==================== 基础函数实现 ====================

yDrvStatus_t yDrvGpioInitStatic(yDrvGpioConfig_t *config, yDrvGpioHandle_t *handle)
{
    yDrvGpioInfo_t gpioInfo;
    LL_GPIO_InitTypeDef gpio_init;

    // 参数有效性检查
    if (config == NULL || handle == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    if (yDrvIsGpioValid(config->pin) != 1)
    {
        return YDRV_INVALID_PARAM;
    }

    // 解析GPIO信息并填充句柄
    if (yDrvParseGpio(config->pin, &gpioInfo) != YDRV_OK)
    {
        return YDRV_INVALID_PARAM;
    }

    // 填充句柄信息
    handle->port = gpioInfo.port;
    handle->pin = gpioInfo.pin;
    handle->pinNumber = gpioInfo.pinNumber;

    switch (handle->pinNumber)
    {
    case 0:
    case 1:
        handle->IRQ = EXTI0_1_IRQn;
        break;
    case 2:
    case 3:
        handle->IRQ = EXTI2_3_IRQn;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        handle->IRQ = EXTI4_15_IRQn;
        break;
    default:
        return YDRV_INVALID_PARAM;
    }

    // 1. 配置GPIO参数
    LL_GPIO_StructInit(&gpio_init);

    gpio_init.Pin = handle->pin;

    // 设置工作模式
    switch (config->mode)
    {
    case YDRV_GPIO_MODE_INPUT:
        gpio_init.Mode = LL_GPIO_MODE_INPUT;
        break;
    case YDRV_GPIO_MODE_OUTPUT_PP:
        gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        break;
    case YDRV_GPIO_MODE_OUTPUT_OD:
        gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
        gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
        break;
    case YDRV_GPIO_MODE_ANALOG:
        gpio_init.Mode = LL_GPIO_MODE_ANALOG;
        break;
    default:
        return YDRV_INVALID_PARAM;
    }

    // 设置输出速度
    switch (config->speed)
    {
    case YDRV_GPIO_SPEED_LOW:
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
        break;
    case YDRV_GPIO_SPEED_MEDIUM:
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
        break;
    case YDRV_GPIO_SPEED_HIGH:
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        break;
    case YDRV_GPIO_SPEED_VERY_HIGH:
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        break;
    default:
        return YDRV_INVALID_PARAM;
    }

    // 设置上下拉
    switch (config->pupd)
    {
    case YDRV_GPIO_PUPD_NONE:
        gpio_init.Pull = LL_GPIO_PULL_NO;
        break;
    case YDRV_GPIO_PUPD_PULLUP:
        gpio_init.Pull = LL_GPIO_PULL_UP;
        break;
    case YDRV_GPIO_PUPD_PULLDOWN:
        gpio_init.Pull = LL_GPIO_PULL_DOWN;
        break;
    default:
        return YDRV_INVALID_PARAM;
    }

    gpio_init.Alternate = 0; // GPIO复用功能

    // 3. 应用GPIO配置
    if (LL_GPIO_Init(handle->port, &gpio_init) != SUCCESS)
    {
        return YDRV_ERROR;
    }

    return YDRV_OK;
}

yDrvStatus_t yDrvGpioDeInitStatic(yDrvGpioHandle_t *handle)
{
    LL_GPIO_InitTypeDef gpio_init;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 反初始化GPIO引脚为默认状态
    LL_GPIO_StructInit(&gpio_init);

    gpio_init.Pin = handle->pin;
    gpio_init.Mode = LL_GPIO_MODE_ANALOG;     // 模拟模式，最低功耗
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW; // 低速
    gpio_init.Pull = LL_GPIO_PULL_NO;         // 无上下拉
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;

    LL_GPIO_Init(handle->port, &gpio_init);

    return YDRV_OK;
}

// ==================== EXTI中断管理函数实现 ====================

static yDrvStatus_t yDrv_Gpio_Exit_Source_Set(yDrvGpioHandle_t *handle)
{
    uint32_t exti_port;
    uint32_t exti_line;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 确保SYSCFG时钟已启用
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

    // 根据GPIO端口确定EXTI端口源
    if (handle->port == GPIOA)
    {
        exti_port = LL_EXTI_CONFIG_PORTA;
    }
    else if (handle->port == GPIOB)
    {
        exti_port = LL_EXTI_CONFIG_PORTB;
    }
    else if (handle->port == GPIOC)
    {
        exti_port = LL_EXTI_CONFIG_PORTC;
    }
    else if (handle->port == GPIOD)
    {
        exti_port = LL_EXTI_CONFIG_PORTD;
    }
    else if (handle->port == GPIOF)
    {
        exti_port = LL_EXTI_CONFIG_PORTF;
    }
    else
    {
        return YDRV_INVALID_PARAM;
    }

    // 根据引脚号确定EXTI线源
    switch (handle->pinNumber)
    {
    case 0:
        exti_line = LL_EXTI_CONFIG_LINE0;
        break;
    case 1:
        exti_line = LL_EXTI_CONFIG_LINE1;
        break;
    case 2:
        exti_line = LL_EXTI_CONFIG_LINE2;
        break;
    case 3:
        exti_line = LL_EXTI_CONFIG_LINE3;
        break;
    case 4:
        exti_line = LL_EXTI_CONFIG_LINE4;
        break;
    case 5:
        exti_line = LL_EXTI_CONFIG_LINE5;
        break;
    case 6:
        exti_line = LL_EXTI_CONFIG_LINE6;
        break;
    case 7:
        exti_line = LL_EXTI_CONFIG_LINE7;
        break;
    case 8:
        exti_line = LL_EXTI_CONFIG_LINE8;
        break;
    case 9:
        exti_line = LL_EXTI_CONFIG_LINE9;
        break;
    case 10:
        exti_line = LL_EXTI_CONFIG_LINE10;
        break;
    case 11:
        exti_line = LL_EXTI_CONFIG_LINE11;
        break;
    case 12:
        exti_line = LL_EXTI_CONFIG_LINE12;
        break;
    case 13:
        exti_line = LL_EXTI_CONFIG_LINE13;
        break;
    case 14:
        exti_line = LL_EXTI_CONFIG_LINE14;
        break;
    case 15:
        exti_line = LL_EXTI_CONFIG_LINE15;
        break;
    default:
        return YDRV_INVALID_PARAM;
    }

    // 设置EXTI源连接
    LL_EXTI_SetEXTISource(exti_port, exti_line);

    return YDRV_OK;
}

yDrvStatus_t yDrvGpioRegisterCallback(yDrvGpioHandle_t *handle,
                                      yDrvGpioExit_t *exit)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct;

    // 参数有效性检查
    if ((handle == NULL) || (exit == NULL))
    {
        return YDRV_INVALID_PARAM;
    }

    EXTI_InitStruct.Line_0_31 = handle->pin;
    EXTI_InitStruct.LineCommand = ENABLE;
    switch (exit->trigger)
    {
    case YDRV_EXTI_TRIGGER_RISING:
        exti_callbacks[handle->pinNumber].rising_edge_flag = 1;
        exti_callbacks[handle->pinNumber].rising_edge_callback.function = exit->function;
        exti_callbacks[handle->pinNumber].rising_edge_callback.arg = exit->arg;
        EXTI_InitStruct.Mode = LL_EXTI_MODE_IT; // 当前只有IT
        EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
        break;
    case YDRV_EXTI_TRIGGER_FALLING:
        exti_callbacks[handle->pinNumber].falling_edge_flag = 1;
        exti_callbacks[handle->pinNumber].falling_edge_callback.function = exit->function;
        exti_callbacks[handle->pinNumber].falling_edge_callback.arg = exit->arg;
        EXTI_InitStruct.Mode = LL_EXTI_MODE_IT; // 当前只有IT
        EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
        break;
    case YDRV_EXTI_TRIGGER_RISING_FALLING:
        exti_callbacks[handle->pinNumber].falling_edge_flag = 1;
        exti_callbacks[handle->pinNumber].falling_edge_callback.function = exit->function;
        exti_callbacks[handle->pinNumber].falling_edge_callback.arg = exit->arg;
        exti_callbacks[handle->pinNumber].rising_edge_flag = 1;
        exti_callbacks[handle->pinNumber].rising_edge_callback.function = exit->function;
        exti_callbacks[handle->pinNumber].rising_edge_callback.arg = exit->arg;
        EXTI_InitStruct.Mode = LL_EXTI_MODE_IT; // 当前只有IT
        EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
        break;
    default:
        return YDRV_INVALID_PARAM;
    }

    if (LL_EXTI_Init(&EXTI_InitStruct) == 0)
    {
        return YDRV_ERROR;
    }

    // 3. 初始化EXTI
    if (yDrv_Gpio_Exit_Source_Set(handle) != YDRV_OK)
    {
        return YDRV_ERROR;
    }

    NVIC_SetPriority(handle->IRQ, exit->prio);
    NVIC_EnableIRQ(handle->IRQ);

    return YDRV_OK;
}

yDrvStatus_t yDrvGpioUnregisterCallback(yDrvGpioHandle_t *handle, yDrvExtiTrigger_t trigger)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct;

    // 参数有效性检查
    if (handle == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    EXTI_InitStruct.Line_0_31 = handle->pin;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT; // 当前只有IT
    switch (trigger)
    {
    case YDRV_EXTI_TRIGGER_RISING:
        exti_callbacks[handle->pinNumber].rising_edge_flag = 0;
        exti_callbacks[handle->pinNumber].rising_edge_callback.function = NULL;
        exti_callbacks[handle->pinNumber].rising_edge_callback.arg = NULL;
        EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
        break;
    case YDRV_EXTI_TRIGGER_FALLING:
        exti_callbacks[handle->pinNumber].falling_edge_flag = 0;
        exti_callbacks[handle->pinNumber].falling_edge_callback.function = NULL;
        exti_callbacks[handle->pinNumber].falling_edge_callback.arg = NULL;
        EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
        break;
    case YDRV_EXTI_TRIGGER_RISING_FALLING:
        exti_callbacks[handle->pinNumber].rising_edge_flag = 0;
        exti_callbacks[handle->pinNumber].rising_edge_callback.function = NULL;
        exti_callbacks[handle->pinNumber].rising_edge_callback.arg = NULL;
        exti_callbacks[handle->pinNumber].falling_edge_flag = 0;
        exti_callbacks[handle->pinNumber].falling_edge_callback.function = NULL;
        exti_callbacks[handle->pinNumber].falling_edge_callback.arg = NULL;
        break;
    default:
        break;
    }

    if ((exti_callbacks[handle->pinNumber].falling_edge_flag == 0) &&
        (exti_callbacks[handle->pinNumber].rising_edge_flag == 0))
    {
        EXTI_InitStruct.LineCommand = DISABLE;
    }
    else
    {
        EXTI_InitStruct.LineCommand = DISABLE;
    }

    if (LL_EXTI_Init(&EXTI_InitStruct) == 0)
    {
        return YDRV_ERROR;
    }

    if (((handle->pinNumber == 1) || (handle->pinNumber == 0)) &&
        (exti_callbacks[0].falling_edge_flag == 0) &&
        (exti_callbacks[0].rising_edge_flag == 0) &&
        (exti_callbacks[1].falling_edge_flag == 0) &&
        (exti_callbacks[1].rising_edge_flag == 0))
    {
        NVIC_DisableIRQ(handle->IRQ);
    }
    else if (((handle->pinNumber == 2) || (handle->pinNumber == 3)) &&
             (exti_callbacks[2].falling_edge_flag == 0) &&
             (exti_callbacks[2].rising_edge_flag == 0) &&
             (exti_callbacks[3].falling_edge_flag == 0) &&
             (exti_callbacks[3].rising_edge_flag == 0))
    {
        NVIC_DisableIRQ(handle->IRQ);
    }
    else if (((handle->pinNumber >= 4) || (handle->pinNumber <= 15)))
    {
        for (size_t i = 4; i < 16; i++)
        {
            if (exti_callbacks[i].falling_edge_flag == 1 ||
                exti_callbacks[i].rising_edge_flag == 1)
            {
                return YDRV_OK;
            }
        }
        NVIC_DisableIRQ(handle->IRQ);
    }

    return YDRV_OK;
}

yDrvStatus_t yDrvGpioClearInterruptFlag(yDrvGpioHandle_t *handle)
{
    // 参数有效性检查
    if (handle == NULL)
    {
        return YDRV_INVALID_PARAM;
    }

    // 清除EXTI中断标志
    LL_EXTI_ClearRisingFlag_0_31(handle->pin);
    LL_EXTI_ClearFallingFlag_0_31(handle->pin);

    return YDRV_OK;
}

uint8_t yDrvGpioGetInterruptFlag(yDrvGpioHandle_t *handle)
{
    // 参数有效性检查
    if (handle == NULL)
    {
        return 0;
    }
    // 检查上升沿或下降沿中断标志
    uint8_t rising_flag = LL_EXTI_IsActiveRisingFlag_0_31(handle->pin) ? 1 : 0;
    uint8_t falling_flag = LL_EXTI_IsActiveFallingFlag_0_31(handle->pin) ? 1 : 0;

    // 返回任一标志为真的结果
    return (rising_flag || falling_flag) ? 1 : 0;
}

// ==================== 私有函数实现 ====================

/**
 * @brief EXTI中断统一处理函数
 * @param extiLine EXTI线编号
 * @note 检查中断标志并调用对应的回调函数
 */
#define HANDLE_EXIT_IRQ(extiLine)                                                                                            \
    do                                                                                                                       \
    {                                                                                                                        \
        if ((LL_EXTI_IsActiveRisingFlag_0_31((1U << extiLine)) != RESET) &&                                                  \
            (exti_callbacks[extiLine].rising_edge_flag == 1))                                                                \
        {                                                                                                                    \
            LL_EXTI_ClearRisingFlag_0_31((1U << extiLine));                                                                  \
            if (exti_callbacks[extiLine].rising_edge_callback.function != NULL)                                              \
            {                                                                                                                \
                exti_callbacks[extiLine].rising_edge_callback.function(exti_callbacks[extiLine].rising_edge_callback.arg);   \
            }                                                                                                                \
        }                                                                                                                    \
        if ((LL_EXTI_IsActiveFallingFlag_0_31((1U << extiLine)) != RESET) &&                                                 \
            (exti_callbacks[extiLine].falling_edge_flag == 1))                                                               \
        {                                                                                                                    \
            LL_EXTI_ClearFallingFlag_0_31((1U << extiLine));                                                                 \
            if (exti_callbacks[extiLine].falling_edge_callback.function != NULL)                                             \
            {                                                                                                                \
                exti_callbacks[extiLine].falling_edge_callback.function(exti_callbacks[extiLine].falling_edge_callback.arg); \
            }                                                                                                                \
        }                                                                                                                    \
    } while (0)

// ==================== EXTI中断处理函数 ====================

/**
 * @brief EXTI0_1中断处理函数
 * @note 此函数由NVIC调用，处理EXTI0-1线中断
 */
void EXTI0_1_IRQHandler(void)
{
    // 检查EXTI0-1各线的中断标志
    HANDLE_EXIT_IRQ(0);
    HANDLE_EXIT_IRQ(1);
}

/**
 * @brief EXTI2_3中断处理函数
 * @note 此函数由NVIC调用，处理EXTI2-3线中断
 */
void EXTI2_3_IRQHandler(void)
{
    // 检查EXTI2-3各线的中断标志
    HANDLE_EXIT_IRQ(2);
    HANDLE_EXIT_IRQ(3);
}

/**
 * @brief EXTI4_15中断处理函数
 * @note 此函数由NVIC调用，处理EXTI4-15线中断
 */
void EXTI4_15_IRQHandler(void)
{
    // 检查EXTI4-15各线的中断标志

    HANDLE_EXIT_IRQ(4);
    HANDLE_EXIT_IRQ(5);
    HANDLE_EXIT_IRQ(6);
    HANDLE_EXIT_IRQ(7);
    HANDLE_EXIT_IRQ(8);
    HANDLE_EXIT_IRQ(9);
    HANDLE_EXIT_IRQ(10);
    HANDLE_EXIT_IRQ(11);
    HANDLE_EXIT_IRQ(12);
    HANDLE_EXIT_IRQ(13);
    HANDLE_EXIT_IRQ(14);
    HANDLE_EXIT_IRQ(15);
}
