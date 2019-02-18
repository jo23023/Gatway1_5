#ifndef 		__COMPORT_H__
#define			__COMPORT_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "jsw_rf_api.h"

#define TRUE	1
#define FALSE	0

#define ON		1
#define OFF		2

#define HEAD_ACK		0x8B
#define HEAD_PAIR		0x87
#define HEAD_NORMAL		0x85

extern unsigned int temp_did;

/********************************
		RF Packet Format
*********************************/
typedef struct 		//JSW - A7129 RF chip Message Payload Format.
{	
	unsigned char deviceID[4];
	unsigned char devicesType;
	unsigned char misc;
	unsigned char status[4];
	unsigned char checksum;
}RF_PacketFormat;


/* Device Type here is defined by JSW sensor, It might be confuse the
*  User with
*/
typedef enum{
	DT_GATEWAY_MCU		= 0x01,

	DT_ONBOARD_TEMP 	= 0x05,	//Joseph add.
	DT_MAG				= 0x10,
	DT_PIR				= 0x11,
	DT_VIBRATION		= 0x12,
	DT_DIGITAL_KEY		= 0x13,
	DT_DIGITAL_KEY_NEW	= 0x14, //New product for new RF protocol
	DT_BUTTON			= 0x15,
	DT_REMOTE			= 0x16,
	//DT_KEYPAD_JSW		= 0x17,
	DT_REMOTE_NEW		= 0x17, //New product for new RF protocol
	DT_DOOR_LOCK		= 0x21,
//	DT_WATERLEVEL		= 0x99,
	DT_SIREN_OUTDOOR	= 0x24,
	DT_SIREN_INDOOR		= 0x25,
	DT_ADAPTER			= 0x26,
	DT_DIMMER			= 0x27,
	DT_DOORCHIME		= 0x28,
	DT_TEMPERATURE		= 0x35,
	DT_SMOKE			= 0x37,
	DT_WATERLEAK		= 0x38,
}DeviceType;

typedef enum{
	RFPACK_DEVACK	= 0x8B,
	RFPACK_MCUACK	= 0x8C,
	RFPACK_SENSOR	= 0x85,

}RFPacketType;

typedef enum{
	CMD_NORMAL		= 0xE1,
	CMD_TEMPERATURE = 0xEC,
	CMD_SIREN		= 0xFE,

}RFCmdType;

unsigned char PutTxBuffer(unsigned char,unsigned char *data,unsigned char bytes,char isSend);
unsigned char DecodeRFMessage(ReportRF *ptr);
unsigned char* IsAckDeviceCorrect(unsigned int devieceID);

unsigned char TranslateDeviceType(unsigned char RFDeviceType);

struct txitem{
	double time;
	short errCount;
	unsigned int did;
	unsigned char type;
	unsigned char cmd;
};

#define TXITEMMAX 120
extern struct txitem txqueue[TXITEMMAX];
extern struct txitem currenttx;
extern short txreadpos;
extern short txwritepos;
extern short txCount;
extern short uartbusy;
void checkAckfromWrite_thread();

void checkAckfromWrite();
void insertToQueue(struct txitem *tx);
#endif

