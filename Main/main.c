#include "led.h"
#include "includes.h"
#include "key.h"
#include "task.h"
#include "usart1.h"
#include "usart5.h"
#include "usart6.h"

/*********************************************************************************
*********************启明欣欣 STM32F407应用开发板(高配版)*************************
**********************************************************************************
* 文件名称: 例程28 UCOSII移植                                                    *
* 文件简述：UCOSII实验                                                           *
* 创建日期：2017.09.30                                                           *
* 版    本：V1.0                                                                 *
* 作    者：Clever                                                               *
* 说    明：                                                                     * 
* 淘宝店铺：https://shop125046348.taobao.com                                     *
* 声    明：本例程代码仅用于学习参考                                             *
**********************************************************************************
*********************************************************************************/



int main(void)
{ 
	
	OS_CPU_SR cpu_sr=0;
	u8 key;
	//SCB->VTOR = FLASH_BASE | 0x10000;
	SCB->VTOR = *((u32 *)0x0800FFF8);
	
	delay_init();		  //初始化延时函数 利用滴答定时器SysTick中断模式延时，也提供了系统节拍
	LED_Init();		    //初始化LED端口
	KEY_Init();
	uart1_init(9600);

	LEDX = 0;
	delay_us(100000);
	if (0 == KEYX) {
		printf("KEYX 0\n");
	} else {
		printf("KEYX 1\n");
	}
#if 0
	uart5_init(9600);
	uart6_init(9600);
	
	printf("hello uart1\n");
	uart5SendChars("usart5_test",11);
	uart6SendChars("usart6_test",11);
#endif
	
	OSInit();        //初始化UCOS系统
	
	/*建立任务*/
	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(LED0_Task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(LED1_Task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	 				   
	OSTaskCreate(SPEED_CAL_Task,(void *)0,(OS_STK*)&SPEED_TASK_STK[SPEED_STK_SIZE-1],SPEED_TASK_PRIO);	 				   
	OS_EXIT_CRITICAL();				 //退出临界区(可以被中断打断)
	
	OSStart();	     //系统开始运行
}









