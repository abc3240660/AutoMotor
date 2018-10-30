#ifndef RFID_READ_H

#define RFID_READ_H

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#define RFID_MAX_BUF_SIZE 64
#define RFID_HEADER_SIZE 3
// TODO: need to check
#define SERIAL_NUM_SIZE 3
#define CARD_ID_SIZE 19

enum RET_RFID {
	RET_RFID_OK = 0,
	RET_RFID_SEND_DATA_LOSS = -1,
	RET_RFID_RECV_DATA_LOSS = -2,
	RET_RFID_RECV_BUF_LACK  = -3,
	RET_RFID_RECV_DAT_VALID = -4
};

extern BYTE serial_num[SERIAL_NUM_SIZE];
extern BYTE card_id[CARD_ID_SIZE+1];

extern RET_RFID rfid_get_card_id();

#endif // RFID_READ_H
