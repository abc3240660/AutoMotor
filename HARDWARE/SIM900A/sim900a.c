#include "sim900a.h" 
#include "delay.h"	
#include "led.h"     
#include "w25qxx.h"  
#include "malloc.h"
#include "string.h"
#include "usart3.h" 
#include "ff.h" 
#include "ucos_ii.h" 
#include "can1.h"
#include "rfid.h"

// BIT7: 0-idle, 1-changed
// BIT0: 0-Close, 1-Open
u8 g_door_sta = 0;

u8 g_iap_update = 0;
u8 g_iap_update_name[128] = "";

u8 g_mp3_play = 0;
u8 g_mp3_name_play[32] = "";

u8 g_mp3_update = 0;
u8 g_mp3_name_update[128] = "";

// BIT7: 0-idle, 1-changed
// BIT0: 0-Start, 1-Stop
u8 g_charge_sta = 0;

u8 g_invaid_move = 0;

// BT(if no BT, use SIM)
u8 g_mac_addr[32] = "";
u8 g_iccid_sim[32] = "";
u8 g_sw_version[32] = "";

extern u8 g_bms_charging;
extern u8 g_bms_temp_max;// 30
extern u8 g_bms_temp_min;// 30
extern u8 g_bms_vol_percent;// 50%
extern u8 g_bms_charged_times;// Save into ExFlash

u8 dat1[32] = "";
u8 dat2[32] = "";
u8 dat3[32] = "";
u8 dat4[32] = "";
u8 dat5[32] = "";
u8 USART3_RX_BUF_BAK[USART3_MAX_RECV_LEN];

void SoftReset(void);
void write_logs(char *module, char *log, u16 size, u8 mode);

__sim7500dev sim7500dev;	//sim7500������

const char* cmd_list[] = {
	CMD_DEV_REGISTER,
	CMD_HEART_BEAT,
	CMD_QUERY_PARAMS,
	CMD_RING_ALARM,
	CMD_OPEN_DOOR,
	CMD_DOOR_CLOSED,
	CMD_DOOR_OPENED,
	CMD_JUMP_LAMP,
	CMD_CALYPSO_UPLOAD,
	CMD_ENGINE_START,
	CMD_INVALID_MOVE,
	CMD_REPORT_GPS,
	CMD_IAP_SUCCESS,
	CMD_CHARGE_STARTED,
	CMD_CHARGE_STOPED,
	CMD_DEV_SHUTDOWN,
	CMD_QUERY_GPS,
	CMD_IAP_UPGRADE,
	CMD_MP3_UPDATE,	
	CMD_MP3_PLAY,
	CMD_START_TRACE,
	CMD_STOP_TRACE,
	CMD_QUERY_BMS,
	CMD_QUERY_MP3,
	NULL
};

const char* ext_ack_list[] = {
	"+NETCLOSE:",
	"+NETOPEN: 0",
	"+CIPOPEN: 0,0",
	NULL
};

enum ACK_MSG_TYPE {
	NET_CLOSE_OK = 0,
	NET_OPEN_OK,
	TCP_CON_OK,
	UNKNOWN_ACK
};

char send_buf[LEN_MAX_SEND] = "";
char recv_buf[LEN_MAX_RECV] = "";

char sync_sys_time[LEN_SYS_TIME+1] = "";

u8 tcp_net_ok = 0;

int power_state = 0;
int door_state = 0;
int ring_times = 0;
int lamp_times = 0;
int lock_state = 0;
int hbeat_time = 6;// default 6s
int gps_report_gap = 20;// default 10s
char bat_vol[LEN_BAT_VOL] = "88";// defaut is fake
char imei[LEN_IMEI_NO] = "88888888";// defaut is fake
char rssi[LEN_RSSI_VAL] = "88";// defaut is fake
char dev_time[LEN_SYS_TIME] = "20181105151955";// defaut is fake

u8 g_longitude[32] = "";
u8 g_latitude[32] = "";
u8 g_imei_str[32] = "";

u8 sim7500e_get_cmd_count()
{
	u8 cnt = 0;
	while(1) {
		if (NULL == cmd_list[cnt]) {
			break;
		}
		cnt++;
	}

	return cnt;
}

