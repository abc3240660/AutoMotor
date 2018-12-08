#include "led.h" 

/*********************************************************************************
*********************�������� STM32F407Ӧ�ÿ�����(�����)*************************
**********************************************************************************
* �ļ�����: led.c                                                                *
* �ļ�������LED��ʼ��                                                            *
* �������ڣ�2015.10.03                                                           *
* ��    ����V1.0                                                                 *
* ��    �ߣ�Clever                                                               *
* ˵    ����LED��ӦIO�ڳ�ʼ��                                                    * 
**********************************************************************************
*********************************************************************************/

//LED��ӦIO��ʼ��
void LED_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOGʱ��

  //PG13��PG14��PG15��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;         //LED0��LED1��LED2��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                  //��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                 //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;             //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                   //����
  GPIO_Init(GPIOE, &GPIO_InitStructure);                         //��ʼ��GPIO
	
	GPIO_SetBits(GPIOE, GPIO_Pin_3 | GPIO_Pin_4);     

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;         //LED0��LED1��LED2��ӦIO��
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                  //��ͨ���ģʽ
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                 //�������
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;             //100MHz
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                   //����
  GPIO_Init(GPIOF, &GPIO_InitStructure);                         //��ʼ��GPIO
	
	GPIO_SetBits(GPIOF, GPIO_Pin_11);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOG, &GPIO_InitStructure); 
	
  GPIO_SetBits(GPIOG, GPIO_Pin_9);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
  GPIO_SetBits(GPIOA, GPIO_Pin_15);
	//��ʼ��PA15�����������
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
  //PA15��ʼ������
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;         //LED0��LED1��LED2��ӦIO��
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                  //��ͨ���ģʽ
  //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                 //�������
  ///\GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;             //100MHz
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                   //����
  //GPIO_Init(GPIOA, &GPIO_InitStructure);                         //��ʼ��GPIO
	
	//GPIO_SetBits(GPIOA, GPIO_Pin_15);     
	
}





