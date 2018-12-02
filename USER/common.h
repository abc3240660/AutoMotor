#ifndef __COMMON_H
#define __COMMON_H 	
#include "sys.h"
//#include "touch.h"	 
#include "includes.h"
//#include "gui.h"
//#include "wm8978.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//APP通用 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/2/16
//版本：V1.2
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//V1.1 20140216
//新增对各种分辨率LCD的支持.
//V1.2 20140727
//修改app_show_float函数的一个bug
////////////////////////////////////////////////////////////////////////////////// 	   

//硬件平台软硬件版本定义	   	
#define HARDWARE_VERSION	   		15		//硬件版本,放大10倍,如1.0表示为10
#define SOFTWARE_VERSION	    	200		//软件版本,放大100倍,如1.00,表示为100

//系统数据保存基址			  
#define SYSTEM_PARA_SAVE_BASE 		100		//系统信息保存首地址.从100开始.

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif 

//π值定义
#define	app_pi	3.1415926535897932384626433832795 
/////////////////////////////////////////////////////////////////////////

#endif




























