u8 sim7500e_is_supported_cmd(u8 cnt, char* str)
{
	u8 i = 0;

	for (i=0; i<cnt; i++) {
		if (0 == strncmp(str, cmd_list[i], strlen(cmd_list[i]))) {
			break;
		}
	}

	if (i != UNKNOWN_CMD) {
		printf("Recved CMD/ACK %s\n", str);
	}

	return i;
}

char* sim7500e_connect_check(void)
{
	char *strx=0;
	strx=strstr((const char*)USART3_RX_BUF,(const char*)"CONNECT OK");
	if (NULL == strx) {
		strx=strstr((const char*)USART3_RX_BUF,(const char*)"ALREADY CONNECT");
		if (NULL == strx) {
			strx=strstr((const char*)USART3_RX_BUF,(const char*)"CONNECT FAIL");
			if (NULL == strx) {
				strx=strstr((const char*)USART3_RX_BUF,(const char*)"ERROR");
				if (strx != NULL) {
					sim7500dev.tcp_status = 2;// Connect Failed/Error
				} else {
					// Unknown Recved MSG
				}
			} else {
				sim7500dev.tcp_status = 2;// Connect Failed/Error
			}
		} else {
			sim7500dev.tcp_status = 1;// Connect OK
		}
	} else {
		sim7500dev.tcp_status = 1;// Connect Failed/Error
	}
	
	return strx;
}

u8 sim7500e_imei_check(void)
{
	u8 i = 0;
	
	memset(g_imei_str, 0, 16);
	while(1) {
		if ((USART3_RX_BUF_BAK[2+i]>='0') && (USART3_RX_BUF_BAK[2+i]<='9')) {// 2:\r\n
			g_imei_str[i] = USART3_RX_BUF_BAK[2+i];
		} else {
			if ((USART3_RX_BUF_BAK[2+i]!='\r') && (USART3_RX_BUF_BAK[2+i]!='\n')) {
				break;
			}
		}
		i++;
	}
	
	printf("IMEI = %s\n", g_imei_str);
	
	if (strlen((const char*)g_imei_str) >= 10) {
		return 1;// OK
	} else {
		return 0;// NG
	}
}

u8 sim7500e_gps_check(void)
{
	sscanf((const char*)USART3_RX_BUF_BAK, "%[^,],%[^,],%[^,],%[^,],%[^,]", dat1,dat2,dat3,dat4,dat5);
	
	// �����;SIM7000E�ϵ���������ô���л��Ե�
	// �ж�GPS PWR�Ƿ���Ϊ�쳣�ϵ絼�±��ر���
	// Exception Process
	
	printf("GPS Latitude(%s), Longitude(%s)\n", dat4, dat5);
	
	return 1;// OK
}

//sim900a���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim7500e_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����

		write_logs("SIM7000E", (char*)USART3_RX_BUF, USART3_RX_STA&0X7FFF, 0);
		
		if (0 == strcmp((const char*)str, "CONNECT")) {
			strx=sim7500e_connect_check();
		} else {
			strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
		}
		
		memcpy(USART3_RX_BUF_BAK, USART3_RX_BUF, USART3_MAX_RECV_LEN);
		USART3_RX_STA=0;
	}
	
	return (u8*)strx;
}
//��sim900a��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:50ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,�յ���Ԥ�ڽ��
//       2,û�յ��κλظ�
u8 sim7500e_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0;  
	
	if(USART3_RX_STA&0X8000) {
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//��ӽ����� 
		write_logs("SIM7000E", (char*)USART3_RX_BUF, USART3_RX_STA&0X7FFF, 0);
	}
	USART3_RX_STA=0;
	
	if (0 == strcmp((const char*)cmd, "AT+CIPSTART")) {
		sim7500dev.tcp_status=0;// IDLE
	}
	
	printf("SIM7000E Send Data %s\n", cmd);
	write_logs("SIM7000E", (char*)cmd, strlen((char*)cmd), 1);
	
	sim7500dev.cmdon=1;//����ָ��ȴ�״̬
	if((u32)cmd<=0XFF)
	{   
		while((USART3->SR&0X40)==0);//�ȴ���һ�����ݷ������  
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//�Ƿ���յ��ڴ���Ӧ����
			{
				if(sim7500e_check_cmd(ack)) {
					res=0;//�յ��ڴ��Ľ����
					break; 
				} else {
					res=1;//�����ڴ��Ľ��
				}
			}
			delay_ms(40);
		}
		if(waittime==0)res=2; 
	}
	return res;
}

