#include "Motor.h"
#include "RS485.h"
#include "can.h"
#include "delay.h"
#include "glb_reg.h"
#include "includes.h"
#include "led.h"
#include "sys.h"
#include "usart.h"

/////////////////////////UCOSII任务堆栈设置///////////////////////////////////
// START 任务
//设置任务优先级
#define START_TASK_PRIO 10  //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE 64  //
//创建任务堆栈空间
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数接口
void start_task(void *pdata);



/*//设置任务优先级
#define LED_TASK_PRIO 5
//设置任务堆栈大小
#define LED_STK_SIZE 64
//创建任务堆栈空间
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数接口
void led_task(void *pdata);
*/

//设置任务优先级
#define Motor_TASK_PRIO 12
//设置任务堆栈大小
#define Motor_STK_SIZE 256
//创建任务堆栈空间
OS_STK Motor_TASK_STK[Motor_STK_SIZE];
//任务函数接口
void Motor_task(void *pdata);

// CAN
//设置任务优先级
#define CAN_TASK_PRIO 9
//设置任务堆栈大小
#define CAN_STK_SIZE 256
//创建任务堆栈空间
OS_STK CAN_TASK_STK[CAN_STK_SIZE];
//任务函数接口
void CAN_task(void *pdata);

const int freq = 20000;  // Freq
u8 key;
u8 i = 0;
u8 canbuf[8];
u8 res;
u32 MoveInteval=5;

int main(void) {
  // const int Duty=50;
  delay_init(168);  //
  LED_Init();
  uart_init(9600);  //

  // Set the Motor speed.
  Para[0] = moveup;  // Direction
  Para[1] = 0;      // Speed
Para[2] = ModeManual;
Para[3] = MoveInteval;
  // Freq=20k,DutyRatio=50
  TIM1_PWM_Init(freq);
  Motor_Init();
  // TIM1_PWM_SET(20000, 50);

  /*Initialize UCOS II*/
  OSInit();
  OSTaskCreate(start_task, (void *)0,
               (OS_STK *)&START_TASK_STK[START_STK_SIZE - 1],
               START_TASK_PRIO);  //创建起始任务
  OSStart();
}

/*System Task*/
void start_task(void *pdata) {
  OS_CPU_SR cpu_sr = 0;
  pdata = pdata;
  // sem_printf=OSSemCreate(1);		//创建打印信号量

  OSStatInit();         //初始化统计任务.这里会延时1秒钟左右
  OS_ENTER_CRITICAL();  //进入临界区(无法被中断打断)
  /*OSTaskCreate(led_task, (void *)0, (OS_STK *)&LED_TASK_STK[LED_STK_SIZE - 1],
               LED_TASK_PRIO);*/
  OSTaskCreate(Motor_task, (void *)0,
               (OS_STK *)&Motor_TASK_STK[Motor_STK_SIZE - 1], Motor_TASK_PRIO);
  OSTaskCreate(CAN_task, (void *)0, (OS_STK *)&CAN_TASK_STK[CAN_STK_SIZE - 1],
               CAN_TASK_PRIO);

  OSTaskSuspend(START_TASK_PRIO);  //挂起起始任务.
  OS_EXIT_CRITICAL();              //退出临界区(可以被中断打断)
}

/*void led_task(void *pdata) {
  while (1) {
delay_ms(1);
}
	}*/

// Motor
void Motor_task(void *pdata) {
  while (1) {
/*Mannual Mode*/
    if(Para[2]==ModeManual){
    MotorCrl(Para[0], Para[1]);
    delay_ms(1);}

    /*Auto Mode*/
    if(Para[2]==ModeAuto){
      MotorCrl(moveup, Para[1]);//MoveUp
      delay_ms(Para[3]*1000);
      MotorCrl(movedown, Para[1]);//MoveDown
      delay_ms(Para[3]*1000);     
  }
  }
}

// CAN
void CAN_task(void *pdata) {
  CAN1_Mode_Init(CAN_SJW_1tq, CAN_BS2_6tq, CAN_BS1_7tq, 6,
                 CAN_Mode_Normal);  // CAN初始化环回模式,波特率500Kbps
  while (1) {
    for (i = 0; i < 8; i++) {
      canbuf[i] = i;  //填充发送缓冲区
      /*Debug*/
  LED0 = !LED0;
      delay_ms(1);
    }
    res = CAN1_Send_Msg(canbuf, 8);  //发送8个字节

    if (!res) LED0 = 0;
    key = CAN1_Receive_Msg(canbuf);
    if (key) {
      for (i = 0; i < key; i++) {
        Para[i]=canbuf[i];
        /*Debug*/
        LED1 = !LED1;
        delay_ms(1);
        //Para[i] = canbuf[i];
      }
    }
  }	
}

