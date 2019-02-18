#include "rf_module.h"

#include "jsw_protocol.h"

#define BAUDRATE		57600
#define COMPORT			"/dev/ttyS0"
#define mSecSleep(ms)	usleep(ms * 1000)

#define DEBUG_CHECKSUM	TRUE



struct txitem txqueue[TXITEMMAX];
struct txitem currenttx;
short txreadpos =0;
short txwritepos = 0;
short txCount=0;
short uartbusy = 0;

/* Global Variable */
fd_set set;
struct termios opt;
struct timeval timeout;
unsigned char defaultSyncWord[4] = {0x54, 0x75, 0x19, 0x12};	//ABUS

//Test SDK variavle
unsigned char *pRxBuf = NULL;
unsigned char *pTxBuf = NULL;
void (*CallBackFunc)(void*) = NULL;
int fdComPort;
//-----------------------------------
#define TX_LIST_SIZE	10
unsigned char txList[TX_LIST_SIZE][12] = {{0}};//To keep the transmited information,compare it when ACK packet is comming.
unsigned char (*pInputTxList)[12] = txList;	//pointer to array[12]


/****************************************************************************
*			RF Module Initialtion
*****************************************************************************/
unsigned char RFModule_Init(int fd, unsigned char *pRx, unsigned char *pTx, void (*RF)(void  *ReportRF)){

	if(fd <= 0){
		printf("[ RFModule Error ] Invalid Comport fd.\n");
		return FALSE;
	}

	if(pRx == NULL){
		printf("[ RFModule Error ] Invalid RxBuf pointer.\n");
		return FALSE;
	}

	if(pTx == NULL){
		printf("[ RFModule Error ] Invalid TxBuf pointer.\n");
		return FALSE;
	}

	if(RF == NULL){
		printf("[ RFModule Error ] Invalid CallBack function.\n");
		return FALSE;
	}

	fdComPort 	= fd;		//printf("RF FD = %x \r\n",fdComPort);
	pRxBuf 		= pRx;		//printf("RF Rx addr = %x \r\n",(unsigned int)pRx);
	pTxBuf 		= pTx;		//printf("RF Tx addr = %x \r\n",(unsigned int)pTx);
	CallBackFunc = RF;		//printf("RF Func addr = %x \r\n",(unsigned int)RF);


	return TRUE;
}



/****************************************************************************
*			RF Module Write
*****************************************************************************/
void RFModuleWrite(unsigned int DeviceID, unsigned char DeviceType, unsigned char CmdNum)
{

	unsigned char CmdType_868 = 1;
	unsigned char ContorlData_868 = 0, Data3_868 = 0, Data1_868 = 0;
	RF_PacketFormat rfPack;

	unsigned char buf[12] = {0};
	if(DeviceType == 100){
		printf("Sending API Command....\n");
		switch(CmdNum){
			case CMD_GW_GET_VERSION:			buf[0] = 0xC1;		break;
			case CMD_GW_GET_CUSTOMER:			buf[0] = 0xC2;		break;
			case CMD_GW_GET_CHANNEL:			buf[0] = 0xC3;		break;
			case CMD_GW_GET_DEFAULT_SYNCWORD:	buf[0] = 0xC4;		break;
			case CMD_GW_GET_CURRENT_DID:		buf[0] = 0xC5;		break;
		}
	//	printf("Put data to uart buffer....\n");
		PutTxBuffer(1,&buf[0], 10, TRUE);
		return;
	}
	//step1. decide the MCU protocol.
	if(CmdNum == 0xFF){	//SDK command num:0xFF represent pairing.
		ContorlData_868 = 0x10;
		DeviceType = TranslateDeviceType(DeviceType);	//DeviceType from Api field to 868 protocol.
		memcpy(&rfPack.deviceID,&DeviceID,4);	//Send the Gateway DID to GatewayMCU.
	//	memcpy(&rfPack.status, defaultSyncWord, 4);//copy the ABUS gateway default channel... in case of sensor error.
	}
	else{
		//When the message is GWAPP->GWMCU, the item "DeviceType" represent of target device Type , Gateway MCU can use different Tx mode to the sensor's sleep period.
		switch(DeviceType){

			case TYPE_SIREN_OUTDOOR:
			case TYPE_SIREN_INDOOR:

				switch(CmdNum){
					case CMD_SIREN_TURNOFF:
						ContorlData_868 = 2;	break;
					case CMD_SIREN_TURNON:
						ContorlData_868 = 1;	break;

					//------ Siren Setting -------//
					case CMD_SIREN_SET_VOLUME_OFF:
						ContorlData_868 = 0x20;
						Data3_868 		= 5;		break;

					case CMD_SIREN_SET_VOLUME_LOW:
						ContorlData_868 = 0x20;
						Data3_868 		= 4;		break;

					case CMD_SIREN_SET_VOLUME_HIGH:
						ContorlData_868 = 0x20;
						Data3_868 		= 0;		break;

					case CMD_SIREN_SET_LED_OFF:
						ContorlData_868 = 5;		break;

					case CMD_SIREN_SET_LED_ON:
						ContorlData_868 = 3;		break;


					case CMD_SIREN_SET_PERIOD_10:
						ContorlData_868 = 0x40;
						Data3_868 		= 10;		break;


					case CMD_SIREN_SET_PERIOD_30:
						ContorlData_868 = 0x40;
						Data3_868 		= 30;		break;


					case CMD_SIREN_SET_PERIOD_60:
						ContorlData_868 = 0x40;
						Data3_868 		= 60;		break;

					case CMD_SIREN_SET_PERIOD_120:
						ContorlData_868 = 0x40;
						Data3_868 		= 120;		break;

					case CMD_SIREN_SET_PERIOD_180:
						ContorlData_868 = 0x40;
						Data3_868 		= 180;		break;
				}
				break;

			case TYPE_POWER_ADAPTER:
				switch(CmdNum){
					case CMD_ADAPTER_TURNOFF:
						ContorlData_868 = 2;	break;

					case CMD_ADAPTER_TURNON:
						ContorlData_868 = 1;	break;
				}
				break;

			case TYPE_VIBRATION:
				switch(CmdNum){
					case RE_VIBRATION_SET_HIGH:
					case RE_VIBRATION_SET_MIDDLE:
					case RE_VIBRATION_SET_LOW:
					case RE_VIBRATION_SET_H_HIGH:
					case RE_VIBRATION_SET_H_MIDDLE:
					case RE_VIBRATION_SET_H_LOW:
						ContorlData_868 = (CmdNum & 0xff) ;
					break;
					default:
						ContorlData_868 = RE_VIBRATION_SET_MIDDLE ;
					break;
				}
				break;
			case TYPE_DOORCHIME:
//				if(CmdNum < 0x0f)
					ContorlData_868 = CmdNum ;
					if(CmdNum == CMD_DOORCHIME_ALARM)
						Data1_868 = 0x0f;
				break;

			case TYPE_ONBOARD_TEMP:
				switch(CmdNum){
					case CMD_ONBOARD_TEMP_GETALL:
						ContorlData_868 = 1;
					break;
				}
				break;


			default:
				return FALSE;
		}//End of switch device type.
	}
	memset(&rfPack,0,11);
	memcpy(&rfPack.deviceID , &DeviceID,4 );
	rfPack.devicesType  = DeviceType;//1;					//GatewayMCU device type number.
	rfPack.misc 		= CmdType_868;			//Target Device types(misc) defined in rf_module.h.
	rfPack.status[0] 	= ContorlData_868;		//Command Number defined in jsw_rf_api.h.
	rfPack.status[1]	= Data1_868;				//Use to set the sensor configuration.
	rfPack.status[3]	= Data3_868;				//Use to set the sensor configuration.

	PutTxBuffer(0,(unsigned char *)&rfPack, 10, TRUE);


}



