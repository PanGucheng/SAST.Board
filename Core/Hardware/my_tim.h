#ifndef MY_TIM_H
#define MY_TIM_H

#include <stdint.h>

void buzzer_init(void);
void buzzer_update(void);
void buzzer_control(int key);

// n: number of beeps
// period_ms: duration of one beep cycle (on + off) in ms
void buzzer_beep(int n, int period_ms);
void buzzer_tick(void);

void buzzer_set_freq(uint32_t freq);
void buzzer_set_duty(uint32_t duty);

#endif // MY_TIM_H
