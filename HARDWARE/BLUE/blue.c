#include "sys.h"
#include "blue.h"	  
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"
#include "timer.h"
#include "ucos_ii.h"
#include "usart6.h"
u8 key;
CTRL_DATA cdata;
void blue_init()
{
    uart6SendChars("AT+BAUD\r\n", sizeof("AT+BAUD\r\n"));
	uart6SendChars("AT+TYPE\r\n", sizeof("AT+TYPE\r\n"));
	uart6SendChars("AT+PIN\r\n", sizeof("AT+PIN\r\n"));
	uart6SendChars("AT+NAMEMobit\r\n", sizeof("AT+NAMEMobit\r\n"));
	uart6SendChars("AT+POWE3\r\n", sizeof("AT+POWE3\r\n"));
}
u8 process_data()
{
	if(cdata.valid != 1)
		return 1;//还没有接受完整的包
	switch(cdata.data.cmd)
	{
		case CMD_GET_KEY:
			
			key = get_random();
			break;
		case CMD_SET_IP:
			break;
		case CMD_UNLOCK_CAR:
			break;
		case CMD_LOCK_CAR:
			break;
		case CMD_FLASH_ON:
			break;
		case CMD_RESTART:
			break;
		default:
			break;
			
	}
}
u8 get_random()
{
	return 1;
}

u8 construct_package(PACKAGE *pkg, u8 random, u8 key, u8 *data, u8 len, u8 cmd)
{
	return 0;
}
u8 encrypt_package(PACKAGE *pkg)
{
	return 0;
}
u8 decrypt_package(PACKAGE *pkg)
{
	return 0;
}