/****************************************************************************
*			RF Module Read
*****************************************************************************/

//Note: the Protocol is compelcate....
void RFModuleRead(ReportRF *ptr)
{

	while(1)	//Infinity loop.
	{
		fd_set set;
		struct timeval time;
		int j;
		int size;
		static int dataNum = 0;
		time.tv_sec = 0;
		time.tv_usec = 5000;

		FD_ZERO(&set); //* clear the set *
		FD_SET(fdComPort, &set); //* add our file descriptor to the set *

		//switch(++boringCount){
		//	case 1:
		//		printf("\r\nRF Module Read.");
		//		break;
		//	case 30:
		//		printf(".");boringCount = 0;
		//		break;
		//	default:
		//		printf(".");
		//		break;
		//}fflush(stdout);

		//Read all the packet.
		if(select(fdComPort + 1, &set, NULL, NULL, &time) > 0 ){
				if(read( fdComPort, (pRxBuf+dataNum), 1) > 0){
				if(*pRxBuf != 0x85 && *pRxBuf != 0xAA)
					dataNum = 0;
				else
					dataNum++;
			}

		}
		if(dataNum < 12)
			continue;

        //printf("RFModuleRead(), RFDeviceType=%d, RFDeviceID=%u, dataNum=%d\n", RFDeviceType, RFDeviceID, dataNum);
        if(dataNum > 0)
        {
            int i;
            printf("----pRxBuf=[");
            for(i=0;i<dataNum;i++)
            {
                printf("%02x ", (unsigned char)(pRxBuf[i]));
            }
            printf("]\n");
        }

		printf("\n");
		for(j = 0; j < 12 ; j++)
			printf("r[%02x]",*(pRxBuf+j));
		printf("\n");

		if(DecodeRFMessage(ptr) == TRUE){
			/*		printf("---   GetFromMCU   ---\r\n");*/
	/*	printf("-RF ID: %08X   ",ptr->DeviceID);
		printf("-Type:  %2X ",ptr->DeviceType);
		printf("-Status: %02X %02X %02X %02X -\r\n", *(ptr->Status),   *(ptr->Status+1),
												   *(ptr->Status+2), *(ptr->Status+3));*/
			CallBackFunc(ptr);
		}
		memset(pRxBuf, 0, 12);
		dataNum = 0;
	}
}


