#include "timer.h"
//////////////////////////////////////////////////////////////////////////////////

//通用定时器3中断初始化
// arr:自动重装值。 psc：时钟预分频数
//定时器溢出计算方法:Tout=((arr+1)*(psc+1))/Ft us.
// Ft=定时器工作频率,单位:Mhz
// TIMER3!
void TIM3_Int_Init(u16 arr, u16 psc) {
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  /// Enable TIMER3!

  TIM_TimeBaseInitStructure.TIM_Period = arr;     //
  TIM_TimeBaseInitStructure.TIM_Prescaler = psc;  //
  TIM_TimeBaseInitStructure.TIM_CounterMode =
      TIM_CounterMode_Up;  // Upcount mode.
  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);  // Initialize TIMER3!

  TIM_ITConfig(TIM3, TIM_IT_Update,
               ENABLE);   // Enable Timer3 to update the interrupt.
  TIM_Cmd(TIM3, ENABLE);  //Ê¹ÄÜ¶¨Ê±Æ÷3

  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  // The interrupt of TIMER3!
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //抢占优先级1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;  //响应优先级3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  // Initialize NVIC.
}

// TIMER3 中断服务函数
void TIM3_IRQHandler(void) {
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)  //溢出中断
  {
    LED1 = !LED1;
  }
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除中断标志位
}

