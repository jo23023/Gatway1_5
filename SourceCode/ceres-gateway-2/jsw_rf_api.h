#ifndef		__RFMODULE_H__
#define 	__RFMODULE_H__

/**************************************  How to getting start? ****************************************************************
 *
 *	Step1.RFModule_Init - Initial the RF API with buffer address, comport file descriptor, and callback function.
 *	Step2.RFModuleWrite	- Send target device ID, device type, and specific command number to get/set the RF sensor status.
 *	Step3.RFModuleRead	- Create a thread to catch the RF message from sensors by RF control MCU(via UART).
 *
 *****************************************************************************************************************************/


 //----------------------------- Initialized the RF SDK  -------------------------------//
unsigned char RFModule_Init(int fd, unsigned char *pRxBuf, unsigned char *pTxBuf, void (*Callback)(void  *ReportRF));
/*********************************************************************
*	Parameter:
*		--@ fd : File Descriptor of Uart Com Port.
*		--@ pRxBuf:A pointer which point to RF Module receive buffer,
			It's the raw data that send from the MCU.
*		--@ pTxBuf:A pointer which point to RF Module transmit buffer,
			It's the data that send from the host.
		--@ Callback:A Fuction pointer, which point to call back function.
			SDK will call this function when receive data from RF Module.
*
*	Return:
*		0 - fail.
		1 - success.
*********************************************************************/



//-----------------------------  Send RF Message to sensors  ---------------------------------//

//-------------------------//
//    Device Type Table    //
//-------------------------//
typedef enum {
	TYPE_UNKNOW			= 0,
	TYPE_SIREN_INDOOR	= 1,
	TYPE_SIREN_OUTDOOR	= 2,
	TYPE_POWER_ADAPTER	= 3,
	TYPE_PIR			= 4,
	TYPE_MAGNETIC		= 5,
	TYPE_ONBOARD_TEMP	= 6,	//Temperture and humidity sensor dependent on Gateway Main Board.
	TYPE_TEMPERATURE 	= 7,	//Independent temperature and humidity sensor.
	TYPE_REMOTE_CONTROL	= 8,
	TYPE_SMOKE			= 9,
	TYPE_ACCELEROMETER	= 10,
	TYPE_WATERLEVEL		= 11,
	TYPE_KEYPAD_JSW		= 12,
	TYPE_CAMERA         = 13,
	TYPE_GATEWAY        = 14,
	TYPE_REMOTE_CONTROL_NEW	= 15, //New product for new RF protocol
	TYPE_KEYPAD_JSW_NEW	= 16, //New product for new RF protocol

	TYPE_VIBRATION		=17,
	TYPE_DOORCHIME		=18,
	TYPE_BUTTON			=19,

	TYPE_DOOR_LOCK       = 20,

	TYPE_MCU_COMMAND	= 100,
	TYPE_NEST_SMOKE		=200,
	TYPE_NEST_THERMO	=201,
	TYPE_NEST_CAM		=202,
	TYPE_MCU			= 0xFF,
}DeviceTypeTable;


typedef enum
{
	Alarm_unknow,
	Alarm_SOS,
	Alarm_fire_emergency,
	Alarm_water_emergency,
	Alarm_warning,
	Alarm_invasion,
	MAX_Alarm,
} Alarm_status;
void RFModuleWrite(unsigned int DeviceID, unsigned char DeviceType, unsigned char CmdNum);
/*********************************************************************
*	Parameter:
*		--@ DeviceID  : 4 bytes length device ID of the target sensor.
*		--@ DeviceType: Sequence number from the predefined table.

*		--@ CndNum	  : The command number to specific device type.
*
*	Return:
*		0 - fail.

		1 - success.
*********************************************************************/

//----------------------------//
//    Command Number Table    //
//----------------------------//
#define CMD_GENERAL_PAIR		0xFF		//Send this command will turn RF control MCU into pairing mode.

/* GatewayMCU */
#define CMD_GW_GET_VERSION				0x01
#define CMD_GW_GET_CUSTOMER				0x02
#define CMD_GW_GET_CHANNEL				0x03
#define CMD_GW_GET_DEFAULT_SYNCWORD		0x04
#define CMD_GW_GET_CURRENT_DID			0x05


/* Siren */
#define CMD_SIREN_TURNON		0x01
#define CMD_SIREN_TURNOFF		0x02

