#include "stdint.h"
#include "rgb_led.h"
#include "tim.h"
#include <math.h>

// TIM2 Period is 10000
#define PWM_PERIOD 10000

static uint8_t target_r = 0;
static uint8_t target_g = 0;
static uint8_t target_b = 0;

static uint8_t breathing_active = 0;
static uint16_t breathing_period = 2000; // Default 2000ms
static uint32_t breathing_tick_cnt = 0;

void RGB_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    RGB_Set_Color(0, 0, 0);
}

void RGB_Set_Color(uint8_t r, uint8_t g, uint8_t b)
{
    // Map 0-255 to 0-10000
    uint32_t pulse_r = (uint32_t)r * PWM_PERIOD / 255;
    uint32_t pulse_g = (uint32_t)g * PWM_PERIOD / 255;
    uint32_t pulse_b = (uint32_t)b * PWM_PERIOD / 255;

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse_r);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse_g);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse_b);
}

void RGB_Breathing_Start(uint8_t r, uint8_t g, uint8_t b, uint16_t period_ms)
{
    target_r = r;
    target_g = g;
    target_b = b;
    breathing_period = period_ms;
    breathing_tick_cnt = 0;
    breathing_active = 1;
}

void RGB_Breathing_Stop(void)
{
    breathing_active = 0;
    RGB_Set_Color(0, 0, 0); // Turn off or keep last state? Usually turn off.
}

void RGB_Breathing_Tick(void)
{
    if (!breathing_active) return;

    // Assume Tick is called every 10ms
    breathing_tick_cnt += 10;

    // Calculate brightness factor (0.0 to 1.0) using a triangle wave or sine wave
    // Using Triangle wave for simplicity: 0 -> 1 -> 0

    float phase = (float)(breathing_tick_cnt % breathing_period) / breathing_period;
    float brightness;

    if (phase < 0.5f) {
        brightness = phase * 2.0f; // 0.0 to 1.0
    } else {
        brightness = 2.0f - (phase * 2.0f); // 1.0 to 0.0
    }

    // Apply brightness to target color
    uint8_t r = (uint8_t)(target_r * brightness);
    uint8_t g = (uint8_t)(target_g * brightness);
    uint8_t b = (uint8_t)(target_b * brightness);

    RGB_Set_Color(r, g, b);
}