unsigned char PutTxBuffer(unsigned char Type, unsigned char *data,unsigned char bytes,char isSend){
	pthread_mutex_lock(&uart_TX_mutex);
	static char count = 0;
	static unsigned char DataByte = 0;
	unsigned char TotalByte;
	unsigned char i,sum=0;
	char send_twice = 0;
//printf("PutTxBuffer(%d)\n",Type);
	while(bytes--){
		*(pTxBuf+1 + DataByte++) = *(data++);
	}
	if(isSend){
		if(Type == 0)
			*pTxBuf = 0x85;		//Command Head 0x85
		else if (Type == 1)
			*pTxBuf = 0xAA;		//API Command
		TotalByte = 12;

		for(i = 1; i<11 ; i++ ){
			sum	+= *(pTxBuf +i);
		}
		*(pTxBuf+11) = sum;
#if 0
		send_twice = 0;
		if(*(pTxBuf+5) == TYPE_POWER_ADAPTER)
			if(*(pTxBuf+7) == RE_ADAPTER_ISOFF)
			{
			//printf("!!!!! change power on_off 0x02 -> %02X  !!!!!",count);
			*(pTxBuf+7) = RE_ADAPTER_ISOFF2;
			//*(pTxBuf+10) = 0xaa;
			send_twice = 1;
			}
#endif
	// wait TX clean for Melody control
	if(*(pTxBuf + 1) == 0xD2)
		{
			if (uartbusy ==1)
				{
				printf(" wait TX clean for Melody control-for 1 sec\n");
				mSecSleep(1000);
				//usleep(1000000);
				}
		printf("clean RF queue,TX_W[%d]TX_R[%d]COUNT[%d] wait TX clean for Melody control-START\n",(int)txwritepos,(int)txreadpos,(int)txCount);
		#if 1
		txwritepos = 0;
		txreadpos = 0;
		txCount = 0;
		#endif
		mSecSleep(100);
		//usleep(100000);
		uartbusy =0;
		}
	printf("Data: ");
	printf("writing to com fd...\r\n");

	for(i=0; i<TotalByte; i++){
		printf(" %x",*(pTxBuf+i));
	}
	printf("\n");



	///*	write(fdComPort,pTxBuf,TotalByte);

		/** Record the Tx packet to Txlist **/
		//memcpy(pInputTxList,pTxBuf+1,11);
		//pInputTxList = (pInputTxList == &txList[TX_LIST_SIZE-1] )? txList : pInputTxList + 1;

		for(i=0; i<TotalByte; i++){
			write(fdComPort,pTxBuf+i,1);
		}


		DataByte = 0;
	}
	tcflush(fdComPort, TCIOFLUSH);
	if(*(pTxBuf + 1) == 0xD2)
	{

		printf(" wait TX clean for Melody control-END\n");
		usleep(100000);
	}
	else
		usleep(50000);

//	usleep(50000);
	pthread_mutex_unlock(&uart_TX_mutex);

	return TRUE;
}

void PrintReport(ReportRF *ptr){

	char type[20];

	switch(ptr->DeviceType){

		case TYPE_MAGNETIC:
			sprintf(type,"Megnetic");			break;

		case TYPE_PIR:
			sprintf(type,"PIR");				break;

		case TYPE_REMOTE_CONTROL:
		case TYPE_REMOTE_CONTROL_NEW:
			sprintf(type,"Remote Control");		break;

		case TYPE_SIREN_INDOOR:
		case TYPE_SIREN_OUTDOOR:
			sprintf(type,"Siren");				break;

		case TYPE_POWER_ADAPTER:
			sprintf(type,"Power Adapter");		break;

		case TYPE_VIBRATION:
			sprintf(type,"Vibration");		break;
		case TYPE_BUTTON:
			sprintf(type,"Button");		break;
		case TYPE_DOORCHIME:
			sprintf(type,"DoorChime");		break;

	}//End of switch device type.

}

int checkRepeatCmd(unsigned int did, unsigned char cmd)
{
	int x=0;

	if ( txCount == TXITEMMAX )
	{
		printf("###tx queue is full, message is discard \n");
		return 0;
	}
	//dont insert redundant commad
	for (x=0;x<txCount;x++)
	{
		int idx = txreadpos + x;
		if (idx >= TXITEMMAX)
			idx = idx - TXITEMMAX;
		if ( txqueue[idx].did == did )
		{
			if ( txqueue[idx].cmd == cmd )
			{
				printf("###tx queue cmd exist , ignore\n");
				return 0;//same cmd, dont insert.
			}
			//break;
		}
	}
	return 1;
}

void insertToQueue(struct txitem *tx)
{
	if ( checkRepeatCmd(tx->did, tx->cmd) ==0)
		return;

	txqueue[txwritepos].did = tx->did;
	txqueue[txwritepos].type = tx->type;
	txqueue[txwritepos].cmd = tx->cmd;
	txqueue[txwritepos].time = 0;
	txqueue[txwritepos].errCount = tx->errCount;
	txwritepos++;
	txCount++;
	if( (txwritepos >=TXITEMMAX) || (txwritepos < 0) )
		txwritepos = 0;
}

