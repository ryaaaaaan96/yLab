/**
 * @file switch.c
 * @brief 开关设备模块实现
 * @version 1.0
 * @date 2025
 * @author YLab Development Team
 */

#include "switch.h"
#include <string.h>

// ==================== 静态变量 ====================

static struct
{
    yDevConfig_Gpio_t config;
    yDevHandle_Gpio_t handle;
} led[SWITCH_TYPE_MAX] = {
    [SWITCH_TYPE_LED] = {
        .config = {
            .base = {
                .type = YDEV_TYPE_GPIO},
            .drv_config = {
                .pin = YDRV_PINC8,
                .mode = YDRV_GPIO_MODE_OUTPUT_OD,
                .pupd = YDRV_GPIO_PUPD_NONE,
                .speed = YDRV_GPIO_SPEED_LOW,
            }},
        .handle = {.base = {0}, .drv_handle = {0}}}};

// ==================== 公共API实现 ====================

/**
 * @brief 初始化开关模块
 */
void SwitchInit(void)
{
    // 初始化LED设备
    yDevInitStatic(&led[SWITCH_TYPE_LED].config, &led[SWITCH_TYPE_LED].handle);
}

/**
 * @brief 开关控制函数
 * @param type 开关类型
 * @param st 开关状态 (0=关闭, 非0=开启)
 */
void SwitchCtrl(usrSwitchType_t type, uint32_t st)
{
    uint32_t gpio_state;

    switch (st)
    {
    case 0:
        // 关
        gpio_state = 0;
        yDevWrite(&led[type].handle, &gpio_state, sizeof(gpio_state));
        break;
    case 1:
        // 开
        gpio_state = 1;
        yDevWrite(&led[type].handle, &gpio_state, sizeof(gpio_state));
        break;
    default:
        // 翻转
        yDevIoctl(&led[type].handle, YDEV_GPIO_TOGGLE_PIN, NULL);
        break;
    }
}