#define CMD_SIREN_SET_LED_OFF		0xA0
#define CMD_SIREN_SET_LED_ON		0xA1

#define CMD_SIREN_SET_VOLUME_OFF	0xB0
#define CMD_SIREN_SET_VOLUME_LOW	0xB1
#define CMD_SIREN_SET_VOLUME_HIGH	0xB2

#define CMD_SIREN_SET_PERIOD_10		0xC1
#define CMD_SIREN_SET_PERIOD_30		0xC2
#define CMD_SIREN_SET_PERIOD_60		0xC3
#define CMD_SIREN_SET_PERIOD_120	0xC4
#define CMD_SIREN_SET_PERIOD_180	0xC5
/* Power Adapter */
#define CMD_ADAPTER_TURNON		0x01
#define CMD_ADAPTER_TURNOFF		0x02

/* PIR */

/* Magnetic */

/* Temperature sensor */
#define CMD_ONBOARD_TEMP_GETALL	0x01

/* Remote Control */

// Doorchime
#define CMD_DOORCHIME_ALARM	    0x03







//-------------------------  Receive RF message from RF control MCU  ---------------------------------//

typedef struct{
	unsigned int  DeviceID;		//4 Bytes Sensor ID. You can get the ID when power on the sensor.
	unsigned char DeviceType;	//device type table are defined as enumeration(DeviceTypeTable) above.
	unsigned char Status[4];	//status number table defined below.
}ReportRF;

void RFModuleRead(ReportRF *ptr);
/*********************************************************************
*	Parameter:
*		--@ ptr  : A pointer which point to a "ReportRF" structure.

*		           The structure will be renew after calling
				   function "RFModuleRead".
*	Return:
*		None.

*********************************************************************/


//----------------------------------------//
//    Report(Read) Status Number Table    //
//----------------------------------------//
/* General */
#define RE_UNKNOW_STATUS		0x00		//Unknow status, host should check the reference RxBuf(which used to Initail the RF Module)
											//for row data.
/*Joseph :0731 Delete All ack, discussed with kafka*/
//#define RE_MCU_ACK			0xA0		//RF MCU(Gateway MCU) ACK.
//#define RE_SENSOR_ACK			0xA1		//Sensor ACK.

#define RE_ABUS_AUTOREPORT		0xA2		//Abus device report a standby packet.
#define RE_SENSOR_PAIR			0xAA		//Sensor pair message.
#define RE_SENSOR_BATLOW		0xAF		//Battery low status.

/* GatewayMCU */
#define RE_MCU_ACPOW_OFF		0x00		//Gateway's AC power is removed.
#define RE_MCU_ACPOW_ON		    0x01		//Gateway's AC power is connected.
#define RE_MCU_TEMP_TIMEOUT     0x08        //Didn'd receive temperature command in 150 seconds, MCU Reboot.
#define RE_MCU_RCV_ERROR        0x09        //Over 12 same RF packet in 3 seconds, MCU reboot.
#define RE_MCU_RESET_RF_IC      0x0A        //MCU reset RF IC every 60 mins
#define RE_MCU_RESET_RF_IC2     0x0B        //MCU reset RF IC every 60 mins (real reset)
#define RE_MCU_WAIT_FOR_UPDATE	0x99		//MCU is in IAP mode and the RF-868 is not work! Please update MCU ASAP.

//gateway event
#define GW_PANIC 0x11
#define DEV_TRIGGER 0x12

#define GW_DISARM 0X01
#define GW_ARM
/* Siren */
#define RE_SIREN_ISON			0x01		//Siren is alarming.
#define RE_SIREN_ISOFF			0x02		//Siren is standby.
#define RE_SIREN_TEMPER			0x03

#define RE_SIREN_SET_LED_OFF		0xA0
#define RE_SIREN_SET_LED_ON			0xA1

#define RE_SIREN_SET_VOLUME_OFF		0xB0
#define RE_SIREN_SET_VOLUME_LOW		0xB1
#define RE_SIREN_SET_VOLUME_HIGH	0xB2

#define RE_SIREN_SET_PERIOD_10		0xC1
#define RE_SIREN_SET_PERIOD_30		0xC2
#define RE_SIREN_SET_PERIOD_60		0xC3
#define RE_SIREN_SET_PERIOD_120		0xC4
#define RE_SIREN_SET_PERIOD_180		0xC5

