#include "rfid.h"

BYTE rfid_recv_size = 0;
BYTE rfid_recv_buf[MAX_BUF_SIZE_RFID] = "";
BYTE rfid_send_buf[MAX_BUF_SIZE_RFID] = "";
BYTE serial_num[SERIAL_NUM_SIZE] = "";
BYTE card_id[CARD_ID_SIZE+1] = "";
static BYTE pGotVal = NULL;

static void get_val(int bit_pos, int bit_num)
{
	int i = 0;
	int j = 0;
	int offset = 0;
	int bit_cnt = 0;
	BYTE tmp_val = 0;

	for (i=0; i<recv_size; i++) {
		for (j=0; j<8; j++) {
			if (bit_pos <= (i*8 + j + 1)) {
				offset = 7 - ((bit_pos+bit_cnt-1)%8);
				tmp_val += ((recv_buf[i]>>offset)&0x01) << (3-bit_cnt%4);
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
				if (bit_num == bit_cnt) {
					break;
				}
			}
		}
		if (bit_num == bit_cnt) {
			break;
		}
	}
}

static RET_RFID rfid_read(void)
{
	int data_length = 0;

	memset(rfid_recv_buf, 0, RFID_MAX_BUF_SIZE);

	rfid_recv_size = 0;
	uart5_recv(rfid_recv_buf, 3);

	if (0x02 == rfid_recv_buf[0]) {
		data_length = (rfid_recv_buf[1] << 8) + rfid_buf[2];
		if (data_length > (RFID_MAX_BUF_SIZE - RFID_HEADER_SIZE)) {
			// TODO: Exception Handle
			printf("RFID: buf is not enough\n");
			return RET_RFID_RECV_BUF_LACK;
		} else {
			uart5_recv((rfid_recv_buf + RFID_HEADER_SIZE), data_length);
			rfid_recv_size = data_length + RFID_HEADER_SIZE;
			return RET_RFID_OK;
		}
	} else {
		// TODO: Exception Handle
		printf("RFID: header is not 02\n");
		return RET_RFID_RECV_DAT_VALID;
	}
}

static RET_RFID rfid_write(BYTE* pSendBuf, int size)
{
	int size_transferred = 0;
	RET_RFID ret = RET_RFID_OK;

	sent_size = uart5_send(pSendBuf, size);

	if (size_transferred != size) {
		ret = RET_RFID_SEND_DATA_LOSS;

		// TODO: Exception Handle
		printf("RFID: not all data are sent out\n");
	}

	return ret;
}

// SEND:020009FFB001001843
// RECV:02001300B0000105710000000053511071D7F0
static RET_RFID rfid_inventory()
{
	RET_RFID ret = RET_RFID_OK;

	rfid_send_buf[0] = 0x02;
	rfid_send_buf[1] = 0x00;
	rfid_send_buf[2] = 0x09;
	rfid_send_buf[3] = 0xFF;
	rfid_send_buf[4] = 0xB0;
	rfid_send_buf[5] = 0x01;
	rfid_send_buf[6] = 0x00;
	rfid_send_buf[7] = 0x18;
	rfid_send_buf[8] = 0x43;

	ret = rfid_write(rfid_send_buf, 9);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_read();
	if (ret != RET_RFID_OK) {
		return ret;
	}

	// Get Serial Number
	pGotVal = serial_num;

	// TODO: need to check
	get_val(8*9+19, 76);

	return ret;
}

// SEND:02001100B0252100000000535110713E59
// RECV:02000D00B00002B3717000957E
static RET_RFID rfid_select_stage1()
{
	RET_RFID ret = RET_RFID_OK;

	rfid_send_buf[0] = 0x02;
	rfid_send_buf[1] = 0x00;
	rfid_send_buf[2] = 0x11;
	rfid_send_buf[3] = 0x00;
	rfid_send_buf[4] = 0xB0;
	rfid_send_buf[5] = 0x25;
	rfid_send_buf[6] = 0x21;
	rfid_send_buf[7] = 0x00;
	rfid_send_buf[8] = 0x00;
	rfid_send_buf[9] = 0x00;
	rfid_send_buf[10] = 0x00;
	rfid_send_buf[11] = 0x53;
	rfid_send_buf[12] = 0x51;
	rfid_send_buf[13] = 0x10;
	rfid_send_buf[14] = 0x71;
	rfid_send_buf[15] = 0x3E;
	rfid_send_buf[16] = 0x59;

	ret = rfid_write(rfid_send_buf, 0x11);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_read();

	return ret;
}

// SEND:0200080066807350
// RECV: 02001800660017E33DD00000000000002D38000000D7F7
static RET_RFID rfid_select_stage2()
{
	RET_RFID ret = RET_RFID_OK;

	rfid_send_buf[0] = 0x02;
	rfid_send_buf[1] = 0x00;
	rfid_send_buf[2] = 0x08;
	rfid_send_buf[3] = 0x00;
	rfid_send_buf[4] = 0x66;
	rfid_send_buf[5] = 0x80;
	rfid_send_buf[6] = 0x73;
	rfid_send_buf[7] = 0x50;

	ret = rfid_write(rfid_send_buf, 8);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_read();

	return ret;
}

// SEND:02000EFFB2BE8100B201E41D011D
// RECV:02002A00B20002000004004C21808400000095584E3F90000805C0C8400019500101492492FC90001E6E
static RET_RFID rfid_apdu_stage1()
{
	RET_RFID ret = RET_RFID_OK;

	rfid_send_buf[0] = 0x02;
	rfid_send_buf[1] = 0x00;
	rfid_send_buf[2] = 0x0E;
	rfid_send_buf[3] = 0xFF;
	rfid_send_buf[4] = 0xB2;
	rfid_send_buf[5] = 0xBE;
	rfid_send_buf[6] = 0x81;
	rfid_send_buf[7] = 0x00;
	rfid_send_buf[8] = 0xB2;
	rfid_send_buf[9] = 0x01;
	rfid_send_buf[10] = 0xE4;
	rfid_send_buf[11] = 0x1D;
	rfid_send_buf[12] = 0x01;
	rfid_send_buf[13] = 0x1D;

	ret = rfid_write(rfid_send_buf, 0x0E);
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_read();
	if (ret != RET_RFID_OK) {
		return ret;
	}

	// Get Card ID
	pGotVal = card_id;

	get_val(8*9+19, 76);

	return ret;
}

RET_RFID rfid_get_card_id()
{
	RET_RFID ret = RET_RFID_OK;

	ret = rfid_inventory();
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_select_stage1();
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_select_stage2();
	if (ret != RET_RFID_OK) {
		return ret;
	}

	ret = rfid_apdu_stage1();

	return ret;
}
