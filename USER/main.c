#include "includes.h"
#include "malloc.h"
#include "common.h"
#include "usart3.h"
#include "usart5.h"
#include "usart6.h"
#include "blue.h"
#include "sim900a.h"
#include "can1.h"
#include "can2.h"
#include "rfid.h"
#include "vs10xx.h"
#include "mp3play.h"
#include "rtc.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
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
 			   
#define LOWER_TASK_PRIO       			7
#define LOWER_STK_SIZE  		    	128
__align(8) static OS_STK LOWER_TASK_STK[LOWER_STK_SIZE];
void lower_task(void *pdata);	

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
#define HIGHER_TASK_PRIO       			3 
//设置任务堆栈大小
#define HIGHER_STK_SIZE  		   		256
//任务堆栈，8字节对齐	
__align(8) static OS_STK HIGHER_TASK_STK[HIGHER_STK_SIZE];
//任务函数
void higher_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////	 

int total_ms=0;
extern int pluse_num_new;
int g_sd_existing = 0;

OS_EVENT * sem_beep;
u8 g_logname[64] = "";
u8 g_logmsg[256] = "";

void create_logfile(void);
void write_logs(char *module, char *log, u16 size, u8 mode);

u8 g_mp3_play = 0;
u8 g_mp3_play_name[32] = "";

u32 g_trip_meters = 0;
u32 g_trip_meters_old = 0;
u32 g_total_meters = 0;

extern u8 g_mp3_update_name[128];
extern u8 g_mp3_update;
extern u8 g_dw_write_enable;
extern vu16 g_data_pos;
extern vu16 g_data_size;
extern u8 USART3_RX_BUF_BAK[U3_RECV_LEN_ONE];
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
	usart5_init(115200);
	usart6_init(9600);	    //串口初始化波特率为9600
 	LED_Init();					//初始化LED 
 	KEY_Init();					//按键初始化 

#if 0
	W25QXX_Init();				//初始化W25Q128

	g_sys_env.charge_times = 123;
	
	sys_env_init();

	// Get ENV Params
	sys_env_dump();

	g_trip_meters_old = g_trip_meters;
#endif

	My_RTC_Init();		 		//初始化RTC
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,0);		//配置WAKE UP中断,1秒钟中断一次
		
	printf("SmartMotor Starting...\n");
	CAN1_Mode_Init(CAN1_mode);//CAN初始化普通模式,波特率250Kbps
	CAN2_Mode_Init(CAN2_mode);//CAN初始化普通模式,波特率500Kbps 
  
	my_mem_init(SRAMIN);		//初始化内部内存池
	my_mem_init(SRAMCCM);		//初始化CCM内存池 

	TIM2_Init(9999,8399);	
	TIM4_Init(9999,8399);
	
	//VS_Init();	  				//初始化VS1053
  
	MPU_Init();					//初始化MPU6050
	delay_ms(200);
	while(mpu_dmp_init())
	{
 		delay_ms(200);
		//usart1_send_char('K');
	}
	//mpu_dmp_init();
#if 0	
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
		g_sd_existing = 1;
		printf("Read SD OK!\r\n");
	} else {
		printf("Read SD Failed!\r\n");
	}
	
	create_logfile();
	
#if 0
	u16 xx = 0;
	delay_ms(1000);
	xx = VS_Ram_Test();
	delay_ms(1000);
	music_play();
	
	while(1)
	{
		delay_ms(1000);
 		//LED1=0; 	   
 		VS_Sine_Test();	   	 
		delay_ms(1000);
		//LED1=1;
	}
#endif
#endif
}   

void SoftReset(void)
{  
	while (1) {
		delay_ms(1000);
	}
	__set_FAULTMASK(1);
 	NVIC_SystemReset();
}

//main函数	  					
int main(void)
{ 	
//	SCB->VTOR = *((u32 *)0x0800FFF8);
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
	sem_beep=OSSemCreate(1);
	OSStatInit();		//初始化统计任务.这里会延时1秒钟左右	
// 	app_srand(OSTime);
	
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)    
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);						   
 	OSTaskCreate(usart_task,(void *)0,(OS_STK*)&USART_TASK_STK[USART_STK_SIZE-1],USART_TASK_PRIO);						   
	OSTaskCreate(higher_task,(void *)0,(OS_STK*)&HIGHER_TASK_STK[HIGHER_STK_SIZE-1],HIGHER_TASK_PRIO); 					   
	// OSTaskCreate(lower_task,(void *)0,(OS_STK*)&LOWER_TASK_STK[LOWER_STK_SIZE-1],LOWER_TASK_PRIO); 					   
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
} 

