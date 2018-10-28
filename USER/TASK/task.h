#ifndef __TASK_H
#define __TASK_H
#include "common.h"
#include "includes.h"

////////////////////////////////////////////////////////////////////////////////////

/***************************UCOSII��������**************************************/

//LED0����

#define LED0_TASK_PRIO       			8   //�����������ȼ�

#define LED0_STK_SIZE  		    		64  //���������ջ��С

extern OS_STK  LED0_TASK_STK[LED0_STK_SIZE];  //�����ջ	

void LED0_Task(void *pdata);          //������


//LED1����

#define LED1_TASK_PRIO       		5    //�����������ȼ�

#define LED1_STK_SIZE  					64   //���������ջ��С


extern OS_STK  LED1_TASK_STK[LED1_STK_SIZE];  //�����ջ

void LED1_Task(void *pdata);         //������


//��������

#define SPEED_TASK_PRIO       		4    //�����������ȼ�

#define SPEED_STK_SIZE  					512   //���������ջ��С

extern OS_STK  SPEED_TASK_STK[SPEED_STK_SIZE];  //�����ջ
void SPEED_CAL_Task(void *pdata);
#endif



