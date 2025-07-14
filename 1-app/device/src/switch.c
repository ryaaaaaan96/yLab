/**
 * @file switch.c
 * @brief 开关设备模块实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 */
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "switch.h"

// ==================== 静态变量 ====================

static yDevConfig_Gpio_t switch_config[SWITCH_TYPE_MAX] = {
    [SWITCH_TYPE_LED] = {
        .base = {
            .type = YDEV_TYPE_GPIO,
        },
        .drv_config = {
            .pin = YDRV_PINC8,
            .mode = YDRV_GPIO_MODE_OUTPUT_OD,
            .pupd = YDRV_GPIO_PUPD_NONE,
            .speed = YDRV_GPIO_SPEED_LEVEL0,
        },
    },
    [SWITCH_TYPE_BUTTON] = {
        .base = {
            .type = YDEV_TYPE_GPIO,
        },
        .drv_config = {
            .pin = YDRV_PINC0,
            .mode = YDRV_GPIO_MODE_INPUT,
            .pupd = YDRV_GPIO_PUPD_PULLUP,
            .speed = YDRV_GPIO_SPEED_LEVEL3,
        },
    },
};

static yDevHandle_Gpio_t switch_handle[SWITCH_TYPE_MAX];

static uint32_t log_flag;
static uint32_t log_time;

static void button_log(void *arg);

static yDrvGpioExtiConfig_t button_exit =
    {
        .trigger = YDRV_GPIO_EXTI_TRIGGER_FALLING,
        .prio = 1,
        .arg = &switch_handle[SWITCH_TYPE_BUTTON],
        .function = button_log,
        .enable = 1,
};

// ==================== 公共API实现 ====================

/**
 * @brief 初始化开关模块
 */
void SwitchInit(void)
{
    // 初始化开关模块变量
    log_flag = 0;
    log_time = 0;
    yDevInitStatic(&switch_config[SWITCH_TYPE_LED], &switch_handle[SWITCH_TYPE_LED]);
    yDevInitStatic(&switch_config[SWITCH_TYPE_BUTTON], &switch_handle[SWITCH_TYPE_BUTTON]);

    yDevIoctl(&switch_handle[SWITCH_TYPE_BUTTON], YDEV_GPIO_REGISTER_EXIT, &button_exit);
}

/**
 * @brief 开关控制函数
 * @param type 开关类型
 * @param st 开关状态 (0=关闭, 1=开启, 其他=翻转)
 */
void SwitchCtrl(usrSwitchType_t type, uint32_t st)
{
    uint32_t gpio_state;

    switch (st)
    {
    case 0:
        // 关
        gpio_state = 0;
        yDevWrite(&switch_handle[type], &gpio_state, sizeof(gpio_state));
        break;
    case 1:
        // 开
        gpio_state = 1;
        yDevWrite(&switch_handle[type], &gpio_state, sizeof(gpio_state));
        break;
    default:
        // 翻转
        yDevIoctl(&switch_handle[type], YDEV_GPIO_TOGGLE_PIN, NULL);
        break;
    }
}

/**
 * @brief 开关状态读取函数
 * @param type 开关类型
 * @return 开关状态 (0=关闭, 非0=开启)
 */
uint32_t SwitchRead(usrSwitchType_t type)
{
    uint32_t gpio_state;
    TickType_t nowtime = xTaskGetTickCount();
    if (pdTICKS_TO_MS(nowtime - log_time) < 1500)
    {
        return 0;
    }

    yDevRead(&switch_handle[type], &gpio_state, sizeof(gpio_state));
    return gpio_state;
}

/**
 * @brief 获取按键按下日志计数
 * @return 按键连续按下次数
 */
uint32_t SwitchGetLog(void)
{
    return log_flag;
}

/**
 * @brief 按键中断回调函数
 * @param arg 回调参数（未使用）
 *
 * @par 功能描述:
 * 按键中断服务函数，统计1.5秒内的按键按下次数
 * 用于实现不同的LED控制模式
 */
static void button_log(void *arg)
{
    (void)arg;
    TickType_t nowtime = xTaskGetTickCountFromISR();
    if (pdTICKS_TO_MS(nowtime - log_time) < 1500)
    {
        log_flag++;
    }
    else
    {
        log_flag = 0;
    }
    log_flag = log_flag > 3 ? 3 : log_flag;
    log_time = nowtime;
}