void writeNextOnQueue()
{
	unsigned int did;
	unsigned char type;
	unsigned char cmd;
	int ignore_resend =0;
	did = txqueue[txreadpos].did ;
	type = txqueue[txreadpos].type;
	cmd = txqueue[txreadpos].cmd;
	printf("writeNextOnQueue next did=%u type=%2X cmd=%2X\n", did, type, cmd);

 	if( (did != 0) && (type != 0) )
	{

		currenttx.did = did;
		currenttx.time = timegettime();

		if ( txqueue[txreadpos].errCount != 0)
			currenttx.errCount = txqueue[txreadpos].errCount;
		else
			currenttx.errCount =  0;
		currenttx.type = type;
		currenttx.cmd = cmd;
#if 0
		// Jeff define for ignore siren in change ARM/DISARM
		if( currenttx.type == TYPE_SIREN_INDOOR || currenttx.type == TYPE_SIREN_OUTDOOR)
			{
			if(currenttx.cmd == CMD_SIREN_TURNOFF && g_armstate == st_partarm ) ignore_resend = 1;
			if(currenttx.cmd == CMD_SIREN_TURNOFF && g_armstate == st_arm ) ignore_resend = 1;
			if(currenttx.cmd == CMD_SIREN_TURNON && g_armstate == st_disarm ) ignore_resend = 1;
			}
#endif
		if(!ignore_resend)
			{
				RFModuleWrite( did, type, cmd);

		//if(PLATFORM == "SN98601")		//SN98601's UART is queer that need write 2 times....
#ifndef SERIAL_RAW
			RFModuleWrite( did, type, cmd);
#endif
			}
		else
			{
			//currenttx.errCount = 12 ;
			printf("!!!! ignore siren in change ARM/DISARM [%08X]!!!!\n",currenttx.did);
			}
		txreadpos++;
		txCount--;
		if( (txreadpos >=TXITEMMAX) || (txreadpos < 0) )
			txreadpos = 0;
		if (txCount < 0)
			txCount = 0;
		if(! ignore_resend)
			uartbusy = 1;
	}
	else
	{
		txreadpos++;
		txCount--;
	}
}

//apple 1226
void updateSensorStatus(int nType, int nDID)
{	
	//printf("updateSensorStatus nType =%d did=%d \n", nType,nDID);

	if(nType == TYPE_POWER_ADAPTER)
	{
		cloud_action_mcu_ack(nDID);
	}

}

int check_sensor_alive(unsigned int sensor_id)
{
	int x=0;
	for ( x=0;x<devcount;x++)
	{
		if (keepalivelist[x].did == sensor_id && keepalivelist[x].sensor_NO_resp != 1)
		{
			return 1;
		}
	}

	return 0;
}

void set_sensor_alive(unsigned int sensor_id)
{
	int x=0;
	for ( x=0;x<devcount;x++)
	{
		if (keepalivelist[x].did == sensor_id )
		{
			keepalivelist[x].sensor_NO_resp = 0;
			break;
		}
	}

}


void set_sensor_dead(unsigned int sensor_id)
{

int x=0;
for ( x=0;x<devcount;x++)
{
	if (keepalivelist[x].did == sensor_id )
	{
		keepalivelist[x].sensor_NO_resp = 1;
		break;
	}
}


}


void checkAckfromWrite()
{
	#define ALIVE_SENSOR_RETRY 3
	#define DEAD_SENSOR_RETRY 1
	
	#define  MAX_ACK_TIMEOUT 2
	double nw;
	int retry_times = ALIVE_SENSOR_RETRY;
	if(currenttx.type == TYPE_SIREN_INDOOR || currenttx.type == TYPE_SIREN_OUTDOOR)
		{
			// retry only 1 time for siren setting
			if(currenttx.cmd >  CMD_SIREN_SET_LED_OFF )
				retry_times = DEAD_SENSOR_RETRY;
		}
	if (uartbusy ==0)
	{
//		printf("#Ack Not busy\n");
		if( ! get_mcu_version )
			{
			printf("CMD_GW_GET_VERSION\n");
			writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_VERSION);
			usleep(UART_WRITE_DELAY);

			usleep(UART_WRITE_DELAY * 10);

			printf("CMD_GW_GET_CHANNEL\n");
			writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_CHANNEL);
			usleep(UART_WRITE_DELAY);
			}

		return;
	}
	if(check_sensor_alive(currenttx.did))
		{
			retry_times = ALIVE_SENSOR_RETRY;
		}
	else
		retry_times = DEAD_SENSOR_RETRY;
	nw = timegettime();
	if ( abs(nw - currenttx.time) < MAX_ACK_TIMEOUT )
//	if ( abs(nw - currenttx.time) < MAX_ACK_TIMEOUT )
	{
		//printf("##Ack time isnt up currenttx.time %f time now %f\n", currenttx.time, nw);
		return;
	}

	currenttx.errCount++;
	if (currenttx.errCount > retry_times)
	{
		printf("##UART2 Timer :write to dev %u fail after %d times, txcount = %d\n", currenttx.did,retry_times, txCount);
		set_sensor_dead(currenttx.did);
		if (txCount == 0)
		{//discard

			uartbusy = 0;
			return;
		}else
		{
			writeNextOnQueue(); //send the next on queue to uart.
		}
	}else
	{
		//printf("##UART Timer :retry writing to  %u , txcount = %d\n", currenttx.did, txCount);
		insertToQueue(&currenttx);
		writeNextOnQueue(); //send the next on queue to uart.
	}
}



void checkAckfromWrite_thread()
{
	#define ALIVE_SENSOR_RETRY 3
	#define DEAD_SENSOR_RETRY 1
	
	#define  MAX_ACK_TIMEOUT 2
	#define DEEP_SLEEP_SENSOR 1.5
	#define NORMAL_SENSOR 1.2
	double nw;
	int retry_times = ALIVE_SENSOR_RETRY;
	while(1)
		{
		mSecSleep(100);
		double sensor_timeout = NORMAL_SENSOR;
		retry_times = ALIVE_SENSOR_RETRY;
		if (g_isPairMode == 1) continue;
		if (uartbusy ==0)
		{
			if( ! get_mcu_version )
				{
				printf("CMD_GW_GET_VERSION\n");
				writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_VERSION);
				usleep(UART_WRITE_DELAY);

				usleep(UART_WRITE_DELAY * 10);

				printf("CMD_GW_GET_CHANNEL\n");
				writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_CHANNEL);
				usleep(UART_WRITE_DELAY);
				}

			continue;
		}
