#ifndef __BLUE_H
#define __BLUE_H 
#include "sys.h"
#include "stdio.h"	  

#define MIAGIC_ID		0xA3A4

#define CMD_GET_KEY		0x01
#define CMD_SET_IP		0x20
#define CMD_UNLOCK_CAR	0x21
#define CMD_LOCK_CAR	0x22
#define CMD_FLASH_ON	0x23
#define CMD_RESTART		0x24

#define CMD_ERROR		0x10


#define MAX_DATA_SIZE   128
typedef struct _package_da_{
	u8 magic[2];
	u8 len;
	u8 random;
	u8 key[2];
	u8 cmd;
	u8 data_crc[MAX_DATA_SIZE];
}PACKAGE, *PPACKAGE;

typedef struct _ctrl_data_
{
	u8 valid;
	PACKAGE data;
}CTRL_DATA, *PCTRL_DATA;








void blue_init();
u8 get_random();
u8 construct_package(PACKAGE *pkg, u8 random, u8 key, u8 *data, u8 len, u8 cmd);
u8 encrypt_package(PACKAGE *pkg);
u8 decrypt_package(PACKAGE *pkg);

#endif	   
