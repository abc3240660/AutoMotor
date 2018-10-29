#include "usart6.h"
#include "string.h"
#include "stdlib.h"  
#include "led.h" 
//#include "beep.h" 

/*********************************************************************************
*************************�������� STM32F407���Ŀ�����*****************************
**********************************************************************************
* �ļ�����: usart6.c                                                             *
* �ļ�������USART6ʹ��                                                           *
* �������ڣ�2015.03.06                                                           *
* ��    ����V1.0                                                                 *
* ��    �ߣ�Clever                                                               *
* ˵    �������ô��ڵ������־���USART1����LED��������������                    * 
**********************************************************************************
*********************************************************************************/	

u8 receive6_str[USART6_REC_NUM];     //���ջ�������,���USART_REC_LEN���ֽ� 
u8 uart6_byte_count=0;

/****************************************************************************
* ��    ��: void uart1_init(u32 bound)
* ��    �ܣ�USART6��ʼ��
* ��ڲ�����bound��������
* ���ز�������
* ˵    ���� 
****************************************************************************/
void uart6_init(u32 bound)
{   //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE);//ʹ��USART1ʱ�� 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6);  //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); //GPIOA10����ΪUSART1
	//USART1�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //����
	GPIO_Init(GPIOC,&GPIO_InitStructure);          //��ʼ��PA9��PA10
   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(USART6, &USART_InitStructure); //��ʼ������1	
  USART_Cmd(USART6, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(USART6, USART_FLAG_TC);
	
	//USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);         //��������ж�
	//Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;      //����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		   //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			   //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	  //����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}

//����6����һ���ַ�
void uart6SendChar(u8 ch)
{      
	while((USART6->SR&0x40)==0);  
    USART6->DR = (u8) ch;      
}

/****************************************************************************
* ��    ��: int fputc(int ch, FILE *f)
* ��    �ܣ��ض�����printf���������  
* ��ڲ�����
* ���ز�����
* ˵    ������printf()֮��ĺ�����ʹ���˰�����ģʽ��ʹ�ñ�׼��ᵼ�³����޷�
            ����,�����ǽ������:ʹ��΢��,��Ϊʹ��΢��Ļ�,����ʹ�ð�����ģʽ. 
            ���ڹ������Եġ�Target��-����Code Generation���й�ѡ��Use MicroLIB����
            ���Ժ�Ϳ���ʹ��printf��sprintf������  
****************************************************************************/
int fputc6(int ch, FILE *f)   //�ض�����printf���������  
{
    uart6SendChar(ch);
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	
    return ch;
}

/****************************************************************************
* ��    ��: void uart1SendChars(u8 *str, u16 strlen)
* ��    �ܣ�����1����һ�ַ���
* ��ڲ�����*str�����͵��ַ���
            strlen���ַ�������
* ���ز�������
* ˵    ���� 
****************************************************************************/
void uart6SendChars(u8 *str, u16 strlen)
{ 
	  u16 k= 0 ; 
   do { uart6SendChar(*(str + k)); k++; }   //ѭ������,ֱ���������   
    while (k < strlen); 
} 

//����1�жϷ������
void USART6_IRQHandler(void)  
{
#if 0
	u8 rec_data;
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)  //�����ж� 
		{
				rec_data =(u8)USART_ReceiveData(USART6);         //(USART1->DR) ��ȡ���յ�������
        if(rec_data=='S')		  	                         //�����S����ʾ��������Ϣ����ʼλ
				{
					uart6_byte_count=0x01; 
				}

			else if(rec_data=='E')		                         //���E����ʾ��������Ϣ���͵Ľ���λ
				{
//					if(strcmp("Light_led1",(char *)receive6_str)==0)        LED1=0;	//����LED1
//					else if(strcmp("Close_led1",(char *)receive6_str)==0)   LED1=1;	//����LED1
//					else if(strcmp("Open_beep",(char *)receive6_str)==0)    BEEP=1; 	//��������
//					else if(strcmp("Close_beep",(char *)receive6_str)==0)   BEEP=0; 	//����������
					
					for(uart6_byte_count=0;uart6_byte_count<32;uart6_byte_count++)receive6_str[uart6_byte_count]=0x00;
					uart6_byte_count=0;    
				}				  
			else if((uart6_byte_count>0)&&(uart6_byte_count<=USART6_REC_NUM))
				{
				   receive6_str[uart6_byte_count-1]=rec_data;
				   uart6_byte_count++;
				}                		 
   } 
#endif
} 
