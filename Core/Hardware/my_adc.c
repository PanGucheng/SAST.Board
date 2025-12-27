#include "my_adc.h"

#include <stdio.h>

#include "sast_lib_st7735s.h"
#include "adc.h"
#include <string.h>

uint32_t ADC_Value[2048];
static uint32_t ADC_Buffer[2048]; // Double buffer for ISR
uint16_t max_adc;
uint16_t min_adc;
float ad1;
float vpp;
float period;
uint8_t adc_dma_ok;
float last_av;
int detect_points;

static uint16_t adc_index = 0;
static uint8_t adc_sampling_done = 0;
static uint8_t adc_display_enabled = 0;

void My_ADC_Init(void)
{
    __HAL_ADC_ENABLE(&hadc1);
    HAL_Delay(1);
    adc_index = 0;
    adc_sampling_done = 0;
    adc_display_enabled = 0;
}

void My_ADC_EnableDisplay(uint8_t enable)
{
    adc_display_enabled = enable ? 1 : 0;
}

uint8_t My_ADC_DisplayEnabled(void)
{
    return adc_display_enabled;
}

void My_ADC_TIM10_Callback(void)
{
    // Start Conversion
    hadc1.Instance->CR2 |= ADC_CR2_SWSTART;

    // Wait for EOC with timeout
    int timeout = 500;
    while(!(hadc1.Instance->SR & ADC_SR_EOC) && --timeout);

    if (timeout > 0 && adc_index < 2048) {
        ADC_Buffer[adc_index++] = hadc1.Instance->DR;
    }

    if (adc_index >= 2048)
    {
        adc_index = 0;
        adc_sampling_done = 1;
    }
}

void My_ADC_Process(void)
{
    if (!adc_sampling_done) return;

    memcpy(ADC_Value, ADC_Buffer, sizeof(ADC_Buffer));
    adc_sampling_done = 0; // ready for next buffer

    // If display is off, skip heavy processing; still keep latest buffer copied
    if (!adc_display_enabled) {
        adc_dma_ok = 0;
        return;
    }

    ad1 = 0;
    max_adc = 0;
    min_adc = 9999;
    detect_points = 0;
    int detected[10] = {0};
    period = 0;

    for(int i = 50; i < 2000; i++)
    {
        ad1 += ADC_Value[i];
        if(ADC_Value[i] > max_adc) max_adc = ADC_Value[i];
        if(ADC_Value[i] < min_adc) min_adc = ADC_Value[i];
        if(ADC_Value[i] > last_av && ADC_Value[i-1] <= last_av)
        {
            if(detect_points < 2)
            {
                detected[detect_points] = i;
                detect_points++;
            }
        }
    }

    if(detect_points >= 2)
    {
        period = (detected[1] - detected[0]) / 100.0f; // samples -> ms
    }
    else
    {
        period = 0;
    }

    ad1 /= 1950.0f;
    last_av = ad1;
    ad1 *= 3.3f/4096.0f;
    vpp = (float)(max_adc - min_adc);
    vpp *= 3.3f/4096.0f;

    adc_dma_ok = 1; // signal main loop to draw
}

static uint8_t adc_waveform_val[160];
static uint8_t is_first_draw = 1;

void showADC(void)
{
    // Initialize on first run
    if (is_first_draw) {
        st7735s_fill_rect(0, 0, 128, 160, ST7735S_BLACK);
        for(int i=0; i<160; i++) adc_waveform_val[i] = 64; // Default center (128/2)
        is_first_draw = 0;
    }

    if (!adc_display_enabled) {
        st7735s_fill_rect(0, 0, 128, 20, ST7735S_BLACK);
        return;
    }

    int trigger_offset = 0;
    uint32_t threshold = (uint32_t)last_av;

    for(int i = 1; i < 1000; i++) {
        if(ADC_Value[i] >= threshold && ADC_Value[i-1] < threshold) {
            trigger_offset = i;
            break;
        }
    }

    int stride = 1;
    int zoom = 1;
    float samples_per_2_periods = 2.0f * period * 100.0f;

    if (samples_per_2_periods > 160) {
        stride = (int)(samples_per_2_periods / 160.0f);
        if (stride < 1) stride = 1;
        if (stride > 10) stride = 10;
        zoom = 1;
    } else if (samples_per_2_periods > 0) {
        zoom = (int)(160.0f / samples_per_2_periods);
        if (zoom < 1) zoom = 1;
        if (zoom > 8) zoom = 8;
        stride = 1;
    }

    int idx0 = trigger_offset;
    if (idx0 >= 2048) idx0 = 2047;
    int x_prev_new = (int)(ADC_Value[idx0] * 128 / 4096);
    if(x_prev_new < 0) x_prev_new = 0; if(x_prev_new > 127) x_prev_new = 127;

    for(int i = 1; i < 160; i++) {
        int idx;
        if (zoom > 1) {
            idx = trigger_offset + (i / zoom);
        } else {
            idx = trigger_offset + i * stride;
        }

        if (idx >= 2048) idx = 2047;

        int x_curr_new = (int)(ADC_Value[idx] * 128 / 4096);
        if(x_curr_new < 0) x_curr_new = 0; if(x_curr_new > 127) x_curr_new = 127;

        st7735s_draw_line(adc_waveform_val[i-1], i-1, adc_waveform_val[i], i, ST7735S_BLACK);
        st7735s_draw_line(x_prev_new, i-1, x_curr_new, i, ST7735S_WHITE);

        adc_waveform_val[i-1] = x_prev_new;
        x_prev_new = x_curr_new;
    }
    adc_waveform_val[159] = x_prev_new;

    st7735s_draw_line(64, 0, 64, 159, ST7735S_GRAY);
    st7735s_draw_line(0, 80, 127, 80, ST7735S_GRAY);

    for(int x = 0; x < 128; x += 16) {
        st7735s_draw_line(x, 78, x, 82, ST7735S_GRAY);
    }

    for(int y = 0; y < 160; y += 16) {
        st7735s_draw_line(62, y, 66, y, ST7735S_GRAY);
    }

    char str[32];
    float freq = 0;
    if (period > 0) freq = 1000.0f / period;

    st7735s_fill_rect(0, 0, 128, 20, ST7735S_BLACK);

    sprintf(str, "F:%.1fHz", freq);
    st7735s_draw_string(0, 0, str, ST7735S_YELLOW, ST7735S_BLACK, ST7735S_SIZE_1206, ST7735S_OVERLAY_MODE);

    float total_time_ms;
    if (zoom > 1) {
        total_time_ms = 160.0f * 0.01f / zoom;
    } else {
        total_time_ms = 160.0f * stride * 0.01f;
    }
    sprintf(str, "T:%.2fms", total_time_ms);
    st7735s_draw_string(64, 0, str, ST7735S_CYAN, ST7735S_BLACK, ST7735S_SIZE_1206, ST7735S_OVERLAY_MODE);
}

void adc_show(void)
{
    static uint32_t last_adc_show_time = 0;

    if (adc_dma_ok) {
        // Limit refresh rate to every 100ms (10 FPS) to reduce screen flickering and CPU load
        if (HAL_GetTick() - last_adc_show_time > 100)
        {
            showADC();
            last_adc_show_time = HAL_GetTick();
        }
        adc_dma_ok = 0; // 数据处理完毕，等待下一次采样
    }
}