/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : rs485.c
* Author             : 
* Date First Issued  : 27/04/2010
* Description        : UART的硬件配置和modbus协议处理
********************************************************************************/

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "glb_reg.h"
#include "RS485.h"


// modbus 协议
OS_EVENT *sem_rs485_rx;
OS_EVENT *sem_rs485_tx;

volatile RS_STR uart2rx;
volatile u8 modbus_busy_flag = 0;
//volatile u16 modbusAddr = 1;	

u16 Frame_deal(void); 

void uart1_init(u32 bound)
{
	//变量初始化 //默认为9600 波特率			 
	
	
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//USART_Parity_Even;//USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
  USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//
	
#if 1	
	//Usart1 NVIC 配置
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);//开启相关中断
#endif	
	
}



void USART1_IRQHandler(void)                	//串口1中断服务程序
{
//	u8 Res;
	u16 temp;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		temp = (uint16_t)(USART1->DR & (uint16_t)0x01FF);
		if(uart2rx.state == S_RX)
		{
		  uart2rx.Rx2[uart2rx.index++] = temp;//USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据 			
			uart2rx.us_end = T35_9600;
			uart2rx.timeout_en = 1;
			//printf("y\n");
			//TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//
      //TIM_SetCounter(TIM3,0);//
			//TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
      //TIM_Cmd(TIM3,ENABLE);//
		}				 		 
  }
	
	if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{		
		// printf("Uart1 send finish \n\t");
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		
    if(uart2rx.send_flag !=0)  //一定要加上此条件，否则信号量容易误触发
		{
			if(uart2rx.send_len < uart2rx.send_end)
			{
				
				USART1->DR = uart2rx.Tx2[uart2rx.send_len++];  //触发第一次写
			}	
			else
			{		 

							
			}		 
	  }
	}

										 

} 


///////////////////////////////////////////////////////////////////////
//以下为定时器代码

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc) 
{	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	

	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	OSIntEnter();     ////如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{		 
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位				
		if(uart2rx.timeout_en != 0)
		{
			if(uart2rx.us_end != 0) //T35_19200
			{
				uart2rx.us_end --;
			}
			else
			{
				//TIM_Cmd(TIM3,DISABLE);//?????	
				 uart2rx.timeout_en = 0;	
				OSSemPost(sem_rs485_rx);				
			}
		}
		
		/*if(Com2TxRx.timeout_en !=0)
		{
		  //printf("k\n");
			if(Com2TxRx.timeout !=0)
			{
				Com2TxRx.timeout --;				
			}
			else
			{
				Com2TxRx.timeout_en = 0;
				USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
				OSSemPost(sem_SrvCom_rx);
			}
		}*/
			
		

	}
	
	OSIntExit();  	
}




/* Private const -------------------------------------------------------------*/
//字地址 0 - 255 (只取低8位)
//位地址 0 - 255 (只取低8位)
/* CRC 高位字节值表 */
static const u8 auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //0          
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //16        
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //32        
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //48        
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //64        
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //80        
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //96        
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //112       
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //128       
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //144       
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //160       
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //176       
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //192       
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //208       
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //224       
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,              
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //240       
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40                 
};

/* CRC低位字节值表*/
static const u8 auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,  //0          
	0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,            
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,  //16          
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,            
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,  //32          
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,            
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,  //48          
	0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,            
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,  //64          
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,            
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,  //80          
	0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,            
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,  //96          
	0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,            
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,  //112          
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,            
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,  //128          
	0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,            
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,  //144          
	0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,            
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,  //160          
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,            
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,  //176
	0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,            
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,  //192          
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,            
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,  //208          
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,            
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,  //224          
	0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,            
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,  //240          
	0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40             
};


 /*******************************************************************************
 * Function Name : crc16
 * Description	 : CRC计算
 * Input		 : puchMsg - 数据包指针，
 *				   usDataLen - 数据包长度
 * Output		 : None.
 * Return		 : 16位CRC码
 *******************************************************************************/
 u16 crc16(u8 *puchMsg, u16 usDataLen)
 {
	 u8 uchCRCHi = 0xFF ; /* 高CRC字节初始化 */
	 u8 uchCRCLo = 0xFF ; /* 低CRC 字节初始化 */
	 u8 uIndex ; /* CRC循环中的索引 */
 
	 while (usDataLen--) /* 传输消息缓冲区 */
	 {
		 uIndex = uchCRCHi ^ *puchMsg++ ; /* 计算CRC */
		 uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		 uchCRCLo = auchCRCLo[uIndex] ;
	 }
 
	 return (uchCRCHi << 8 | uchCRCLo) ;
 }

 
