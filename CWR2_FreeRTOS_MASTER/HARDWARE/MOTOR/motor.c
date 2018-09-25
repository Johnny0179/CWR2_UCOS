#include "motor.h"

const u32 PWMFreq = 20000;  // PWM Freq

/*Motor IO Init*/
void Motor_IO_Init(void) {
  /*IO Init*/
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  //使能GPIOD时钟

  GPIO_InitStructure.GPIO_Pin =
      (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);  //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;             //普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;            //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;        // 100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;              //上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);                    //初始化GPIO

  GPIO_SetBits(GPIOD, GPIO_Pin_2 | GPIO_Pin_3);
  GPIO_ResetBits(GPIOD, GPIO_Pin_0 | GPIO_Pin_1);
}

/*Init TIM1 in PWM mode*/
/*Frequency in Hz*/
void TIM1_PWM_Init(u32 freq) {
  /* Private typedef
   * -----------------------------------------------------------*/
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  // TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

  /* Private variables
   * ---------------------------------------------------------*/
  int TimerPeriod = 0;
  /* TIM1 Configuration */
  TIM1_Config();

  /* Compute the value to be set in ARR register to generate the desired signal
   * frequency */
  TimerPeriod = ((168000000 / 1) / freq) - 1;  // SystemCoreClock is 168MHz.

  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Channel 1~4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;

  /*50%*/
  TIM_OCInitStructure.TIM_Pulse = (TimerPeriod * 50 / 100);

  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

  TIM_OC2Init(TIM1, &TIM_OCInitStructure);

  TIM_OC3Init(TIM1, &TIM_OCInitStructure);

  TIM_OC4Init(TIM1, &TIM_OCInitStructure);

  /*  TIM_OCInitStructure.TIM_Pulse = (TimerPeriod/6) ;
    TIM_OC2Init(TIM8, &TIM_OCInitStructure);*/

  /* Automatic Output enable, Break, dead time and lock configuration*/
  /*TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
  TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
  TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;
  TIM_BDTRInitStructure.TIM_DeadTime = 25; ///////// the right value for 250ns
  delay //////// TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;
  TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
  TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;*/

  // TIM_BDTRConfig(TIM8, &TIM_BDTRInitStructure);

  /* TIM1 counter enable */
  TIM_Cmd(TIM1, ENABLE);

  /* Main Output Enable */
  TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/**
 *  Configure the TIM1 Pins.
 */
void TIM1_Config(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOE, GPIOB and GPIOC clocks enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  /* TIM1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  /*GPIOE Configuration: Channel 1N and BKIN as alternate function push-pull*/
  GPIO_InitStructure.GPIO_Pin =
      GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Connect TIM pins to AF1 */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);
}

void TIM1_PWM_SET(u32 freq, u32 Duty) {
  TIM1->CCR1 = (((168000000 / 1) / freq) - 1) * Duty / 100;
  TIM1->CCR2 = (((168000000 / 1) / freq) - 1) * Duty / 100;
  TIM1->CCR3 = (((168000000 / 1) / freq) - 1) * Duty / 100;
  TIM1->CCR4 = (((168000000 / 1) / freq) - 1) * Duty / 100;
}

void MoveUp(void) {
  GPIO_SetBits(GPIOD, GPIO_Pin_2 | GPIO_Pin_3);
  GPIO_ResetBits(GPIOD, GPIO_Pin_0 | GPIO_Pin_1);
}

void MoveDown(void) {
  GPIO_ResetBits(GPIOD, GPIO_Pin_2 | GPIO_Pin_3);
  GPIO_SetBits(GPIOD, GPIO_Pin_0 | GPIO_Pin_1);
}

void MotorCrl(u32 direction, u32 speed) {
  switch (direction) {
    case moveup:
      MoveUp();
      break;
      // TIM1_PWM_SET(20000, (100 - speed));
    case movedown:
      MoveDown();
      break;
      // TIM1_PWM_SET(20000, (100 - speed));
    case stop:;
    default:;
  }
  TIM1_PWM_SET(20000, (100 - speed));
}

/*Input Capture*/

