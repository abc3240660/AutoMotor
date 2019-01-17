#include "sys.h"
#include "usart6.h"	  
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"
#include "timer.h"
#include "ucos_ii.h"
#include "blue.h"
u8 receive_str[UART6_REC_NUM];     //���ջ�������,���USART_REC_LEN���ֽ� 
u8 uart_byte_count=0;
u8 APP_mode=0; 
void usart6_init(u32 bound)
{   
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOBʱ�� 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE); //ʹ��USART3ʱ�� 
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6);  //GPIOB10����ΪUSART6
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); //GPIOB11����ΪUSART6
	//USART6�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOB10��GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //����
	GPIO_Init(GPIOC,&GPIO_InitStructure);          //��ʼ��PB10��PB11
	//USART6 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART6, &USART_InitStructure); //��ʼ������1	
	USART_Cmd(USART6, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(USART6, USART_FLAG_TC);
	
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);         //��������ж�
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;      //����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		   //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			   //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	  //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

	//RESET pin
	/******************************* ����IO�ڳ�ʼ��******************************/
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;    //GPIOA7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;             //��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       //100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;          
	GPIO_Init(GPIOD,&GPIO_InitStructure);          //��ʼ�� 
	
	PDout(0) = 0;
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	PDout(0) = 1;
}
void uart6SendChar(u8 ch)
{      
	while((USART6->SR&0x40)==0);  
    USART6->DR = (u8) ch;      
}
void uart6SendChars(u8 *str, u16 strlen)
{ 
	u16 k= 0 ; 
	do{
		uart6SendChar(*(str + k)); k++; 
	}while (k < strlen); //ѭ������,ֱ���������   
    
} 
extern CTRL_DATA cdata;
u16 magic_data=0;
u8 magic_flag=0;
u8 temp[2];
u8 pos=0;
u8 total_len;
void USART6_IRQHandler(void)
{
	u8 rec_data;
	u8 *p;
	
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)  //�����ж� 
	{
		if(magic_flag==0)
		{//��δ�յ�ͷ����
			temp[pos++] =(u8)USART_ReceiveData(USART6);
			if(pos==1)
			{
				pos=0;
				if(temp[0]==(MIAGIC_ID>>8) && temp[1]==(MIAGIC_ID&0xFF))
				{
					magic_flag=1;//�Ѿ��յ�ͷ��������յľ�������
					cdata.data.magic[0] = temp[0];
					cdata.data.magic[1] = temp[1];
					p = (u8 *)(&cdata.data)+2;
					total_len=2;
				}else{
					total_len=0;
				}
			}
		}else{
			//��ʼ��������
			*p = (u8)USART_ReceiveData(USART6);
			p++;
			total_len++;
			if((cdata.data.len+7)==total_len)
			{
				//���������ݰ���������
				cdata.valid=1;//���Դ���������
				magic_flag=0;//�ȴ���һ����ͷ
			}
		}
		
		printf("%c",rec_data);
	}
}

