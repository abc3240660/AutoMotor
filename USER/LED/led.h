#ifndef __LED_H
#define __LED_H
#include "common.h"

////////////////////////////////////////////////////////////////////////////////////	

//LED�˿ڶ���
#define LED0 PEout(3)	 
#define LED1 PEout(4)	 
#define LED2 PGout(9)	  
#define LEDX PFout(11)

#define PA15 PAout(15)	  

//��������
void LED_Init(void);//��ʼ��	

#endif