#include "task.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "timer.h"
#include "usart.h"
/*********************************************************************************
*********************启明欣欣 STM32F407应用开发板(高配版)*************************
**********************************************************************************
* 文件名称: task.c                                                               *
* 文件简述：ucos下的所有任务                                                     *
* 创建日期：2017.08.90                                                           *
* 版    本：V1.0                                                                 *
* 作    者：Clever                                                               *
* 说    明：LED对应IO口初始化                                                    * 
**********************************************************************************
*********************************************************************************/

//LED0任务
OS_STK  LED0_TASK_STK[LED0_STK_SIZE];  //任务堆栈	

void LED0_Task(void *pdata)
{	 	
	while(1)
	{
		//LED0=0;
		OSTimeDlyHMSM(0,0,0,300);
		//LED0=1;
		OSTimeDlyHMSM(0,0,0,300);
	};
}


//LED1任务
OS_STK  LED1_TASK_STK[LED1_STK_SIZE];  //任务堆栈

void LED1_Task(void *pdata)
{	  
	while(1)
	{
		//LED1=0;
		OSTimeDlyHMSM(0,0,0,300);
		//LED1=1;
		OSTimeDlyHMSM(0,0,0,600);
	};
}
//测速task
	int total_ms=0;
OS_STK  SPEED_TASK_STK[SPEED_STK_SIZE];  //任务堆栈
void SPEED_CAL_Task(void *pdata)
{	
	float speed=0;
	float Freq=0;
	float trans_rate=12;
	u8 temp[64];
	uart1_init(9600);	    //串口初始化波特率为9600  
	//EXTIX_Init();			//外部中断初始化
	TIM4_Init(4999,8399);	//定时器2时钟84M，分频系数8400，84M/8400=10K 所以计数5000次为500ms
	uart1SendChars("Hello Word!",11);
	//printf("Hello Word!",8);
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,1000);
		Freq=1.0/total_ms*1000;//霍尔脉冲频率
		speed=((Freq/8.0)*3.14*0.465*trans_rate);
		memset(temp,' ',64);
		sprintf(temp,"total_ms= %d  speed=%f km/h \n",total_ms,speed*1000/3600);
		uart1SendChars(temp,strlen(temp));
	};
}