//		printf("checkAckfromWrite_thread type(%d)\n",currenttx.type);
		if(currenttx.type == TYPE_SIREN_INDOOR || currenttx.type == TYPE_SIREN_OUTDOOR)
			{
				// retry only 1 time for siren setting
				if(currenttx.cmd >  CMD_SIREN_SET_LED_OFF )
					{
					sensor_timeout = NORMAL_SENSOR;
					retry_times = DEAD_SENSOR_RETRY;
					}
				else
					{
					sensor_timeout = DEEP_SLEEP_SENSOR;
					}
			}
		else
			{
			sensor_timeout = NORMAL_SENSOR;
			}

		if(check_sensor_alive(currenttx.did))
			{
				retry_times = ALIVE_SENSOR_RETRY;
			}
		else
			retry_times = DEAD_SENSOR_RETRY;
		nw = timegettime();
		if(currenttx.time > nw)
			currenttx.time = nw;
		if ( nw - currenttx.time < sensor_timeout )
	//	if ( abs(nw - currenttx.time) < MAX_ACK_TIMEOUT )
		{
	//		printf("##Ack time isnt up in %f\n", (nw - currenttx.time));
			continue;
		}

		currenttx.errCount++;
		if (currenttx.errCount > retry_times)
		{
			//apple
			printf("##UART1 Timer :write to dev %u fail after %d times, txcount = %d\n", currenttx.did,retry_times, txCount);
			set_sensor_dead(currenttx.did);
			//apple 1226
			updateSensorStatus(currenttx.type, currenttx.did);
			if (txCount == 0)
			{//discard

				uartbusy = 0;
				continue;
			}else
			{
				writeNextOnQueue(); //send the next on queue to uart.
			}
		}else
		{
			//printf("##UART Timer :retry writing to  %u , txcount = %d\n", currenttx.did, txCount);
			insertToQueue(&currenttx);
			writeNextOnQueue(); //send the next on queue to uart.
		}
	}
}