// Play MP3 task
void lower_task(void *pdata)
{
	while(1) {
		if (g_mp3_play) {
			music_play((const char*)g_mp3_play_name);

			g_mp3_play = 0;
			memset(g_mp3_play_name, 0, 32);
		}

		printf("test lower task\n");
   	OSTimeDlyHMSM(0,0,0,500);// 500ms
	}
}

void sim7500e_mobit_process(u8 index);

// MPU6050 Check and BT Data Parse
void main_task(void *pdata)
{
	u8 i = 0;
	u8 loop_cnt = 0;
	
	blue_init();
	while(1) {
		// TBD: Add MPU6050 Check
		// TBD: Add Invalid Moving Check
		// TBD: Add BT Data Parse
		for (i=0; i<U3_RECV_BUF_CNT; i++) {
			if (USART3_RX_STA[i]&0X8000) {
				if (strstr((const char*)(USART3_RX_BUF+U3_RECV_LEN_ONE*i), PROTOCOL_HEAD)) {
					sim7500e_mobit_process(i);
				}
			}
		}
		
		if ((g_trip_meters - g_trip_meters_old) > 10) {// Unit: 0.1KM/BIT
			g_total_meters += (g_trip_meters - g_trip_meters_old) / 10;
			g_trip_meters_old = g_trip_meters;
		}

		// Update total_meters into flash
		if (50 == loop_cnt) {
			loop_cnt = 0;
			// sys_env_save();
		}
   		OSTimeDlyHMSM(0,0,0,100);// 500ms
	}
}

