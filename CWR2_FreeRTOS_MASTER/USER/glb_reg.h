
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLB_REG_H
#define __GLB_REG_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"



/* Exported types ------------------------------------------------------------*/
	 
#ifndef Para
#define Para dGlbReg
#endif
/* Exported constants --------------------------------------------------------*/
#define SLAVE_ADDR 0
#define Judge_SAVE_PAR_ADDR 198 //用这个地址判断是否首次加载程序
#define SAVE_PAR_ADDR 299
#define DEBUG_EN_ADDR SAVE_PAR_ADDR
#define CMD_CODE_ADDR 206 
	 
#define SAVE_PAR_CMD 2049
#define SAVE_PAR_NUM 200//299
#define DEBUG_EN_CMD 2050
#define DEBUG_DIS_CMD 2051

#define CFG_CWR_PAR_BASE 100
#define CWR_STATE_BASE 200
#define CWR_RUNED_BASE  216

/* Exported functions ------------------------------------------------------- */
//void SysTick_Handler(void); 
#define HOLDING_REG_END 1024//4096 //1024

/* Exported struct types ------------------------------------------------------------*/
typedef __packed struct{
	
	
	u16 CtrlWord; // 设计为持续有效的命令，非1次性的
  u16 StateWord; //状态字	
	u16 ErrCode;	//错误字
	u16 MotionMode;  //0：自动模式或 1：单步模式	
	u16 MotionDir; // 移动到方向	
	u16 MotionStop; //连贯动作可以暂停，通过该位实现
	u16 MotionCmdCode; //计划为1次性的命令,取消调试模式时为14
	u16 Rev1[3];
//
	s16 Pos1;//电机1脉冲计数器
	s16 Pos2;//电机2脉冲计数器
	u16 PosRuned1;//电机1已运行路程；
	u16 PosRuned2;//电机1已运行路程；
	u16 AutocycleLeft;// 剩余来回循环次数
	s16 PosLeft;// 机器人剩余距离
	u16 PosRuned;//机器人已运行多少路程
	u16 Rev2[3];

//	
	u16 PosFactor; //调试位置因子设定，实际位置 = posset*DebugPosFactor;需要初始化
	u16 SpdLimit; //最大爬行速度限制，需要初始化
	u16 PosLimit; //最大爬行位置限制，需要初始化
	u16 SpdUp;   // 向上爬行速度
	u16 SpdDown; // 向下爬行速度
	u16 MotionSpeed;//转化成模拟量的数字量
	u16 Rev3[4];
//	
	u16 Ai;
	u16 Di;
	u16 Do;	
	u16 AO1;
	u16 AO2;
	u16 BatVolt;
	u16 Rev4[4];
	
}CWR_VAR_TYPE;//只写，40个字节

typedef __packed struct{
	
	u16 CWRCtrlEN;
	u16 CWRCtrlAuto;
	u16 CWRStartDir;
	u16 CWRStart;
	u16 CWRPause;

	u16 ClimbPos; //爬行距离
	u16 AutoCycleNum;// 自动来回循环次数
	u16 ClimbUpSpd; // 向上爬升速度。
	u16 ClimbDownSpd; // 向下爬升速度。
	
	u16 ClimbTimeLimit;
	
} CFG_CWR_PAR_TYPE;//只读,10个字节


extern u16 DebugFlag;
extern u16 SlaveIdAddr;
extern const u16  Bit[];	 	 
extern volatile u16 dGlbReg [];
extern CWR_VAR_TYPE * const ptrCWRBlk;
extern CFG_CWR_PAR_TYPE * const ptrCfgCWRPar;
#ifdef __cplusplus
}
#endif

#endif 

