#include "countdown.h"
#include "sast_lib_st7735s.h"
#include <stdio.h>

volatile uint32_t countdown_seconds = 300; // 默认5分钟
volatile uint8_t countdown_running = 0;
volatile uint32_t tick_counter = 0;

void countdown_init(void)
{
    countdown_display();
}

void countdown_tick(void)
{
    if (countdown_running && countdown_seconds > 0)
    {
        tick_counter++;
        if (tick_counter >= 1000) // 1秒
        {
            tick_counter = 0;
            countdown_seconds--;
            countdown_display();

            if (countdown_seconds == 0)
            {
                countdown_running = 0;
                // 倒计时结束，可以在这里添加蜂鸣器提示等
            }
        }
    }
}

void countdown_display(void)
{
    char str[20];
    uint32_t min = countdown_seconds / 60;
    uint32_t sec = countdown_seconds % 60;

    sprintf(str, "Time: %02lu:%02lu", min, sec);
    st7735s_draw_string(10, 100, str, ST7735S_BLACK, ST7735S_RED, ST7735S_SIZE_2412, ST7735S_OVERLAY_MODE);
}

void countdown_control(int key)
{
    switch(key)
    {
        case 9: // 增加1分钟
            if (!countdown_running)
            {
                if (countdown_seconds < 600) // 上限10分钟
                    countdown_seconds += 60;
                countdown_display();
            }
            break;
        case 10: // 减少1分钟
            if (!countdown_running)
            {
                if (countdown_seconds >= 360) // 下限5分钟 (300s) + 60s buffer for decrement logic check
                     countdown_seconds -= 60;
                else if (countdown_seconds > 300)
                     countdown_seconds = 300;

                countdown_display();
            }
            break;
        case 11: // 开始/暂停
            countdown_running = !countdown_running;
            break;
        case 12: // 重置
            countdown_running = 0;
            countdown_seconds = 300;
            countdown_display();
            break;
    }
}

