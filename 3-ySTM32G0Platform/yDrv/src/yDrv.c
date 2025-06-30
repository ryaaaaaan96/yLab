/**
 * @file yDrv.c
 * @brief yDrv核心初始化模块实现
 * @details 提供系统时钟配置、定时器初始化等核心功能
 * @author Ryan
 * @date 2025-06-30
 * @version 1.0
 * @copyright Copyright (c) 2025 YLab
 */

#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_utils.h"

#include "yDrv_basic.h"

// ==================== 私有函数声明 ====================
static void SystemClock_Config(void);

// ==================== 公共函数实现 ====================

/**
 * @brief yDrv驱动模块初始化
 * @details 初始化HAL库和系统时钟配置
 * @param 无
 * @return yDrvStatus_t 初始化状态
 *         @retval YDRV_OK 初始化成功
 * @note 此函数必须在使用任何yDrv功能前调用
 */
yDrvStatus_t yDrvInit(void)
{
    HAL_Init();
    SystemClock_Config();

    return YDRV_OK;
}

// ==================== 私有函数实现 ====================

/**
 * @brief 系统时钟配置
 * @details 配置系统时钟为64MHz，使用外部高速晶振(HSE)和PLL
 * @param 无
 * @return 无
 * @note 时钟配置：HSE(8MHz) -> PLL(x16/2) -> 64MHz系统时钟
 *       - Flash延迟：2个等待周期
 *       - AHB分频：1 (64MHz)
 *       - APB1分频：1 (64MHz)
 *       - APB2分频：1 (64MHz)
 */
static void SystemClock_Config(void)
{
    // 设置Flash延迟为2个等待周期（64MHz系统时钟需要）
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
    {
        // 等待Flash延迟设置完成
    }

    // 启用外部高速晶振(HSE)
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1)
    {
        // 等待HSE就绪
    }

    // 配置主PLL：HSE作为输入源，M=1, N=16, R=2
    // PLL输出频率 = HSE * N / (M * R) = 8MHz * 16 / (1 * 2) = 64MHz
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 16, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while (LL_RCC_PLL_IsReady() != 1)
    {
        // 等待PLL就绪
    }

    // 设置AHB预分频器为1（不分频）
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    // 切换系统时钟源到PLL
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
        // 等待系统时钟源切换完成
    }

    // 设置APB1预分频器为1（不分频）
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    // 更新CMSIS系统时钟变量
    LL_SetSystemCoreClock(64000000);

    // 初始化系统滴答定时器
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
    {
        while (1)
        {
            __NOP(); // 初始化失败，进入无限循环
        }
    }
}

// ==================== HAL MSP初始化函数 ====================

/**
 * @brief HAL MSP初始化函数
 * @details 初始化全局的微控制器支持包，启用必要的系统时钟
 * @param 无
 * @return 无
 * @note 此函数由HAL_Init()自动调用
 */
void HAL_MspInit(void)
{
    // 启用SYSCFG时钟（GPIO外部中断配置需要）
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // 启用电源管理时钟（低功耗模式和电压监控需要）
    __HAL_RCC_PWR_CLK_ENABLE();
}

// ==================== 系统滴答定时器实现 ====================

/**
 * @brief 系统滴答定时器初始化
 * @details 使用TIM17配置为1ms时基的系统滴答定时器
 * @param TickPriority 滴答定时器中断优先级
 * @return HAL_StatusTypeDef 初始化状态
 *         @retval HAL_OK 初始化成功
 *         @retval HAL_ERROR 初始化失败
 * @note 定时器配置：
 *       - 时钟源：64MHz APB2时钟
 *       - 预分频：64 (得到1MHz计数时钟)
 *       - 自动重装载：1000 (得到1ms周期)
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    uint32_t uwTimclock;
    uint32_t uwPrescalerValue;

    // 启用TIM17时钟
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM17);

    // 配置TIM17中断
    NVIC_SetPriority(TIM17_IRQn, TickPriority);
    NVIC_EnableIRQ(TIM17_IRQn);

    // 获取TIM17时钟频率
    // TIM17位于APB2总线上，根据当前时钟配置获取频率
    // 基于系统时钟配置：64MHz系统时钟，APB2分频器为1，所以APB2时钟为64MHz
    uwTimclock = 64000000U;

    // 计算预分频器值，使TIM17计数时钟为1MHz
    uwPrescalerValue = (uwTimclock / 1000000U) - 1U;

    // 配置定时器参数
    TIM_InitStruct.Prescaler = uwPrescalerValue;              // 预分频器: 63 (64MHz/64 = 1MHz)
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;       // 向上计数模式
    TIM_InitStruct.Autoreload = (1000000U / 1000U) - 1U;      // 自动重装载值: 999 (1MHz/1000 = 1ms周期)
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1; // 时钟分频: 不分频
    TIM_InitStruct.RepetitionCounter = 0;                     // 重复计数器: 0 (每次更新都产生中断)

    // 初始化定时器
    LL_TIM_Init(TIM17, &TIM_InitStruct);

    // 禁用自动重装载预装载
    LL_TIM_DisableARRPreload(TIM17);

    // 启用更新中断
    LL_TIM_EnableIT_UPDATE(TIM17);

    // 启动定时器计数器
    LL_TIM_EnableCounter(TIM17);

    return HAL_OK;
}

/**
 * @brief 暂停系统滴答计数
 * @details 通过禁用TIM17更新中断来暂停系统滴答计数
 * @param 无
 * @return 无
 * @note 暂停期间HAL_GetTick()返回值将不会递增
 */
void HAL_SuspendTick(void)
{
    // 禁用TIM17更新中断
    LL_TIM_DisableIT_UPDATE(TIM17);
}

/**
 * @brief 恢复系统滴答计数
 * @details 通过启用TIM17更新中断来恢复系统滴答计数
 * @param 无
 * @return 无
 * @note 恢复后HAL_GetTick()返回值将继续递增
 */
void HAL_ResumeTick(void)
{
    // 启用TIM17更新中断
    LL_TIM_EnableIT_UPDATE(TIM17);
}

// ==================== 中断处理函数 ====================

/**
 * @brief TIM17全局中断处理函数
 * @details 处理TIM17更新中断，用于系统滴答计数
 * @param 无
 * @return 无
 * @note 此函数每1ms被调用一次，维护系统滴答计数器
 */
void TIM17_IRQHandler(void)
{
    // 检查更新中断是否待处理
    if (LL_TIM_IsActiveFlag_UPDATE(TIM17) && LL_TIM_IsEnabledIT_UPDATE(TIM17))
    {
        // 清除更新中断标志
        LL_TIM_ClearFlag_UPDATE(TIM17);

        // 调用HAL滴答函数
        HAL_IncTick();
    }
}