void sim7500e_tcp_send(char* send)
{
	if(sim7500e_send_cmd("AT+CIPSEND",">",40)==0)//��������
	{
		sim7500e_send_cmd((u8*)send,0,500);	//��������:0X00  
		delay_ms(20);						//�������ʱ
		sim7500e_send_cmd((u8*)0X1A,0,0);	//CTRL+Z,�������ݷ���,����һ�δ���	
	}else sim7500e_send_cmd((u8*)0X1B,0,0);	//ESC,ȡ������ 	
}

// DEV ACK
void sim7500e_do_engine_start_ack(char* send)
{
	CAN1_StartEngine();

	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%d$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_ENGINE_START, power_state);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_open_door_ack(char* send)
{
	CAN1_OpenDoor();

	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%d$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_OPEN_DOOR, door_state);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_jump_lamp_ack(char* send)
{
	CAN1_JumpLamp();

	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_JUMP_LAMP);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_ring_alarm_ack(char* send)
{
	CAN1_RingAlarm();

	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_RING_ALARM);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_query_params_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_QUERY_PARAMS, g_mac_addr, g_iccid_sim);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_start_trace_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_START_TRACE);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_stop_trace_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_STOP_TRACE);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_dev_shutdown_ack(char* send)
{
	// do something power saving
	// let sim7000e goto sleep

#if 0
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_DEV_SHUTDOWN);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
#endif
}

// DEV ACK
void sim7500e_do_query_gps_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);

	if (strlen((const char*)g_longitude) > 5) {
		sprintf(send, "%s,%s,%s,%s,%s,%s,%s,0$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_QUERY_GPS, g_longitude, g_latitude);
	} else {
		sprintf(send, "%s,%s,%s,%s,%s,F,F,0$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_QUERY_GPS);
	}

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_iap_upgrade_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_IAP_UPGRADE);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);

	// Must stop other send/recv first
	// Do http get and Flash
	// Do SoftReset
}

// DEV ACK
void sim7500e_do_mp3_play_ack(char* send)
{
	FIL f_txt;
	char filename[64] = "";

	memset(send, 0, LEN_MAX_SEND);
	sprintf(filename, "0:/MUSIC/%s", g_mp3_name_play);

	if (0 == f_open(&f_txt,(const TCHAR*)filename, FA_READ)) {// existing
		g_mp3_play = 1;
		sprintf(send, "%s,%s,%s,%s,%s,%s,1$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_MP3_PLAY, g_mp3_name_play);
		// Play mp3 in other task
	} else {// file non-existing
		g_mp3_play = 0;
		sprintf(send, "%s,%s,%s,%s,%s,%s,0$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_MP3_PLAY, g_mp3_name_play);
	}

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_query_bms_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%d,%d,%d$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_QUERY_BMS, g_bms_vol_percent, g_bms_charged_times, g_bms_temp_max);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV ACK
void sim7500e_do_mp3_dw_success_ack(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_ACK, CMD_MP3_UPDATE, g_mp3_name_update);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto Send
u8 sim7500e_do_dev_register_auto(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DEV_REGISTER, HW_VERSION, SW_VERSION, bat_vol);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
	
	return 0;
}

// DEV Auto Send
void sim7500e_do_heart_beat_auto(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%d,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_HEART_BEAT, dev_time, lock_state, rssi, bat_vol);
	
	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_door_closed_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DOOR_CLOSED);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_door_opened_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_DOOR_OPENED);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_invalid_moving_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_INVALID_MOVE);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_gps_location_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);

	if (strlen((const char*)g_longitude) > 5) {
		sprintf(send, "%s,%s,%s,%s,%s,%s,0$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_REPORT_GPS, g_longitude, g_latitude);
	} else {
		sprintf(send, "%s,%s,%s,%s,F,F,0$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_REPORT_GPS);
	}

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_iap_success_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_IAP_SUCCESS, g_sw_version);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_charge_start_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%d$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_CHARGE_STARTED, (char*)g_bms_vol_percent, g_bms_charged_times);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_charge_stop_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s,%d$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_CHARGE_STOPED, (char*)g_bms_vol_percent, g_bms_charged_times);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