//RS485 通信
void RS485Init(void)
{
	
	
	sem_rs485_rx = OSSemCreate(0); //创建信号量
	sem_rs485_tx = OSSemCreate(0); //创建信号量
	uart2rx.state = S_RX; 
	uart2rx.index = 0;
	uart2rx.us_end = T35_9600;	// 其实该值是影响TIM3的load值的,CCR1为9600
  uart2rx.send_len = RESET;
	uart2rx.send_flag = 0;
	uart2rx.timeout_en = 0;
//uart2rx.busy = RESET;
	modbus_busy_flag = 0; 	
	uart1_init(9600);
	TIM3_Int_Init(10-1,8400-1);	//1ms//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms  
}

void Protocol_RS485 (void)
{
	u8 err;
	
	switch(uart2rx.state)
	{
		case S_RX:
			OSSemPend(sem_rs485_rx,0,&err);	       
			uart2rx.state = S_Frame;
		  //printf("S_RX\n\t");
			break;
		case S_Frame:		
			if(Frame_deal() !=0)
			{
				uart2rx.state = S_TX;	
			}		
			else
			{//校验失败
				uart2rx.state = S_RX;
				uart2rx.index = 0;	
				uart2rx.len = 0;
				uart2rx.send_end = 0;		  
				uart2rx.send_flag = 0;				
			}
		//	printf("S_Frame \n\t");		
			break; //触发第1帧数据发送在这里
		case S_TX:			
			//等待信号量
			OSSemPend(sem_rs485_tx,0,&err);
			uart2rx.state = S_RX;
		  uart2rx.index = 0;	
		  uart2rx.len = 0;
		  uart2rx.send_end = 0;		  
		  uart2rx.send_flag = 0;
		 // printf("S_TX\n\t");
			break;
		default:;
	}
}



/*******************************************************************************
* Function Name  : void Para_deal(void)
* Description    : 接收一帧处理，如果接收完，自动转为发送
			一帧，
* Input          : 接收区Rx2,发送Tx2
* Output         : None
* Return         : None
*******************************************************************************/
void Para_deal(void)
{
    u16 i;
    u16 addre,bytecount,wordcount;
    s16 data;
   // u16 *Par;
  //	u16 dword_L;

uart2rx.err= 0;

switch(uart2rx.Rx2[COMMAND])
{
case 3://发送数据
	addre = uart2rx.Rx2[ADDREH]<<8|uart2rx.Rx2[ADDREL];
	wordcount =  uart2rx.Rx2[SEND_DATAH]<<8|uart2rx.Rx2[SEND_DATAL];
	if(wordcount==0)//至少读一个字
		{
		wordcount = 1;
		}
	else if(wordcount>MAX_ACCESS_REG_NUM)//最多读60个字
		{
		wordcount = MAX_ACCESS_REG_NUM;
		}
		
	if(&Para[HOLDING_REG_END]<(&Para[0]+addre+wordcount))//大于参数表报错
		{
		uart2rx.err=3;
		}
	else
		{ 

		//
		wordcount<<=1;
		for(i=0;i<wordcount;)
			{
			data=Para[addre++];
			uart2rx.Tx2[ADDREL+i]=data>>8;
			uart2rx.Tx2[ADDREL+1+i]=data&0xff;
			i +=2;
			}
		
		uart2rx.send_end =5+i;
		uart2rx.Tx2[2] = wordcount;
		}
	break;
case 6://接收数据
	addre = uart2rx.Rx2[ADDREH]<<8|uart2rx.Rx2[ADDREL];
	data=(uart2rx.Rx2[SEND_DATAH]<<8|uart2rx.Rx2[SEND_DATAL]);
		
	//	if(addre <500) //地址0～200以内的是保存在eeprom的
		if(1)
		{
	//20150416//暂时去掉数据范围的判断和读写属性的判断	
		
			if(uart2rx.err ==RESET)
				{

				if(addre == CMD_CODE_ADDR)
				{					
					if(DebugFlag == 1)
					{
						Para[addre]=data;	
					}
					else
					{
						uart2rx.err=2;
					}
				}
				else
				{
					Para[addre]=data;	
				}
					
				/*if(Para[addre]!=data)
				{  
				    Para[addre]=data;		   		  
		          
				}*/ 	

				uart2rx.Tx2[4]=data>>8;//复制参数
				uart2rx.Tx2[5]=data&0xff;
				uart2rx.Tx2[2]=uart2rx.Rx2[2];//复制地址
				uart2rx.Tx2[3]=uart2rx.Rx2[3];
	
				uart2rx.send_end =8;
				}
		}
		else  //写大于500的地址，报错
		{
		  uart2rx.err=2;
		  break;
		}
	
	
	break;
case 16:
	addre=uart2rx.Rx2[ADDREH]<<8|uart2rx.Rx2[ADDREL];
	wordcount=uart2rx.Rx2[SEND_DATAH]<<8|uart2rx.Rx2[SEND_DATAL];//写多少个字
	 data= uart2rx.Rx2[SEND_DATAL+1];//写多少个字节
	 if(wordcount>MAX_ACCESS_REG_NUM)
	 	{
		 wordcount =MAX_ACCESS_REG_NUM;
		 data =MAX_ACCESS_REG_NUM<<1;
		}
	//if(&Para[256]<(&Para[0]+addre+wordcount) 
	if(&Para[500]<(&Para[0]+addre+wordcount) \
		||(data>>1!=wordcount)||(data==0))//大于写参数表报错
		{
		uart2rx.err=3;
		}
	else
		{
	for(i=0;i < wordcount;i++)
		{
		bytecount=i<<1;
		//data = uart2rx.Rx2[7+bytecount]<<8|uart2rx.Rx2[8+bytecount];
		data = (uart2rx.Rx2[7+bytecount]<<8|uart2rx.Rx2[8+bytecount]);
	
		if(uart2rx.err==RESET)
		{
			Para[addre+i] = data;
		}
		else
		{
			break;
		}
		}

		uart2rx.Tx2[2]=uart2rx.Rx2[ADDREH];//回送地址
		uart2rx	.Tx2[3]=uart2rx.Rx2[ADDREL];
		uart2rx.Tx2[4]=uart2rx.Rx2[SEND_DATAH];//回送数据量
		uart2rx	.Tx2[5]=uart2rx.Rx2[SEND_DATAL];
		uart2rx.send_end =8;
		}
	break;

default:
	uart2rx.err=2;
	break;
}



uart2rx.Tx2[0]=uart2rx.Rx2[0];//复制地址
uart2rx.Tx2[1]=uart2rx.Rx2[1];//复制命令

if(uart2rx.err)//如果有错
{
uart2rx.Tx2[1] |=0x80;//最高位置1
uart2rx.Tx2[2] =uart2rx.err;//送错误码
uart2rx.send_end =5;
}
for(i=0;i<sizeof(uart2rx.Rx2);i++)
{
//uart2rx.Tx2[i]=uart2rx.Rx2[i];
uart2rx.Rx2[i]=0;//接收内容清0，避免一下重复执行
}




wordcount=crc16(uart2rx.Tx2,uart2rx.send_end-2);

uart2rx.Tx2[uart2rx.send_end-2]=wordcount>>8;
uart2rx.Tx2[uart2rx.send_end-1]=wordcount&0xff;


}





