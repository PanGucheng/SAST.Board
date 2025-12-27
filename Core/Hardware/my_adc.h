#ifndef MY_ADC_H
#define MY_ADC_H

#include "../Inc/main.h"
#include "../Inc/adc.h"

extern uint32_t ADC_Value[2048];
extern uint16_t max_adc;
extern uint16_t min_adc;
extern float ad1;
extern float vpp;
extern float period;
extern uint8_t adc_dma_ok;
extern float last_av;

void My_ADC_Init(void);
void My_ADC_EnableDisplay(uint8_t enable);
uint8_t My_ADC_DisplayEnabled(void);
void showADC(void);
void adc_show(void);
void My_ADC_TIM10_Callback(void);
void My_ADC_Process(void);

#endif // MY_ADC_H
