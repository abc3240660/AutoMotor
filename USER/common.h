#ifndef __COMMON_H
#define __COMMON_H 	
#include "sys.h"
#include "includes.h"

//Ӳ��ƽ̨��Ӳ���汾����	   	
#define HARDWARE_VERSION	   		15		//Ӳ���汾,�Ŵ�10��,��1.0��ʾΪ10
#define SOFTWARE_VERSION	    	200		//����汾,�Ŵ�100��,��1.00,��ʾΪ100

// For VS1003 to Suport Double HW Board
#define QMXX_STM32 1

// Ex SPI Flash
#define ENV_SECTOR_INDEX	0

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif 

//��ֵ����
#define	app_pi	3.1415926535897932384626433832795 
/////////////////////////////////////////////////////////////////////////

typedef struct {
	u16 charge_times;
} SYS_ENV;

extern SYS_ENV g_sys_env;


void sys_env_init(void);
void sys_env_save(void);
void sys_env_dump(void);
	
#endif




























































