#ifndef __SIM7500E_H__
#define __SIM7500E_H__	 
#include "sys.h"

#define SIM7500E_MAX_NEWMSG	10		//最大10条新消息

typedef struct 
{							  
 	u8 status;		//SIM7500EA状态
					//bit7:0,没有找到模块;1,找到模块了
					//bit6:0,SIM卡不正常;1,SIM卡正常
					//bit5:0,未获得运营商名字;1,已获得运营商名字
					//bit4:0,中国移动;1,中国联通
					//bit3:0,TCP Connect NG;1,TCP Connect OK
					//[2:0]:保留
	
	u8 mode;		//当前工作模式
					//0,号码输入模式/短信模式
					//1,拨号中
					//2,通话中
					//3,来电响应中
	
	vu8 cmdon;		//标记是否有指令在发送等待状态
					//0,没有指令在等待回应
					//1,有指令在等待回应
	
	u8 csq;			//信号质量
	
	vu8 newmsg;		//新消息条数,0,没有新消息;其他,新消息条数
	u8 newmsgindex[SIM7500E_MAX_NEWMSG];//新短信在SIM卡内的索引,最长记录SIM7500E_MAX_NEWMSG条新短信
	u8 incallnum[20];//来电号码缓存区,最长20位
}__sim7500dev; 

extern __sim7500dev sim7500dev;	//sim900控制器


#define PROTOCOL_HEAD	"^MOBIT"
#define DEV_TAG			"ECAR"
#define SW_VERSION		"V1.0"
#define HW_VERSION		"V1.0"

#define CMD_DEV_ACK		"Re"// DEV ACK

#define CMD_DEV_REGISTER	"R0"// DEV CMD
#define CMD_HEART_BEAT		"H0"// DEV CMD
#define CMD_INQUIRE_PARAM	"C0"// DEV ACK
#define CMD_RING_ALARM		"R2"// DEV ACK
#define CMD_DOOR_OPEN		"O0"// DEV ACK
#define CMD_DOOR_CLOSE		"C1"// DEV CMD
#define CMD_JUMP_LAMP		"S2"// DEV ACK

#define LEN_SYS_TIME	32
#define LEN_IMEI_NO	32
#define LEN_BAT_VOL	32
#define LEN_RSSI_VAL	32
#define LEN_MAX_SEND	32
#define LEN_MAX_RECV	32

#define DEBUG_USE 1

enum CMD_TYPE {
	DEV_REGISTER = 0,
	HEART_BEAT,
	INQUIRE_PARAM,
	RING_ALARM,
	DOOR_OPEN,
	DOOR_CLOSE,
	JUMP_LAMP,
	UNKNOWN_CMD
};
 
#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8		//高低字节交换宏定义

u8* sim7500e_check_cmd(u8 *str);
u8 sim7500e_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
void sim7500e_cmd_over(void);
u8 sim7500e_chr2hex(u8 chr);
u8 sim7500e_hex2chr(u8 hex);
void sim7500e_unigbk_exchange(u8 *src,u8 *dst,u8 mode);
void sim7500e_cmsgin_check(void);
void sim7500e_status_check(void);
void sim7500e_tcp_connect(u8 mode,u8* ipaddr,u8* port);

#endif/* __SIM7500E_H__ */





