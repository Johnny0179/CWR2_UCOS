
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
#define Judge_SAVE_PAR_ADDR 198 //�������ַ�ж��Ƿ��״μ��س���
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
	
	
	u16 CtrlWord; // ���Ϊ������Ч�������1���Ե�
  u16 StateWord; //״̬��	
	u16 ErrCode;	//������
	u16 MotionMode;  //0���Զ�ģʽ�� 1������ģʽ	
	u16 MotionDir; // �ƶ�������	
	u16 MotionStop; //���ᶯ��������ͣ��ͨ����λʵ��
	u16 MotionCmdCode; //�ƻ�Ϊ1���Ե�����,ȡ������ģʽʱΪ14
	u16 Rev1[3];
//
	s16 Pos1;//���1���������
	s16 Pos2;//���2���������
	u16 PosRuned1;//���1������·�̣�
	u16 PosRuned2;//���1������·�̣�
	u16 AutocycleLeft;// ʣ������ѭ������
	s16 PosLeft;// ������ʣ�����
	u16 PosRuned;//�����������ж���·��
	u16 Rev2[3];

//	
	u16 PosFactor; //����λ�������趨��ʵ��λ�� = posset*DebugPosFactor;��Ҫ��ʼ��
	u16 SpdLimit; //��������ٶ����ƣ���Ҫ��ʼ��
	u16 PosLimit; //�������λ�����ƣ���Ҫ��ʼ��
	u16 SpdUp;   // ���������ٶ�
	u16 SpdDown; // ���������ٶ�
	u16 MotionSpeed;//ת����ģ������������
	u16 Rev3[4];
//	
	u16 Ai;
	u16 Di;
	u16 Do;	
	u16 AO1;
	u16 AO2;
	u16 BatVolt;
	u16 Rev4[4];
	
}CWR_VAR_TYPE;//ֻд��40���ֽ�

typedef __packed struct{
	
	u16 CWRCtrlEN;
	u16 CWRCtrlAuto;
	u16 CWRStartDir;
	u16 CWRStart;
	u16 CWRPause;

	u16 ClimbPos; //���о���
	u16 AutoCycleNum;// �Զ�����ѭ������
	u16 ClimbUpSpd; // ���������ٶȡ�
	u16 ClimbDownSpd; // ���������ٶȡ�
	
	u16 ClimbTimeLimit;
	
} CFG_CWR_PAR_TYPE;//ֻ��,10���ֽ�


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

