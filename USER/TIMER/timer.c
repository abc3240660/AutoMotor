#include "timer.h"
#include "led.h"
#include "usart1.h"

/*********************************************************************************
************************启明欣欣 STM32F407核心开发板******************************
**********************************************************************************
* 文件名称: timer.c                                                              *
* 文件简述：timer定时                                                            *
* 创建日期：2017.08.30                                                           *
* 版    本：V1.0                                                                 *
* 作    者：Clever                                                               *
* 说    明：定时器2定时到LED1翻转和蜂鸣器响闭                                    * 
**********************************************************************************
*********************************************************************************/ 

/****************************************************************************
* 名    称: TIM2_Init(u16 auto_data,u16 fractional)
* 功    能：定时器2初始化
* 入口参数：auto_data: 自动重装值
*           fractional: 时钟预分频数
* 返回参数：无
* 说    明：定时器溢出时间计算方法:Tout=((auto_data+1)*(fractional+1))/Ft(us)  Ft定时器时钟
****************************************************************************/
void TIM2_Init(u16 auto_data,u16 fractional)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
        TIM_ICInitTypeDef TIM_ICInitStructure;
        
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);        //GPIOA????
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);                //TIM2????
        
        GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;        //????
        GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;        //
        GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
        GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;        //
        GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
        GPIO_Init(GPIOA,&GPIO_InitStructure);                //???GPIOA_0
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM2);
	
        TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
        TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
        TIM_TimeBaseInitStructure.TIM_Period=0xFFFFFFFF;
        TIM_TimeBaseInitStructure.TIM_Prescaler=0x00;
        TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
        TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure); //?????????        
        
        TIM_ITRxExternalClockConfig(TIM2,TIM_TS_ETRF);        //??????ETRF
        TIM_ETRClockMode2Config(TIM2,TIM_ExtTRGPSC_OFF,TIM_ExtTRGPolarity_NonInverted,7);        //???,???
        
        TIM_SetCounter(TIM2,0);
        TIM_Cmd(TIM2,ENABLE);
}

//定时器2中断服务函数
int pluse_num=0;
int pluse_num_old=0;
int pluse_num_new=0;
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{

		LED0=!LED0;//LED1翻转
  	
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_CC3|TIM_IT_Update);  //清除中断标志位
}
int exp_num=0;
extern int total_ms;
int total_msTtemp=0;
int capture1=0;
int capture2=0;
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//timer溢出
	{
		exp_num++;
		total_msTtemp=exp_num*1000;
		pluse_num_new = TIM_GetCounter(TIM2);//每1s中读取脉冲数
		if (pluse_num_old != pluse_num_new) {
			total_ms = 1;
		}
		pluse_num =pluse_num_new - pluse_num_old;
		pluse_num_old = pluse_num_new;
	}
	if(TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)//捕获1发生捕获事件
	{
		LED1=!LED1;//LED1翻转
		capture1= TIM_GetCapture3(TIM4)/10;
		total_msTtemp+=capture1;
		total_msTtemp-=capture2;
		capture2 = capture1;
		total_ms=total_msTtemp;
		total_msTtemp=0;
		exp_num=0;
	}
	TIM_ClearITPendingBit(TIM4, TIM_IT_CC3|TIM_IT_Update); //清除中断标志位
}


void TIM4_Init(u16 auto_data,u16 fractional)
{
	TIM_ICInitTypeDef  TIM4_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  	//TIM5时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 	//使能PORTA时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14; //GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //下拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化PA0

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_TIM4); //PA0复用位定时器5
  
	  
	TIM_TimeBaseStructure.TIM_Prescaler=fractional;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=auto_data;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	

	//初始化TIM5输入捕获参数
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
		
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC3,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
  TIM_Cmd(TIM4,ENABLE ); 	//使能定时器5

 
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
}
