#include "led.h"
#include "includes.h"
#include "task.h"
#include "usart1.h"
#include "usart5.h"
#include "usart6.h"

/*********************************************************************************
*********************�������� STM32F407Ӧ�ÿ�����(�����)*************************
**********************************************************************************
* �ļ�����: ����28 UCOSII��ֲ                                                    *
* �ļ�������UCOSIIʵ��                                                           *
* �������ڣ�2017.09.30                                                           *
* ��    ����V1.0                                                                 *
* ��    �ߣ�Clever                                                               *
* ˵    ����                                                                     * 
* �Ա����̣�https://shop125046348.taobao.com                                     *
* ��    ���������̴��������ѧϰ�ο�                                             *
**********************************************************************************
*********************************************************************************/



int main(void)
{ 
  OS_CPU_SR cpu_sr=0;
	
	delay_init();		  //��ʼ����ʱ���� ���õδ�ʱ��SysTick�ж�ģʽ��ʱ��Ҳ�ṩ��ϵͳ����
	LED_Init();		    //��ʼ��LED�˿� 
	uart1_init(9600);

#if 0
	uart5_init(9600);
	uart6_init(9600);
	
	printf("hello uart1\n");
	uart5SendChars("usart5_test",11);
	uart6SendChars("usart6_test",11);
#endif
	
	OSInit();        //��ʼ��UCOSϵͳ
	
	/*��������*/
	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
 	OSTaskCreate(LED0_Task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(LED1_Task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	 				   
	OSTaskCreate(SPEED_CAL_Task,(void *)0,(OS_STK*)&SPEED_TASK_STK[SPEED_STK_SIZE-1],SPEED_TASK_PRIO);	 				   
	OS_EXIT_CRITICAL();				 //�˳��ٽ���(���Ա��жϴ��)
	
	OSStart();	     //ϵͳ��ʼ����
}









