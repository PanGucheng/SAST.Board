#include "key_callbacks.h"
#include "sast_lib_st7735s.h"
#include "my_tim.h"
#include "rgb_led.h"
#include "main.h"
#include "keyboard.h"
#include "my_adc.h"
#include <stdio.h>

void Key1_Handler(void)
{
    st7735s_printf(0, 20, ST7735S_RED, ST7735S_BLACK, ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE, "Action: Key 1!   ");
}

void Key2_Handler(void)
{
    st7735s_printf(0, 20, ST7735S_GREEN, ST7735S_BLACK, ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE, "Action: Key 2!   ");
}

void Key3_Handler(void)
{
    st7735s_fill_rect(0, 0, 127, 159, ST7735S_BLACK);
    buzzer_beep(2, 200); // 响 2 声，每声周期 200ms（快响）
}

void Key4_Handler(void)
{
    st7735s_fill_rect(0, 0, 127, 159, ST7735S_BLACK);
    st7735s_printf(0, 20, ST7735S_BLUE, ST7735S_BLACK, ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE, "Action: Buzzer Test");

    // Test Sequence: Frequency Sweep
    for(int f = 500; f <= 2000; f += 500) {
        buzzer_set_freq(f);
        buzzer_set_duty(50);
        buzzer_update();

        // Update progress bar (0-100%)
        // f goes from 500 to 2000. Map to 0-100.
        // (f - 500) / 1500 * 100
        uint8_t progress = (f - 500) * 100 / 1500;
        st7735s_draw_progress_bar(0, 40, 128, 10, progress, ST7735S_GREEN, ST7735S_GRAY);

        HAL_Delay(200);
    }
    st7735s_draw_progress_bar(0, 40, 128, 10, 100, ST7735S_GREEN, ST7735S_GRAY); // Ensure 100% at end
    buzzer_set_duty(0); // Turn off
    buzzer_update();
}

void Key5_Handler(void)
{
    // Enable ADC waveform display
    st7735s_fill_rect(0, 0, 127, 159, ST7735S_BLACK);
    My_ADC_EnableDisplay(1);
    //st7735s_printf(0, 20, ST7735S_YELLOW, ST7735S_BLACK, ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE, "ADC: Display ON ");
}

void Key6_Handler(void)
{
    // Disable ADC waveform display
    My_ADC_EnableDisplay(0);
    st7735s_fill_rect(0, 0, 127, 159, ST7735S_BLACK);
    //st7735s_printf(0, 20, ST7735S_YELLOW, ST7735S_BLACK, ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE, "ADC: Display OFF");
}

void Key7_Handler(void) {}
void Key8_Handler(void) {}
void Key9_Handler(void) {}
void Key10_Handler(void) {}
void Key11_Handler(void) {}
void Key12_Handler(void) {}
void Key13_Handler(void) {}
void Key14_Handler(void) {}
void Key15_Handler(void) {}
void Key16_Handler(void) {}

void Key_Register_All_Callbacks(void)
{
    Key_Register_Callback(1, Key1_Handler);
    Key_Register_Callback(2, Key2_Handler);
    Key_Register_Callback(3, Key3_Handler);
    Key_Register_Callback(4, Key4_Handler);
    Key_Register_Callback(5, Key5_Handler);
    Key_Register_Callback(6, Key6_Handler);
    Key_Register_Callback(7, Key7_Handler);
    Key_Register_Callback(8, Key8_Handler);
    Key_Register_Callback(9, Key9_Handler);
    Key_Register_Callback(10, Key10_Handler);
    Key_Register_Callback(11, Key11_Handler);
    Key_Register_Callback(12, Key12_Handler);
    Key_Register_Callback(13, Key13_Handler);
    Key_Register_Callback(14, Key14_Handler);
    Key_Register_Callback(15, Key15_Handler);
    Key_Register_Callback(16, Key16_Handler);
}
