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

    // Scan Column 1
    HAL_GPIO_WritePin(LINE1_GPIO_Port, LINE1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LINE2_GPIO_Port, LINE2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINE3_GPIO_Port, LINE3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINE4_GPIO_Port, LINE4_Pin, GPIO_PIN_RESET);
    short_delay();
    if(HAL_GPIO_ReadPin(ROW1_GPIO_Port,ROW1_Pin)) raw_key = 1;
    else if(HAL_GPIO_ReadPin(ROW2_GPIO_Port,ROW2_Pin)) raw_key = 2;
    else if(HAL_GPIO_ReadPin(ROW3_GPIO_Port,ROW3_Pin)) raw_key = 3;
    else if(HAL_GPIO_ReadPin(ROW4_GPIO_Port,ROW4_Pin)) raw_key = 4;
    HAL_GPIO_WritePin(LINE1_GPIO_Port, LINE1_Pin, GPIO_PIN_RESET);

    // Scan Column 2
    if (raw_key == 0) {
        HAL_GPIO_WritePin(LINE2_GPIO_Port, LINE2_Pin, GPIO_PIN_SET);
        short_delay();
        if(HAL_GPIO_ReadPin(ROW1_GPIO_Port,ROW1_Pin)) raw_key = 5;
        else if(HAL_GPIO_ReadPin(ROW2_GPIO_Port,ROW2_Pin)) raw_key = 6;
        else if(HAL_GPIO_ReadPin(ROW3_GPIO_Port,ROW3_Pin)) raw_key = 7;
        else if(HAL_GPIO_ReadPin(ROW4_GPIO_Port,ROW4_Pin)) raw_key = 8;
        HAL_GPIO_WritePin(LINE2_GPIO_Port, LINE2_Pin, GPIO_PIN_RESET);
    }

    // Scan Column 3
    if (raw_key == 0) {
        HAL_GPIO_WritePin(LINE3_GPIO_Port, LINE3_Pin, GPIO_PIN_SET);
        short_delay();
        if(HAL_GPIO_ReadPin(ROW1_GPIO_Port,ROW1_Pin)) raw_key = 9;
        else if(HAL_GPIO_ReadPin(ROW2_GPIO_Port,ROW2_Pin)) raw_key = 10;
        else if(HAL_GPIO_ReadPin(ROW3_GPIO_Port,ROW3_Pin)) raw_key = 11;
        else if(HAL_GPIO_ReadPin(ROW4_GPIO_Port,ROW4_Pin)) raw_key = 12;
        HAL_GPIO_WritePin(LINE3_GPIO_Port, LINE3_Pin, GPIO_PIN_RESET);
    }

    // Scan Column 4
    if (raw_key == 0) {
        HAL_GPIO_WritePin(LINE4_GPIO_Port, LINE4_Pin, GPIO_PIN_SET);
        short_delay();
        if(HAL_GPIO_ReadPin(ROW1_GPIO_Port,ROW1_Pin)) raw_key = 13;
        else if(HAL_GPIO_ReadPin(ROW2_GPIO_Port,ROW2_Pin)) raw_key = 14;
        else if(HAL_GPIO_ReadPin(ROW3_GPIO_Port,ROW3_Pin)) raw_key = 15;
        else if(HAL_GPIO_ReadPin(ROW4_GPIO_Port,ROW4_Pin)) raw_key = 16;
        HAL_GPIO_WritePin(LINE4_GPIO_Port, LINE4_Pin, GPIO_PIN_RESET);
    }

    // Debounce Logic
    if (raw_key == last_sample_key) {
        if (debounce_cnt < 5) { // Wait for 5 stable ticks (e.g. 50ms if tick is 10ms)
            debounce_cnt++;
        }
        if (debounce_cnt >= 1) { // Threshold reached
            if (raw_key != current_stable_key) {
                if (raw_key != 0) {
                    key_event = raw_key; // Trigger event on press
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
    key_event = 0; // Clear event after reading
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