unsigned char DecodeRFMessage(ReportRF *ptr){
	static unsigned int last_MAG_did;
	static time_t last_RF_time_tag;
	static char last_RF_time_state;
	RF_PacketFormat *pRFPack = (RF_PacketFormat*)(pRxBuf+1);
	unsigned int  RFDeviceID;
	unsigned char RFDeviceType  ;
	unsigned char RFCommandByte ;
	unsigned char RFControlByte ;
	unsigned char RFControlData ;
	unsigned char *pTxHistory;

	unsigned int did;
	unsigned char type;
	unsigned char cmd;

	memset(ptr,0,sizeof(ReportRF));
	printf("g_sirenstate=%d\tsiren_t=%d\tg_smokeOn=%d\n",g_sirenstate,siren_t, g_smokeOn);
	//---------------------- Acknowledge packet check  -------------------------------//
	// Gateway MCU ACK.
	if(pRFPack->devicesType == 0xFF)
		{

			printf(" Gateway MCU ACK.[%d]\n",RFDeviceID);
//			uartbusy = 1 ;
			return FALSE;		//Ignore Package.
		}
	//Copy the Device Infomation.
	memcpy(&RFDeviceID,&pRFPack->deviceID, 4);
	RFDeviceType  = pRFPack->devicesType;
	RFCommandByte = pRFPack->misc;
	RFControlByte = pRFPack->status[0];
	RFControlData = pRFPack->status[3];

	ptr->DeviceID = RFDeviceID;

	//If is an ACK packet from Rx devices.
	if(RFCommandByte == 0x10){
		printf("Act from sensor uartbusy %d,currentx %u,rfid %u, %d\n", uartbusy,  currenttx.did, RFDeviceID, ptr->Status[0]);
		if (uartbusy ==1 && RFDeviceID == currenttx.did)
		{
			//apple 1226
		    updateSensorStatus(currenttx.type, currenttx.did);
			
			if (RFDeviceID == 0x12345678)
			{
				sendCmdtoClient(CM_RFTX, 0,  0,  0,NULL) ;
			}
			//send next on queue.
			if ( txCount > 0)
			{
				writeNextOnQueue();
			}else
				uartbusy = 0;
			//if(uartbusy != 0)
				return FALSE;
		}
		//pTxHistory = IsAckDeviceCorrect(RFDeviceID);
		//if( pTxHistory != 0 ){		//If ID correct.
		//	ptr->DeviceType = *(pTxHistory + 5);
		//	ptr->Status[0] = *(pTxHistory + 6);	//Copy the status[0],
		//	printf("Translate ACK packet.\n");
		//	return TRUE;
		//}

/*		if(RFDeviceID == (unsigned int)*(pTxBuf+1) ){
			ptr->DeviceType = RFCommandByte;//ACK packet "misc" represent the target devicetype.
			RFControlByte = *(pTxBuf + 7);	//Filled with status ever send.
		}	*/
	}
	if (RFDeviceType == DT_ONBOARD_TEMP )
	{
		if (currenttx.did == temp_did)
		{
			//printf("get onbaord temp\n");
			if ( txCount > 0)
			{
				//printf("getting onbaord temperature write next on queue\n");
				writeNextOnQueue();
			}else
				uartbusy = 0;
		}
	}

	//Part1. Analyse the Device Type.
	switch(RFDeviceType){
		case DT_GATEWAY_MCU:
			ptr->DeviceType = TYPE_MCU;			break;

		case DT_MAG:
			ptr->DeviceType = TYPE_MAGNETIC;	break;

		case DT_PIR:
			ptr->DeviceType = TYPE_PIR;			break;

		case DT_VIBRATION:
			ptr->DeviceType = TYPE_VIBRATION; 		break;

		case DT_REMOTE:
			ptr->DeviceType = TYPE_REMOTE_CONTROL;		break;

		case DT_REMOTE_NEW:
//			ptr->DeviceType = TYPE_REMOTE_CONTROL_NEW;		break;
			ptr->DeviceType = TYPE_REMOTE_CONTROL;		break;

		case DT_SIREN_OUTDOOR:
			ptr->DeviceType = TYPE_SIREN_OUTDOOR;		break;

		case DT_SIREN_INDOOR:
			ptr->DeviceType = TYPE_SIREN_INDOOR;		break;

		case DT_ADAPTER:
			ptr->DeviceType = TYPE_POWER_ADAPTER;		break;

		case DT_DOORCHIME:
			ptr->DeviceType = TYPE_DOORCHIME;		break;

		case DT_BUTTON:
			ptr->DeviceType = TYPE_BUTTON;		break;

		case DT_TEMPERATURE:
			ptr->DeviceType = TYPE_TEMPERATURE;	break;

		case DT_ONBOARD_TEMP:
			ptr->DeviceType = TYPE_ONBOARD_TEMP;break;

		case DT_SMOKE:
			ptr->DeviceType = TYPE_SMOKE;		break;

		case DT_DIGITAL_KEY: //DT_KEYPAD_JSW:
			ptr->DeviceType = TYPE_KEYPAD_JSW;	break;

		case DT_DIGITAL_KEY_NEW:
			//ptr->DeviceType = TYPE_KEYPAD_JSW_NEW;	break;
			ptr->DeviceType = TYPE_KEYPAD_JSW;	break;

		case DT_WATERLEAK:
			ptr->DeviceType = TYPE_WATERLEVEL;	break;
		default:
			ptr->DeviceType = TYPE_UNKNOW;		break;

	}//End of switch device type.

	//Part2:Fot translate packet general information.
	if(*pRxBuf == 0xAA){	//API command
		memcpy(ptr->Status,pRFPack->status,4);
		ptr->DeviceType = TYPE_MCU_COMMAND;
		printf("MISC[%d]\n",RFCommandByte);
		switch(RFCommandByte)
		{
			case 0xC1:
				//CMD_GW_GET_VERSION:
			{
				int vv = 0;
				unsigned char bb = ptr->Status[0]-'0';
				vv += (bb*1000);
				bb = ptr->Status[1]-'0';
				vv += (bb*100);
				bb = ptr->Status[2]-'0';
				vv += (bb*10);
				bb = ptr->Status[3]-'0';
				vv += (bb*1);
				printf("MCU version=%d\n", vv);
				if( (vv >= 1000) && (vv <= 9999) )
				{
					get_mcu_version = 1;
					g_setting.gwprop.mcu_version = vv;
					memcpy(mcuversion, &ptr->Status, sizeof(mcuversion));
				}

				//g_setting.gwprop.mcu_version = 1411; //for MCU 1411 only
				printf("g_setting.gwprop.mcu_version=%d\n", g_setting.gwprop.mcu_version);
			}

			break;
			case 0xC3:
				//CMD_GW_GET_CUSTOMER:
				{
					int vv = 0;
					unsigned char bb = ptr->Status[0]-'0';
					vv += (bb*1000);
					bb = ptr->Status[1]-'0';
					vv += (bb*100);
					bb = ptr->Status[2]-'0';
					vv += (bb*10);
					//bb = ptr->Status[3]-'0';
					//vv += (bb*1);
					printf("RF Type=%d\n", vv);
					if(vv == 9160)
						g_setting.gwprop.ledon = 916;
					if(vv == 8680)
						g_setting.gwprop.ledon = 868;
					if(vv == 9210)
						g_setting.gwprop.ledon = 921;
					//g_setting.gwprop.mcu_version = 1411; //for MCU 1411 only
					printf("g_setting.gwprop.ledon=%d\n", g_setting.gwprop.ledon);
				}

			break;
			case 0xC4:
				//CMD_GW_GET_CHANNEL:

			break;
			case 0xC5:
				//CMD_GW_GET_DEFAULT_SYNCWORD:

			break;
			default:
				printf("NOKNOW MCU COMMAND\n");
			break;

		}
		//return TRUE;
		return 0;
	}

	if(RFCommandByte == 0x20){	//Pair Packet
		ptr->Status[0] = RE_SENSOR_PAIR;
		return TRUE;
	}
	else if (RFCommandByte == 0x80){ // Abus auto report status.
		ptr->Status[0] = RE_ABUS_AUTOREPORT;
		set_sensor_alive(ptr->DeviceID);
		return TRUE;
	}

	//if(RFControlByte == 0x80){	//Low battery Packet
	//	ptr->Status[0] = RE_SENSOR_BATLOW;
	//	return TRUE;
	//}

	 if(RFControlByte & 0x80){ //Low battery Packet Flag.
		  ptr->Status[0] = RE_SENSOR_BATLOW;
		  CallBackFunc(ptr);
		  RFControlByte = (RFControlByte & ~(1<<7)); //Mask the Battery Low flag.

		  if (RFControlByte ==0 )
			  return 0; //no payload
	}
	memcpy(ptr->Status, pRFPack->status, 4); 	//ControlByte[4].

	set_sensor_alive(ptr->DeviceID);
	switch(ptr->DeviceType){

		case TYPE_UNKNOW:
				ptr->Status[0] = RE_UNKNOW_STATUS;		break;

		case TYPE_MCU:
			switch(RFControlByte){
				case 0x00:
					ptr->Status[0] = RE_MCU_ACPOW_OFF;	break;
				case 0x01:
					ptr->Status[0] = RE_MCU_ACPOW_ON;	break;
				case 0x08:
					ptr->Status[0] = RE_MCU_TEMP_TIMEOUT; break;
				case 0x09:
					ptr->Status[0] = RE_MCU_RCV_ERROR; break;
				case 0x0A:
					ptr->Status[0] = RE_MCU_RESET_RF_IC; break;
				case 0x0B:
					ptr->Status[0] = RE_MCU_RESET_RF_IC2; break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;

		case TYPE_MAGNETIC:

			if(((time(NULL) - last_RF_time_tag) <= 1) && ((time(NULL) - last_RF_time_tag) >=0))
				{
				if( RFDeviceID == last_MAG_did)
					{
						if(ptr->Status[0] == last_RF_time_state)
							{
							printf("bypass multiply tigger !!!!!!");
							return 0;
							}
					}
				}
			last_MAG_did = RFDeviceID;
			last_RF_time_tag = time(NULL);
			last_RF_time_state = ptr->Status[0];
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_MAG_ISON;		break;
				case 0x02:
					ptr->Status[0] = RE_MAG_ISOFF;		break;
				case 0x10:
					ptr->Status[0] = RE_MAG_TEMPER;		break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;


		case TYPE_PIR:
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_PIR_MOTION;		break;
				case 0x10:
					ptr->Status[0] = RE_PIR_TEMPER;		break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;


		case TYPE_VIBRATION:
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_VIBRATION_TRIGGER;		break;
				case 0x10:
                    ptr->Status[0] = RE_VIBRATION_TAMPER; 	break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;
		case TYPE_BUTTON:
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_BUTTON_PRESS;		break;
				case 0x13:
					ptr->Status[0] = RE_BUTTON_LONG_PRESS;		break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;


		case TYPE_REMOTE_CONTROL:
		case TYPE_REMOTE_CONTROL_NEW:
			switch(RFControlByte){
				case 0x01:
					if(Customer_code == CUSTOMER_ALC)
						ptr->Status[0] = RE_REMOTE_PART_ARM;
					else
						ptr->Status[0] = RE_REMOTE_CAMERA;
					break;
				case 0x02:
					ptr->Status[0] = RE_REMOTE_PANIC;	break;
				case 0x04:
					ptr->Status[0] = RE_REMOTE_UNLOCK;	break;
				case 0x08:
					ptr->Status[0] = RE_REMOTE_LOCK;	break;
				case 0x10:
					ptr->Status[0] = RE_REMOTE_TEMPER;	break;
				case 0x20:
					ptr->Status[0] = RE_REMOTE_PART_ARM;break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;


		case TYPE_KEYPAD_JSW:
		case TYPE_KEYPAD_JSW_NEW:
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_KEYPAD_CAMERA;	break;
				case 0x02:
					ptr->Status[0] = RE_KEYPAD_PANIC;	break;
				case 0x04:
					ptr->Status[0] = RE_KEYPAD_UNLOCK;	break;
				case 0x08:
					ptr->Status[0] = RE_KEYPAD_LOCK;	break;
				case 0x10:
					ptr->Status[0] = RE_KEYPAD_TEMPER;	break;
				case 0x20:
					ptr->Status[0] = RE_KEYPAD_PART_ARM;break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;



		case TYPE_SIREN_INDOOR:
		case TYPE_SIREN_OUTDOOR:
//			set_sensor_alive(ptr->DeviceID);
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_SIREN_ISON;		break;
				case 0x02:
					ptr->Status[0] = RE_SIREN_ISOFF;	break;
				case 0x10:
					// Jeff to void RE_SIREN_TEMPER at Siren ACK package
					if(RFCommandByte == 0x10)
						{
						printf("to void RE_SIREN_TEMPER at Siren ACK package\n");
						return FALSE;
						}
					ptr->Status[0] = RE_SIREN_TEMPER;		break;
				case 0x03:
					ptr->Status[0] = CMD_SIREN_SET_LED_ON; 	break;
				case 0x05:
					ptr->Status[0] = CMD_SIREN_SET_LED_OFF;	break;

				case 0x20:
					switch(RFControlData){
						case 0:
							ptr->Status[0] = RE_SIREN_SET_VOLUME_OFF;	break;
						case 4:
							ptr->Status[0] = RE_SIREN_SET_VOLUME_LOW;	break;
						case 5:
							ptr->Status[0] = RE_SIREN_SET_VOLUME_HIGH;	break;

						default : ptr->Status[0] = RE_UNKNOW_STATUS;	break;
					}
				case 0x40:
					switch(RFControlData){
						case 10:
							ptr->Status[0] = RE_SIREN_SET_PERIOD_10;	break;
						case 30:
							ptr->Status[0] = RE_SIREN_SET_PERIOD_30;	break;
						case 60:
							ptr->Status[0] = RE_SIREN_SET_PERIOD_60;	break;
						case 120:
							ptr->Status[0] = RE_SIREN_SET_PERIOD_120;	break;
						case 180:
							ptr->Status[0] = RE_SIREN_SET_PERIOD_180;	break;

						default : ptr->Status[0] = RE_UNKNOW_STATUS;	break;
					}
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;


		//apple
		case TYPE_POWER_ADAPTER:
	//		set_sensor_alive(ptr->DeviceID);
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_ADAPTER_ISON;	break;
				case 0x02:
					ptr->Status[0] = RE_ADAPTER_ISOFF;	break;
				case 0x20:
					ptr->Status[0] = RE_ADAPTER_ISOFF;	break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}
			break;

		case TYPE_SMOKE:
			switch(RFControlByte){
				case 0x01:
					ptr->Status[0] = RE_SMOKE_TRIGGERED;break;
				case 0x04:
					ptr->Status[0] = RE_SMOKE_OVERHEAT;break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;

		case TYPE_ONBOARD_TEMP:
			//Only needs to bypass the data.
			//printf("By pass temp decode \n");
			break;

		case TYPE_WATERLEVEL:
			switch(RFControlByte){
				case 0x20:
					ptr->Status[0] = RE_WATERLEVEL_ARM1;break;
				case 0x40:
					ptr->Status[0] = RE_WATERLEVEL_ARM2;break;
				default:
					ptr->Status[0] = RE_UNKNOW_STATUS;	break;
			}break;
		default:
			return FALSE;
	}//End of switch device type.
//	printf("Decode Success\r\n");
	return TRUE;
}



//Return zero when fail,else return the history tx message in txList.
unsigned char* IsAckDeviceCorrect(unsigned int devieceID){
	printf("ACK ID:%d\n",devieceID);
	unsigned char (*pCheckTxList)[12] = pInputTxList;	//pointer to array[12].
	unsigned int ID;
	unsigned char count = 0;

	do{
		if (pCheckTxList == txList)	//Reach the edge of the buffer.
			pCheckTxList = &txList[TX_LIST_SIZE-1];
		else
			pCheckTxList--;

		memcpy(&ID, pCheckTxList,4);


		unsigned char i;
	//	printf("data:");
	/*	for(i=0;i<11;i++)
			printf(" %x",*((unsigned char*)pCheckTxList+i));
		printf("\n");	*/

		if(++count > TX_LIST_SIZE){
			printf("Can't find ID.\n");
			return 0;
		}
	}while(devieceID != ID);

	//printf("Find ACK in list\n");
	return (unsigned char *)pCheckTxList;
}

unsigned char TranslateDeviceType(unsigned char RFDeviceType){

	switch(RFDeviceType){
		case TYPE_MAGNETIC:
			return DT_MAG;

		case  TYPE_PIR:
			return DT_PIR;

		case  TYPE_VIBRATION:
			return DT_VIBRATION;

		case  TYPE_DOORCHIME:
			return DT_DOORCHIME;

		case  TYPE_BUTTON:
			return DT_BUTTON;

		case  TYPE_REMOTE_CONTROL:
			return DT_REMOTE;

		case  TYPE_REMOTE_CONTROL_NEW:
			return DT_REMOTE_NEW;

		case  TYPE_SIREN_OUTDOOR:
			return DT_SIREN_OUTDOOR;

		case  TYPE_SIREN_INDOOR:
			return DT_SIREN_INDOOR;

		case  TYPE_POWER_ADAPTER:
			return DT_ADAPTER;

		case  TYPE_TEMPERATURE:
			return DT_TEMPERATURE;

		case  TYPE_ONBOARD_TEMP:
			return DT_ONBOARD_TEMP;

		case  TYPE_SMOKE:
			return DT_SMOKE;

		case  TYPE_KEYPAD_JSW:
			return DT_DIGITAL_KEY; //DT_KEYPAD_JSW;

		case  TYPE_KEYPAD_JSW_NEW:
			return DT_DIGITAL_KEY_NEW; //DT_KEYPAD_JSW;

		case  TYPE_WATERLEVEL:
			return DT_WATERLEAK;



		default:
			return  0;

	}
}
//-----------------------------------

//=================================================================================
void McuGpio(unsigned char Gpio, unsigned char OnDelay, unsigned char OffDelay, unsigned short Times){

                unsigned char buffer[12] = {0};

                                buffer[0] = 0xD1;
                                buffer[1] = Gpio;
                                buffer[2] = OnDelay;
                                buffer[3] = OffDelay;
                                buffer[4] = Times & 0xFF;
                                buffer[5] = (Times >> 8) & 0xFF;
                                PutTxBuffer(1,buffer, 10, TRUE);
}

//send RF cmd to GW MCU (with data)
void McuGpio2(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4, unsigned char b5, unsigned char b6)
{
    unsigned char buffer[12] = {0};
	//printf("McuGpio2(%d)\n",(int)b1);
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = b1;
    buffer[1] = b2;
    buffer[2] = b3;
    buffer[3] = b4;
    buffer[4] = b5;
    buffer[5] = b6;
    PutTxBuffer(1,buffer, 10, TRUE);
}

void set_comstom_code(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4, unsigned char b5, unsigned char b6)
{
    unsigned char buffer[12] = {0};

    memset(buffer, 0, sizeof(buffer));
    buffer[0] = b1;
    buffer[6] = b2;
    buffer[7] = b3;
    buffer[8] = b4;
    buffer[9] = b5;
    buffer[10] = b6;
    PutTxBuffer(1,buffer, 10, TRUE);


}

//=================================================================================
