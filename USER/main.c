#include "includes.h"
#include "malloc.h"
#include "common.h"
#include "usart3.h"
#include "usart5.h"
#include "sim900a.h"
#include "can1.h"
#include "can2.h"
#include "rfid.h"
#include "vs10xx.h"
#include "mp3play.h"

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈，8字节对齐	
__align(8) static OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//串口任务
//设置任务优先级
#define USART_TASK_PRIO       			7 
//设置任务堆栈大小
#define USART_STK_SIZE  		    	128
//任务堆栈，8字节对齐	
__align(8) static OS_STK USART_TASK_STK[USART_STK_SIZE];
//任务函数
void usart_task(void *pdata);
							 
//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			6 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					1200
//任务堆栈，8字节对齐	
__align(8) static OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);

//监视任务
//设置任务优先级
#define WATCH_TASK_PRIO       			3 
//设置任务堆栈大小
#define WATCH_STK_SIZE  		   		256
//任务堆栈，8字节对齐	
__align(8) static OS_STK WATCH_TASK_STK[WATCH_STK_SIZE];
//任务函数
void watch_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////	 

int total_ms=0;
extern int pluse_num_new;
//////////////////////////////////////////////////////////////////////////////	 

//系统初始化
void system_init(void)
{
	u8 res;
	u16 temp=0;
	u32 dtsize,dfsize;
	u8 CAN1_mode=0; //CAN工作模式;0,普通模式;1,环回模式
	u8 CAN2_mode=0; //CAN工作模式;0,普通模式;1,环回模式	
	
	delay_init(168);			//延时初始化  
	uart_init(115200);		//初始化串口波特率为115200
	usart3_init(115200);		//初始化串口3波特率为115200
	usart5_init(38400);
 	LED_Init();					//初始化LED 
 	KEY_Init();					//按键初始化 
//	W25QXX_Init();				//初始化W25Q128

	printf("SmartMotor Starting...\n");
	CAN1_Mode_Init(CAN1_mode);//CAN初始化普通模式,波特率250Kbps
	CAN2_Mode_Init(CAN2_mode);//CAN初始化普通模式,波特率500Kbps 
  
	my_mem_init(SRAMIN);		//初始化内部内存池
	my_mem_init(SRAMCCM);		//初始化CCM内存池 

	TIM2_Init(9999,8399);	
	TIM4_Init(9999,8399);
	
	VS_Init();	  				//初始化VS1053
	
	delay_ms(1500);
	
 	exfuns_init();// alloc for fats
	// Call SD_Init internally
  f_mount(fs[0],"0:",1);
	
	temp=0;	
 	do {
		temp++;
 		res=exf_getfree("0:",&dtsize,&dfsize);
		delay_ms(200);		   
	} while(res&&temp<5);
	
 	if(res==0) {
		printf("Read SD OK!\r\n");
	} else {
		printf("Read SD Failed!\r\n");
	}
	
#if 0
	u16 xx = 0;
	delay_ms(1000);
	xx = VS_Ram_Test();
	delay_ms(1000);
	music_play();
	
	while(1)
	{
		delay_ms(1000);
 		LED1=0; 	   
 		VS_Sine_Test();	   	 
		delay_ms(1000);
		LED1=1;
	}
#endif
}   

//main函数	  					
int main(void)
{ 	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
  system_init();		//系统初始化 
 	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  						    
}
//extern OS_EVENT * audiombox;	//音频播放任务邮箱
//开始任务
void start_task(void *pdata)
{  
	OS_CPU_SR cpu_sr=0;
	pdata = pdata; 	   
	OSStatInit();		//初始化统计任务.这里会延时1秒钟左右	
// 	app_srand(OSTime);
	
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)    
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);						   
 	OSTaskCreate(usart_task,(void *)0,(OS_STK*)&USART_TASK_STK[USART_STK_SIZE-1],USART_TASK_PRIO);						   
	OSTaskCreate(watch_task,(void *)0,(OS_STK*)&WATCH_TASK_STK[WATCH_STK_SIZE-1],WATCH_TASK_PRIO); 					   
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
} 
	
//主任务
void main_task(void *pdata)
{
//	while(1) {
//		u3_printf("Hello Uart3 TTL\n");
//		sim900a_send_cmd("AT","OK",100);
//		delay_ms(1000);
//	}
	sim7500e_tcp_connect(0,NULL,NULL);
//	delay_ms(5000);
//	cpr74_read_calypso();
}

//执行最不需要时效性的代码
void usart_task(void *pdata)
{	 
	u8 res;
	u32 br;
	char log_buf[] = "hello log"; 
	FIL f_txt;
	
	delay_ms(2000);
	delay_ms(2000);
	delay_ms(2000);
	
	while(1) {
		delay_ms(3000);
		cpr74_read_calypso();
		
		printf("Hall Counter = %d\n", pluse_num_new);
/*
		res=f_open(&f_txt,(const TCHAR*)"0:/test01.txt",FA_READ|FA_WRITE);
		if(res==0)
		{
			f_lseek(&f_txt,0);
			f_write(&f_txt,log_buf,strlen((const char*)log_buf),(UINT*)&br);
			f_close(&f_txt);
		}
*/
	}
}

//监视任务
void watch_task(void *pdata)
{
	u8 key;
  OS_CPU_SR cpu_sr=0; 
	
	while(1)
	{
		// TBD to check Why
		// Only K3(=WKUP) is useful
		key=KEY_Scan(0);
	  if(key)
		{						   
			switch(key)
			{				 
				case WKUP_PRES:
					LED0=!LED0;
					LEDX=!LEDX;
					break;
				case KEY0_PRES:
					LED0=!LED0;
					LEDX=!LEDX;
					break;
				case KEY1_PRES:
					LED1=!LED1;
					break;
				case KEY2_PRES:
					LED0=!LED0;
					LED1=!LED1;
					break;
			}
		}else delay_ms(10);
    OSTimeDlyHMSM(0,0,0,10);
	}
}

//硬件错误处理
void HardFault_Handler(void)
{
	u32 i;
	u8 t=0;
	u32 temp;
	temp=SCB->CFSR;					//fault状态寄存器(@0XE000ED28)包括:MMSR,BFSR,UFSR
 	printf("CFSR:%8X\r\n",temp);	//显示错误值
	temp=SCB->HFSR;					//硬件fault状态寄存器
 	printf("HFSR:%8X\r\n",temp);	//显示错误值
 	temp=SCB->DFSR;					//调试fault状态寄存器
 	printf("DFSR:%8X\r\n",temp);	//显示错误值
   	temp=SCB->AFSR;					//辅助fault状态寄存器
 	printf("AFSR:%8X\r\n",temp);	//显示错误值
 	LED1=!LED1;
 	while(t<5)
	{
		t++;
		LED0=!LED0;
		//BEEP=!BEEP;
		for(i=0;i<0X1FFFFF;i++);
 	}
}
