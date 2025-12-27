#include "my_tim.h"
#include "tim.h"
#include "sast_lib_st7735s.h"
#include <stdio.h>

// 默认频率 1kHz (ARR=1000-1, Prescaler=83) -> 84MHz / (83+1) / 1000 = 1000Hz
// 默认占空比 50% (Pulse=500)

uint32_t buzzer_freq = 1000;
uint32_t buzzer_duty = 0; // 0-100%

void buzzer_init(void)
{
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 0);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
}

void buzzer_set_freq(uint32_t freq)
{
    buzzer_freq = freq;
}

void buzzer_set_duty(uint32_t duty)
{
    buzzer_duty = duty;
}

void buzzer_update(void)
{
    // 计算 ARR
    // Freq = Clock / ((PSC + 1) * (ARR + 1))
    // ARR = (Clock / (PSC + 1) / Freq) - 1
    // Clock = 84MHz, PSC = 83
    // Clock / (PSC + 1) = 1MHz

    if (buzzer_freq == 0) buzzer_freq = 1;

    uint32_t arr = (1000000 / buzzer_freq) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim9, arr);

    uint32_t pulse = (arr + 1) * buzzer_duty / 100;
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, pulse);
}

static uint32_t beep_count = 0;
static uint32_t beep_timer = 0;
static uint32_t beep_half_period = 0;
static uint8_t beep_active = 0;

// Internal helper to set buzzer output state without changing global settings
static void buzzer_set_output(uint8_t on)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim9);
    uint32_t pulse = 0;

    if (on) {
        // Use current duty if non-zero, else 50%
        uint32_t duty = (buzzer_duty > 0) ? buzzer_duty : 50;
        pulse = (arr + 1) * duty / 100;
    } else {
        pulse = 0;
    }
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, pulse);
}

void buzzer_beep(int n, int period_ms)
{
    if (n <= 0) return;

    __disable_irq();
    beep_count = n * 2; // 2 phases per beep (ON, OFF)
    beep_half_period = period_ms / 2;
    beep_timer = 0;
    beep_active = 1;
    __enable_irq();

    buzzer_set_output(1); // Start with ON
}

void buzzer_tick(void)
{
    if (!beep_active) return;

    beep_timer += 10; // Assume 10ms tick

    if (beep_timer >= beep_half_period)
    {
        beep_timer = 0;
        beep_count--;

        if (beep_count > 0)
        {
            // Toggle state
            // If remaining count is odd (e.g. 2n-1), we just finished ON phase, so turn OFF.
            // If remaining count is even (e.g. 2n-2), we just finished OFF phase, so turn ON.
            buzzer_set_output((beep_count % 2) == 0);
        }
        else
        {
            buzzer_set_output(0); // Ensure OFF
            beep_active = 0;

            // Restore original state if needed?
            // buzzer_update() restores the continuous PWM based on buzzer_duty.
            // If we want the buzzer to go back to continuous mode if it was on, we should call buzzer_update().
            // But usually beep() is an event sound.
            // If the user had the buzzer ON continuously (buzzer_duty > 0), this beep function interrupts it.
            // After beeping, should it return to continuous ON?
            // Let's assume yes, if buzzer_duty > 0.
            buzzer_update();
        }
    }
}

void buzzer_control(int key)
{
    char str[30];
    int changed = 0;

    switch(key)
    {
        case 1: // 频率增加
            buzzer_freq += 100;
            if (buzzer_freq > 20000) buzzer_freq = 20000;
            changed = 1;
            break;
        case 2: // 频率减少
            if (buzzer_freq > 100) buzzer_freq -= 100;
            else buzzer_freq = 10;
            changed = 1;
            break;
        case 3: // 占空比增加
            if (buzzer_duty < 100) buzzer_duty += 5;
            changed = 1;
            break;
        case 4: // 占空比减少
            if (buzzer_duty > 0) buzzer_duty -= 5;
            changed = 1;
            break;
        case 5: // 开关蜂鸣器 (占空比置0或恢复)
             if (buzzer_duty > 0) buzzer_duty = 0;
             else buzzer_duty = 50;
             changed = 1;
             break;
    }

    if (changed)
    {
        buzzer_update();

        st7735s_fill_rect(0, 60, ST7735S_W - 1, 40, ST7735S_WHITE);
        sprintf(str, "Freq: %lu Hz", buzzer_freq);
        st7735s_draw_string(10, 60, str, ST7735S_BLACK, ST7735S_WHITE, ST7735S_SIZE_1608, ST7735S_OVERLAY_MODE);

        sprintf(str, "Duty: %lu %%", buzzer_duty);
        st7735s_draw_string(10, 80, str, ST7735S_BLACK, ST7735S_WHITE, ST7735S_SIZE_1608, ST7735S_OVERLAY_MODE);
    }
}