void create_logfile(void)
{
	if (0 == g_sd_existing) {
		return;
	} else {
		u8 res;
		FIL f_txt;
		RTC_TimeTypeDef RTC_TimeStruct;
		RTC_DateTypeDef RTC_DateStruct;
		
		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		
		sprintf((char*)g_logname,"0:/LOG/20%02d%02d%02d_%02d%02d%02d.log",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
		
		res = f_open(&f_txt,(const TCHAR*)g_logname,FA_READ|FA_WRITE|FA_CREATE_ALWAYS);
		if (0 == res) {
			f_close(&f_txt);
		}
	}
}

void write_logs(char *module, char *log, u16 size, u8 mode)
{
	if (0 == g_sd_existing) {
		return;
	} else {
		u8 err;
		u8 res;
		u32 br;
		//char log_buf[] = "hello log"; 
		FIL f_txt;
		RTC_TimeTypeDef RTC_TimeStruct;
		
		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
		
		memset(g_logmsg, 0, 256);
		
		if (0 == mode) {
			sprintf((char*)g_logmsg,"%02d%02d%02d:RECV Data(%s) %s\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds,module,log);
		} else if (1 == mode) {
			sprintf((char*)g_logmsg,"%02d%02d%02d:SEND Data(%s) %s\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds,module,log);
		} else if (2 == mode) {
			sprintf((char*)g_logmsg,"%02d%02d%02d:IMPT Data(%s) %s\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds,module,log);
		} else {
			sprintf((char*)g_logmsg,"%02d%02d%02d:OTHR Data(%s) %s\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds,module,log);
		}
		
		printf("%s", g_logmsg);
		
		OSSemPend(sem_beep,0,&err);
		res= f_open(&f_txt,(const TCHAR*)g_logname,FA_READ|FA_WRITE);
		if(res==0)
		{			
			f_lseek(&f_txt, f_txt.fsize);
			f_write(&f_txt,g_logmsg, strlen((const char*)g_logmsg), (UINT*)&br);
			f_close(&f_txt);
		}
		OSSemPost(sem_beep);
	}
}
//传送数据给匿名四轴上位机软件(V2.6版本)
//fun:功能字. 0XA0~0XAF
//data:数据缓存区,最多28字节!!
//len:data区有效数据个数
void usart1_niming_report(u8 fun,u8*data,u8 len)
{
	u8 send_buf[32];
	u8 i;
	if(len>28)return;	//最多28字节数据 
	send_buf[len+3]=0;	//校验数置零
	send_buf[0]=0X88;	//帧头
	send_buf[1]=fun;	//功能字
	send_buf[2]=len;	//数据长度
	for(i=0;i<len;i++)send_buf[3+i]=data[i];			//复制数据
	for(i=0;i<len+3;i++)send_buf[len+3]+=send_buf[i];	//计算校验和	
	// for(i=0;i<len+4;i++)usart1_send_char(send_buf[i]);	//发送数据到串口1 
}
//通过串口1上报结算后的姿态数据给电脑
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
//roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
//pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
//yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
void usart1_report_imu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;//清0
	tbuf[0]=(aacx>>8)&0XFF;
	tbuf[1]=aacx&0XFF;
	tbuf[2]=(aacy>>8)&0XFF;
	tbuf[3]=aacy&0XFF;
	tbuf[4]=(aacz>>8)&0XFF;
	tbuf[5]=aacz&0XFF; 
	tbuf[6]=(gyrox>>8)&0XFF;
	tbuf[7]=gyrox&0XFF;
	tbuf[8]=(gyroy>>8)&0XFF;
	tbuf[9]=gyroy&0XFF;
	tbuf[10]=(gyroz>>8)&0XFF;
	tbuf[11]=gyroz&0XFF;	
	tbuf[18]=(roll>>8)&0XFF;
	tbuf[19]=roll&0XFF;
	tbuf[20]=(pitch>>8)&0XFF;
	tbuf[21]=pitch&0XFF;
	tbuf[22]=(yaw>>8)&0XFF;
	tbuf[23]=yaw&0XFF;
	usart1_niming_report(0XAF,tbuf,28);//飞控显示帧,0XAF
} 

void MPU6050_Risk_Check()
{
	u8 ret = 0;
	float pitch,roll,yaw; 		//欧拉角
	short aacx,aacy,aacz;		//加速度传感器原始数据
	short gyrox,gyroy,gyroz;	//陀螺仪原始数据
	short temp;					//温度

	ret = mpu_dmp_get_data(&pitch,&roll,&yaw);
	//usart1_send_char(ret);

	if (ret == 0) {
		temp=MPU_Get_Temperature();	//得到温度值
		MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据
		MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据
		//mpu6050_send_data(aacx,aacy,aacz,gyrox,gyroy,gyroz);//用自定义帧发送加速度和陀螺仪原始数据
#if 0
		usart1_report_imu(aacx,aacy,aacz,gyrox,gyroy,gyroz,(int)(roll*100),(int)(pitch*100),(int)(yaw*10));
#else
		//printf(" (int)(roll*100)=%d ,(int)(pitch*100)=%d ,(int)(yaw*10)=%d \r\n",(int)(roll*100),(int)(pitch*100),(int)(yaw*10));
		if((pitch*100>4500)||(pitch*100<-4500)||(yaw*10>450)||(yaw*10<-450))
			printf("FCL \r\n");
		else
			printf("MFC \r\n");
#endif
	}
}

//执行最不需要时效性的代码
void usart_task(void *pdata)
{
	u8 loop_cnt = 0;

	while(1) {
 	   if((UART5_RX_STA&(1<<15)) != 0) {
	       cpr74_read_calypso();
           UART5_RX_STA = 0;
        }

        if (loop_cnt++ == 3) {
            loop_cnt = 0;
            printf("Hall Counter = %d\n", pluse_num_new);
		}

#if 1
		if (1 == g_dw_write_enable) {
			u32 br = 0;
			u8 res = 0;
			FIL f_txt;
			
			res = f_open(&f_txt,(const TCHAR*)g_mp3_update_name,FA_READ|FA_WRITE);
			if (0 == res) {
				f_lseek(&f_txt, f_txt.fsize);
				f_write(&f_txt, USART3_RX_BUF_BAK+g_data_pos, g_data_size, (UINT*)&br);
				f_close(&f_txt);
			}
			
			g_dw_write_enable = 0;
		}
#endif

		OSTimeDlyHMSM(0,0,0,500);// 500ms
		OSTimeDlyHMSM(0,0,0,500);// 500ms
	}
}

//监视任务
void higher_task(void *pdata)
{
	while (1) {
		sim7500e_communication_loop(0,NULL,NULL);
    	OSTimeDlyHMSM(0,0,0,500);// 500ms
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
 	// LED1=!LED1;
 	while(t<5)
	{
		t++;
		LED2=!LED2;
		//BEEP=!BEEP;
		for(i=0;i<0X1FFFFF;i++);
 	}
}
