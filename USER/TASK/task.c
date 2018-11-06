#include "task.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "timer.h"
#include "usart1.h"
/*********************************************************************************
*********************�������� STM32F407Ӧ�ÿ�����(�����)*************************
**********************************************************************************
* �ļ�����: task.c                                                               *
* �ļ�������ucos�µ���������                                                     *
* �������ڣ�2017.08.90                                                           *
* ��    ����V1.0                                                                 *
* ��    �ߣ�Clever                                                               *
* ˵    ����LED��ӦIO�ڳ�ʼ��                                                    * 
**********************************************************************************
*********************************************************************************/

//LED0����
OS_STK  LED0_TASK_STK[LED0_STK_SIZE];  //�����ջ	

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


//LED1����
OS_STK  LED1_TASK_STK[LED1_STK_SIZE];  //�����ջ

void LED1_Task(void *pdata)
{	  
	while(1)
	{
		//LED1=0;
		PA15=0;
		OSTimeDlyHMSM(0,0,0,5);
		//LED1=1;
		PA15=1;
		OSTimeDlyHMSM(0,0,0,5);
	};
}
//����task
	int total_ms=0;
extern int pluse_num;
OS_STK  SPEED_TASK_STK[SPEED_STK_SIZE];  //�����ջ
void SPEED_CAL_Task(void *pdata)
{	
	float speed=0;
	float Freq=0;
	float trans_rate=12;
	char temp[128];
	uart1_init(9600);	    //���ڳ�ʼ��������Ϊ9600  
	//EXTIX_Init();			//�ⲿ�жϳ�ʼ��
	TIM2_Init(9999,8399);	
	TIM4_Init(9999,8399);	//��ʱ��2ʱ��84M����Ƶϵ��8400��84M/8400=10K ���Լ���10000��Ϊ1000ms
	printf("Hello Word!\n");
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,1500);
		Freq=1.0/total_ms*1000;//��������Ƶ��
		speed=((Freq/8.0)*3.14*0.465*trans_rate);
		memset(temp,0x00,128);
		
		printf("total_ms= %d speed=%d km/h\n",total_ms,(int)(speed*1000/3600));

		Freq=1.0/(1000.0/pluse_num)*1000;//��������Ƶ��
		speed=((Freq/8.0)*3.14*0.465*trans_rate);
		memset(temp,0x00,128);

		printf("pluse_num= %d speed=%d km/h\n",pluse_num,(int)(speed*1000/3600));

	};
}

