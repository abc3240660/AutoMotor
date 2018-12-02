#ifndef __LED_H
#define __LED_H
#include "common.h"

////////////////////////////////////////////////////////////////////////////////////	

//LED端口定义
#define LED0 PEout(3)	 
#define LED1 PEout(4)	 
#define LED2 PGout(9)	  
#define LEDX PFout(11)

#define PA15 PAout(15)	  

//函数声明
void LED_Init(void);//初始化	

#endif