// DEV Auto SEND
void sim7500e_do_calypso_report(char* send)
{
	memset(send, 0, LEN_MAX_SEND);
	sprintf(send, "%s,%s,%s,%s,%s$", PROTOCOL_HEAD, DEV_TAG, imei, CMD_CALYPSO_UPLOAD, calypso_card_id);

	printf("SEND:%s\n", send);
	
	sim7500e_tcp_send(send);
}

void sim7500e_parse_msg(char* msg, char* send)
{
	int index = 0;
	int data_pos = 0;
	char delims[] = ",";
	char* split_str = NULL;

	enum CMD_TYPE cmd_type = UNKNOWN_CMD;

	int cmd_count = sim7500e_get_cmd_count();

#ifdef DEBUG_USE
	//printf("Support %d CMDs\n", cmd_count);
#endif

	split_str = strtok(msg, delims);
	while(split_str != NULL) {
#ifdef DEBUG_USE
		//printf("split_str = %s\n", split_str);
#endif
		// index = 3: SVR CMD
		// index = 4: SVR ACK
		if ((3 == index) || (4 == index)) {
			if (UNKNOWN_CMD == cmd_type) {
				cmd_type = (enum CMD_TYPE)sim7500e_is_supported_cmd(cmd_count, split_str);

				if (cmd_type != UNKNOWN_CMD) {
					if (0 == data_pos) {
						data_pos = index;
						printf("data_pos = %d, cmd_type = %d\n", data_pos, cmd_type);
					}
					
					if (OPEN_DOOR == cmd_type) {
						sim7500e_do_open_door_ack(send);
					} else if (ENGINE_START == cmd_type) {
						sim7500e_do_engine_start_ack(send);
					}
				} else {
					// TBD
				}
			}
		}

		if (index > data_pos) {
			if (DEV_REGISTER == cmd_type) {
				if (5 == index) {
					strncpy(sync_sys_time, split_str, LEN_SYS_TIME);
					sync_sys_time[LEN_SYS_TIME] = '\0';
					printf("sync_sys_time = %s\n", sync_sys_time);
				} else if (6 == index) {
					hbeat_time = atoi(split_str);
					printf("hbeat_time = %d\n", hbeat_time);
				}
			} else if (HEART_BEAT == cmd_type) {
			} else if (QUERY_PARAMS == cmd_type) {
				sim7500e_do_query_params_ack(send);
			} else if (DEV_SHUTDOWN == cmd_type) {
				sim7500e_do_dev_shutdown_ack(send);
			} else if (QUERY_GPS == cmd_type) {
				sim7500e_do_query_gps_ack(send);
			} else if (IAP_UPGRADE == cmd_type) {
				g_iap_update = 1;
				strcpy((char*)g_iap_update_name, split_str);
				sim7500e_do_iap_upgrade_ack(send);
			} else if (QUERY_BMS == cmd_type) {
				sim7500e_do_query_bms_ack(send);
			} else if (MP3_PLAY == cmd_type) {
				strcpy((char*)g_mp3_name_play, split_str);
				sim7500e_do_mp3_play_ack(send);
			} else if (MP3_UPDATE == cmd_type) {
				g_mp3_update = 1;
				strcpy((char*)g_mp3_name_update, split_str);
			} else if (START_TRACE == cmd_type) {
				gps_report_gap = atoi(split_str);
				printf("gps_report_gap = %d\n", gps_report_gap);
				sim7500e_do_start_trace_ack(send);
			} else if (STOP_TRACE == cmd_type) {
				gps_report_gap = 0;
				printf("gps_report_gap = %d\n", gps_report_gap);
				sim7500e_do_stop_trace_ack(send);
			} else if (RING_ALARM == cmd_type) {
				ring_times = atoi(split_str);
				printf("ring_times = %d\n", ring_times);
				sim7500e_do_ring_alarm_ack(send);
			} else if (JUMP_LAMP == cmd_type) {
				lamp_times = atoi(split_str);
				printf("lamp_times = %d\n", lamp_times);
				sim7500e_do_jump_lamp_ack(send);
			}
		}
		split_str = strtok(NULL, delims);
		index++;
	};
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
const u8 *modetbl[2]={"TCP","UDP"};
void sim7500e_tcp_connect(u8 mode,u8* ipaddr,u8* port)
{ 
	u8 i = 0;
	u8 *p,*p1,*p2;
	u16 timex=0;
	u8 count=0;
	u16 gps_cnt=0;
	u8 connectsta=0;			//0,��������;1,���ӳɹ�;2,���ӹر�; 
	u8 hbeaterrcnt=0;			//�������������,����5�������ź���Ӧ��,����������
	u8 iap_success = 0;
	u8 oldsta=0XFF;
	p=mymalloc(SRAMIN,100);		//����100�ֽ��ڴ�
	p1=mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�
	
	for (i=0; i<5; i++) {
		if (0 == sim7500e_send_cmd("AT","OK",20))break;
		if (4 == i) SoftReset();
		delay_ms(50);
	}
	
	if(sim7500e_send_cmd("ATE0","OK",40)) {
		if(sim7500e_send_cmd("ATE0","OK",40))SoftReset();// �رջ���
	}

	// Get IMEI
	if(sim7500e_send_cmd("AT+GSN","OK",40)) {
		if(sim7500e_send_cmd("AT+GSN","OK",40))SoftReset();
	}
	sim7500e_imei_check();
	
	// Open GPS
	if(sim7500e_send_cmd("AT+CGNSPWR=1","OK",40)) {
		if(sim7500e_send_cmd("AT+CGNSPWR=1","OK",40))SoftReset();
	}
	
	if(sim7500e_send_cmd("AT+CGATT?","+CGATT: 1",40)) {
		if(sim7500e_send_cmd("AT+CGATT?","+CGATT: 1",40))SoftReset();
	}

	if(sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200)) {
		if(sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200))SoftReset();
	}
	
	if(sim7500e_send_cmd("AT+CSTT=\"CMNET\"","OK",40)) {
		if(sim7500e_send_cmd("AT+CSTT=\"CMNET\"","OK",40))SoftReset();
	}
	
	if(sim7500e_send_cmd("AT+CIICR","OK",200)) {
		if(sim7500e_send_cmd("AT+CIICR","OK",200))SoftReset();
	}
	
	sim7500e_send_cmd("AT+CIFSR",0,40);
	delay_ms(100);
	
	CAN1_JumpLamp();

	LED1 = 1;
	LED2 = 1;
	
	delay_ms(100);

	sim7500dev.tcp_status=0;// IDLE
	
	printf("Start CIPSTART...\n");

	while (1) {
		if (sim7500e_send_cmd("AT+CIPSTART=\"TCP\",\"47.105.222.239\",88", "CONNECT",600)) {// Max 600*50ms = 30s		
			if(sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200)) {
				sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200);
			}
		}
		
		if (1 == sim7500dev.tcp_status) {// Connected OK
			break;
		} else {
			//write_logs("SIM7000E", (char*)"Cannot Setup TCP Connect, just soft restart...\n", strlen((char*)"Cannot Setup TCP Connect, just soft restart...\n"), 3);
			//printf("Cannot Setup TCP Connect, just soft restart...\n");
			// SoftReset();
			// Re-Close->Open Try
			delay_ms(100);
		}
	}
				
	delay_ms(100);
	if(sim7500e_send_cmd("AT+CIPSEND=5",">",40))SoftReset();
	delay_ms(100);
	delay_ms(100);
	
	// �������ͻȻ�յ�һ��TCP����CLOSED����Ϣ
	// ��ʱ���޷��յ�SEND OK��
	if(sim7500e_send_cmd("Hello","SEND OK",200))SoftReset();
	delay_ms(100);
	delay_ms(100);
	if(sim7500e_do_dev_register_auto(send_buf))SoftReset();
	delay_ms(100);
	delay_ms(100);

	//	SoftReset();
	if (iap_success) {
		sim7500e_do_iap_success_report(send_buf);
	}

	while(1)
	{ 
		if((timex%20)==0)
		{
			LED0=!LED0;
			count++;	
			if(connectsta==2||hbeaterrcnt>8)//�����ж���,��������8������û����ȷ���ͳɹ�,����������
			{
				while (1) {//������������
					sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200);
					if (sim7500e_send_cmd("AT+CIPSTART=\"TCP\",\"47.105.222.239\",88", "CONNECT",600)) {// Max 600*50ms = 30s		
						if(sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200)) {
							sim7500e_send_cmd("AT+CIPSHUT","SHUT OK",200);
						}
					}
					
					if (1 == sim7500dev.tcp_status) {// Connected OK
						break;
					} else {
						//write_logs("SIM7000E", (char*)"Cannot Setup TCP Connect, just soft restart...\n", strlen((char*)"Cannot Setup TCP Connect, just soft restart...\n"), 3);
						//printf("Cannot Setup TCP Connect, just soft restart...\n");
						// SoftReset();
						// Re-Close->Open Try
						delay_ms(100);
					}
				}

				connectsta=0;	
 				hbeaterrcnt=0;
			}
		}
		if(connectsta==0&&(timex%200)==0)//���ӻ�û������ʱ��,ÿ2���ѯһ��CIPSTATUS.
		{
			sim7500e_send_cmd("AT+CIPSTATUS","OK",500);	//��ѯ����״̬
			if(strstr((const char*)USART3_RX_BUF_BAK,"CLOSED"))connectsta=2;
			if(strstr((const char*)USART3_RX_BUF_BAK,"CONNECT OK"))connectsta=1;
			connectsta=1;
		}
		if(connectsta==1&&timex>=600)//����������ʱ��,ÿ6�뷢��һ������
		{
			timex=0;
			
			printf("test00\n");
			sim7500e_do_heart_beat_auto(send_buf);
				
			hbeaterrcnt++; 
			printf("hbeaterrcnt:%d\r\n",hbeaterrcnt);//������Դ���
		} 
		delay_ms(10);
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{
			printf("test01\n");
			u8 data_lenth = 0;
			USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//��ӽ����� 
			write_logs("SIM7000E", (char*)USART3_RX_BUF, USART3_RX_STA&0X7FFF, 0);
			
			//printf("RECVED %s",USART3_RX_BUF);				//���͵�����  
			if(hbeaterrcnt)							//��Ҫ�������Ӧ��
			{
				if(strstr((const char*)USART3_RX_BUF,"SEND OK"))hbeaterrcnt=0;//��������
			}
			
			// Received User Data
			p2 = (u8*)strstr((const char*)USART3_RX_BUF, PROTOCOL_HEAD);
			if (p2) {
				data_lenth = strlen((const char*)p2);
				
				memset(recv_buf, 0, LEN_MAX_RECV);
				memcpy(recv_buf, p2, LEN_MAX_RECV);
				
				if (data_lenth < LEN_MAX_RECV) {
					recv_buf[data_lenth] = '\0';// $ -> 0
				}
				
				USART3_RX_STA=0;// Let Interrupt Go On Saving DATA
				
				printf("RECVED MSG(%dB): %s\n", data_lenth, recv_buf);
				
				sim7500e_parse_msg(recv_buf, send_buf);
			} else {
				USART3_RX_STA=0;
			}
		} else {
			if (calypso_card_id[0] != 0) {
				sim7500e_do_calypso_report(send_buf);
				calypso_card_id[0] = 0;
			}
		}
		if(oldsta!=connectsta)
		{
			oldsta=connectsta;
		} 
		timex++; 
		gps_cnt++;
		
		if (0 == (gps_cnt%300)) {
			sim7500e_send_cmd("AT+CGNSINF","OK",40);
			sim7500e_gps_check();
			
			printf("test02\n");
		}
		
		if (30000 == gps_cnt) {
			gps_cnt = 1;
		}
//		if (20000 == gps_cnt) {
//			printf("gps_cnt = 20000 SoftReset\n");
//			SoftReset();
//		}
	}
	
	myfree(SRAMIN,p);
	myfree(SRAMIN,p1);
}
