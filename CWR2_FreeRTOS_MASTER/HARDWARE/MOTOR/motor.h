#ifndef _MOTOR_H
#define _MOTOR_H
#include "sys.h"
#include "stm32f4xx_conf.h"
//////////////////////////////////////////////////////////////////////////////////
// Copyright(C) IRIM 2018
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////
#define moveup 2
#define movedown 1
#define stop 0
#define ModeAuto 1
#define ModeManual 0

extern const u32 PWMFreq;  // PWM Freq

void Motor_Init(void);
void Motor_IO_Init(void);
void MoveUp(void);
void MoveDown(void);
void MotorCrl(u32 direction, u32 speed);

void TIM1_Config(void);
void TIM1_PWM_Init(u32 freq);
void TIM1_PWM_SET(u32 freq, u32 Duty);

void TIM12_CH1_Cap_Init(u16 arr, u16 psc);

#endif