/* Power Adapter */
#define RE_ADAPTER_ISON			0x01		//Power adapter is turned on.
#define RE_ADAPTER_ISOFF		0x02		//Power adapter is off.
#define RE_ADAPTER_ISOFF2		0x20		//Power adapter is off. for

/* PIR */
#define RE_PIR_MOTION			0x01		//PIR detected.
#define RE_PIR_TEMPER			0x02		//Temper Key.

/* Magnetic */
#define RE_MAG_ISOFF			0x01		//Magnetic is closed
#define RE_MAG_ISON				0x02		//Magnetic is opened
#define RE_MAG_TEMPER			0x03		//Tempter triggered.

#define CAMERA_TRIGGER  0x01
#define CAMERA_DOORKEY          0x02

/* Vibration Sensor*/
#define RE_VIBRATION_TRIGGER		0x01		//Vibration detected.
#define RE_VIBRATION_TAMPER			0x10

#define RE_VIBRATION_SET_HIGH			0x03
#define RE_VIBRATION_SET_MIDDLE			0x04
#define RE_VIBRATION_SET_LOW			0x05
#define RE_VIBRATION_SET_H_HIGH			0x13
#define RE_VIBRATION_SET_H_MIDDLE		0x14
#define RE_VIBRATION_SET_H_LOW			0x15

#define RE_VIBRATION_SET_TRIGGER_MODE 0x00
#define RE_VIBRATION_SET_TAMPER_MODE 0x01


/* Button sensor */
#define RE_BUTTON_PRESS		0x01
#define RE_BUTTON_LONG_PRESS		0x02


/* Temperature sensor */
//Reserve

/* Remote Control */
#define RE_REMOTE_LOCK			0x01
#define RE_REMOTE_UNLOCK		0x02
#define RE_REMOTE_CAMERA		0x03
#define RE_REMOTE_PANIC			0x04
#define RE_REMOTE_TEMPER		0x05
#define RE_REMOTE_PART_ARM		0x06

/* Smoke sensor */
#define RE_SMOKE_TRIGGERED		0x01
#define RE_SMOKE_OVERHEAT	    0x02

/* KeyPad-JSW */
#define RE_KEYPAD_LOCK			0x01
#define RE_KEYPAD_UNLOCK		0x02
#define RE_KEYPAD_CAMERA		0x03
#define RE_KEYPAD_PANIC			0x04
#define RE_KEYPAD_TEMPER		0x05
#define RE_KEYPAD_PART_ARM		0x06

/* Water level sensor */
#define RE_WATERLEVEL_IDLE		0x01
#define	RE_WATERLEVEL_ARM1		0x02
#define RE_WATERLEVEL_ARM2 		0x03

extern void (*CallBackFunc)(void*);
int serial_main(void); //entry
int writePairing(unsigned int deviceID, unsigned char type, unsigned char cmd);
int writeCommand(unsigned int deviceID, unsigned char type, unsigned char cmd);




//=================================================================================
//----------------------------- Schedule MCU GPIO -------------------------------//
#define              PIN_BUZZER                    1
#define              PIN_LED_R                     2
#define              PIN_LED_G                     4
#define              PIN_LED_B                     8
#define              PIN_BUZZER_ALARM              16

void McuGpio(unsigned char Gpio, unsigned char OnDelay, unsigned char OffDelay, unsigned short Times);
void McuGpio2(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4, unsigned char b5, unsigned char b6);//send RF cmd to GW MCU (with data)
void set_comstom_code(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4, unsigned char b5, unsigned char b6);//send RF cmd to GW MCU (with data)


//for melody IC
//#define   MELODY_JSW_BEEPBEEP      1
//#define   MELODY_JSW_LONGBEEP    2
//#define   MELODY_JSW_SHORTBEEP   3
//#define   MELODY_JSW_ALARM         4
//
//#define   MELODY_UNIDEN_POWERON      1
//#define   MELODY_UNIDEN_NETOK    2
//#define   MELODY_UNIDEN_BEEPBEEP       3
//#define   MELODY_UNIDEN_ARM       4
//#define   MELODY_UNIDEN_ALARM    5
//#define   MELODY_UNIDEN_DISARM 6

//----------------------------- MCU Melody control -------------------------------//
//void McuMelody(unsigned char Track, unsigned char count);

//=================================================================================


#endif


