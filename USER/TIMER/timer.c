#include "timer.h"
#include "led.h"
#include "usart1.h"

/*********************************************************************************
************************�������� STM32F407���Ŀ�����******************************
**********************************************************************************
* �ļ�����: timer.c                                                              *
* �ļ�������timer��ʱ                                                            *
* �������ڣ�2017.08.30                                                           *
* ��    ����V1.0                                                                 *
* ��    �ߣ�Clever                                                               *
* ˵    ������ʱ��2��ʱ��LED1��ת�ͷ��������                                    * 
**********************************************************************************
*********************************************************************************/ 

/****************************************************************************
* ��    ��: TIM2_Init(u16 auto_data,u16 fractional)
* ��    �ܣ���ʱ��2��ʼ��
* ��ڲ�����auto_data: �Զ���װֵ
*           fractional: ʱ��Ԥ��Ƶ��
* ���ز�������
* ˵    ������ʱ�����ʱ����㷽��:Tout=((auto_data+1)*(fractional+1))/Ft(us)  Ft��ʱ��ʱ��
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

//��ʱ��2�жϷ�����
int pluse_num=0;
int pluse_num_old=0;
int pluse_num_new=0;
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{

		LED0=!LED0;//LED1��ת
  	
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_CC3|TIM_IT_Update);  //����жϱ�־λ
}
int exp_num=0;
extern int total_ms;
int total_msTtemp=0;
int capture1=0;
int capture2=0;
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//timer���
	{
		exp_num++;
		total_msTtemp=exp_num*1000;
		pluse_num_new = TIM_GetCounter(TIM2);//ÿ1s�ж�ȡ������
		if (pluse_num_old != pluse_num_new) {
			total_ms = 1;
		}
		pluse_num =pluse_num_new - pluse_num_old;
		pluse_num_old = pluse_num_new;
	}
	if(TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)//����1���������¼�
	{
		LED1=!LED1;//LED1��ת
		capture1= TIM_GetCapture3(TIM4)/10;
		total_msTtemp+=capture1;
		total_msTtemp-=capture2;
		capture2 = capture1;
		total_ms=total_msTtemp;
		total_msTtemp=0;
		exp_num=0;
	}
	TIM_ClearITPendingBit(TIM4, TIM_IT_CC3|TIM_IT_Update); //����жϱ�־λ
}


void TIM4_Init(u16 auto_data,u16 fractional)
{
	TIM_ICInitTypeDef  TIM4_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  	//TIM5ʱ��ʹ��    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 	//ʹ��PORTAʱ��	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14; //GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //����
	GPIO_Init(GPIOD,&GPIO_InitStructure); //��ʼ��PA0

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_TIM4); //PA0����λ��ʱ��5
  
	  
	TIM_TimeBaseStructure.TIM_Prescaler=fractional;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=auto_data;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	

	//��ʼ��TIM5���벶�����
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
		
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC3,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
  TIM_Cmd(TIM4,ENABLE ); 	//ʹ�ܶ�ʱ��5

 
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}
