#include "rfid.h"
#include "usart5.h" 
#include "delay.h"

u8 cpr74_recv_size = 0;
u8 cpr74_send_buf[RFID_MAX_BUF_SIZE+1] = "";
u8 calypso_serial_num[SERIAL_NUM_SIZE+1] = "";
u8 calypso_card_id[CARD_ID_SIZE+1] = "";
u8* pGotVal = NULL;

static u16 cpr74_crc16_calc(u8* dat, u8 len)
{
	u8 i = 0;
	u8 j = 0;
	u16 crc = 0xFFFF;

	for (i=0; i<len; i++) {
		crc ^= dat[i];
		for (j=0; j<8; j++) {
			if (crc & 0x01)
				crc = (crc >> 1) ^ 0x8408;
			else
				crc = crc >> 1;
		}
	}

	//printf("crc16 = 0x%.4X\n", crc);

	return crc;
}

static u8 cpr74_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}

static u8 cpr74_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}

static void cpr74_parse_ack(u8 bit_pos, u8 bit_len)
{
	u8 i = 0;
	u8 j = 0;
	u8 offset = 0;
	u8 bit_cnt = 0;
	u8 tmp_val = 0;

	for (i=0; i<(UART5_RX_STA&0X7FFF); i++) {
		for (j=0; j<8; j++) {
			if (bit_pos <= (i*8 + j + 1)) {
				offset = 7 - ((bit_pos+bit_cnt-1)%8);
				tmp_val += ((UART5_RX_BUF[i]>>offset)&0x01) << (3-bit_cnt%4);
				bit_cnt++;
				if (0 == (bit_cnt%4)) {
					if (pGotVal != NULL) {
						if (tmp_val <= 9) {
							pGotVal[bit_cnt/4-1] = (tmp_val - 0x00) + '0';
						} else {
							pGotVal[bit_cnt/4-1] = (tmp_val - 0x0a) + 'A';
						}
					}
					tmp_val = 0;
				}
				if (bit_len == bit_cnt) {
					break;
				}
			}
		}
		if (bit_len == bit_cnt) {
			break;
		}
	}
	
	printf("RFID GET DATA: %s\n", pGotVal);
}

static void cpr74_error_msg(RET_RFID ret)
{
	if (RET_RFID_RECV_DAT_VALID == ret) {
		printf("CPR74 ERR: Header is not 02\n");
	} else if (RET_RFID_RECV_LEN_VALID == ret) {
		if (0x08 == cpr74_recv_size) {
			printf("CPR74 WAR: No Card Detected\n");
		} else {
			printf("CPR74 ERR: Non Supported Card Detected\n");
		}
	}
}

static RET_RFID cpr74_check_ack(u8 expect_len)
{
	u8 data_len = 0;
	RET_RFID ret = RET_RFID_OK;

	if (UART5_RX_STA&0X8000) {
		if (UART5_RX_BUF[0] != 0x02) {// Header Check
			ret = RET_RFID_RECV_DAT_VALID;
		} else {
			data_len = (UART5_RX_BUF[1] << 8) + UART5_RX_BUF[2];

			if (data_len != (UART5_RX_STA&0X7FFF)) {// Length Check
				ret = RET_RFID_RECV_LEN_VALID;
			}
			if (data_len != expect_len) {// Length Check
				ret = RET_RFID_RECV_LEN_VALID;
			}
			
			cpr74_recv_size = data_len;
		}
	} 

	return ret;
}

static RET_RFID cpr74_send_cmd(u8 *cmd, u8 len, u8 expect_len, u16 waittime)
{
	u8 i = 0;
	RET_RFID ret = RET_RFID_OK;

	UART5_RX_STA = 0;

	//printf("RFID SEND:");
	for (i=0; i<len; i++) {
		while((UART5->SR&0X40) == 0);
		UART5->DR = cmd[i];
		//printf("%.2X ", cmd[i]);
	}
	//printf("\n");

	if (waittime) {
		while (--waittime) {
			delay_ms(10);
			if (UART5_RX_STA&0X8000) {// ACK RECVED
				ret = cpr74_check_ack(expect_len);
				break;
			} 
		}

		if (waittime == 0) {
			ret = RET_RFID_RECV_TIM_OUT;// NO ACK RECVED
		}
	}

	return ret;
}

