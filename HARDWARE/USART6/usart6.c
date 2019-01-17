#include "sys.h"
#include "usart6.h"	  
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"
#include "timer.h"
#include "ucos_ii.h"
#include "blue.h"
u8 receive_str[UART6_REC_NUM];     //接收缓存数组,最大USART_REC_LEN个字节 
u8 uart_byte_count=0;
u8 APP_mode=0; 
void usart6_init(u32 bound)
{   
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOB时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE); //使能USART3时钟 
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6);  //GPIOB10复用为USART6
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); //GPIOB11复用为USART6
	//USART6端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOB10与GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure);          //初始化PB10，PB11
	//USART6 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART6, &USART_InitStructure); //初始化串口1	
	USART_Cmd(USART6, ENABLE);  //使能串口1 
	
	USART_ClearFlag(USART6, USART_FLAG_TC);
	
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);         //开启相关中断
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;      //串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		   //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			   //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	  //根据指定的参数初始化VIC寄存器、

	//RESET pin
	/******************************* 蓝牙IO口初始化******************************/
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;    //GPIOA7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;             //普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       //100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;          
	GPIO_Init(GPIOD,&GPIO_InitStructure);          //初始化 
	
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
	}while (k < strlen); //循环发送,直到发送完毕   
    
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
	
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)  //接收中断 
	{
		if(magic_flag==0)
		{//还未收到头数据
			temp[pos++] =(u8)USART_ReceiveData(USART6);
			if(pos==1)
			{
				pos=0;
				if(temp[0]==(MIAGIC_ID>>8) && temp[1]==(MIAGIC_ID&0xFF))
				{
					magic_flag=1;//已经收到头，后面接收的就是数据
					cdata.data.magic[0] = temp[0];
					cdata.data.magic[1] = temp[1];
					p = (u8 *)(&cdata.data)+2;
					total_len=2;
				}else{
					total_len=0;
				}
			}
		}else{
			//开始接受数据
			*p = (u8)USART_ReceiveData(USART6);
			p++;
			total_len++;
			if((cdata.data.len+7)==total_len)
			{
				//完整的数据包接受完了
				cdata.valid=1;//可以处理数据了
				magic_flag=0;//等待下一个包头
			}
		}
		
		printf("%c",rec_data);
	}
}

