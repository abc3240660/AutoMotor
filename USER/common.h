#ifndef __COMMON_H
#define __COMMON_H 	
#include "sys.h"
#include "includes.h"

//硬件平台软硬件版本定义	   	
#define HARDWARE_VERSION	   		15		//硬件版本,放大10倍,如1.0表示为10
#define SOFTWARE_VERSION	    	200		//软件版本,放大100倍,如1.00,表示为100

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

//π值定义
#define	app_pi	3.1415926535897932384626433832795 
/////////////////////////////////////////////////////////////////////////

typedef struct {
	u16 active_flag;
	u16 charge_times;
	u32 total_meters;
	u8 svr_ip[32];
	u8 svr_port[8];
	u8 svr_apn[32];
} SYS_ENV;

void sys_env_init(void);
void sys_env_save(void);
void sys_env_dump(void);
	
#endif




























