// SEND:020009FFB001001843
// RECV:02001300B0000105710000000053511071D7F0
static RET_RFID cpr74_inventory()
{
	RET_RFID ret = RET_RFID_OK;

	cpr74_send_buf[0] = 0x02;
	cpr74_send_buf[1] = 0x00;
	cpr74_send_buf[2] = 0x09;
	cpr74_send_buf[3] = 0x00;
	cpr74_send_buf[4] = 0xB0;
	cpr74_send_buf[5] = 0x01;
	cpr74_send_buf[6] = 0x00;
	cpr74_send_buf[7] = 0xCA;
	cpr74_send_buf[8] = 0x86;

	ret = cpr74_send_cmd(cpr74_send_buf, 0x09, 0x13, 100);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	// Get Serial Number
	pGotVal = calypso_serial_num;

	cpr74_parse_ack(18*4+1, 64);

	return ret;
}

// SEND:02001100B0252100000000535110713E59
// RECV:02000D00B00002B3717000957E
static RET_RFID cpr74_select_stage1()
{
	u8 i = 0;
	u16 crc16 = 0;
	RET_RFID ret = RET_RFID_OK;

	cpr74_send_buf[0] = 0x02;
	cpr74_send_buf[1] = 0x00;
	cpr74_send_buf[2] = 0x11;
	cpr74_send_buf[3] = 0x00;
	cpr74_send_buf[4] = 0xB0;
	cpr74_send_buf[5] = 0x25;
	cpr74_send_buf[6] = 0x21;

	// serial number
	for (i=0; i<SERIAL_NUM_SIZE/2; i++) {
		cpr74_send_buf[7+i]  = cpr74_chr2hex(calypso_serial_num[2*i])<<4;
		cpr74_send_buf[7+i] += cpr74_chr2hex(calypso_serial_num[2*i+1]);
	}

	crc16 = cpr74_crc16_calc(cpr74_send_buf, cpr74_send_buf[2]-2);

	cpr74_send_buf[15] = (u8)crc16;
	cpr74_send_buf[16] = (u8)(crc16>>8);

	ret = cpr74_send_cmd(cpr74_send_buf, 0x11, 0x0D, 100);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	return ret;
}

// SEND:0200080066807350
// RECV: 02001800660017E33DD00000000000002D38000000D7F7
static RET_RFID cpr74_select_stage2()
{
	RET_RFID ret = RET_RFID_OK;

	cpr74_send_buf[0] = 0x02;
	cpr74_send_buf[1] = 0x00;
	cpr74_send_buf[2] = 0x08;
	cpr74_send_buf[3] = 0x00;
	cpr74_send_buf[4] = 0x66;
	cpr74_send_buf[5] = 0x80;
	cpr74_send_buf[6] = 0x73;
	cpr74_send_buf[7] = 0x50;

	ret = cpr74_send_cmd(cpr74_send_buf, 0x08, 0x18, 100);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	return ret;
}

// SEND:02000EFFB2BE8100B201E41D011D
// RECV:02002A00B20002000004004C21808400000095584E3F90000805C0C8400019500101492492FC90001E6E
static RET_RFID cpr74_apdu_stage1()
{
	RET_RFID ret = RET_RFID_OK;

	cpr74_send_buf[0] = 0x02;
	cpr74_send_buf[1] = 0x00;
	cpr74_send_buf[2] = 0x0E;
	cpr74_send_buf[3] = 0xFF;
	cpr74_send_buf[4] = 0xB2;
	cpr74_send_buf[5] = 0xBE;
	cpr74_send_buf[6] = 0x81;
	cpr74_send_buf[7] = 0x00;
	cpr74_send_buf[8] = 0xB2;
	cpr74_send_buf[9] = 0x01;
	cpr74_send_buf[10] = 0xE4;
	cpr74_send_buf[11] = 0x1D;
	cpr74_send_buf[12] = 0x01;
	cpr74_send_buf[13] = 0x1D;

	ret = cpr74_send_cmd(cpr74_send_buf, 0x0E, 0x2A, 100);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	// Get Card ID
	pGotVal = calypso_card_id;

	cpr74_parse_ack(22*4+3, 76);

	return ret;
}

RET_RFID cpr74_read_calypso(void)
{
	RET_RFID ret = RET_RFID_OK;

	ret = cpr74_inventory();
	if (ret != RET_RFID_OK) {
		cpr74_error_msg(ret);
		return ret;
	}
#if 1
	ret = cpr74_select_stage1();
	if (ret != RET_RFID_OK) {
		cpr74_error_msg(ret);
		return ret;
	}
#if 0
	ret = cpr74_select_stage2();
	if (ret != RET_RFID_OK) {
		return ret;
	}
#endif
	ret = cpr74_apdu_stage1();
	if (ret != RET_RFID_OK) {
		cpr74_error_msg(ret);
	}
#endif
	return ret;
}