u16 Frame_deal(void)
{
union{
	u16 data;
	struct{
		u8 crch;
		u8 crcl;
		}u;
}crc;
u16 temp; 
	
	//if((sizeof(uart2rx.Rx2)-uart2rx.len) < 6)
	  //return;
	if((uart2rx.Rx2[COMMAND]==3)||(uart2rx.Rx2[COMMAND]==6))
		{
		uart2rx.send_end =6;
		
		}
	else if(uart2rx.Rx2[COMMAND]==16)
		{
		uart2rx.send_end =uart2rx.Rx2[RECE_BYTE]+7;
		}
	else
		{
		uart2rx.send_end =RESET;
		}
	crc.data=crc16(uart2rx.Rx2,uart2rx.send_end);
	temp = uart2rx.Rx2[uart2rx.send_end]<<8|uart2rx.Rx2[uart2rx.send_end+1];

 
	//if(((1 == uart2rx.Rx2[0])||(uart2rx.Rx2[0] ==0))&&(temp==crc.data))//增加地址为0的广播功能
	if(((SlaveIdAddr == uart2rx.Rx2[0])||(uart2rx.Rx2[0] ==0))&&(temp==crc.data))//增加地址为0的广播功能
	{ 
		
	 if(uart2rx.Rx2[0] !=0)	//地址为0的广播帧，不作应答
		{
		uart2rx.send_len = RESET;
		uart2rx.send_flag = 1;
		}
	
		
  //上层帧协议处理
	Para_deal(); //上层帧协议处理	

		if(uart2rx.send_flag != 0)	//地址为0的广播帧，不作应答
		{
			
			if(uart2rx.send_len < uart2rx.send_end)
			{
				//printf("a:%d,%d\n ",uart2rx.send_len,uart2rx.send_end);
				USART1->DR = uart2rx.Tx2[uart2rx.send_len++];  //触发第一次写
				//printf("b:%d,%d\n ",uart2rx.send_len,uart2rx.send_end);
			}			
		}	
		
		return (1); //ok
	}	
	else
	{//如果校验错误，那么不做处理，重新接受
		return (0);//	
	}

}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void RS485_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	//RS485_TX_EN=1;			//设置为发送模式
  	for(t=0;t<len;t++)		//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		
    USART_SendData(USART2,buf[t]); //发送数据
	}	 
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		
	//RS485_RX_CNT=0;	  
	//RS485_TX_EN=0;				//设置为接收模式	
}

/*
//RS485查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
void RS485_Receive_Data(u8 *buf,u8 *len)
{
	u8 rxlen=RS485_RX_CNT;
	u8 i=0;
	*len=0;				//默认为0
	delay_ms(10);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
	if(rxlen==RS485_RX_CNT&&rxlen)//接收到了数据,且接收完成了
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}		
		*len=RS485_RX_CNT;	//记录本次数据长度
		RS485_RX_CNT=0;		//清零
	}
}
*/
