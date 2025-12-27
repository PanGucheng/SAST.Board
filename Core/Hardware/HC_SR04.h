/*
*  HC_SR04.c
 *  Created on: Dec 4, 2025
 *  By: PanGucheng
 */
#ifndef HC_SR04_HC_SR04_H
#define HC_SR04_HC_SR04_H

#include <stdint.h>

/**
 * @brief Delay for a specified number of microseconds.
 *
 * This function provides a delay in the range of microseconds, utilizing
 * the internal clock of the microcontroller. It is used for timing
 * requirements of the HC-SR04 ultrasonic sensor.
 *
 * @param microseconds Number of microseconds to delay.
 */
void DelayUs(uint32_t microseconds);

/**
 * @brief 触发并开始一次 HC-SR04 超声波测距过程。
 *
 * 该函数通过对触发引脚（TRIG）输出一个约 20 微秒的高脉冲来启动测距，
 * 随后传感器在回响引脚（ECHO）上输出高电平的持续时间，表示超声波往返时间。
 * 通常配合定时器/中断或输入捕获模块在 ECHO 高电平期间进行计时，
 * 以计算出目标到传感器的距离。
 *
 * 使用说明：
 * - 在调用本函数前，请确保 TRIG/ECHO 引脚以及相关定时器/中断已正确初始化。
 * - 如果在同一测量周期内需要重复触发，请保证两次触发之间有足够的间隔（例如 >60ms），
 *   以避免回波相互干扰。
 */
void HC_SR04_Start(void);

#endif //HC_SR04_HC_SR04_H