// TIM12通道1输入捕获配置
// arr：自动重装值(TIM2,TIM12是32位的!!)
// psc：时钟预分频数
void TIM12_CH1_Cap_Init(u16 arr, u16 psc) {
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_ICInitTypeDef TIM12_ICInitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);  // TIM12时钟使能
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  //使能PORTB时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;          // GPIOB14
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用功能
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //速度100MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;      //下拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);              //初始化PB14

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14,
                   GPIO_AF_TIM12);  // PB14复用位定时器12

  TIM_TimeBaseStructure.TIM_Prescaler = psc;                   //定时器分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数模式
  TIM_TimeBaseStructure.TIM_Period = arr;  //自动重装载值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

  //初始化TIM12输入捕获参数
  TIM12_ICInitStructure.TIM_Channel =
      TIM_Channel_1;  // CC1S=01  选择输入端 IC1映射到TI1上
  TIM12_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;  //上升沿捕获
  TIM12_ICInitStructure.TIM_ICSelection =
      TIM_ICSelection_DirectTI;                            //映射到TI1上
  TIM12_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;  //配置输入分频,不分频
  TIM12_ICInitStructure.TIM_ICFilter = 0x00;  // IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM12, &TIM12_ICInitStructure);

  TIM_ITConfig(TIM12, TIM_IT_Update | TIM_IT_CC1,
               ENABLE);  //允许更新中断 ,允许CC1IE捕获中断

  TIM_Cmd(TIM12, ENABLE);  //使能TIM12

  NVIC_InitStructure.NVIC_IRQChannel = TIM8_BRK_TIM12_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //抢占优先级3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //子优先级3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            // IRQ通道使能
  NVIC_Init(&NVIC_InitStructure);  //根据指定的参数初始化NVIC寄存器、
}

//捕获状态
//[7]:0,没有成功的捕获;1,成功捕获到一次.
//[6]:0,还没捕获到低电平;1,已经捕获到低电平了.
//[5:0]:捕获低电平后溢出的次数(对于32位定时器来说,1us计数器加1,溢出时间:4294秒)
u8 TIM12CH1_CAPTURE_STA = 0;  //输入捕获状态
u16 TIM12CH1_CAPTURE_VAL;     //输入捕获值(TIM12是16位)
// TIM12中断服务程序
void TIM12_IRQHandler(void) {
  if ((TIM12CH1_CAPTURE_STA & 0X80) == 0)  //还未成功捕获
  {
    if (TIM_GetITStatus(TIM12, TIM_IT_Update) != RESET)  //溢出
    {
      if (TIM12CH1_CAPTURE_STA & 0X40)  //已经捕获到高电平了
      {
        if ((TIM12CH1_CAPTURE_STA & 0X3F) == 0X3F)  //高电平太长了
        {
          TIM12CH1_CAPTURE_STA |= 0X80;  //标记成功捕获了一次
          TIM12CH1_CAPTURE_VAL = 0XFFFF;
        } else
          TIM12CH1_CAPTURE_STA++;
      }
    }
    if (TIM_GetITStatus(TIM12, TIM_IT_CC1) != RESET)  //捕获1发生捕获事件
    {
      if (TIM12CH1_CAPTURE_STA & 0X40)  //捕获到一个下降沿
      {
        TIM12CH1_CAPTURE_STA |= 0X80;  //标记成功捕获到一次高电平脉宽
        TIM12CH1_CAPTURE_VAL = TIM_GetCapture1(TIM12);  //获取当前的捕获值.
        /*TIM_OC1PolarityConfig(
            TIM12, TIM_ICPolarity_Rising);  // CC1P=0 设置为上升沿捕获*/
      } else  //还未开始,第一次捕获上升沿
      {
        TIM12CH1_CAPTURE_STA = 0;  //清空
        TIM12CH1_CAPTURE_VAL = 0;
        TIM12CH1_CAPTURE_STA |= 0X40;  //标记捕获到了上升沿
        /*TIM_Cmd(TIM12, DISABLE);       //关闭TIM12
        TIM_SetCounter(TIM12, 0);
        TIM_OC1PolarityConfig(
            TIM12, TIM_ICPolarity_Falling);  // CC1P=1 设置为下降沿捕获
        TIM_Cmd(TIM12, ENABLE);              //使能TIM12*/
      }
    }
  }
  TIM_ClearITPendingBit(TIM12, TIM_IT_CC1 | TIM_IT_Update);  //清除中断标志位
}



/*Motor Init*/
void Motor_Init(void){
  
  TIM1_PWM_Init(PWMFreq);

  Motor_IO_Init();
  
  // Initialize the motor with speed = 0.
  TIM1_PWM_SET(PWMFreq, 100);
  
  

  /*Speed Feedback*/
  //TIM12_CH1_Cap_Init(0XFFFF, 84 - 1);  ////以1Mhz的频率计数
}
