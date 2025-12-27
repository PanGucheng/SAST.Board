#ifndef RGB_LED_H
#endif // RGB_LED_H

void RGB_Breathing_Tick(void);
// Update Breathing Effect (Call this in timer interrupt, e.g., every 10ms)

void RGB_Breathing_Stop(void);
// Stop Breathing Effect

void RGB_Breathing_Start(uint8_t r, uint8_t g, uint8_t b, uint16_t period_ms);
// period_ms: Breathing cycle period in milliseconds
// r, g, b: Target color (max brightness)
// Start Breathing Effect

void RGB_Set_Color(uint8_t r, uint8_t g, uint8_t b);
// Set RGB Color (0-255 for each channel)

void RGB_Init(void);
// Initialize RGB LED (Start PWM)

#include "../Inc/main.h"

#define RGB_LED_H

