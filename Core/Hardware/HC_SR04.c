/*
 *  HC_SR04.c
 *  Created on: Dec 4, 2025
 *  By: PanGucheng
 */

#include "HC_SR04.h"

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_rcc.h"
#include "tim.h"
#include "usart.h"

#define HC_SR04_TIM htim4
#define HC_SR04_Trig_PIN GPIO_PIN_9
#define HC_SR04_Trig_GPIO_Port GPIOB

static float HC_SR04_GetTimerTickUs(void);

uint16_t rising_cnt = 0;
uint16_t falling_cnt = 0;
uint8_t echo_flag = 0; // 0: 等待捕获上升沿 1: 上升沿捕获完成


/*
 * 向 HC-SR04 发送触发脉冲，并在捕获到完整的回波脉冲后
 * 依据定时器实际计数周期计算声波往返时间与距离。
 */
void HC_SR04_Start(void)
{
    // 触发
    HAL_GPIO_WritePin(HC_SR04_Trig_GPIO_Port, HC_SR04_Trig_PIN, GPIO_PIN_SET);
    DelayUs(20);
    HAL_GPIO_WritePin(HC_SR04_Trig_GPIO_Port, HC_SR04_Trig_PIN, GPIO_PIN_RESET);
    __HAL_TIM_SET_COUNTER(&HC_SR04_TIM, 0);
    // 开始捕获
    __HAL_TIM_SET_CAPTUREPOLARITY(&HC_SR04_TIM, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&HC_SR04_TIM, TIM_CHANNEL_3);

    if (rising_cnt != 0 && falling_cnt != 0)
    {
        float tick_us = HC_SR04_GetTimerTickUs();
        if (tick_us <= 0.0f)
        {
            return;
        }
        // 捕获值差值=回波脉宽的定时器 tick 数 → 转换为微秒后乘声速并折半得到单程距离
        uint16_t pulse_ticks = (falling_cnt >= rising_cnt)
                                   ? (falling_cnt - rising_cnt)
                                   : (uint16_t)((0x10000U - rising_cnt) + falling_cnt);
        float pulse_us = pulse_ticks * tick_us;
        float distance = (pulse_us * 0.034f) * 0.5f;

        // 发送到串口
        char buf[32];
        sprintf(buf, "Distance: %.2f cm\r\n", distance);
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf),1000);

        // 清零
        rising_cnt = 0;
        falling_cnt = 0;
        echo_flag = 0;
        // 停止捕获
        HAL_TIM_IC_Stop_IT(&HC_SR04_TIM, TIM_CHANNEL_3);
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &HC_SR04_TIM && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
        // 捕获到上升沿
        if (!echo_flag)
        {
            rising_cnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
            echo_flag = 1;
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
        }
        // 捕获到下降沿
        else
        {
            falling_cnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
            echo_flag = 0;
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
        }
    }

}

#define DWT_LAR_UNLOCK_VALUE 0xC5ACCE55U

static inline uint32_t get_dwt_cycle_count(void)
{
    return DWT->CYCCNT;
}

/*
 * 利用 DWT 的 CYCCNT 计数器实现忙等待延时：
 * 1. 首次调用时打开跟踪单元、解锁 DWT 并启动 CYCCNT；
 * 2. 根据当前 SystemCoreClock 计算目标 tick；
 * 3. 轮询 CYCCNT，处理可能的 32 位回绕，直到累计 tick 达标。
 */
void DelayUs(uint32_t microseconds)
{
    static uint8_t dwt_initialized = 0;
    const uint32_t cycles_per_us = SystemCoreClock / 1000000U;

    if (!dwt_initialized)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U)
        {
            return;
        }

        if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
        {
#if defined(DWT_LAR)
            DWT->LAR = DWT_LAR_UNLOCK_VALUE;
#endif
            DWT->CYCCNT = 0U;
            DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        }
        dwt_initialized = 1U;
    }

    const uint32_t start = get_dwt_cycle_count();
    const uint32_t target_ticks = microseconds * cycles_per_us;
    const uint32_t end = start + target_ticks;

    if (target_ticks == 0U)
    {
        return;
    }

    if (end >= start)
    {
        while (get_dwt_cycle_count() < end)
        {
        }
    }
    else
    {
        while (get_dwt_cycle_count() >= start)
        {
        }
        while (get_dwt_cycle_count() < end)
        {
        }
    }
}

static float HC_SR04_GetTimerTickUs(void)
{
    // 保存系统当前时钟配置，以便判断 APB1 分频
    RCC_ClkInitTypeDef clk_config;
    // 接收 HAL 调用返回的 Flash 等待周期值（此处不用）
    uint32_t flash_latency = 0U;
    // 读取系统时钟树配置，填充 clk_config（返回值为 void）
    HAL_RCC_GetClockConfig(&clk_config, &flash_latency);

    // 获取 APB1 外设时钟频率，TIM4 位于该总线
    uint32_t tim_clk = HAL_RCC_GetPCLK1Freq();
    // 当 APB1 分频不为 1 时，定时器时钟需额外乘 2
    if (clk_config.APB1CLKDivider != RCC_HCLK_DIV1)
    {
        // 对应 STM32 定时器倍频规则
        tim_clk *= 2U;
    }

    // 防止后续做除法时出现 0 频率
    if (tim_clk == 0U)
    {
        // 返回 0 表示计算失败
        return 0.0f;
    }

    // (PSC+1)/tim_clk → 单个计数周期，乘 1e6 转换为微秒
    return ((HC_SR04_TIM.Init.Prescaler + 1U) * 1000000.0f) / tim_clk;
}
