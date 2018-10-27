#include "task.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "timer.h"
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

OS_STK  SPEED_TASK_STK[SPEED_STK_SIZE];  //任务堆栈
void SPEED_CAL_Task(void *pdata)
{	  
	//KEY_Init();       //按键初始化
	EXTIX_Init();			//外部中断初始化
	TIM2_Init(4999,8399);	//定时器2时钟84M，分频系数8400，84M/8400=10K 所以计数5000次为500ms
	while(1)
	{
		LED0=0;
		OSTimeDlyHMSM(0,0,0,300);
		LED0=0;
		OSTimeDlyHMSM(0,0,0,600);
	};
}

