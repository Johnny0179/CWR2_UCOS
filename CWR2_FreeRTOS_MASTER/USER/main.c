#include "Motor.h"
#include "RS485.h"
#include "can.h"
#include "delay.h"
#include "glb_reg.h"
#include "led.h"
#include "sys.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

u8 key;
u8 i = 0;
u8 canbuf[8];
u8 res;
u8 MoveInteval = 5;

//任务优先级
#define START_TASK_PRIO 1
//任务堆栈大小
#define START_STK_SIZE 128
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void* pvParameters);

/*LED TASK*/
//任务优先级
#define LED_TASK_PRIO 2
//任务堆栈大小
#define LED_STK_SIZE 50
//任务句柄
TaskHandle_t LEDTask_Handler;
//任务函数
void led_task(void* pvParameters);

/*PCCom TASK*/
//任务优先级
#define PCCom_TASK_PRIO 3
//任务堆栈大小
#define PCCom_STK_SIZE 128
//任务句柄
TaskHandle_t PCComTask_Handler;
//任务函数
void PCCom_task(void* pvParameters);


/*CAN TASK*/
//任务优先级
#define CAN_TASK_PRIO 4
//任务堆栈大小
#define CAN_STK_SIZE 128
//任务句柄
TaskHandle_t CANTask_Handler;
//任务函数
void CAN_task(void* pvParameters);

/*// Speed
//设置任务优先级
#define SPEED_TASK_PRIO 16
//设置任务堆栈大小
#define SPEED_STK_SIZE 256
//创建任务堆栈空间
OS_STK SPEED_TASK_STK[SPEED_STK_SIZE];
//任务函数接口
void SPEED_task(void *pdata);*/

int main(void) {
  // const int Duty=50;
  delay_init(168);  //
  LED_Init();
  uart_init(9600);  //

  // Freq=20k,DutyRatio=50
  Motor_Init();

  // Set the Motor speed.
  Para[0] = moveup;  // Direction
  Para[1] = 0;       // Speed
  Para[2] = ModeManual;
  Para[3] = MoveInteval;

  //创建开始任务
  xTaskCreate((TaskFunction_t)start_task,    //任务函数
              (const char*)"start_task",     //任务名称
              (uint16_t)START_STK_SIZE,      //任务堆栈大小
              (void*)NULL,                   //传递给任务函数的参数
              (UBaseType_t)START_TASK_PRIO,  //任务优先级
              (TaskHandle_t*)&StartTask_Handler);  //任务句柄
  vTaskStartScheduler();                           //开启任务调度
}

//开始任务任务函数
void start_task(void* pvParameters) {
  taskENTER_CRITICAL();  //进入临界区
  //创建LED0任务
  xTaskCreate((TaskFunction_t)led_task, (const char*)"led_task",
              (uint16_t)LED_STK_SIZE, (void*)NULL, (UBaseType_t)LED_TASK_PRIO,
              (TaskHandle_t*)&LEDTask_Handler);
  //创建PCCom任务
  xTaskCreate((TaskFunction_t)PCCom_task, (const char*)"PCCom_task",
              (uint16_t)PCCom_STK_SIZE, (void*)NULL,
              (UBaseType_t)PCCom_TASK_PRIO, (TaskHandle_t*)&PCComTask_Handler);
  //CAN任务
  xTaskCreate((TaskFunction_t )CAN_task,
              (const char*    )"CAN_task",
              (uint16_t       )CAN_STK_SIZE,
              (void*          )NULL,
              (UBaseType_t    )CAN_TASK_PRIO,
              (TaskHandle_t*  )&CANTask_Handler);  

  vTaskDelete(StartTask_Handler);  //删除开始任务
  taskEXIT_CRITICAL();             //退出临界区
}

void led_task(void* pvParameters) {
  while (1) {
    /*Mannual Mode*/
    if (Para[2] == 0) {
      MotorCrl(Para[0], Para[1]);
      vTaskDelay(1);
    }

    /*Auto Mode*/
    if (Para[2] == 1) {
      MotorCrl(2, Para[1]);  // MoveUp
      vTaskDelay(Para[3] * 1000);
      MotorCrl(1, Para[1]);  // MoveUp
      vTaskDelay(Para[3] * 1000);
    }
  }
}

//通信任务
void PCCom_task(void* pvParameters) {
  RS485Init();
  while (1) {
    Protocol_RS485();
  }
}

// CAN
void CAN_task(void* pvParameters) {
  CAN1_Mode_Init(CAN_SJW_1tq, CAN_BS2_6tq, CAN_BS1_7tq, 6,
                 CAN_Mode_Normal);  // CAN初始化环回模式,波特率500Kbps
  while (1) {
    for (i = 0; i < 8; i++) {
      canbuf[i] = Para[i];  //填充发送缓冲区
      LED0 = !LED0;
      vTaskDelay(1);
    }
    res = CAN1_Send_Msg(canbuf, 8);  //发送8个字节

    if (!res) LED0 = 0;
    key = CAN1_Receive_Msg(canbuf);
    if (key) {
      for (i = 0; i < key; i++) {
        LED1 = !LED1;
        vTaskDelay(1);
        //Para[i] = canbuf[i];
      }
    }
  }	
}

/*void SPEED_task(void *pdata){
  while(1){
delay_ms(1);
  }
}*/

