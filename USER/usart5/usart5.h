#ifndef __USART5_H
#define __USART5_H
#include "stdio.h"	
#include "common.h" 

//////////////////////////////////////////////////////////////////////////////////	 

#define USART5_REC_NUM  			100  	//定义最大接收字节数 200
extern u8 uart_byte_count;          //uart_byte_count要小于USART_REC_LEN
extern u8 receive_str[USART5_REC_NUM];  

void uart5_init(u32 bound);
void uart5SendChars(u8 *str, u16 strlen);

#endif


