#include "exti.h"
#include "timer.h" 
#include "led.h" 
#include "key.h"
//#include "beep.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//�ⲿ�ж� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//�ⲿ�ж�0�������
void EXTI0_IRQHandler(void)
{
//	delay_us(10);	//����
//	if(WK_UP==1)	 
//	{
//		BEEP=!BEEP; //��������ת 
//	}		 
//	 EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ 
}	
//�ⲿ�ж�2�������
void EXTI2_IRQHandler(void)
{
//	delay_ms(10);	//����
//	if(KEY2==0)	  
//	{				 
//   LED0=!LED0; 
//	}		 
//	 EXTI_ClearITPendingBit(EXTI_Line2);//���LINE2�ϵ��жϱ�־λ 
}
//�ⲿ�ж�3�������
void EXTI3_IRQHandler(void)
{
//	delay_ms(10);	//����
//	if(KEY1==0)	 
//	{
//		LED1=!LED1;
//	}		 
//	 EXTI_ClearITPendingBit(EXTI_Line3);  //���LINE3�ϵ��жϱ�־λ  
}
//�ⲿ�ж�4�������
void EXTI4_IRQHandler(void)
{
//	delay_ms(10);	//����
//	if(KEY0==0)	 
//	{				 
//		LED0=!LED0;	
//		LED1=!LED1;	
//	}		 
//	 EXTI_ClearITPendingBit(EXTI_Line4);//���LINE4�ϵ��жϱ�־λ  
}
extern int exp_num;
extern int total_ms;
void EXTI9_5_IRQHandler(void)
{
	int cap=0;
	int exp=0;


	//����һ������,��ȡ��ǰ����ֵ�������������Ȼ���������¿�ʼ������һ������
	cap=TIM_GetCounter(TIM4);
	exp=exp_num;
	
	exp_num=0;
	TIM_SetCounter(TIM4,0);
	//�����ٶ�
	total_ms=cap/10 + 500*exp;//��ǰ��ȡ�ļ���ֵ+���ʱ��=��������

	EXTI_ClearITPendingBit(EXTI_Line6);
}

//�ⲿ�жϳ�ʼ������
//��ʼ��PE2~4,PA0Ϊ�ж�����.
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	KEY_Init(); //������Ӧ��IO�ڳ�ʼ��
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ��SYSCFGʱ��
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource6);//PE6 ���ӵ��ж���6
	
  /* ����EXTI_Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;//LINE0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE0
  EXTI_Init(&EXTI_InitStructure);//����
	

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//�ⲿ�ж�0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
  
}












