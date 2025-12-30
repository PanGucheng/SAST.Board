#include "keyboard.h"
#include "gpio.h"
#include "main.h" 

static int key_event = 0;
static int current_stable_key = 0;
static int last_sample_key = 0;
static int debounce_cnt = 0;

// Simple delay for signal stabilization
static void short_delay(void) {
    for(volatile int i = 0; i < 100; i++);
}

// Non-blocking scan function, to be called in timer interrupt (e.g., every 10ms)
void Key_Scan_Tick(void)
{
    int raw_key = 0;
    
    // Column and Row configuration tables for cleaner code
    static const struct {
        GPIO_TypeDef *port;
        uint16_t pin;
    } columns[] = {
        {LINE1_GPIO_Port, LINE1_Pin},
        {LINE2_GPIO_Port, LINE2_Pin},
        {LINE3_GPIO_Port, LINE3_Pin},
        {LINE4_GPIO_Port, LINE4_Pin}
    };
    
    static const struct {
        GPIO_TypeDef *port;
        uint16_t pin;
    } rows[] = {
        {ROW1_GPIO_Port, ROW1_Pin},
        {ROW2_GPIO_Port, ROW2_Pin},
        {ROW3_GPIO_Port, ROW3_Pin},
        {ROW4_GPIO_Port, ROW4_Pin}
    };
    
    // 扫描列
    for (int col = 0; col < 4 && raw_key == 0; col++) {
        // 设置 HIGH/LOW
        for (int i = 0; i < 4; i++) {
            HAL_GPIO_WritePin(columns[i].port, columns[i].pin, 
                            (i == col) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        
        short_delay(); // 短暂延时
        
        // 扫描行
        for (int row = 0; row < 4; row++) {
            if (HAL_GPIO_ReadPin(rows[row].port, rows[row].pin)) {
                raw_key = col * 4 + row + 1;
                break;
            }
        }
    }
    
    // 全部重置为low
    for (int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(columns[i].port, columns[i].pin, GPIO_PIN_RESET);
    }
    
    // Debounce Logic
    if (raw_key == last_sample_key) {
        if (debounce_cnt < 5) { 
            debounce_cnt++;
        }
        
        if (debounce_cnt >= 5) {
            if (raw_key != current_stable_key) {
                if (raw_key != 0) {
                    key_event = raw_key;
                }
                current_stable_key = raw_key;
            }
        }
    } else {
        debounce_cnt = 0;
        last_sample_key = raw_key;
    }
}

int Key_Get_Event(void)
{
    int k = key_event;
    key_event = 0;
    return k;
}

static KeyCallback_t Key_Callbacks[17] = {0};

void Key_Register_Callback(int key, KeyCallback_t callback)
{
    if (key >= 1 && key <= 16)
    {
        Key_Callbacks[key] = callback;
    }
}

void Key_Execute_Callback(int key)
{
    if (key >= 1 && key <= 16 && Key_Callbacks[key] != 0)
    {
        Key_Callbacks[key]();
    }
}

// Legacy function kept for compatibility if needed, but Key_Scan_Tick is preferred
void key_scan(void)
{
    // ... (Original blocking implementation if needed, or redirect to new logic)
}
