/*
 * ceres.c
 *
 *  Created on: 2014年1月26日
 *      Author: yulin
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <crypt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>

#include "ceres.h"
#include "jsw_protocol.h"
#include "AES/AES_utils.h"
#include "rf_module.h"

#include <signal.h>
#include "snx_gpio.h"


#ifdef SEARCH_BROADCAST_ENABLE
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include "jsw_rf_api.h"
#include "ceres_util.h"
#include "nest_realtime.h"
#include "cloud_action_helper.h"
#include "cloud_event_sender.h"

#define WARN_MSG(_STR_, args...) printf("<%s> "_STR_, __FILE__, ##args);

#define LED_SETTING_P2P_OK ". jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 14 -v 1" //". jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 14 -v 0; . jsw_control_led.sh 12 -v 1"
#define LED_SETTING_P2P_NG2 ". jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 14 -v 0" //". jsw_control_led.sh 14 -v 0; . jsw_control_led.sh 14 -b 1&" //". jsw_control_led.sh 12 -v 0; . jsw_control_led.sh 14 -b 1&"
#define LED_SETTING_P2P_NG ". jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 14 -v 0" //". jsw_control_led.sh 12 -v 0; . jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 14 -v 0"
#define LED_SETTING_P2P_FLASH ". jsw_control_led.sh 14 -v 0; . jsw_control_led.sh 14 -b 1&" //". jsw_control_led.sh 12 -v 0; . jsw_control_led.sh 14 -b 1&"
//#define LED_FW_UPGRADE ". jsw_control_led.sh 12 -b 1&"
#define LED_FW_UPGRADE ". jsw_control_led.sh 11 -b 1&"

#define PLAY_BUZZER_NETWORK_CONNECTED ". jsw_network_connected.sh&"
#define PLAY_BUZZER_ARM_OK ". jsw_arm_ok.sh&"

#ifndef CERES_UTIL
CURL *gCurl;
#endif

#define SCRIPT_LEN  1024
#define SCRIPT_KEYWORD  "---Gateway FW Update Script---"
#define OUTPUT_FILENAME "ceres"

#define MCU_ACK_THREAD
char mcuversion[4];
char get_mcu_version;
unsigned int g_DID; //did for mcu
CHAR gDID[32] = { 0 };
//CHAR gLicense[16] = { 0 };
CHAR gPassword[32] = { 0 };
CHAR gInitString[256] = { 0 };
CHAR gAPIChecksum[32] = { 0 };
//CHAR gLuaScript[32] = { 0 };
CHAR gVersion[16] = { 0 };

char gRunningLog[LEN_RUNNING_LOG]; //running log in ram
int g_selftest_flag = 0;//self-test flag, write to running log

//INT32 gRequestEventSession = -1;

char sdpath[128];

unsigned int temp_did = 90; //HARDCODE DEVICE ID FOR ONBOARD TEMPERATURE
short g_temp = 0;
short g_humid = 0;


#define AES_KEY_LENGTH		16
static char *AES_KEY_DEFAULT = "0123456789ABCDEF";



static int gAESKeyNum = 1;
static unsigned char **gAESKeys = NULL;

short g_fwupgrade = 0;
struct fwupdateinfo g_fwinfo;
FILE *fpfileupdate=NULL;
char fwbuffer[90000];

//for UART/P2P thread block
int g_get_temp = 0;//count for write get temp command to uart
int g_get_temp2 = 0;//count for judge p2p thread blocking

int g_network_detect = 1;//if local network, use PPPP_NetworkDetect or not
int previous_net_status = -1;//0:green(w network), 1:off(w/o network, w rj45), 2:flash red(w/o network, w/o rj45),


 timer_t pairing_t =0;
 timer_t pairing_current_t =0;
 timer_t gettemp_t = 0;
 timer_t exitdelay_t = 0;
 timer_t exitdelay_current_t = 0;
 timer_t entrydelay_t = 0;
 timer_t entrydelay_current_t = 0;
 timer_t rftest_t = 0;
 timer_t writeack_t = 0;
 timer_t schedule_t =0;
 timer_t keepalive_t =0;
 timer_t siren_t = 0;
 timer_t siren_current_t = 0;
 timer_t service_t = 0;
 timer_t smoke_t = 0;
 timer_t ipcreconnect_t = 0;
 timer_t acctreset_t = 0;
 timer_t rftx_t =0;
 timer_t testmode_t =0;
 timer_t NEST_device_update_t = 0;

int NEST_device_update = 0;

extern pthread_t alarmstop_th;
void *stopgatewayalarm_thread(void *ptr );

extern struct setting4info g_setting4;
extern short exitdelayTarget;
extern struct setting5info g_setting5;
extern int g_ShowAlarmView;//=0; //0=not show, 1=Show.
extern int g_AlarmState;//=ALARM_STATE_NO_ALARM;
extern int g_smokeOnLastTime;// = 0; //Last smoke trigger time
extern pthread_t threadID;
extern jswsunrisesunset g_sunrisesunset[2];

pthread_t philipt_hue_handle = NULL;


extern stNEST_parm g_stNEST_parm;
extern int g_NEST_running;
extern char g_NEST_active_token[256];
extern stUSER_parm g_stUserParam;
extern int g_cloud_running; 

//CHAR *gFirmwareSavePath = "/tmp/gateway.zip";
//CHAR *gFirmwareSavePathMmcblk0 = "/media/mmcblk0/gateway.zip";
//CHAR *gFirmwareSavePathMmcblk0p1 = "/media/mmcblk0p1/gateway.zip";

// 解压之后,不要立刻执行升级脚本,而是立刻重启
//CHAR *update_cmd_mmcblk0 = "/usr/bin/unzip /media/mmcblk0/gateway.zip -d /media/mmcblk0/;chmod +x /media/mmcblk0/gateway/update.sh;sync;reboot";
//CHAR *update_cmd_mmcblk0p1 = "/usr/bin/unzip /media/mmcblk0p1/gateway.zip -d /media/mmcblk0p1/;chmod +x /media/mmcblk0p1/gateway/update.sh;sync;reboot";
//CHAR *delete_mmcblk0_error_zipfile = "rm /media/mmcblk0/gateway.zip";
//CHAR *delete_mmcblk0p1_error_zipfile = "rm /media/mmcblk0p1/gateway.zip";



st_p2p_session_record gP2PSession[P2P_SESSION_MAX];

//int globalSession = -1;
//
//#define CH_CTRL		0
//#define CH_DAT		2
//#define CH_FILE		3

// chenjian 2014-04-22 add GCM
//const char *URL_SERVER = "http://54.251.116.21/";		// StarxNet
const char *URL_SERVER = "http://54.243.57.233/";		// JSW 54.166.73.92



extern unsigned char *delTailStr(unsigned char *decryptStr);

#ifdef LOG_FILE
CHAR *gLogFilePathMmcblk0 = "/media/mmcblk0/dqa_log.txt";
CHAR *gLogFilePathMmcblk0p1 = "/media/mmcblk0p1/dqa_log.txt";
CHAR *gLogfilePath = NULL;
FILE *fLogfile;

time_t timep;
char tStr[128] = { 0 };

void writeLogFile(const char *log) {
	if (fLogfile) {
		time (&timep);
		sprintf(tStr, "%s", asctime(gmtime(&timep)));
		fwrite(tStr, strlen(tStr), 1, fLogfile);
		fwrite("> ", strlen("> "), 1, fLogfile);
		fwrite(log, strlen(log), 1, fLogfile);
		fwrite("\n\n", strlen("\n\n"), 1, fLogfile);
		fflush(fLogfile);
	}
}


void openLogFile() {
	if (isMmcblk0Valid())
		gLogfilePath = gLogFilePathMmcblk0;
	else if(isMmcblk0p1Valid())
		gLogfilePath = gLogFilePathMmcblk0p1;
	if (gLogfilePath) {
		fLogfile = fopen(gLogfilePath, "a+");
		if (NULL == fLogfile) {
			DBG_PRINT("create log file error in %s !\n", gLogfilePath);
		}
		else{
			writeLogFile("\n================ Gateway Poweron ==============\n");
		}
	}
}
#endif



void mSecSleep(unsigned int ms) {
	usleep(ms * 1000);
}


// free old stack
void free_AES_key_stack()
{
	int k = 0;
	if (NULL != gAESKeys) {
		for (k = 0; k < gAESKeyNum; k++) {
			if (NULL != gAESKeys[k])
				free((void *)gAESKeys[k]);
		}

		free((void *)gAESKeys);
	}
}

void malloc_AES_key_stack(int num) {
//	DBG_PRINT("malloc_AES_key_stack(). num = %d \n", num);
	int k = 0;
	free_AES_key_stack();
	gAESKeyNum = num;
	// malloc new stack
	gAESKeys = (unsigned char **) malloc(gAESKeyNum * sizeof(unsigned char *));
	for (k = 0; k < gAESKeyNum; k++) {
		gAESKeys[k] = (unsigned char *) malloc(AES_KEY_LENGTH + 1);
		memset(gAESKeys[k], 0, AES_KEY_LENGTH + 1);
	}
}


#ifndef CERES_UTIL
void *email_thread_rountine(void *ptr){
	pthread_mutex_lock(&email_mutex);
//	DBG_PRINT("email_thread_rountine running...\n");

	const char *email_cmd = (const char *)ptr;
//	DBG_PRINT("email_cmd = %s\n", email_cmd);
	system(email_cmd);
	free((void *)email_cmd);
	pthread_mutex_unlock(&email_mutex);
//	DBG_PRINT("email_thread_rountine exit \n");
}
#endif


int isMmcblk0Valid()
{
	return (0 == access("/media/mmcblk0/sn98601/ceres", 0));
}

int isMmcblk0p1Valid()
{
	return (0 == access("/media/mmcblk0p1/sn98601/ceres", 0));
}



INT32 PPPP_Read_Wrap(int session, unsigned char Channel, char * Buf,
	int *SizeToRead, unsigned int TimeOut_ms) {
	INT32 count = 0;
	INT32 ret;
	unsigned int nTotalRead=0;
	unsigned int nReadSize=0;
	int err = 0;

	int nSize = *SizeToRead;
	while (nTotalRead < nSize) {
		nReadSize = nSize - nTotalRead;

		ret = PPPP_Read(session,
						  Channel,
						  Buf + nTotalRead,
						  &nReadSize, TimeOut_ms);

		nTotalRead += nReadSize;

		if (ret == ERROR_PPPP_SUCCESSFUL)
			continue;
		else if ((TimeOut_ms < 0xFFFFFFFF) && (ret == ERROR_PPPP_TIME_OUT))
		{
			err++;
		}
		else
			break;

		if (err > 50)
			break;

	}

	*SizeToRead = nTotalRead;

	return ret;
}

int getValidP2PSessionNum() {
	int total = 0;

	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (1 == gP2PSession[i].valid)
			total++;
	}

	return total;
}

int getP2PSessionFreeIndex() {
	int index = -1;
	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (0 == gP2PSession[i].valid) {
			index = i;
			break;
		}
	}
	if(index == -1)
	{//full, return first one (connected time)
		index = 0;
		for (i = 1; i < P2P_SESSION_MAX; i++) {
			if(gP2PSession[i].connect_time < gP2PSession[index].connect_time)
			{
				index = i;
			}
		}
		gP2PSession[index].valid = 0;
		gP2PSession[index].session = -1;
		memset(gP2PSession[index].aeskey, 0, AES_KEY_LENGTH);
		gP2PSession[index].connect_time = 0;
		gP2PSession[index].use_AES_data = 0;
	}

	//DBG_PRINT("getP2PSessionFreeIndex(). index = %d \n", index);
	return index;
}

void freeP2PSession(INT32 session) {
	//DBG_PRINT("freeP2PSession(). session = %d \n", session);

	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (session == gP2PSession[i].session) {
		//	DBG_PRINT("find: index = %d\n", i);
			gP2PSession[i].valid = 0;
			gP2PSession[i].session = -1;
			memset(gP2PSession[i].aeskey, 0, AES_KEY_LENGTH);
			gP2PSession[i].connect_time = 0;
			gP2PSession[i].use_AES_data = 0;
			break;
		}
	}
}
void freeOtherP2PSession(INT32 session) {
	//DBG_PRINT("freeP2PSession(). session = %d \n", session);

	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (session == gP2PSession[i].session || gP2PSession[i].valid == 0) continue;
			else {
			printf("Force release user [%d]\n", i);

			PPPP_Close(gP2PSession[i].session);
			gP2PSession[i].valid = 0;
			gP2PSession[i].session = -1;
			memset(gP2PSession[i].aeskey, 0, AES_KEY_LENGTH);
			gP2PSession[i].connect_time = 0;
			gP2PSession[i].use_AES_data = 0;
			}
	}
}


void addP2PSession(int index, INT32 session) {
	//DBG_PRINT("freeP2PSession(). index = %d, session = %d \n", index, session);

	gP2PSession[index].valid = 1;
	gP2PSession[index].session = session;
	gP2PSession[index].connect_time = time(NULL);
}

void updateP2PSessionAesKey(INT32 session, const char *aeskey, int useAES) {
	//DBG_PRINT("updateP2PSessionAesKey(). session = %d, aeskey = %s \n", session, aeskey);
	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (1 == gP2PSession[i].valid && session == gP2PSession[i].session) {
			strncpy((char*) (gP2PSession[i].aeskey), aeskey, AES_KEY_LENGTH);
			gP2PSession[i].use_AES_data = useAES;
			break;
		}
	}
}

const char *getP2PSessionAesKey(INT32 session) {
//	DBG_PRINT("getP2PSessionAesKey(). session = %d \n", session);
	int i = 0;
	char *res = NULL;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (1 == gP2PSession[i].valid && session == gP2PSession[i].session) {
			res = (char*)(gP2PSession[i].aeskey);
			break;
		}
	}

//	DBG_PRINT("*************res = %s\n", res);
	return (const char*)res;
}

int getP2PSessionUseAESData(INT32 session) {
	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (1 == gP2PSession[i].valid && session == gP2PSession[i].session) {
		    return gP2PSession[i].use_AES_data;
		}
	}

	return 1;//default is useing AES Data
}

int getP2PSessionIndex(INT32 session) {
	int i = 0;
	for (i = 0; i < P2P_SESSION_MAX; i++) {
		if (1 == gP2PSession[i].valid && session == gP2PSession[i].session) {
		    return i;
		}
	}

	return -1;//not found
}

void dumpSessionInfo(INT32 session) {
	st_PPPP_Session Sinfo;
	if (PPPP_Check(session, &Sinfo) == ERROR_PPPP_SUCCESSFUL) {
		DBG_PRINT("*************** Session Information ***************\n");
		//DBG_PRINT("Socket : %d\n", Sinfo.Skt);
		DBG_PRINT("Remote Addr : %s:%d\n", inet_ntoa(Sinfo.RemoteAddr.sin_addr),
				ntohs(Sinfo.RemoteAddr.sin_port));
		//DBG_PRINT("My Lan Addr : %s:%d\n", inet_ntoa(Sinfo.MyLocalAddr.sin_addr),
		//		ntohs(Sinfo.MyLocalAddr.sin_port));
		//DBG_PRINT("My Wan Addr : %s:%d\n", inet_ntoa(Sinfo.MyWanAddr.sin_addr),
		//		ntohs(Sinfo.MyWanAddr.sin_port));
		//DBG_PRINT("Connection time : %d second before\n", Sinfo.ConnectTime);
		//DBG_PRINT("DID : %s\n", Sinfo.DID);
		//DBG_PRINT("I am %s\n", (Sinfo.bCorD == 0) ? "Client" : "Device");
		DBG_PRINT("Connection mode: %s\n", (Sinfo.bMode == 0) ? "P2P" : "RLY");
		//DBG_PRINT("************* Session Information End *************\n");
	}
}


void *
p2p_thread_service(void *arg) {
	INT32 tSession = (INT32) arg;

	struct _jswproto_hdr *phdr;
	int P2P_USE_AES_DATA = 1;//default use AES data

	SetRunningLog_str_int("Running: Session create:", tSession);

	do {

		struct _jswproto_hdr hdr;
		char buf2[256], random_code[32], random_code2[32], hash_code[32];
		int i, outsize;
		CHAR buf[512];
		int DataSize;
		int plen = 0;
		INT32 ret = 0;
		st_Control_Proto_Auth *auth=NULL;

		if(P2P_USE_AES_DATA == 1)
		{
			memset(&hdr, 0, sizeof(struct _jswproto_hdr));
			hdr.version = PRO_VERSION;
			hdr.cmd = CM_AUTH_ENCRYPT;//CM_AUTH
			hdr.payload_length = 16;
			hdr.count = 1;
			memset(buf2, 0, sizeof(buf2));
			memcpy(buf2, &hdr, sizeof(struct _jswproto_hdr));
			memset(random_code, 0, sizeof(random_code));
			srand((int)time(0)-97531246);
			for(i=0;i<16;i++)
			{
				random_code[i] = get_rand_alphanumeris();
			}
			memcpy(buf2+sizeof(struct _jswproto_hdr), random_code, 16);//strlen(random_code));

			PPPP_Write(tSession, CH_CTRL, (CHAR*) &buf2, sizeof(struct _jswproto_hdr)+16);//strlen(random_code));

			// Wait for Auth
			auth = (st_Control_Proto_Auth *) (buf + sizeof(struct _jswproto_hdr));
			memset(&buf, 0, sizeof(buf));
			//DataSize = sizeof(st_Control_Proto_Header)
			//		+ sizeof(st_Control_Proto_Auth);

			DataSize = sizeof(struct _jswproto_hdr)	+ sizeof(st_Control_Proto_Auth);
			//DBG_PRINT("wait for auth key, datasize:%d!!\n", DataSize);
			ret = PPPP_Read_Wrap(tSession, CH_CTRL, buf, &DataSize, 3000);

			//DBG_PRINT("PPPP_Read_Wrap return value:%d , datasize%d!!\n",ret,  DataSize);

			phdr = (struct _jswproto_hdr*)buf;
			plen = sizeof(jswhdr);

			if (ret == ERROR_PPPP_TIME_OUT) {
				P2P_USE_AES_DATA = 0;//no aes data
			}
		}

		if(P2P_USE_AES_DATA == 0)
		{//try to connect without aes data
			memset(&hdr, 0, sizeof(struct _jswproto_hdr));
			hdr.version = PRO_VERSION;
			hdr.cmd = CM_AUTH;
			PPPP_Write(tSession, CH_CTRL, (CHAR*) &hdr, sizeof(struct _jswproto_hdr));

			// Wait for Auth
			auth = (st_Control_Proto_Auth *) (buf + sizeof(struct _jswproto_hdr));
			memset(&buf, 0, sizeof(buf));
			//DataSize = sizeof(st_Control_Proto_Header)
			//		+ sizeof(st_Control_Proto_Auth);

			DataSize = sizeof(struct _jswproto_hdr)	+ sizeof(st_Control_Proto_Auth);
			//DBG_PRINT("wait for auth key, datasize:%d!!\n", DataSize);
			ret = PPPP_Read_Wrap(tSession, CH_CTRL, buf, &DataSize, 3000);

			//DBG_PRINT("PPPP_Read_Wrap return value:%d , datasize%d!!\n",ret,  DataSize);

			phdr = (struct _jswproto_hdr*)buf;
			plen = sizeof(jswhdr);
		}

		if (ret == ERROR_PPPP_TIME_OUT) {
			DBG_PRINT("Auth failed - time out!!\n");
			pthread_mutex_lock(&p2psession_mutex);
			freeP2PSession(tSession);
			pthread_mutex_unlock(&p2psession_mutex);
			PPPP_Close(tSession);
            SetRunningLog_str_int("Running: Session closed by timeout", ret);
			pthread_exit(0);
		}
		if (ret != ERROR_PPPP_SUCCESSFUL) {
			DBG_PRINT("Auth failed -connection failed, Error=%d\n", ret);
			pthread_mutex_lock(&p2psession_mutex);
			freeP2PSession(tSession);
			pthread_mutex_unlock(&p2psession_mutex);
			PPPP_Close(tSession);
            SetRunningLog_str_int("Running: Session closed by unknown", ret);
			pthread_exit(0);
		}

		// Check header
		//DBG_PRINT("info->%x, cmd:%x\n", pheader->info, pheader->info >> 24);
		if( ( phdr->cmd != CM_AUTH) && ( phdr->cmd != CM_AUTH_ENCRYPT) )
		{
			DBG_PRINT("Not Auth message, phdr->cmd=%d!!\n", phdr->cmd);
			pthread_mutex_lock(&p2psession_mutex);
			freeP2PSession(tSession);
			pthread_mutex_unlock(&p2psession_mutex);
			PPPP_Close(tSession);
            SetRunningLog_str_int("Running: Session closed by auth failed", tSession);
			pthread_exit(0);
		}
		// Check DataSize
		/*if ( phdr->payload_length != sizeof(st_Control_Proto_Auth)) {
			DBG_PRINT("Not correct Auth message, Size mismatch!!\n");
			pthread_mutex_lock(&p2psession_mutex);
			freeP2PSession(tSession);
			pthread_mutex_unlock(&p2psession_mutex);
			PPPP_Close(tSession);
			pthread_exit(0);
		}	*/

		//DBG_PRINT("####start auth decode payload length =%d, command type = %d\n", phdr->payload_length , phdr->cmd );
		int bAuthResult = -1;
		int aeskeyIndex = 0;
		char *decryptStr = NULL;
		//unsigned char *decrytStr = NULL;
		unsigned char *plainStr = NULL;
		//new decrypt
		if(P2P_USE_AES_DATA == 1)
		{
            SetRunningLog_str("Running: Session mode: with AES connection");
			memset(hash_code, 0, sizeof(hash_code));
			if (AES_ENABLE)
			{
				if (0 == (DataSize - plen) % 16)
				{
					gen_hashcode(gDID, g_setting.acct.loginpwd, hash_code);
					pthread_mutex_lock(&aes_mutex);
					decryptStr = decryptAES((unsigned char *) (auth->Cypher), DataSize-sizeof(struct _jswproto_hdr), hash_code);//g_setting.acct.loginpwd);
					pthread_mutex_unlock(&aes_mutex);
					if(decryptStr == NULL)
					{
						bAuthResult = -1;
						DBG_PRINT("decryptAES failed!!\n");
					}else
					{
						memset(random_code2, 0, sizeof(random_code2));
						memcpy(random_code2, decryptStr, 16);
						if (strcmp(random_code, random_code2) != 0) {
							bAuthResult = -1;
							DBG_PRINT("Key is not matched2!!\n");
						} else {
							bAuthResult = 0;
							DBG_PRINT("Auth OK2!!\n");
						}
					}
				}

                //for remote access (random code)
                if(bAuthResult < 0)
                {
                    if( (g_setting4.extra.remote_access > 0) &&
                       (g_setting4.extra.random_code >= 1000) && (g_setting4.extra.remote_access < 10000) )
                    {//enable remote access and has random code
                        char code[32];
                        memset(hash_code, 0, sizeof(hash_code));
                        memset(code, 0, sizeof(code));
                        sprintf(code, "%04d", g_setting4.extra.random_code);
                        gen_hashcode(gDID, code, hash_code);
                        pthread_mutex_lock(&aes_mutex);
                        decryptStr = decryptAES((unsigned char *) (auth->Cypher), DataSize-sizeof(struct _jswproto_hdr), hash_code);
                        pthread_mutex_unlock(&aes_mutex);
                        if(decryptStr == NULL)
                        {
                            bAuthResult = -1;
                            DBG_PRINT("decryptAES failed2!!\n");
                        }else
                        {
                            memset(random_code2, 0, sizeof(random_code2));
                            memcpy(random_code2, decryptStr, 16);
                            if (strcmp(random_code, random_code2) != 0) {
                                bAuthResult = -1;
                                DBG_PRINT("Key is not matched3!!\n");
                            } else {
                                bAuthResult = 0;
                                DBG_PRINT("Auth OK3!!\n");

                                log_remote_access("Remote tool login via random code.");
                            }
                        }
                    }
                }
			} else {
				if (strcmp(auth->Cypher, g_setting.acct.loginpwd/*gPassword*/) != 0) {
					bAuthResult = -1;
					DBG_PRINT("Key is not matched!!\n");
				} else {
					bAuthResult = 0;
					DBG_PRINT("Auth OK!!\n");
				}
			}
		}else
		{
            SetRunningLog_str("Running: Session mode: without AES connection");
			// TODO
			if (AES_ENABLE) {
				//尝试不同的密钥进行解密
				for (aeskeyIndex = 0; aeskeyIndex < gAESKeyNum; aeskeyIndex++) {
					//DBG_PRINT("==================> try AES key:%s\n", gAESKeys[aeskeyIndex]);

					// AES 解密
					if (0 == (DataSize - plen) % 16) {
						pthread_mutex_lock(&aes_mutex);
						decryptStr = decryptAES((unsigned char *) (auth->Cypher),
						DataSize - plen, gAESKeys[aeskeyIndex]);
						pthread_mutex_unlock(&aes_mutex);

							if (NULL != decryptStr)
								plainStr = delTailStr(decryptStr);
					}

					//	DBG_PRINT("planStr = %s\n", plainStr);
					//	DBG_PRINT("gPassword = %s, Login acct = %s\n", gPassword, g_setting.acct.loginpwd);

					//if (strcmp((const char*)plainStr, (const char*)gPassword) != 0) {
					if (strcmp((const char*)plainStr, (const char*)g_setting.acct.loginpwd) != 0) {
						bAuthResult = -1;
					} else {
						bAuthResult = 0;
					}
					if (plainStr)
						free((void *)plainStr);

					if (0 == bAuthResult)		// 密码匹配正确，该p2p session 使用该密钥
					{
						DBG_PRINT("This AES Key is OK2!!\n");
						updateP2PSessionAesKey(tSession, (const char*)(gAESKeys[aeskeyIndex]), P2P_USE_AES_DATA);
						break;
					}
				}
			} else {
				if (strcmp(auth->Cypher, g_setting.acct.loginpwd/*gPassword*/) != 0) {
					bAuthResult = -1;
					DBG_PRINT("Key is not matched!!\n");
				} else {
					bAuthResult = 0;
					DBG_PRINT("Auth OK!!\n");
				}
			}
		}

		//st_Control_Proto_Header authResult;
		//memset(&authResult, 0, sizeof(st_Control_Proto_Header));
		//unsigned int authResultDataSize = sizeof(st_Control_Proto_Header);

		char *encryptStr = NULL;
		if (bAuthResult == 0) {
			if(P2P_USE_AES_DATA == 1)
			{
				hdr.cmd = CM_AUTH_OK_ENCRYPT;//CM_AUTH_OK
				memset(random_code, 0, sizeof(random_code));
				//srand((int)time(0));
				for(i=0;i<16;i++)
				{//new AES KEY
					random_code[i] = get_rand_alphanumeris();
				}
				pthread_mutex_lock(&aes_mutex);
				//int outsize;
				encryptStr = encryptAES((unsigned char *) (random_code), &outsize, hash_code);//g_setting.acct.loginpwd);
				pthread_mutex_unlock(&aes_mutex);
				if((outsize != 16) || (encryptStr == NULL))
				{
					hdr.cmd = CM_AUTH_FAIL;
					printf("AES Key: encryptAES failed, size is not 16!!\n");
					hdr.payload_length = 0;
					hdr.count = 0;
					PPPP_Write(tSession, CH_CTRL, (CHAR*) &hdr,	sizeof(struct _jswproto_hdr) );
				}else
				{
					memset(random_code2, 0, sizeof(random_code2));
					memcpy(random_code2, encryptStr, outsize);
					hdr.payload_length = outsize;
					hdr.count = 1;
					hdr.errCode = 0;
					memset(buf2, 0, sizeof(buf2));
					memcpy(buf2, &hdr, sizeof(struct _jswproto_hdr));
					memcpy(buf2+sizeof(struct _jswproto_hdr), random_code2, 16);
					PPPP_Write(tSession, CH_CTRL, buf2,	sizeof(struct _jswproto_hdr)+16 );
					DBG_PRINT("This AES Key is OK3!!\n");
					updateP2PSessionAesKey(tSession, (const char*)random_code, P2P_USE_AES_DATA);
				}
			}else
			{
				hdr.cmd = CM_AUTH_OK;
				hdr.payload_length = 0;
				hdr.count = 0;
				PPPP_Write(tSession, CH_CTRL, (CHAR*) &hdr,	sizeof(struct _jswproto_hdr) );
			}
		} else {
			hdr.cmd = CM_AUTH_FAIL;
			DBG_PRINT("CM_AUTH_FAIL Key is not matched!!\n");
			hdr.payload_length = 0;
			hdr.count = 0;
			PPPP_Write(tSession, CH_CTRL, (CHAR*) &hdr,	sizeof(struct _jswproto_hdr) );
		}

		if (bAuthResult != 0) {
            SetRunningLog_str_int("Error: failed bAuthResult", bAuthResult);
			break; //exit loop
		}

		//DBG_PRINT("new P2P Session goto loop...\n");

		//wait for CMD
		ret = -1;
		while (1) {
			g_get_temp2 = 0;
			ret = p2p_process_cmd(tSession);
			if (ret == -1)
				break;
			else if (ret == -2)
				break;
			//else
			//	mSecSleep(10);
		}

		if (ret == -1 || ret == -2) {
			//session is closed or malloc failed, break to close session
            SetRunningLog_str_int("Error: p2p_process_cmd() return failed", ret);
			break;
		}

	} while (1);

	DBG_PRINT("Session %d is closed.\n", tSession);
	pthread_mutex_lock(&p2psession_mutex);
	freeP2PSession(tSession);
	pthread_mutex_unlock(&p2psession_mutex);
	PPPP_Close(tSession);

	SetRunningLog_str_int("Running: Session closed", tSession);

	return 0;
}



char getc_popen(char *catfile)
{
	char cc = 0;
	FILE *fp;
	fp = popen(catfile, "r");
	if(fp != NULL)
	{
		cc = fgetc(fp);
		pclose(fp);
	}
	return cc;
}

//check date, if error then correct by ntpdate
// change  time.stdtime.gov.tw to time-b.nist.gov
void check_date(int updatenow)
{
    time_t time_now = time(NULL);
    if(updatenow > 0)
    {//update immediately
        system("ntpdate time-b.nist.gov &");
    }else if((time_now < 1400000000) || (time_now > 1800000000))
    {//invalid time
        system("ntpdate time-b.nist.gov &");
    }
}

//set beep volume, from 0 - 3, low - high
void set_beep_vol(int vol)
{
    int i2 = 0, i5 = 0;

    switch(vol)
    {
    case 0:
        i2 = 0;
        i5 = 1;
        break;
    case 1:
        i2 = 1;
        i5 = 1;
        break;
    case 2:
        i2 = 0;
        i5 = 0;
        break;
    case 3:
        i2 = 1;
        i5 = 0;
        break;
    }

    printf("beep vol=%d, i2=%d, i5=%d\n", vol, i2, i5);

    gpio_pin_info info;

    //snx_ms1_gpio_open();
    snx_ms1_gpio_open();

    info.pinumber = MS1_GPIO_PIN2;//GPIO_PIN_5;//MS1_GPIO_PIN5;
    info.mode = 0;
    info.value = -1;
    int r = snx_ms1_gpio_read(&info);
    if(r == GPIO_FAIL)
    {
        printf("Error: GPIO_FAIL, break\n");
        SetRunningLog_str_int("Error: snx_ms1_gpio_read() failed", r);
        return;
    }
//printf("PIN2 info.value=%d\n", info.value);
//printf("PIN2 info.mode=%d\n", info.mode);
    info.pinumber = MS1_GPIO_PIN5;//GPIO_PIN_5;//MS1_GPIO_PIN5;
    info.mode = 0;
    info.value = -1;
    r = snx_ms1_gpio_read(&info);
    if(r == GPIO_FAIL)
    {
        printf("Error: GPIO_FAIL, break\n");
        SetRunningLog_str_int("Error: snx_ms1_gpio_read() failed", r);
        return;
    }
//printf("PIN5 info.value=%d\n", info.value);
//printf("PIN5 info.mode=%d\n", info.mode);



    info.pinumber = MS1_GPIO_PIN2;//GPIO_PIN_5;//MS1_GPIO_PIN5;
    info.mode = 1;
    info.value = i2;
    if(snx_ms1_gpio_write(info) == GPIO_FAIL)
    {
        printf("Error: Write GPIO error2, break\n");
        SetRunningLog_str_int("Error: snx_ms1_gpio_write() failed", r);
        return;
    }
    info.pinumber = MS1_GPIO_PIN5;//GPIO_PIN_5;//MS1_GPIO_PIN5;
    info.mode = 1;
    info.value = i5;
    if(snx_ms1_gpio_write(info) == GPIO_FAIL)
    {
        printf("Error: Write GPIO error2, break\n");
        SetRunningLog_str_int("Error: snx_ms1_gpio_write() failed", r);
        return;
    }

    info.pinumber = MS1_GPIO_PIN2;//GPIO_PIN_5;//MS1_GPIO_PIN5;
    info.mode = 0;
    info.value = -1;
    r = snx_ms1_gpio_read(&info);
    if(r == GPIO_FAIL)
    {
        printf("Error: GPIO_FAIL, break\n");
        SetRunningLog_str_int("Error: snx_ms1_gpio_read() failed", r);
        return;
    }
//printf("PIN2 info.value=%d\n", info.value);
//printf("PIN2 info.mode=%d\n", info.mode);
    info.pinumber = MS1_GPIO_PIN5;//GPIO_PIN_5;//MS1_GPIO_PIN5;
    info.mode = 0;
    info.value = -1;
    r = snx_ms1_gpio_read(&info);
    if(r == GPIO_FAIL)
    {
        printf("Error: GPIO_FAIL, break\n");
        SetRunningLog_str_int("Error: snx_ms1_gpio_read() failed", r);
        return;
    }
//printf("PIN5 info.value=%d\n", info.value);
//printf("PIN5 info.mode=%d\n", info.mode);

//    r = snx_ms1_gpio_read(&info);
//    if(r == GPIO_FAIL)
//    {
//        printf("Error: GPIO_FAIL2, break\n");
//        return;
//    }
//printf("info.value2=%d\n", info.value);
//printf("info.mode2=%d\n", info.mode);

    snx_ms1_gpio_close();
}

//play beep when network connected
void play_beep_network_connected()
{
	printf("play_beep_network_connected\n");
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        McuGpio(PIN_BUZZER, 1, 2, 2);
        break;
    case 2://melody
        McuGpio2(0xD2, 1, 1, 0, 0, 0);
        break;
    }
}

//play beep when swicth to arm/part-arm
void play_beep_arm()
{
	printf("play_beep_arm\n");	
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        McuGpio(PIN_BUZZER, 1, 20, 1);
        break;
    case 2://melody
        McuGpio2(0xD2, 2, 1, 0, 0, 0);
        break;
    }
}

//play beep when armed/part-armed
void play_beep_armed()
{
	printf("play_beep_armed\n");
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        McuGpio(PIN_BUZZER, 2, 3, 2);
        break;
    case 2://melody
        McuGpio2(0xD2, 1, 1, 1, 0, 0);
        break;
    }
}

//play beep when disarm
void play_beep_disarm()
{
	printf("play_beep_disarm\n");
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        McuGpio(PIN_BUZZER, 1, 5, 1);
        break;
    case 2://melody
        McuGpio2(0xD2, 3, 1, 0, 0, 0);
        break;
    }

}

//play beep when alarm
void play_beep_alarm()
{
    //unsigned char count = 0x5A;	 // 0x5a for 60 times with 180sec
    unsigned char count = ((g_setting.gwprop.duration) / 2 );
	printf("play_beep_alarm with %d secs\n",(int)g_setting.gwprop.duration);
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        count = (3*60)*10/(1+0);
        //McuGpio(PIN_BUZZER, 1, 2, (unsigned short)count);
        McuGpio(PIN_BUZZER_ALARM, 0, 0, (unsigned short)count);
        break;
    case 2://melody
        McuGpio2(0xD2, 0x04, (unsigned short)count, 0, 0, 0); // 0x5a for 60 times with 180sec
        break;
    }
}

//stop alarm
void stop_beep_alarm()
{
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        McuGpio(PIN_BUZZER, 1, 2, 1);
        break;
    case 2://melody
        McuGpio2(0xD2, 3, 1, 0, 0, 0);
        break;
    }
}

//set volume
void play_beep_set_volume(int vv)
{
    g_setting.gwprop.alarmOn = vv;
    if( (g_setting.gwprop.alarmOn < JSW_VOLUME_MUTE) || (g_setting.gwprop.alarmOn > JSW_VOLUME_HIGH) )
        g_setting.gwprop.alarmOn = JSW_VOLUME_MEDIUM;
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        break;
    case 2://melody
        switch(g_setting.gwprop.alarmOn)
        {
        case JSW_VOLUME_HIGH:
            McuGpio2(0xD2, 0x10, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_MEDIUM:
            McuGpio2(0xD2, 0x11, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_LOW:
            McuGpio2(0xD2, 0x12, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_MUTE:
            McuGpio2(0xD2, 0x13, 1, 0, 0, 0);
            break;
        }
        break;
    }
}

//volume up
void play_beep_volume_up()
{
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        break;
    case 2://melody
        g_setting.gwprop.alarmOn++;
        if(g_setting.gwprop.alarmOn > JSW_VOLUME_HIGH)
            g_setting.gwprop.alarmOn = JSW_VOLUME_HIGH;
        play_beep_set_volume(g_setting.gwprop.alarmOn);
        break;
    }
}

//volume down
void play_beep_volume_down()
{
    switch(DEF_BEEP_SOURCE_SPEAKER)
    {
    case 0://speaker
        break;
    case 1://buzzer
        break;
    case 2://melody
        g_setting.gwprop.alarmOn--;
        if(g_setting.gwprop.alarmOn < JSW_VOLUME_MUTE)
            g_setting.gwprop.alarmOn = JSW_VOLUME_MUTE;
        play_beep_set_volume(g_setting.gwprop.alarmOn);
        break;
    }
}

void *
p2p_sub_thread_routine(void *ptr) {
	int rc = 0;
	static int session_failed_cnt = 0;
	static int PPPP_listen_count = 0;
	pthread_mutex_init(&p2psession_mutex, NULL);
	pthread_mutex_init(&aes_mutex, NULL);

	//pthread_mutex_init(&gcm_mutex, NULL);
	//pthread_mutex_init(&email_mutex, NULL);

	unsigned int APIVersion = PPPP_GetAPIVersion();
	DBG_PRINT("P2P Version: %d.%d.%d.%d\n", (APIVersion & 0xFF000000) >> 24,
			(APIVersion & 0x00FF0000) >> 16, (APIVersion & 0x0000FF00) >> 8,
			(APIVersion & 0x000000FF) >> 0);


	PPPP_Initialize(gInitString);

	st_PPPP_NetInfo NetInfo;
	rc = PPPP_NetworkDetect(&NetInfo, 0);
	DBG_PRINT("-------------- NetInfo: -------------------\n");
	DBG_PRINT("Internet Reachable     : %s\n",
			(NetInfo.bFlagInternet == 1) ? "YES" : "NO");
	DBG_PRINT("P2P Server IP resolved : %s\n",
			(NetInfo.bFlagHostResolved == 1) ? "YES" : "NO");
	DBG_PRINT("P2P Server Hello Ack   : %s\n",
			(NetInfo.bFlagServerHello == 1) ? "YES" : "NO");
//	DBG_PRINT("Local NAT Type         :");

	// 通知gateway
	if (NetInfo.bFlagServerHello == 1)
	{
		if(0 == access(MCU_REBOOT_FILE, 0))
		{
            remove(MCU_REBOOT_FILE);
			sync();
		}else
		{
		    play_beep_network_connected();
		}
        if(previous_net_status != 0)
        {
            previous_net_status = 0;
            system(LED_SETTING_P2P_OK);
        }
	}
	else
	{
		sleep(3);
		PPPP_NetworkDetect(&NetInfo, 0);

		if (NetInfo.bFlagServerHello == 1)
		{
			if(0 == access(MCU_REBOOT_FILE, 0))
			{
                remove(MCU_REBOOT_FILE);
				sync();
			}else
            {
                play_beep_network_connected();
            }
            if(previous_net_status != 0)
            {
                previous_net_status = 0;
                system(LED_SETTING_P2P_OK);
            }
        }
        else
        {
            //notifyGatewayP2PConnState("P2PConnectFail");
            if(getc_popen("cat /sys/class/net/eth0/carrier") == '1')
            {
                if(previous_net_status != 1)
                {
                    if(previous_net_status < 0)
                    {//startup with no network, only change LED status, keep the LED flashing
                        previous_net_status = 1;
                    }else
                    {
                        previous_net_status = 1;
                        system(LED_SETTING_P2P_NG);
                    }
                }
            }else
            {
                if(previous_net_status != 2)
                {
                    if(previous_net_status < 0)
                    {//startup with no network, only change LED status, keep the LED flashing
                        previous_net_status = 2;
                    }else
                    {
                        previous_net_status = 2;
                        system(LED_SETTING_P2P_NG2);
                    }
                }
            }
        }
    }

	//previous_net_status = NetInfo.bFlagServerHello;

	PPPP_Share_Bandwidth(1);
	switch (NetInfo.NAT_Type) {
	case 0:
		DBG_PRINT(" Unknown NAT type\n");
		break;
	case 1:
		DBG_PRINT(" IP-Restricted Cone\n");
		break;
	case 2:
		DBG_PRINT(" Port-Restricted Cone\n");
		break;
	case 3:
		DBG_PRINT(" Symmetric\n");
		break;
	}
	DBG_PRINT("My Wan IP : %s\n", NetInfo.MyWanIP);
	DBG_PRINT("My Lan IP : %s\n", NetInfo.MyLanIP);
	if(strcmp(NetInfo.MyWanIP,g_setting4.latlng.wlan_ip) !=0)
		{
		printf("My Wan IP changed\n");
		strcpy(g_setting4.latlng.wlan_ip,NetInfo.MyWanIP);
		savesetting4();
		}

//	int p2pSessionCount = 0;
	while (1) {
		if(g_fwupgrade == 1 && (getValidP2PSessionNum() == 1))
			{
			printf("F/W upgrading...........STOP P2P service=%d\n");
			mSecSleep(10000);
			continue;
			}

		// ensure p2p client count <= P2P_SESSION_MAX(8)
//		pthread_mutex_lock(&p2psession_mutex);
//		p2pSessionCount = getValidP2PSessionNum();
//		pthread_mutex_unlock(&p2psession_mutex);

        if(g_network_detect > 0)
        {//check by PPPP_NetworkDetect
            PPPP_NetworkDetect(&NetInfo, 0);

            if (NetInfo.bFlagServerHello == 1)
            {
                //notifyGatewayP2PConnState("P2PConnectOk");
                if(previous_net_status != 0)
                {
                    previous_net_status = 0;
                    system(LED_SETTING_P2P_OK);
                }
                check_date(0);
            }
            else
            {
                sleep(3);
                PPPP_NetworkDetect(&NetInfo, 0);

                if (NetInfo.bFlagServerHello != 1)
                {//not connect
                        //notifyGatewayP2PConnState("P2PConnectFail");
                    if(getc_popen("cat /sys/class/net/eth0/carrier") == '1')
                    {//rj45 connected
                        if(previous_net_status != 1)
                        {
                            previous_net_status = 1;
                            system(LED_SETTING_P2P_NG);
                        }
                    }else
                    {
                        if(previous_net_status != 2)
                        {
                            previous_net_status = 2;
                            system(LED_SETTING_P2P_NG2);
                        }
                    }

                    //renew ip
                    if(g_setting.network.dhcp == 1)
                        system("udhcpc -q -n&");

                    //check time, if error then set time as watchtemp or tmpevent
                    time_t time_now = time(NULL);
                    if( (time_now < 1400000000) || (time_now > 1800000000) )
                    {//invalid time
                        struct stat buf1, buf2;
                        if(stat(WATCH_TEMP_FILENAME, &buf1) == 0)
                        {//has file
                            if( (buf1.st_mtime < 1400000000) || (buf1.st_mtime > 1800000000) )
                                buf1.st_mtime = 0;
                        }else
                        {
                            buf1.st_mtime = 0;
                        }
                        char path[256];
                        sprintf(path, "%sEventList/%s", sdpath, TMPEVENT_NAME);
                        if(stat(path, &buf2) == 0)
                        {//has file
                            if( (buf2.st_mtime < 1400000000) || (buf2.st_mtime > 1800000000) )
                                buf2.st_mtime = 0;
                        }else
                        {
                            buf2.st_mtime = 0;
                        }
                        if( (buf1.st_mtime > 0) || (buf2.st_mtime > 0) )
                        {
                            struct timeval tv;
                            struct timezone tz;
                            if(gettimeofday(&tv, &tz) == 0)
                            {
                                if(buf1.st_mtime > buf2.st_mtime)
                                {//set time as watchtemp
                                    tv.tv_sec = buf1.st_mtime;
                                }else
                                {//set time as tmpevent
                                    tv.tv_sec = buf2.st_mtime;
                                }
                                if(settimeofday(&tv, &tz) == 0)
                                {//set time successful
                                }
                            }
                        }
                    }
                }else
                {//connect
                    if(previous_net_status != 0)
                    {
                        previous_net_status = 0;
                        system(LED_SETTING_P2P_OK);
                    }
                    check_date(0);
                }
            }
        }
		//previous_net_status = NetInfo.bFlagServerHello;

		// Wait for Client to connect
		//DBG_PRINT("On Listening ...\n");
		INT32 hSession = PPPP_Listen(gDID, 60, 0, 1, gAPIChecksum);
		if(g_fwupgrade == 1)
			{
			printf("F/W upgrading...........reject P2P service=%d\n");

			mSecSleep(5000);
			continue;
			}


		if (hSession == ERROR_PPPP_TIME_OUT)
		{
			printf("PPPP listen timeout\n");
			g_get_temp2 = 0;
			continue;
		}
		else if (hSession > -1) //// Connection established!!
		{
			//DBG_PRINT("new hSession=%d\n", hSession);

			g_get_temp2 = 0;
			dumpSessionInfo(hSession);
//			globalSession = hSession;
			pthread_mutex_lock(&p2psession_mutex);
			int index = getP2PSessionFreeIndex();
			pthread_mutex_unlock(&p2psession_mutex);
			//DBG_PRINT("gP2PSession free index = %d\n\n", index);
			if (-1 == index) {
				DBG_PRINT("##PPP Listen max out, index = %d\n\n", index);
				//REPLY TO CLIENT
				jswhdr hdr;
				memset(&hdr, 0, sizeof(hdr));
				hdr.cmd = CM_AUTH_FAIL;
				hdr.errCode = 3;
				hdr.version = PRO_VERSION;
				PPPP_Write(hSession, CH_CTRL, (CHAR*) &hdr,	sizeof(struct _jswproto_hdr) );

			} else {


				pthread_mutex_lock(&p2psession_mutex);
				addP2PSession(index, hSession);	// add to p2p session record
				pthread_mutex_unlock(&p2psession_mutex);

				pthread_t hthread_Service;
				pthread_create(&hthread_Service, NULL, &p2p_thread_service,
						(void *) hSession);
				pthread_detach(hthread_Service);
			}
		} else      //// Some thing wrong
		{
            SetRunningLog_str_int("Error: p2p_sub_thread_routine() session failed", hSession);

#if 0
			char cmd2[64];
			sprintf(cmd2, "echo \"ERROR: PPPP_Listen error code=%d \" >> ./pppp_error.log", hSession);
			system(cmd2);
			sprintf(cmd2, "date >> ./pppp_error.log");
			system(cmd2);
			sprintf(cmd2, "echo >> ./pppp_error.log");
			system(cmd2);
#endif

			session_failed_cnt++;
			printf("PPP Listen unknown error while client connecting....hSession=%d, session_failed_cnt=%d\n", hSession, session_failed_cnt);
			if(session_failed_cnt > 3)
			{
				printf("Error: Stop P2P thread\n");
				session_failed_cnt = 0;
				usleep(3*1000*1000);
				//break; //major error, stop p2p thread
			}
			//mSecSleep(1000);
			//mSecSleep(60 * 1000);
		}
	}
	PPPP_DeInitialize();
	return 0;
}


#ifdef SEARCH_BROADCAST_ENABLE

/*
getMsg include Server IP:Port: "search_palals:10.0.0.107:13000"
*/
int sendGatewayInfoByTCP(const char* getMsg, const char* info)
{
	//DBG_PRINT("sendGatewayInfoByTCP. get = %s, info = %s\n", getMsg, info);

	int socketId;

	int ipPos = -1;
  	char ip[32] = { 0 };

  	int portPos = -1;
  	char port[32] = { 0 };


  	int len = strlen(getMsg);
  	int i = 0;
  	for (; i < len; ++i)
  	{
  		if (getMsg[i] == ':')
  		{
  			if (-1 == ipPos)
  				ipPos = i + 1;
  			else
  				portPos = i + 1;
  		}
  	}

  	if (-1 == ipPos || -1 == portPos)
  	{
  		DBG_PRINT("get message error");
  		return 1;
  	}

  	strncpy(ip, getMsg + ipPos, (portPos - ipPos - 1));
  	strncpy(port, getMsg + portPos, (len - portPos));
  	DBG_PRINT("sendGatewayInfoByTCP remote device ip = %s, port = %s\n", ip, port);

  	struct sockaddr_in remote_addr;
  	memset (&remote_addr, 0, sizeof (remote_addr));
  	remote_addr.sin_family = AF_INET;
  	remote_addr.sin_addr.s_addr = inet_addr(ip);
  	remote_addr.sin_port = htons (atoi(port));


  	if ((socketId = socket (PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror ("socket create error!\n");
	    return 1;
	}
	int sop = 1;
	setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &sop, sizeof(sop) );

	if (connect (socketId, (struct sockaddr *) &remote_addr, sizeof (struct sockaddr)) <0)
	{
		perror ("connect error!\n");
		close (socketId);
		return 1;
	}
	//DBG_PRINT ("connected to server\n");

	// send gatewayinfo
	send (socketId, info, strlen (info), 0);
	close (socketId);
	return 0;
}

//局域网广播发送，配合该工具检测http://download.csdn.net/download/ben395575481/1796582
static const char * IN6ADDR_ALLNODES = "FF02::1";      //v6 addr
void broadcaseGatewayInfo(const char *info) {
	DBG_PRINT("broadcaseGatewayInfo(). info = %s\n", info);
	//检测参数
	in_port_t port = htons((in_port_t) SEARCH_BROADCAST_ANDROID_PORT);
	//配置地址结构
	struct sockaddr_storage destStoreage;
	memset(&destStoreage, 0, sizeof(destStoreage));

	size_t addr_size = 0;
	if (SEARCH_BROADCAST_IPV4) {
		//使用IP4
		struct sockaddr_in *destAddr4 = (struct sockaddr_in*) &destStoreage;
		destAddr4->sin_family = AF_INET;
		destAddr4->sin_port = port;
		destAddr4->sin_addr.s_addr = INADDR_BROADCAST;      //广播
		addr_size = sizeof(struct sockaddr_in);
	} else if (SEARCH_BROADCAST_IPV6) {
		//使用IP6
		struct sockaddr_in6 *destAddr6 = (struct sockaddr_in6*) &destStoreage;
		destAddr6->sin6_family = AF_INET6;
		destAddr6->sin6_port = port;
		inet_pton(AF_INET6, IN6ADDR_ALLNODES, &destAddr6->sin6_addr);
		addr_size = sizeof(struct sockaddr_in6);
	} else {
		DBG_PRINT("Unknown address family");
		return;
	}

	//转化为最后的地址
	struct sockaddr *destAddress = (struct sockaddr*) &destStoreage;
	size_t msg_len = strlen(info);
	if (msg_len > 4096) {
		DBG_PRINT("string too long ");
		return;
	}
	//建立socket
	int sock = socket(destAddress->sa_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		DBG_PRINT("socket() failed!");
		return;
	}
	int sop = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sop, sizeof(sop) );

	int broadcastPerm = 1;
	//设置socket允许发送局域网广播
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastPerm,
			sizeof(broadcastPerm)) < 0) {
		DBG_PRINT("setsockopt() failed!");
		return;
	}

	//发送广播数据
	int k = 0;
	for (k = 0; k < 3; k++) {
		ssize_t numBytes = sendto(sock, info, msg_len, 0, destAddress,
				addr_size);
		if (numBytes < 0) {
			DBG_PRINT("sendto() failed!\n");
		} else if (numBytes != msg_len) {
			DBG_PRINT("sendto(), %s\n", "sent unexpected number of bytes");
		}

		mSecSleep(1);
	}

	//DBG_PRINT("send ok!");
}

#define MAXRECVSTRING 255 /* Longest string to receive */

void DieWithError(char *errorMessage); /* External error handling function */
void DieWithError(char *errorMessage) {
	perror(errorMessage);
}


void* search_bc_thread_rountine(void *ptr) {
	int sock; /* Socket */
	struct sockaddr_in broadcastAddr; /* Broadcast Address */
	unsigned short broadcastPort; /* Port */
	char recvString[MAXRECVSTRING + 1]; /* Buffer for received string */
	int recvStringLen; /* Length of received string */

	broadcastPort = SEARCH_BROADCAST_GATEWAY_PORT; /* First arg: broadcast port */
RESTART:
	/* Create a best-effort datagram socket using UDP */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("bcast socket() faile\n");
		return 0;
	}
	int sop = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sop, sizeof(sop) );


	/* Construct bind structure */
	memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
	broadcastAddr.sin_family = AF_INET; /* Internet address family */
	broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	broadcastAddr.sin_port = htons(broadcastPort); /* Broadcast port */

	/* Bind to the broadcast port */
	if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr))
			< 0) {
		return 0;
	}
	const char *intrestStr = "search_gateway";

	//char *gatewayInfo = getGatewayInfo();

	char gatewayInfo[1024];
	memset(gatewayInfo, 0, sizeof(gatewayInfo) );
	sprintf(gatewayInfo, "%s,%s,%s", gDID, gAPIChecksum, DEF_SECURITY_CODE); //g_setting.acct.loginpwd); //gPassword);

	char bcInfo[64] = { 0 };
	memset(bcInfo, 0, 64);
	memcpy(bcInfo, gatewayInfo, strlen(gatewayInfo));

	while (1) {
		/* Receive a single datagram from the server */
		if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL,
				0)) < 0) {
			printf("bcast recvfrom() failed %d\n",errno);
			usleep(1000*1000);
			close(sock);
			goto RESTART;
			continue;
		}
		recvString[recvStringLen] = '\0';
		DBG_PRINT("Received: %s\n", recvString); /* Print the received string */

		// 把 gateway 的信息再发送UDP广播出去
/*		if (0 == strcmp(intrestStr, recvString))
			broadcaseGatewayInfo(bcInfo);*/

		// 把 gateway 的信息通过TCP发送出去
		char *p = strstr(recvString, intrestStr);
		if (NULL != p)
		{
			sendGatewayInfoByTCP(recvString, bcInfo);
		}

		mSecSleep(2);
	}

	close(sock);
}

#endif

char *getvalbyname(char *name, char* str, char *val)
{
	char *p = str;
	char *st, *et;

	st = strstr(p, name);
	if (st ==NULL)
		return NULL;

	p = st + strlen(name);

	st = strstr(p, "\"");
	if (st ==NULL)
		return NULL;
	et = strstr(st, "\";");
	if (et ==NULL)
		return NULL;
	p = st + 1;

	memcpy(val, p, et - st -1);
	p = et +1;

	if (p == NULL)
		DBG_PRINT("Invalid start up parameter\n");

	return p;
}

char *strupr(char *str){
    char *orign=str;
    for (; *str!='\0 '; str++)
        *str = toupper(*str);
    return orign;
}

int gwinfo()
{
	char buf[4096];
	char val[1024];
	char *p;
	int len;

	char str[2000];

	memset(str, 0, sizeof(str));

	FILE *fpold;


	FILE *fp;
	fp = fopen("/mnt/gatewayinfoEx.db", "rb");

	if (fp ==NULL)
	{
		printf("Using default did\n");

		fpold =  fopen("/mnt/gatewayinfo.db", "rb");
		if (fpold != NULL)
		{
			memset(val, 0, sizeof(val));
			int bread = fread(buf, 1, 4096, fpold);
			fclose(fpold);
			p = getvalbyname( "[\"password\"]",buf, gPassword );
			if (p ==NULL)
			{
				DBG_PRINT("gateinfodb error\n");
				return 0;
			}
			p = getvalbyname( "[\"apichecksum\"]",buf, gAPIChecksum );
			if (p ==NULL)
			{
				DBG_PRINT("gateinfodb error\n");
				return 0;
			}
			p = getvalbyname( "[\"did\"]",buf, gDID );
			if (p ==NULL)
			{
				DBG_PRINT("gateinfodb error\n");
				return 0;
			}
			p = getvalbyname( "[\"initstring\"]",buf, gInitString );
			if (p ==NULL)
			{
				DBG_PRINT("gateinfodb error\n");
				return 0;
			}

			if (strcmp(gDID, "AHUA-000099-DGCEX") == 0)
			{//fix older error db in linux image.
				strcpy(gAPIChecksum, "UNRJNW");
			}

		}else { // the old one does not exist
			strcpy(gPassword, "123456");
			strcpy(gAPIChecksum, "UNRJNW");
			strcpy(gDID, "AHUA-000099-DGCEX");
			strcpy(gInitString, "ECGBFFBJKAIEGHJAEBHLFGEMHLNBHCNIGEFCBNCIBIJALMLFCFAPCHODHOLCJNKIBIMCLDCNOBMOAKDMJGNMIJBJML");
		}




		//create new db
		fp = fopen("/mnt/gatewayinfoEx.db", "wb+");
		sprintf(str, "[\"initstring\"] = \"ECGBFFBJKAIEGHJAEBHLFGEMHLNBHCNIGEFCBNCIBIJALMLFCFAPCHODHOLCJNKIBIMCLDCNOBMOAKDMJGNMIJBJML\";\n[\"password\"] = \"%s\";\n[\"did\"] = \"%s\";\n[\"license\"] = \"%s\";\n[\"apichecksum\"] = \"%s\";\n",gPassword, gDID,"IKSLMX" ,gAPIChecksum);
		if (fp != NULL)
		{
			fwrite(str, strlen(str), 1, fp);
			fclose(fp);
		}

		//return 0;
	}else
	{
		memset(val, 0, sizeof(val));
		int bread = fread(buf, 1, 4096, fp);
		fclose(fp);
		p = getvalbyname( "[\"password\"]",buf, gPassword );
		if (p ==NULL)
		{
			DBG_PRINT("gateinfodb error\n");
			return 0;
		}
		p = getvalbyname( "[\"apichecksum\"]",buf, gAPIChecksum );
		if (p ==NULL)
		{
			DBG_PRINT("gateinfodb error\n");
			return 0;
		}
		p = getvalbyname( "[\"did\"]",buf, gDID );
		if (p ==NULL)
		{
			DBG_PRINT("gateinfodb error\n");
			return 0;
		}
		p = getvalbyname( "[\"initstring\"]",buf, gInitString );
		if (p ==NULL)
		{
			DBG_PRINT("gateinfodb error\n");
			return 0;
		}
	}

	int l = strlen(gDID);
	l = (l -4);
	char *ptr = gDID + l;
	memcpy(&g_DID, ptr, sizeof(int));
	strupr(gDID);

	printf("\nGateway Info %s,%s,%s, g_did %u\n\n", gDID, gAPIChecksum, gPassword, g_DID);

	return 1;
}

void *net_detect_thread(void *arg)
{
	int start_time_t = time((time_t*)NULL);
	int diff_time_t = 0;
#if 0
	while(diff_time_t < 300)
	{

		printf("!!!!!!!!!!!!!!!!!!!!!!net_detect_thread!!!!!!!!!!!!!!!!!!\n");
		diff_time_t = difftime(time((time_t*)NULL), start_time_t);
		system("udhcpc -q -n");
		if(getc_popen("cat /sys/class/net/eth0/carrier") == '1' && getc_popen("ifconfig eth0 | grep -c \"inet addr\"") == "1")
			{
			printf("!!!!!!!!!!!!!!!!!!!!!!net_detect_thread break!!!!!!!!!!!!!!!!!!\n");
			break;
			}

	}
#else
	while(1)
	{
		if (g_network_detect ==0 && g_setting.network.dhcp == 1)
		{
			printf("net_detect_thread\n");
            system("udhcpc -q -n");
			mSecSleep(5000);
            if(getc_popen("cat /sys/class/net/eth0/carrier") == '1' && getc_popen("ifconfig eth0 | grep -c 'inet addr'") == '1')
			{
				//printf("!!!!!!!!!!!!!!!!!!!!!!net_detect_thread break!!!!!!!!!!!!!!!!!!\n");
				printf("DHCP OK...\n");
//                if(previous_net_status != 0)
//                {
//                    previous_net_status = 0;
//                    system(LED_SETTING_P2P_OK);
//                }
				//break;
            }
        }
        mSecSleep(5000);
	}
#endif
	pthread_exit(0);
}


void *hardware_reset_thread(void *arg)
{
	static int count = 0;
	int r;
	gpio_pin_info info;
	snx_gpio_open();
	while(1)
	{
//		gpio_pin_info info;
//		snx_gpio_open();

		info.pinumber = GPIO_PIN_1;
		info.mode = 0;
		info.value = -1;
		if(snx_gpio_write(info) == GPIO_FAIL)
		{
			printf("Error: Write GPIO error, break\n");
			break;
		}
		r = snx_gpio_read(&info);
		if(r == GPIO_FAIL)
		{
			printf("Error: GPIO_FAIL, break\n");
			break;
		}
		if (info.value == 1)
		{
			count = 0;
		}else if (info.value == 0)
		{
			count++;
			if(count >= 5)
			{
				count = 0;
				factoryReset2();
			}
		}

//		snx_gpio_close();

		usleep(1*1000*1000);
	}
	snx_gpio_close();


	pthread_exit(0);
}

//check ping hinet DNS
void *ping_hinet_thread_routine(void *arg)
{
    struct stat buf1;
	static int last_network_detect =0;
	char ping_ret;
	while(1)
	{
#if 0
//        g_network_detect = 1;
        if(previous_net_status == 1)
        {//local network, ping first
            system("rm ./pinghinet");
            system("ping -c 3 168.95.1.1 > pinghinet");//hinet DNS 168.95.1.1, goolge 74.125.203.105
            sync();
            if(stat("./pinghinet", &buf1) == 0)
            {//has file
                if(buf1.st_size < 200)
                {//ping failed
                    g_network_detect = 0;
                }else
                {//ping successful
                    g_network_detect = 1;
                }
            }else
            {//no file, ping failed
                g_network_detect = 0;
            }
        }
		printf("last_network_detect = [%d / %d]\n",last_network_detect,g_network_detect);
		if(last_network_detect == 1 && g_network_detect ==0)
			{
			printf("Network router table changed, need to reboot...................\n");
			usleep(1*1000*1000);
			system("reboot");
			}
		last_network_detect = g_network_detect;
#else
		 ping_ret = getc_popen("ping -c 3 8.8.8.8 -W 3 | grep -c ttl");
		 if(ping_ret > '0')
		 	g_network_detect = 1;
		 else
		 	g_network_detect = 0;
		//printf("last_network_detect = [%d / %d]\n",last_network_detect,g_network_detect);
		if(last_network_detect == 1 && g_network_detect ==0)
        {
			printf("Network router table changed, restart Network...................\n");
            if(previous_net_status != 1)
            {
                previous_net_status = 1;
                system(LED_SETTING_P2P_NG);
            }
			usleep(1*1000*1000);
			//system("reboot");

		    //log_and_reboot_no_beep("Reboot by network routing table changed", 1);
		}
		last_network_detect = g_network_detect;
#endif
        usleep(5*1000*1000);
	}

	pthread_exit(0);
}

void updateV2()
{
    char filename[256];
    char script_buf2[SCRIPT_LEN];
	FILE *fp3;
	sprintf(filename, "./%s", OUTPUT_FILENAME);
	fp3 = fopen(filename, "rb");
	if (fp3 == NULL)
	{
		printf("No ceres found, return\n");
		return;
	}
	fseek(fp3, -1024, SEEK_END);
    memset(script_buf2, 0, SCRIPT_LEN);
	int bread = fread(script_buf2, 1, SCRIPT_LEN, fp3);
	if(memcmp(script_buf2, SCRIPT_KEYWORD, strlen(SCRIPT_KEYWORD)) != 0)
	{
		printf("Keyword not found, return\n");
        fclose(fp3);
		return;
	}
	rewind(fp3);

    //LED flash when FW upgrade
    //system(LED_SETTING_P2P_NG2);
    system(LED_FW_UPGRADE);

	system("rm ceres.pkg;sync");
	system("cp ceres ceres.pkg;sync");

	//srtok2
	int filesize = 0;
	char pathname[256], mcufilename[256];
	char chars2[] = "\n";
	char filename2[256], filename3[256];
	char cmd[1024];
	int i;
	char *c1, *c2, *c3, *c4, *c5;
	char cc[32];
	char replace_str[64];
	int call_update = 0, call_reboot = 0, call_mcu = 0;
	strcpy(replace_str, "./");
	char *p2 = strtok(script_buf2, chars2);
	while(p2)
	{
	    printf("line=%s\n", p2);
	    if( (p2[0] == '/') && (p2[1] == '/') )
	    {
            p2 = strtok(NULL, chars2);
            continue;
	    }
        memset(filename, 0, 256);
	    c3 = strchr(p2, ':');
	    if(c3)
	    {
	        memset(cmd, 0, sizeof(cmd));
	        memcpy(cmd, p2, c3-p2);
	        if(strlen(cmd) > 2)
	        {
                printf("cmd=%s\n", cmd);
                if(strcmp(cmd, "update") == 0)
                {//update
                    c4 = strrchr(p2, '/');
                    if(c4)
                    {//with path
                        memset(pathname, 0, sizeof(pathname));
                        memcpy(pathname, c3+1, c4-c3-1);
                        if(strlen(pathname) > 0)
                        {
                            printf("pathname=%s\n", pathname);
                        }
                        c5 = strchr(c4, ',');
                        if(c5)
                        {
                            memset(filename, 0, sizeof(filename));
                            memcpy(filename, c4+1, c5-c4-1);
                            filesize = atoi(c5+1);
                        }else
                        {
                            filename[0] = 0;
                            filesize = 0;
                        }
                        printf("filename=%s\n", filename);
                        printf("filesize=%d\n", filesize);
                    }else
                    {//without path
                        pathname[0] = 0;
                        printf("pathname=%s\n", pathname);
                        c5 = strrchr(p2, ',');
                        if(c5)
                        {
                            memset(filename, 0, sizeof(filename));
                            memcpy(filename, c3+1, c5-c3-1);
                            filesize = atoi(c5+1);
                        }else
                        {
                            filename[0] =0;
                            filesize = 0;
                        }
                        printf("filename=%s\n", filename);
                        printf("filesize=%d\n", filesize);
                    }
                    if( (strlen(filename) > 0) && (filesize > 0) )
                    {
                        //update mcu
                        if(strstr(filename, ".sn8"))
                        {//update mcu
                            call_mcu = 1;
                            strcpy(mcufilename, filename);
                        }
                        //replace sdcard
                        sprintf(filename2, "%s/%s.u2", pathname, filename);
                        if(strstr(filename2, "sdcard"))
                        {//replace sdcard
                            c1 = strstr(filename2, "sdcard");
                            sprintf(filename3, "%s%s", replace_str, c1+7);
                            strcpy(filename2, filename3);
                        }
                        //del *.u2
                        sprintf(filename3, "rm %s", filename2);
                        system(filename3);
                        FILE *fp4;
                        fp4 = fopen(filename2, "wb");
                        if (fp4 == NULL)
                        {
                            printf("Create file %s failed, exit\n", filename2);
                            return -1;
                        }
                        //write file
                        for(i=0;i<filesize;i++)
                        {
                            fread(cc, 1, 1, fp3);
                            fwrite(cc, 1, 1, fp4);
                        }
                        fclose(fp4);
                        sync();
                        //replace file
                        if(strcmp(filename, "ceres") == 0)
                        {//ceres, rename ceresnew first, kill gateway_wdt, call update, exit ceres
                            system("rm ./ceresnew.old;sync");
                            system("cp ceresnew ceresnew.old;sync");
                            system("cp ceres.u2 ceresnew;sync");
                            system("killall -9 gateway_wdt");
                            call_update = 1;
                        }else
                        {//other file, rename old file, copy new file
                            sprintf(filename3, "killall -9 %s", filename);
                            system(filename3);
                            sprintf(filename3, "rm %s.old;sync", filename);
                            system(filename3);
                            sprintf(filename3, "cp %s %s.old;sync", filename, filename);
                            system(filename3);
                            sprintf(filename3, "rm %s;sync", filename);
                            system(filename3);
                            //apple add for ALC
                            if(strlen(pathname) > 0)

                            {

                                char filename4[256];

                                sprintf(filename4, "%s/%s", pathname, filename);
                                if(strstr(filename4, "sdcard"))

                                {//replace sdcard
                                    c1 = strstr(filename4, "sdcard");
                                    sprintf(filename3, "%s%s", replace_str, c1+7);
                                    strcpy(filename4, filename3);

                                }
                                sprintf(filename3, "cp %s %s;sync", filename2, filename4);
                                system(filename3);

                            }else
                            {
                                sprintf(filename3, "cp %s %s;sync", filename2, filename);
                                system(filename3);
                            }
                        }
                    }
                }else if(strcmp(cmd, "kill") == 0)
                {//kill
                    sprintf(filename3, "killall -9 %s", c3+1);
                    system(filename3);
                }else if(strcmp(cmd, "copy") == 0)
                {//copy
                }else if(strcmp(cmd, "del") == 0)
                {//del
                }else if(strcmp(cmd, "mkdir") == 0)
                {//mkdir
                }else if(strcmp(cmd, "rmdir") == 0)
                {//rmdir
                }else if(strcmp(cmd, "run") == 0)
                {//run
                }else
                {//not match
                    printf("Error, cmd=%s not found!\n", cmd);
                }
	        }
	    }
	    p2 = strtok(NULL, chars2);
	}

	fclose(fp3);

    system("killall -9 gateway_wdt");
    usleep(5*1000);

    system("rm -f *.u2");
    system("rm -f *.old");
    system("rm -f *.pkg");
    system("rm -f *.rv");
    system("chmod 755 *");
    //system("chmod 777 lib");
    //system("chmod 777 out");
    //system("chmod 777 EventList");
    system("sync");

	if(call_mcu == 1)
	{
printf("MCU update1!!!\n");
        system("rm ./NewMCU1.sn8;sync");
        sprintf(filename3, "cp ./%s ./NewMCU1.sn8;sync", mcufilename);
printf(filename3);
printf("\n");
        system(filename3);
	}

	if(call_update == 1)
	{
printf("Update ceres!!!\n");
        system("./update&");
	}

	if(call_reboot == 1)
	{
printf("Reboot system!!!\n");
        system("reboot");
	}

    sync();
    usleep(5*1000);
    exit(0);
}

//write dns setting to /etc/resolv.conf if manual ip setting
void set_DNS()
{
//printf("g_setting.network.dhcp=%d\n", g_setting.network.dhcp);
    if(g_setting.network.dhcp == 0)
    {//using ip manual
        if(strlen(g_setting.network.dns) >= 7)
        {
            char cmd2[256];
            sprintf(cmd2, "nameserver %s\n", g_setting.network.dns);
//printf("cmd2=%s\n", cmd2);
            char file2[32] = "/etc/resolv.conf";
            FILE *fp;
            remove(file2);
            fp = fopen(file2, "wb");
            if(fp == NULL)
                return;
            fwrite(cmd2, strlen(cmd2), 1, fp);
            fclose(fp);
            sync();
printf("set_DNS successful!!\n");
        }
    }
}

//set str to running log
void SetRunningLog_str(char *str)
{
    int len = strlen(str);
    if(len >= (LEN_RUNNING_LOG-10))
    {
        char buf[256];
        sprintf(buf, "Error: string len is too long(%d)!", len);
        SetRunningLog_str(buf);
        return;
    }
    if(len <= 0)
    {
        char buf[256];
        sprintf(buf, "Error: string len is too short(%d)!", len);
        SetRunningLog_str(buf);
        return;
    }

    time_t timep;
    struct tm *p;
    struct timeb tb;
    time(&timep);
    ftime(&tb);
    p = gmtime(&timep);
    //sprintf(str2, "%04d/%02d/%02d %02d:%02d:%02d:%03d %s", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tb.millitm, str);
    char buf2[LEN_RUNNING_LOG], buf3[LEN_RUNNING_LOG];
    memset(buf2, 0, sizeof(buf2));
    sprintf(buf3, "%04d/%02d/%02d %02d:%02d:%02d:%03d %s",
            p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tb.millitm, str);
    int len2 = strlen(buf3)+1;
//tl_add_hex16(1, &gRunningLog[1599], 200);
    memcpy(buf2, &gRunningLog[len2], LEN_RUNNING_LOG-len2);
    memcpy(&buf2[LEN_RUNNING_LOG-len2], buf3, strlen(buf3));
    memcpy(gRunningLog, buf2, LEN_RUNNING_LOG);
//tl_add_hex16(1, &gRunningLog[1599], 200);
}

//set str+int to running log
void SetRunningLog_str_int(char *str, int ii)
{
    int len = strlen(str);
    if(len >= (LEN_RUNNING_LOG-20))
    {
        char buf[256];
        sprintf(buf, "Error: string len is too long(%d)!", len);
        SetRunningLog_str(buf);
        return;
    }
    if(len <= 0)
    {
        char buf[256];
        sprintf(buf, "Error: string len is too short(%d)!", len);
        SetRunningLog_str(buf);
        return;
    }

    time_t timep;
    struct tm *p;
    struct timeb tb;
    time(&timep);
    ftime(&tb);
    p = gmtime(&timep);
    //sprintf(str2, "%04d/%02d/%02d %02d:%02d:%02d:%03d %s", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tb.millitm, str);
    char buf2[LEN_RUNNING_LOG], buf3[LEN_RUNNING_LOG];
    memset(buf2, 0, sizeof(buf2));
    //memset(buf3, 0, sizeof(buf3));
    sprintf(buf3, "%04d/%02d/%02d %02d:%02d:%02d:%03d %s %d",
            p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tb.millitm, str, ii);
    int len2 = strlen(buf3)+1;
    memcpy(buf2, &gRunningLog[len2], LEN_RUNNING_LOG-len2);
    memcpy(&buf2[LEN_RUNNING_LOG-len2], buf3, strlen(buf3));
    memcpy(gRunningLog, buf2, LEN_RUNNING_LOG);
}

//check and add sunrise/sunset in scenario
void check_add_sunrisesunset_in_scenario()
{
    int has_sunrise = 0;
    int has_sunset = 0;
    signed char highb = 0;
    signed char lowb = 0;
    int k;
    for (k=0;k<srcCount;k++)
    {
        if(scenariolist[k].src.did == SCENARIO_GATEWAY_DID)
        {
            highb = scenariolist[k].src.onoff >> 8;
            lowb = scenariolist[k].src.onoff & 0x00FF;
            if(lowb == SCENARIO_SOURCE_SUNRISE)
                has_sunrise++;
            else if(lowb == SCENARIO_SOURCE_SUNSET)
                has_sunset++;
        }
    }
    if(has_sunrise == 0)
    {//no sunrise, create
        memset(&scenariolist[srcCount], 0, sizeof(jswscenario));
        scenariolist[srcCount].src.did = SCENARIO_GATEWAY_DID;
        scenariolist[srcCount].src.onoff = SCENARIO_SOURCE_SUNRISE;
        srcCount++;
    }
    if(has_sunset == 0)
    {//no sunset, create
        memset(&scenariolist[srcCount], 0, sizeof(jswscenario));
        scenariolist[srcCount].src.did = SCENARIO_GATEWAY_DID;
        scenariolist[srcCount].src.onoff = SCENARIO_SOURCE_SUNSET;
        srcCount++;
    }
    if( (has_sunrise==0) || (has_sunset==0) )
        saveScenario();
}

//check and add sunrise/sunset in scenario
void check_add_2_MAG_in_scenario()
{
    int has_open = 0;
    int has_close = 0;
    int k, m;
    unsigned char bChange = 0;

    for(m=0;m<devcount;m++)
    {
        if( (devlist[m].model == TYPE_MAGNETIC) || (devlist[m].model == TYPE_BUTTON) )
        {
            has_open = 0;
            has_close = 0;
            for (k=0;k<srcCount;k++)
            {
                if(devlist[m].did != scenariolist[k].src.did)
                    continue;

                if(scenariolist[k].src.onoff == SCENARIO_ONOFF_MAG_OPEN) //SCENARIO_BUTTON_LONG_PRESS
                    has_open++;
                if(scenariolist[k].src.onoff == SCENARIO_ONOFF_MAG_CLOSE) //SCENARIO_BUTTON_SHORT_PRESS
                    has_close++;
            }

            if( (has_open == 0) || (has_close == 0) )
            {//need to add scenario
                if(has_open == 0)
                {//add MAG open scenario
                    memset(&scenariolist[srcCount], 0, sizeof(jswscenario));
                    scenariolist[srcCount].src.did = devlist[m].did;
                    scenariolist[srcCount].src.onoff = SCENARIO_ONOFF_MAG_OPEN; //SCENARIO_BUTTON_LONG_PRESS
                    srcCount++;
                    bChange++;
                }
                if(has_close == 0)
                {//add MAG close scenario
                    memset(&scenariolist[srcCount], 0, sizeof(jswscenario));
                    scenariolist[srcCount].src.did = devlist[m].did;
                    scenariolist[srcCount].src.onoff = SCENARIO_ONOFF_MAG_CLOSE; //SCENARIO_BUTTON_SHORT_PRESS
                    srcCount++;
                    bChange++;
                }
            }
        }
    }

    if(bChange > 0)
        saveScenario();
}

//write customer id to MCU
void SendCustomerIDtoMCU(char *did)
{
	  Customer_code = CUSTOMER_UNKNOW;
printf("DID = %s\n", did);
    if(strstr(did, "WGXX-"))
    {//Custom JSW
printf("JSW DID\n");
        SetRunningLog_str("JSW DID");
		Customer_code = CUSTOMER_JSW;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }else if(strstr(did, "WGAG-"))
    {//Custom ABUS
printf("ABUS DID\n");
        SetRunningLog_str("ABUS DID");
		Customer_code = CUSTOMER_ABUS;
        set_comstom_code(0xC6, 0x54, 0x75, 0x19, 0x12, 0);
    }else if(strstr(did, "WGAA-"))
    {//ALC/HKL
printf("ALC DID\n");
        SetRunningLog_str("ALC DID");
		Customer_code = CUSTOMER_ALC;
        set_comstom_code(0xC6, 0x54, 0x75, 0x33, 0x2B, 0);
    }else if(strstr(did, "WGGA-"))
    {//German Losada
printf("Losada DID\n");
        SetRunningLog_str("Losada DID");
		Customer_code = CUSTOMER_LOSADA;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }else if(strstr(did, "WGHS-"))
    {//Home ON International Limited
printf("Home ON DID\n");
        SetRunningLog_str("Home ON DID");
		Customer_code = CUSTOMER_HomeON;
        set_comstom_code(0xC6, 0x54, 0x75, 0x36, 0x2E, 0);
    }else if(strstr(did, "WGHY-"))
    {//Raylios
printf("Raylios DID\n");
        SetRunningLog_str("Raylios DID");
		Customer_code = CUSTOMER_RAYLOIOS;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }else if(strstr(did, "WGIG-"))
    {//Indexa
printf("Indexa DID\n");
        SetRunningLog_str("Indexa DID");
		Customer_code = CUSTOMER_INDEXA;
        set_comstom_code(0xC6, 0x54, 0x75, 0x1F, 0x18, 0);
    }else if(strstr(did, "WGJE-"))
    {//JunLian
printf("JunLian DID\n");
        SetRunningLog_str("JunLian DID");
		Customer_code = CUSTOMER_JunLian;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }else if(strstr(did, "WGPT-"))
    {//PWS Security Outlet Co. Lte
printf("PWS Security DID\n");
        SetRunningLog_str("PWS Security DID");
		Customer_code = CUSTOMER_PWS;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }else if(strstr(did, "WGTN-"))
    {//Teco Asia Limited (Nedis)
printf("Nedis DID\n");
        SetRunningLog_str("Nedis DID");
		Customer_code = CUSTOMER_Nedis;
        set_comstom_code(0xC6, 0x54, 0x75, 0x3C, 0x35, 0);
    }else if(strstr(did, "WGTT-"))
    {//SecuFirst
printf("SecuFirst DID\n");
        SetRunningLog_str("SecuFirst DID");
		Customer_code = CUSTOMER_SecuFirst;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }else if(strstr(did, "WGUA-"))
    {//Uniden America
printf("Uniden America DID\n");
        SetRunningLog_str("Uniden America DID");
		Customer_code = CUSTOMER_Uniden_USA;
        set_comstom_code(0xC6, 0x54, 0x75, 0x2F, 0x28, 0);
    }else if(strstr(did, "WGUB-"))
    {//Uniden Australia/New Zealand
printf("Uniden Australia DID\n");
        SetRunningLog_str("Uniden Australia DID");
		Customer_code = CUSTOMER_Uniden_AUS;
        set_comstom_code(0xC6, 0x54, 0x75, 0x2F, 0x28, 0);
    }else if(strstr(did, "WGUJ-"))
    {//Uniden Japan
printf("Uniden Japan DID\n");
        SetRunningLog_str("Uniden Japan DID");
		Customer_code = CUSTOMER_Uniden_JP;
        set_comstom_code(0xC6, 0x54, 0x75, 0x2F, 0x28, 0);
    }else if(strstr(did, "WGYU-"))
    {//Y3K Group
printf("Y3K DID\n");
        SetRunningLog_str("Y3K DID");
		Customer_code = CUSTOMER_Y3K;
        set_comstom_code(0xC6, 0x54, 0x75, 0x39, 0x32, 0);
    }else if(strstr(did, "WGKI-"))
    {//Konelco
printf("Konelco DID\n");
        SetRunningLog_str("Konelco DID");
		Customer_code = CUSTOMER_Konelco;
        set_comstom_code(0xC6, 0x54, 0x75, 0x26, 0x1E, 0);
    }else if(strstr(did, "WGEF-"))
    {//Extel
printf("Extel DID\n");
        SetRunningLog_str("Extel DID");
		Customer_code = CUSTOMER_Extel;
        set_comstom_code(0xC6, 0x54, 0x75, 0x29, 0x22, 0);
    }else if(strstr(did, "WGSN-"))
    {//Smartwares
printf("Smartwares DID\n");
        SetRunningLog_str("Smartwares DID");
		Customer_code = CUSTOMER_Smartwares;
        set_comstom_code(0xC6, 0x54, 0x75, 0x2C, 0x25, 0);
    }
	else if(strstr(did, "WGAM-"))
	{//ARCHTRON
printf("ARCHTRON DID\n");
		SetRunningLog_str("ARCHTRON DID");
		Customer_code = CUSTOMER_ARCHTRON;
		set_comstom_code(0xC6, 0x54, 0x75, 0x4C, 0x45, 0);
	}
	else if(strstr(did, "WGJA-"))
    {//Jims Security
printf("Jims Security DID\n");
        SetRunningLog_str("Jims Security DID");
		Customer_code = CUSTOMER_JimsSecurity;
        set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }
	else if(strstr(did, "WGNF-"))
    {//New Deal
printf("New Deal DID\n");
        SetRunningLog_str("New Deal DID");
		Customer_code = CUSTOMER_NewDeal;
        set_comstom_code(0xC6, 0x54, 0x75, 0x49, 0x42, 0);
    }
	else if(strstr(did, "WGMT-"))
	{//MAXWELL
printf("MAXWELL DID\n");
		SetRunningLog_str("MAXWELL DID");
		Customer_code = CUSTOMER_MAXWELL;
		set_comstom_code(0xC6, 0x54, 0x75, 0x53, 0x4B, 0);
	}
	else if(strstr(did, "WGAN-"))
    {//ALC BV
printf("ALC BV DID\n");
        SetRunningLog_str("ALC BV DID");
        Customer_code = CUSTOMER_ALCBV;
        set_comstom_code(0xC6, 0x54, 0x75, 0x33, 0x2B, 0);
	}
	else if(strstr(did, "WGST-"))
	{//SYSLINK
printf("SYSLINK DID\n");
		SetRunningLog_str("SYSLINK DID");
		Customer_code = CUSTOMER_SYSLINK;
		set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);
    }
	else
    {//Not found
        printf("Invalid DID = %s\n", did);
//        SetRunningLog_str("SendCustomIDtoMCU: Invalid DID=");
//        SetRunningLog_str(did);
			printf("Set to Factory DID(JSW)\n");
			SetRunningLog_str("JSW DID");
			Customer_code = CUSTOMER_JSW;
			set_comstom_code(0xC6, 0x54, 0x75, 0xC5, 0x2A, 0);

    }
}
//--------------------------------------- --------------------------------------
static void thermo_data_handler(thermo_data_t *before, thermo_data_t *after)
{

	char text[256] = {0};
    printf("%s | [%s] | %s\n", __func__, before->device_id,
        after->device_id[0] ? "changed" : "deleted");
	if(after->device_id[0] == 0)
		{
		sprintf(text, "Nest [%s] Removed",load_NEST_Thermo_name(before));
		 gcm_send_event_list(text);
		NEST_device_update = 1;
		return;
		}

	if(check_NEST_joined(before->device_id) != 1) return;
//	sprintf(text, "Nest [%s] state changed", after->name);
//    gcm_send_event_list(text);
}
//------------------------------------------------------------------------------
static void smoke_data_handler(smoke_data_t *before, smoke_data_t *after)
{
    printf("%s | [%s] | %s\n", __func__, before->device_id,
        after->device_id[0] ? "changed" : "deleted");

	char text[256] = {0};

	if(after->device_id[0] == 0)
		{
		sprintf(text, "Nest [%s] Removed",load_NEST_Smoke_name(before));
		gcm_send_event_list(text);
		NEST_device_update = 1;
		return;
		}
	if(check_NEST_joined(before->device_id) != 1) return;
	if(before->co_alarm_state != after->co_alarm_state)
		{
			if(after->co_alarm_state == SMOKE_STATE_WARNING || after->co_alarm_state == SMOKE_STATE_EMERGENCY)
				{
				//sprintf(text, "Nest [%s] co_alarm=%s", after->name, nrt_smoke_state_str(after->co_alarm_state));
				newNEST_Event(get_NEST_Smoke_index(after),TYPE_NEST_SMOKE,after->co_alarm_state,1);
				triggerAlarm(1, 1);
				sprintf(text, "Nest [%s] co_alarm=%s", load_NEST_Smoke_name(after), nrt_smoke_state_str(after->co_alarm_state));
				}
		}
	else if(before->smoke_alarm_state != after->smoke_alarm_state)
		{

			if(after->smoke_alarm_state == SMOKE_STATE_WARNING || after->smoke_alarm_state == SMOKE_STATE_EMERGENCY)
				{
				newNEST_Event(get_NEST_Smoke_index(after),TYPE_NEST_SMOKE,after->smoke_alarm_state,1);
				//sprintf(text, "Nest [%s] smoke_alarm=%s", after->name, nrt_smoke_state_str(after->smoke_alarm_state));
				triggerAlarm(1, 1);
				sprintf(text, "Nest [%s] smoke_alarm=%s", load_NEST_Smoke_name(after), nrt_smoke_state_str(after->smoke_alarm_state));
				}
		}
	else if(before->battery_health != after->battery_health)
		{
			//newNEST_Event(get_NEST_Smoke_index(after),TYPE_NEST_SMOKE,after->co_alarm_state,0);
			//if(after->battery_health == SMOKE_STATE_WARNING)
			//sprintf(text, "Nest [%s] battery=%s", after->name, nrt_smoke_state_str(after->smoke_alarm_state));
			sprintf(text, "Nest [%s] battery=%s", load_NEST_Smoke_name(after), nrt_smoke_state_str(after->smoke_alarm_state));
		}
	else
		{
		newNEST_Event(get_NEST_Smoke_index(after),TYPE_NEST_SMOKE,after->co_alarm_state,0);
		//sprintf(text, "Nest [%s] state=%s", after->name, nrt_smoke_state_str(after->ui_color_state));
		sprintf(text, "Nest [%s] state=%s",load_NEST_Smoke_name(after), nrt_smoke_state_str(after->ui_color_state));
		}
    gcm_send_event_list(text);
}

static void auth_revoked_handler(void)
{
    printf("%s\n", __func__);
	memset(&g_stNEST_parm, 0 , sizeof(stNEST_parm) );
	NEST_device_update = 1;
}


int check_NEST_joined(const char *NEST_dviceID)
{
	int index = 0;
	for(index = 0 ; index < MAX_NEST_DEVICE ; index++)
	{
		if(strcmp(NEST_dviceID,g_stNEST_parm.stNEST_device_list[index].device_ID) == 0)
		{
			return g_stNEST_parm.stNEST_device_list[index].joined;
		}
	}
	return 0;
}
char* load_NEST_Smoke_name(smoke_data_t *this_smoke_data)
{
	int index = 0;

	for(index = 0 ; index < MAX_NEST_DEVICE ; index++)
	{
		if(strcmp(this_smoke_data->device_id,g_stNEST_parm.stNEST_device_list[index].device_ID) == 0)
		{
			printf("load_NEST_Smoke_name[%s] = MATCH\n",g_stNEST_parm.stNEST_device_list[index].device_name);
			if(strlen(g_stNEST_parm.stNEST_device_list[index].device_name) > 1)
				return g_stNEST_parm.stNEST_device_list[index].device_name;
		}
	}
	return this_smoke_data->name;

}
void remove_NEST_Smoke(smoke_data_t *this_smoke_data)
{
	int index = 0;

	for(index = 0 ; index < MAX_NEST_DEVICE ; index++)
	{
		if(strcmp(this_smoke_data->device_id,g_stNEST_parm.stNEST_device_list[index].device_ID) == 0)
		{
			memset(&g_stNEST_parm.stNEST_device_list[index],0,sizeof(stNEST_device));
			return;
		}
	}
}

unsigned int get_NEST_Smoke_index(smoke_data_t *this_smoke_data)
{
	int index = 0;
	for(index = 0 ; index < MAX_NEST_DEVICE ; index++)
	{
		if(strcmp(this_smoke_data->device_id,g_stNEST_parm.stNEST_device_list[index].device_ID) == 0)
		{
			return 0xffffff00 | index;
		}
	}
	return 0xffffffff;

}


char* load_NEST_Thermo_name(thermo_data_t *this_thermo_data)
{
	int index = 0;

	for(index = 0 ; index < MAX_NEST_DEVICE ; index++)
	{
		if(strcmp(this_thermo_data->device_id,g_stNEST_parm.stNEST_device_list[index].device_ID) == 0)
		{
			if(strlen(g_stNEST_parm.stNEST_device_list[index].device_name) > 1)
				return g_stNEST_parm.stNEST_device_list[index].device_name;
		}
	}
	return this_thermo_data->name;
}
void remove_NEST_Thermo(thermo_data_t *this_thermo_data)
{
	int index = 0;

	for(index = 0 ; index < MAX_NEST_DEVICE ; index++)
	{
		if(strcmp(this_thermo_data->device_id,g_stNEST_parm.stNEST_device_list[index].device_ID) == 0)
		{
			memset(&g_stNEST_parm.stNEST_device_list[index],0,sizeof(stNEST_device));
				return;
		}
	}
}


int update_NEST_devicelist(void)
{
	printf("update_NEST_devicelist [%d-%d]@%s]\n",g_NEST_running, g_stNEST_parm.nest_bind,g_stNEST_parm.access_token);

	if( g_NEST_running==1 && g_stNEST_parm.nest_bind ==0)
		nrt_stop();
	else if(g_NEST_running == 0 && g_stNEST_parm.nest_bind ==1)
	{
		g_NEST_running =1;
		int nest_index =0;
	    nrt_start(g_stNEST_parm.access_token);
		strcpy(g_NEST_active_token,g_stNEST_parm.access_token);
		for(nest_index = 0 ; nest_index < MAX_NEST_DEVICE ; nest_index++)
		{
			if(g_stNEST_parm.stNEST_device_list[nest_index].joined == 1)
			{
				if(	g_stNEST_parm.stNEST_device_list[nest_index].device_type == TYPE_NEST_SMOKE)
					{
					printf("Join Smoke-%s[%s]\n",g_stNEST_parm.stNEST_device_list[nest_index].device_name,\
						g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					nrt_add_smoke_id(g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					}
				if(	g_stNEST_parm.stNEST_device_list[nest_index].device_type == TYPE_NEST_THERMO)
					{
					printf("Join Thermo-%s[%s]\n",g_stNEST_parm.stNEST_device_list[nest_index].device_name,\
						g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					nrt_add_thermo_id(g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					}
			}
		}

	}
	else if(g_NEST_running == 1 && g_stNEST_parm.nest_bind ==1)
	{

		if(strcmp(g_NEST_active_token,g_stNEST_parm.access_token) != 0)
		{

			strcpy(g_NEST_active_token,g_stNEST_parm.access_token);
	    	nrt_start(g_NEST_active_token);
		}
		int nest_index =0;
		for(nest_index = 0 ; nest_index < MAX_NEST_DEVICE ; nest_index++)
		{
			if(g_stNEST_parm.stNEST_device_list[nest_index].joined == 1)
			{
				if(	g_stNEST_parm.stNEST_device_list[nest_index].device_type == TYPE_NEST_SMOKE)
					{
					printf("Join Smoke-%s[%s]\n",g_stNEST_parm.stNEST_device_list[nest_index].device_name,\
						g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					nrt_add_smoke_id(g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					}
				if(	g_stNEST_parm.stNEST_device_list[nest_index].device_type == TYPE_NEST_THERMO)
					{
					printf("Join Thermo-%s[%s]\n",g_stNEST_parm.stNEST_device_list[nest_index].device_name,\
						g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					nrt_add_thermo_id(g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					}
			}
		}
	}
}


void stopcCloud()
{
	
	//printf(" ### stopcCloud g_cloud_running[%d]\n", g_cloud_running);
	if( g_cloud_running == 1)
	{
		g_cloud_running = 0;
		cloud_action_stop();
		cloud_event_stop();
	}

}


void startCloud()
{
	//stop current cloud thread 
	stopcCloud();

	if(strlen(g_stUserParam.szEntryId) ==0 || strlen(g_stUserParam.szUsername) ==0)
	{	
		printf(" ### startCloud entryid/username empty \n");
		return;
	}
	
	printf(" ### startCloud entryid[%s]\n",g_stUserParam.szEntryId);

	
	//printf(" ### startCloud entryid[%s]\n",g_stUserParam.szEntryId);
	if( strlen(g_stUserParam.szEntryId) > 10 )
	{
		g_cloud_running =1;
		cloud_event_start(gDID, g_stUserParam.szEntryId);	
		cloud_action_start(&g_stUserParam);
	}

}



//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {

	int k;

	system("killall -9 telnetd");
	system("killall -9 boa");

	updateV2();

	memset(sdpath, 0, sizeof(sdpath));
	if (isMmcblk0Valid())
		strcpy( sdpath ,"/media/mmcblk0/sn98601/");
	else if(isMmcblk0p1Valid())
		strcpy( sdpath ,"/media/mmcblk0p1/sn98601/");

    if(0 == access("./NewMCU1.sn8", 0))
    {//start MCU update (filename ./NewMCU2.sn8)
	printf("MCU update2!!!\n");

        //LED flash when FW upgrade
        //system(LED_SETTING_P2P_NG2);
       system(LED_FW_UPGRADE);

        system("rm ./NewMCU2.sn8;sync");
        system("cp ./NewMCU1.sn8 ./NewMCU2.sn8;sync");
        system("rm ./NewMCU1.sn8;sync");
        sync();
        usleep(1*1000*1000);

	    //initial uart thread
        //serial_main();
        //usleep(3*1000*1000);

        //call mcu update
        char pathname[256];
        sprintf(pathname, "./McuUpdate %s%s", sdpath, "NewMCU2.sn8");
printf("pathname=%s!!!\n", pathname);
        system(pathname);

        //usleep(60*1000*1000);
        //system("reboot");
        exit(0);
	}

	if(0 == access("./NewMCU2.sn8", 0))
	{//MCU update failed
printf("MCU update3!!!\n");
        system("rm ./NewMCU1.sn8;sync");
        usleep(100*1000);
        system("cp ./GatewayMCU.sn8 ./NewMCU1.sn8;sync");
        sync();

        if(0 == access("./NewMCU1.sn8", 0))
        {//start MCU update (filename ./NewMCU2.sn8)
            system("reboot");
            exit(0);
        }
//        system("rm ./NewMCU2.sn8");
//        sync();
	}

	//// Save pid itself
	{
		char sCmd[128] = "";

		snprintf(sCmd, sizeof(sCmd), "echo %d > /var/run/ceres.pid", getpid());
		system(sCmd);
	}

#ifdef LOG_FILE
	// 打开测试文件
	//openLogFile();
#endif


	loadmac();


	memset(gP2PSession, 0, sizeof(gP2PSession));
	memset(gInitString, 0, sizeof(gInitString));
	memset(gPassword, 0, sizeof(gPassword));
	memset(gDID, 0, sizeof(gDID));
	reset_last_schedule();
	memset(gRunningLog, 0, sizeof(gRunningLog));
	memset(g_battlowlist, 0, sizeof(g_battlowlist));
	g_battlowcount = 0;
	memset(g_sunrisesunset, 0, sizeof(g_sunrisesunset));
	//apple
	memset(&g_stUserParam, 0, sizeof(g_stUserParam));

    SetRunningLog_str("Running: main() start");
//	printf("=========NO gateway_wdt Mode =============\n");
//	system("killall -9 gateway_wdt");

	//
	int ret = gwinfo();
	if (ret ==0 )
	{
	    SetRunningLog_str("Error: gwinfo() return failed");
		return 0;
	}

    if(loaddata() <= 0) //load new jswitem
        loaddata2(); //load old jswitem

	// snx_gpio_open();

	if (devcount > 0)
	{
		loadScenario();

        //check and add two MAG in scenario
        check_add_2_MAG_in_scenario();

		loadPAprop();
		loadzone();

		for (k=0;k<devcount;k++)
		{
			keepalivelist[k].did = devlist[k].did;
			keepalivelist[k].lastcheckin = time(0);
		}
	}
	loadpush();
	loadsetting();
	loadsetting5();


    load_batterylow();
	printf("g_battlowcount=%d\n", g_battlowcount);
    if(g_battlowcount > 0)
    {
        if (g_setting.gwprop.ledon == 1)
            yellowled(0);
        else if (g_setting.gwprop.ledon == 2)
            yellowled(1);
	}

    //check and add sunrise/sunset in scenario
    check_add_sunrisesunset_in_scenario();

    //pushdata.smson = 0; //disable sms notification

	checkfactoryreset();

	gcmRegister(gDID);

	DBG_PRINT("Gateway version v%d\n", GATEWAY_VERSION);

	initled();

	malloc_AES_key_stack(gAESKeyNum);
	for (k = 0; k < gAESKeyNum; k++)
		strncpy((char*)(gAESKeys[k]), AES_KEY_DEFAULT, AES_KEY_LENGTH);

	//load tmpevent2 first, if fail, load tmpevent
	if(loadTmpEvdata(2) == 0)
		loadTmpEvdata(1);//to evtlist
	delTmpEvdata(2);
	loadLastEvdata();//to o_evtlist

	serial_main();

	//write custom id to MCU
	SendCustomerIDtoMCU(gDID);
	usleep(UART_WRITE_DELAY*20);
	usleep(UART_WRITE_DELAY*20);
	usleep(UART_WRITE_DELAY*20);

    //start to flash Network LED
    system(LED_SETTING_P2P_FLASH);

#if 0
// Disable g-sensor
    //set g-sensor to middle
    g_setting4.extra.g_sensor_level = JSW_GSENSOR_MIDDLE;
    McuGpio2(0xC7, g_setting4.extra.g_sensor_level, 0, 0, 0, 0);
	usleep(UART_WRITE_DELAY);
#endif
	pthread_create(&p2p_thread, NULL, p2p_sub_thread_routine, 0);

	makeTimer( &gettemp_t, 3, 120);
#ifndef MCU_ACK_THREAD
	makeTimer( &writeack_t, 4, 2);
#endif
	makeTimer(&schedule_t, 30, 30);
	makeTimer(&keepalive_t, 60*10, (int)(60*60*DEF_SUPERVISION_TIMER_DUR));//3.5
	makeTimer(&ipcreconnect_t, 60, 41); //60 * 3, 60 *3);
	//makeTimer(&acctreset_t, 5, 3);
	makeTimer(&NEST_device_update_t,5,6); //check nest device list upgate

	// stop Pairing
		writePairing(g_DID, 0, 0xFF);
			usleep(UART_WRITE_DELAY);

	load_Nest_parm();
	load_alarm_info();
	load_cloud_goolg_DA();
	load_cloud_user_parm(); //apple
	
#if 0
		writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_VERSION);
		usleep(UART_WRITE_DELAY);
#endif


	pthread_t net_detect_handle = NULL;
#if 0
	if( (getc_popen("cat /sys/class/net/eth0/carrier") == '0') && (g_setting.network.dhcp == 1) )
	{
		pthread_create(&net_detect_handle, NULL, net_detect_thread, NULL);
		//pthread_join(net_detect_handle, NULL);
	}
#else
	pthread_create(&net_detect_handle, NULL, net_detect_thread, NULL);

#endif
	initipcam();
	//usleep(2000);

#ifdef SEARCH_BROADCAST_ENABLE
	pthread_create(&search_bc_thread, NULL, search_bc_thread_rountine, 0);
#endif

	//hardware reset thread
	pthread_create(&hardwarereset_thread, NULL, hardware_reset_thread, NULL);
	//pthread_join(hardwarereset_thread, NULL);

    //ping hinet DNS thread
    pthread_create(&ping_hinet_thread, NULL, ping_hinet_thread_routine, NULL);

	//come back to last armstate
	loadsetting4();
	exitdelayTarget = g_armstate;
	if( (g_armstate == st_arm) || (g_armstate == st_partarm) )
		{
		#if 0
		 // JSW_VOLUME_MUTE:
         McuGpio2(0xD2, 0x13, 1, 0, 0, 0);
		systemArm2();
		//load default volume
		switch(g_setting.gwprop.alarmOn)
        {
        case JSW_VOLUME_HIGH:
            McuGpio2(0xD2, 0x10, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_MEDIUM:
            McuGpio2(0xD2, 0x11, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_LOW:
            McuGpio2(0xD2, 0x12, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_MUTE:
            McuGpio2(0xD2, 0x13, 1, 0, 0, 0);
            break;
        }
		#else
		set_SHC_PRO_alarm_LED(1);
		systemArm2();
		#endif
		}
	else if(g_armstate == st_disarm)
		{
		#if 0
		 // JSW_VOLUME_MUTE:
         McuGpio2(0xD2, 0x13, 1, 0, 0, 0);
		systemDisarm2();
		//load default volume
		switch(g_setting.gwprop.alarmOn)
        {
        case JSW_VOLUME_HIGH:
            McuGpio2(0xD2, 0x10, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_MEDIUM:
            McuGpio2(0xD2, 0x11, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_LOW:
            McuGpio2(0xD2, 0x12, 1, 0, 0, 0);
            break;
        case JSW_VOLUME_MUTE:
            McuGpio2(0xD2, 0x13, 1, 0, 0, 0);
            break;
        }
		#else
		systemDisarm2();
		set_SHC_PRO_alarm_LED(0);
		#endif
		}
    if( (pushdata.pushon > 0) && (g_setting4.extra.push_selection_1 == 0) )
    {//just apply new FW, need to change default push selection
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_ARM;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_PART_ARM;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_DISARM;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_PANIC;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_TAMPER;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_ALARM;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_SENSOR_BATTLOW;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_GATEWAY_BATTLOW;
        g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_LOST_SIGNAL;
    }

    //come back to last LED state
    set_alarm_LED();

    //set DNS setting
    set_DNS();

    //set volume level
    play_beep_set_volume(g_setting.gwprop.alarmOn);

//#ifndef CERES_UTIL
//	gCurl = curl_easy_init();
//	if (gCurl) {
//		gcm_reg_server();
//	}
//#endif
#ifdef MCU_ACK_THREAD
		// RF_resp_check_therad
		pthread_create(&RF_resp_check_therad, NULL, checkAckfromWrite_thread, NULL);
#endif

    nrt_init(thermo_data_handler, smoke_data_handler, auth_revoked_handler);
	if(g_stNEST_parm.nest_bind)
	{
		g_NEST_running =1;
		int nest_index =0;
	    nrt_start(g_stNEST_parm.access_token);
		for(nest_index = 0 ; nest_index < MAX_NEST_DEVICE ; nest_index++)
		{
			if(g_stNEST_parm.stNEST_device_list[nest_index].joined == 1)
			{
				if(	g_stNEST_parm.stNEST_device_list[nest_index].device_type == TYPE_NEST_SMOKE)
					{
					printf("Join Smoke-%s[%s]\n",g_stNEST_parm.stNEST_device_list[nest_index].device_name,\
						g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					nrt_add_smoke_id(g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					}
				if(	g_stNEST_parm.stNEST_device_list[nest_index].device_type == TYPE_NEST_THERMO)
					{
					printf("Join Thermo-%s[%s]\n",g_stNEST_parm.stNEST_device_list[nest_index].device_name,\
						g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					nrt_add_thermo_id(g_stNEST_parm.stNEST_device_list[nest_index].device_ID);
					}
			}
		}
		

	}

	cloud_action_init();
	cloud_event_init();

    //apple
    if(strlen(g_stUserParam.szEntryId) > 0)
	{
		printf("main startCloud \n");
	    startCloud();
    }
	
    SetRunningLog_str("Running: main() end");

	//pthread_mutex_init(&event_session_mutex, NULL);
	pthread_join(p2p_thread, &p2p_sub_thread_result);

//#ifdef SEARCH_BROADCAST_ENABLE
	pthread_join(search_bc_thread, &search_bc_sub_thread_result);
//#endif

	if(net_detect_handle != NULL)
		pthread_join(net_detect_handle, NULL);

	pthread_join(hardwarereset_thread, NULL);

    pthread_join(ping_hinet_thread, NULL);
#ifdef MCU_ACK_THREAD
	if(RF_resp_check_therad != NULL)
		pthread_join(RF_resp_check_therad, NULL);
#endif
	free_AES_key_stack();

//#ifndef CERES_UTIL
//	curl_easy_cleanup(gCurl);
//#endif
   //apple
	cloud_action_stop();
	cloud_event_stop();
	
	nrt_destroy();
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



INT32 p2p_process_cmd(INT32 session) {

	jswhdr hdr;
	struct _jswproto_hdr *hdr2;
	int Size;
	int ret;
	unsigned int tmp;
	time_t t;
	int r;

	static char isUpgrading = 0;
	//unsigned char *aeskey = (unsigned char*)getP2PSessionAesKey(session);
    int hlen = sizeof(jswhdr);
	memset(&hdr, 0, hlen );
	// Read header
	int nRead = hlen;

	ret = PPPP_Read(session, CH_CTRL, (CHAR *) &hdr, &nRead,10000);

	if (ret == ERROR_PPPP_TIME_OUT)
		return 0;
	else
	{
		if (ret != ERROR_PPPP_SUCCESSFUL) {
			DBG_PRINT("Session Closed err(%d) at PPPP_Read\n", ret);
            SetRunningLog_str_int("Error: p2p_process_cmd() read failed", ret);
			return -1;
		}
	}
	if ( nRead != hlen )
	{
		DBG_PRINT("protocol hdr length error\n");
        SetRunningLog_str_int("Error: p2p_process_cmd() read length not equal", nRead);
		return -1;
	}

	if(hdr.cmd != CM_DATA_ENCRYPT)
	{
        if((hdr.cmd >= 9000) && (hdr.cmd <= 9100))
        {
            DBG_PRINT("##Command from client2(%d) cmd(xx) payload length %d\n", session, hdr.payload_length);
        }else
        {
            DBG_PRINT("##Command from client2(%d) cmd(%d) payload length %d\n", session, hdr.cmd, hdr.payload_length);
        }
	}

	Size = hdr.payload_length;
	char inbuf[MAX_P2P_BUFFER], inbuf_encode[MAX_P2P_BUFFER];
	memset(inbuf, 0, sizeof(inbuf));
	memset(inbuf_encode, 0, sizeof(inbuf_encode));
	if (Size > 0 && Size < MAX_P2P_BUFFER) //count > 1 ?
	{
		ret = PPPP_Read(session, CH_CTRL, inbuf_encode, &Size, 10000);

		if (ret != ERROR_PPPP_SUCCESSFUL) {
			DBG_PRINT("Session Closed at reading client cmd %d errcode : (%d)\n",hdr.cmd, ret);
            SetRunningLog_str_int("Error: p2p_process_cmd() read failed", ret);
			return -1;
		}
		if(hdr.cmd == CM_DATA_ENCRYPT)
		{
			if(PPPP_Read_Wrap2(inbuf_encode, Size, inbuf, 1, getP2PSessionAesKey(session)) < 0)
			{
				DBG_PRINT("P2P data encrypted failed, cmd %d size:(%d)\n",hdr.cmd, Size);
                SetRunningLog_str_int("Error: p2p_process_cmd() decode failed", Size);
				return -1;
			}else
			{
				hdr2 = (struct _jswproto_hdr *)inbuf;
				if( (hdr2->payload_length < 0) || (hdr2->payload_length >= MAX_P2P_BUFFER) )
				{
					printf("payload length error2: cmd %d payload_length=%d\n", hdr2->cmd, hdr2->payload_length);
                    SetRunningLog_str_int("Error: p2p_process_cmd() invalid length", hdr2->payload_length);
					return -1;
				}
				inbuf[ sizeof(struct _jswproto_hdr) + hdr2->payload_length] = 0;
				memcpy(&hdr, hdr2, sizeof(struct _jswproto_hdr));
				if(hdr.payload_length >= MAX_P2P_BUFFER)
				{
					printf("payload length error3: cmd %d payload_length=%d\n", hdr.cmd, hdr.payload_length);
                    SetRunningLog_str_int("Error: p2p_process_cmd() invalid length", hdr.payload_length);
					return -1;
				}else
				{
					memcpy(inbuf_encode, &inbuf[sizeof(struct _jswproto_hdr)], hdr.payload_length);
					inbuf_encode[hdr.payload_length] = 0;
					memcpy(inbuf, inbuf_encode, hdr.payload_length);
					inbuf[hdr.payload_length] = 0;
				}
				if((hdr.cmd >= 9000) && (hdr.cmd <= 9100))
				{
                    DBG_PRINT("##Command from client2 cmd (xx) payload length %d\n", hdr.payload_length);
				}else
				{
                    DBG_PRINT("##Command from client2 cmd (%d) payload length %d\n", hdr.cmd, hdr.payload_length);
				}
			}
		}else
		{
			memcpy(inbuf, inbuf_encode, Size);
		}
	}else
	{
		if(hdr.payload_length < 0)
		{
			printf("payload length error: cmd %d payload_length=%d\n", hdr.cmd, hdr.payload_length);
            SetRunningLog_str_int("Error: p2p_process_cmd() invalid length", hdr.payload_length);
			return -1;
		}
	}
	switch (hdr.cmd) {
		case CM_DEV:  //0x12

			//DBG_PRINT("##packDevlist count %d\n", devcount);

			if (devcount > 0)
			{
				/*pthread_mutex_lock(&aes_mutex);
				unsigned char *encrytStr = encryptAES((unsigned char *) devlist, &datalen, aeskey);
				pthread_mutex_unlock(&aes_mutex);*/

				int payload_len =  sizeof(jswdev) * devcount;
				// Jeff define to change get command resp to uni-cast
//				sendCmdtoClient(CM_DEV, 0,  devcount,  payload_len,(char*) &devlist) ;
				sendCmdtoClient2(CM_DEV, 0,	devcount,  payload_len,(char*) &devlist,session) ;


			}else
				{
				sendCmdtoClient2(CM_DEV, 3,  0,  0, session) ;

				//sendCmdtoClient(CM_DEV, 3,  0,  0, NULL) ;
				}

			if( (g_sirenstate == 1) || (g_smokeOn == 1) || (g_ShowAlarmView == 1) )
			{
				//printf("Trigger Alarm\n");
				//sendCmdtoClient(CM_SIREN_ON, 0, 0,  0,NULL);
				sendCmdtoClient(CM_SIREN_ON, 0, 1,  sizeof(jsw_alarm_event),(char*)&last_trigger);
				printf("id=%d , time=%d , type=%d , status=%d\n",last_trigger.id,last_trigger.time,last_trigger.type,last_trigger.status);
			}


			break;

		case CM_GETEVENT://inbuf: start time, end time, by unsigned int
			getevent(hdr.payload_length, inbuf, session);
			break;

		case CM_CLEAREVENT:
			clearevent();
			break;

		case CM_SEARCHEVENT://inbuf: start time, end time, did, model, status, by unsigned int
			searchevent(hdr.payload_length, inbuf, session);
			break;

		case CM_NEWDEVICE:
			addNewDevice(inbuf);
			break;

		case CM_DELETEDEV:

			if (hdr.count ==1 )
			{
				memcpy(&tmp, inbuf, sizeof(int));
				deleteDev(tmp );
			}else
				DBG_PRINT("##delete count \n");

			break;

		case CM_EDITDEV:
			if (hdr.count ==1 )
			{
				editDev( inbuf );
			}
			break;

		case CM_DISCARDPAIRING:
			if (g_isPairMode ==1)
			{
				if ( pairing_current_t != 0)
					{
					killTimer(pairing_current_t);
					makeTimer( &pairing_t, 0, 0);
					}
				pairing_t = pairing_current_t= 0;

				g_isPairMode = 0;
				memset(&g_pairDevinfo, 0, sizeof(jswdev));
				if ( uartbusy ==1)
					uartbusy = 0;
				writePairing(g_DID, 0, 0xFF);
			}
			break;

			//////ARM DISARM
		case CM_ARM:
			armCommand(c_arm);
			break;
		case CM_DISARM:
			if(g_armstate != st_disarm)
				armCommand(c_disarm);
			else
			{
				g_ShowAlarmView = 0;
				g_sirenstate = 0;
				g_smokeOn = 0;
				//systemDisarm2();
				systemDisarm();
				//play_beep_disarm();
			}
			//	system("date -s '1988-12-10 10:10:00'");
			break;
		case CM_PARTARM:
			armCommand(c_partarm);
			break;
		case CM_EXITDELAY:
			sendCmdtoClient2(CM_EXITDELAY, 0,1 , sizeof(stExitDelay_parm),(char*)&g_ExitDelay_parm,session) ;
			break;
		case CM_ENTRYDELAY:
			sendCmdtoClient2(CM_ENTRYDELAY, 0,1 , sizeof(stExitDelay_parm),(char*)&g_EntryDelay_parm,session) ;
			break;

		case CM_PANIC:
			panic();
			break;
		case CM_GWSTATE:
			sendCmdtoClient(CM_GWSTATE, 0, 1, sizeof(int), &g_armstate);
			printf("Gateway state now %d\n", g_armstate);
			break;

		case CM_SETLINK:
			//if (hdr.count > 1)
			client_setlink(hdr.count, inbuf);
			break;
		case CM_GETLINKMENU:
		    client_linkmenu(session);
			break;

		case CM_SETLINKMENU:
			set_linkmenu(inbuf);
			break;

		case CM_GETSOURCELINK:
			client_sourcelink(inbuf, hdr.payload_length, session);
			break;
		case CM_SETARMTABLE:
			setarmtable(hdr.count, inbuf);
			break;
		case CM_GETARMTABLE:
			// Jeff define to change get command resp to uni-cast
/*
			if (devcount > 0)
			{
				int payload_len =  sizeof(jswdev) * devcount;
				sendCmdtoClient(CM_DEV, 0,  devcount,  payload_len,(char*) &devlist) ;

			}else
				sendCmdtoClient(CM_DEV, 3,  0,  0, NULL) ;
*/
			if (devcount > 0)
			{
				int payload_len =  sizeof(jswdev) * devcount;
				sendCmdtoClient2(CM_DEV, 0,	devcount,  payload_len,(char*) &devlist,session) ;

			}else
				sendCmdtoClient2(CM_DEV, 3,	0,	0, NULL,session) ;

			break;

		case CM_PASETAUTOOFF:
			setPAofftime( inbuf);
			break;
		case CM_PAGETAUTOOFF:
			getPAofftime(inbuf );
			break;
		case CM_PAON:
			setPAon(inbuf, 1);
			break;
		case CM_PAOFF:
			setPAon(inbuf, 0);
			break;
		case CM_SIRENON:
			setSiren(inbuf,	1);
			break;
		case CM_SIRENOFF:
			setSiren(inbuf,	0);
			break;

		case CM_ADDSCHEDULE:
			setsched(inbuf);
			break;
		case CM_DELETESCHEDULE:
			delsched(inbuf);
			break;
		case CM_GETSCHEDULE:
			getsched(inbuf);
			break;

		case CM_SETPUSH:
			setpush(inbuf);
            //pushdata.smson = 0; //disable sms notification
			break;
		case CM_SET_MAG_ON_BEEP:
			setOthers(inbuf);
			break;
		case CM_GETPUSH:
			getpush();
			break;
		case CM_STARTZONE:
			startzone(inbuf, CMD_ADAPTER_TURNON);
			break;
		case CM_STOPZONE:
			startzone(inbuf, CMD_ADAPTER_TURNOFF);
			break;
		case CM_SETZONE:
			setzone(inbuf);
			break;
		case CM_GETALLZONE:
			getzone(CM_GETALLZONE, 0, session);
			break;
			///////////
		case CM_GETTEMP:
			gettemperature();
			break;
		case CM_FACTORYRESET:
			factoryReset(0);
			system("reboot");
			exit(1);
			break;
		case CM_FACTORYRESET2:
			factoryReset2();
			break;
		case CM_GETTIMEZONE:
			// Jeff define to change get command resp to uni-cast
//			sendCmdtoClient(CM_GETTIMEZONE, 0,  1,  sizeof(short),(char*) &g_setting.timezone) ;
			sendCmdtoClient2(CM_GETTIMEZONE, 0,  1,  sizeof(short),(char*) &g_setting.timezone,session) ;
			break;
		case CM_SETTIMEZONE:
			if (hdr.payload_length == sizeof(short))
			{
				memcpy(&g_setting.timezone, inbuf, sizeof(short));
				g_setting.gwprop.timezonechanged = 1;
				savesetting(); //need this ?
			}
			break;

		case CM_SETNETWORK:
			setnetwork(hdr.payload_length, inbuf);
			break;
		case CM_GETNETWORK:
			getnetwork();
			break;
		case CM_GETACCT:
			break;
		case CM_SETACCT:
			if ( hdr.payload_length == sizeof(struct _acct_client) )
			{
				updatepwd(inbuf, session);
			}
			break;
		case CM_GETGWPROP:
			getgatewayinfo(session);
			break;
		case CM_SETGWPROP:
			//if ( hdr.payload_length == sizeof(jswgwprop) )
			//{
			setgatewayinfo(inbuf);
            sendCmdtoClient(CM_SETGWPROP, 0, 1, sizeof(jswgwprop) , &g_setting.gwprop);
			//}else
			//	printf("Set Gateway properties size not match \n");
			break;
		case CM_SETSIRENPROP:
			//if ( hdr.payload_length == sizeof(struct sirenprop) )
			//{
			setsirenprop(inbuf);
			//}else
			//{
			//	printf("##CM_SETSIRENPROP ,client size %d, struct size %d\n\n", hdr.payload_length, sizeof(struct sirenprop));

			//}
			break;
		case CM_GETSIRENPROP:
			if ( hdr.payload_length == sizeof(int) )
			{
				getsirenprop(inbuf);
			}
			break;

		case CM_SERVICEMODE:

			if ( hdr.payload_length == sizeof(short) )
			{
				short mode;
				memcpy(&mode, inbuf, sizeof(short));
				if (mode == 1)
				{
					if (g_armstate == 0 && g_sirenstate ==0)
					{
						if (service_t !=0)
						{
							makeTimer(&service_t, 0, 0);
							service_t = 0;
						}
						makeTimer(&service_t, 60*60*1, 0);
						g_armstate = st_maintain;
						sirenmaintain(0);
						newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
						sendCmdtoClient(CM_GWSTATE, 0, 1, sizeof(int), &g_armstate);
					}
				}else if (mode == 0)
				{
					if (service_t !=0)
					{
						makeTimer(&service_t, 0, 0);
						service_t = 0;
					}
					g_armstate = st_disarm;
					sirenmaintain(1);
                    if(g_battlowcount <= 0)
                        if(getMAGOpenCount() <= 0)
                            ledoff(); //turn off trouble led
					newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
				}
				printf("set service mode Gateway state now %d\n", g_armstate);
				sendCmdtoClient(CM_GWSTATE, 0, 1, sizeof(int), &g_armstate);
			}

			break;
		case CM_TESTMODE:
			if ( hdr.payload_length == sizeof(short) )
			{
				short mode;
				memcpy(&mode, inbuf, sizeof(short));
				if (mode == 1)
				{//enter test mode
					if (g_armstate == 0 && g_sirenstate ==0)
					{
						newEvent(g_DID, TYPE_GATEWAY, st_testmode, 0);//g_armstate
						g_armstate = st_testmode;
						sendCmdtoClient(CM_GWSTATE, 0, 1, sizeof(int), &g_armstate);
                        makeTimer( &testmode_t, 60*5, 60*5);
					}
				}else if (mode == 0)
				{//leave test mode
					//g_armstate = st_disarm;
					//newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
					systemDisarm();
					timer_delete(testmode_t);
				}
				printf("set test mode Gateway state now %d\n", g_armstate);
				sendCmdtoClient(CM_GWSTATE, 0, 1, sizeof(int), &g_armstate);
			}
			break;

		case CM_SETENTRYDELAY:
			if ( hdr.payload_length == sizeof(short) )
			{//valid
				memcpy(&g_setting.gwprop.entrydelay , inbuf, sizeof(short));
				if (g_setting.gwprop.entrydelay  < 0)
					g_setting.gwprop.entrydelay  = 0;
				if (g_setting.gwprop.entrydelay > 60)
					g_setting.gwprop.entrydelay = 60;

				//printf("client set entrydelay %d\n",g_setting.gwprop.entrydelay );

				savesetting();
				sendCmdtoClient(CM_SETENTRYDELAY, 0, 1, sizeof(short), (char*)&g_setting.gwprop.entrydelay);
			}else
			{//invalid
				sendCmdtoClient(CM_SETENTRYDELAY, 1, 0, 0, NULL);
			}
			break;
		case CM_SETEXITDELAY:
			if ( hdr.payload_length == sizeof(short) )
			{//valid
				memcpy(&g_setting.gwprop.exitdelay , inbuf, sizeof(short));
				if (g_setting.gwprop.exitdelay  < 0)
					g_setting.gwprop.exitdelay  = 0;
				if (g_setting.gwprop.exitdelay > 60)
					g_setting.gwprop.exitdelay = 60;
//printf("client set EXIT Dely %d\n",g_setting.gwprop.exitdelay );
				savesetting();
				sendCmdtoClient(CM_SETEXITDELAY, 0, 1, sizeof(short), (char*)&g_setting.gwprop.exitdelay);
			}else
			{//invalid
				sendCmdtoClient2(CM_SETEXITDELAY, 1, 0, 0, NULL,session);
			}
			break;
		case CM_ADMIN: //client login to setting page
			if ( strcmp(inbuf , g_setting.acct.adminpwd) ==0 )
				sendCmdtoClient2(CM_ADMIN, 0,  0,  0, NULL, session);
			else
				sendCmdtoClient2(CM_ADMIN, 4,  0,  0, NULL, session);

			break;

		case CM_SETTIME:
			if ( hdr.payload_length == sizeof(int) )
			{
				struct timeval tv;
			    	struct timezone tz;
			    	if(gettimeofday(&tv, &tz) == 0)
			    	{
                    			memcpy(&tv.tv_sec , inbuf, sizeof(int));
                    			if(settimeofday(&tv, &tz) == 0)
                    			{
                    				printf("Align time %d successful!!!\n", tv.tv_sec);
                    			}
			    	}
			}
			break;
		case CM_GETTIME:
			t = time(0);
			// Jeff define to change get command resp to uni-cast
//			sendCmdtoClient(CM_GETTIME, 0,  1,  sizeof(time_t), &t) ;
			sendCmdtoClient2(CM_GETTIME, 0,  1,  sizeof(time_t), &t,session) ;
			break;

		case CM_ALARM_STATE:
            sendCmdtoClient(CM_ALARM_STATE, 0, 1, sizeof(int), &g_AlarmState);
            break;

		case CM_GET_EXTRASETTING:
			// Jeff define to change get command resp to uni-cast
            //sendCmdtoClient(CM_GET_EXTRASETTING, 0, 1, sizeof(ExtraSetting), &g_setting4);
			sendCmdtoClient2(CM_GET_EXTRASETTING, 0, 1, sizeof(ExtraSetting), &g_setting4,session);
            break;

		case CM_SET_EXTRASETTING:
			if ( hdr.payload_length == sizeof(ExtraSetting) )
			{
			    ExtraSetting *es = (ExtraSetting *)inbuf;
			    g_setting4.extra.DST_enable = es->DST_enable;
			    savesetting4();
			}
            break;



        //for FW update
		case CM_FW:
			if (g_armstate != st_disarm )
				sendCmdtoClient2(CM_FW, 4,  0, 0, NULL, session) ;
			else
			{
				startfwupdate(inbuf, session);
				freeOtherP2PSession(session);
				//sendCmdtoClient(CM_FW, 0,  0, 0, NULL) ;
			}
			break;
		case CM_FW_START:
			if (g_fwupgrade ==1)
				sendCmdtoClient2(CM_FW_START, 0,  0, 0, NULL, session);
			else
				sendCmdtoClient2(CM_FW_START, 4,  0, 0, NULL, session);

			break;
		case CM_FW_STOP:
			stopfwupdate(session);
			break;
		case CM_FW_DATA:
			savefw(hdr.payload_length, inbuf);
			break;



        //for Production Test
		case CM_MCU_VERSION:
			//writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_VERSION);

			//sendCmdtoClient(CM_MCU_VERSION, 0,  1, sizeof(int), (int)g_setting.gwprop.mcu_version);

			// Jeff define to change get command resp to uni-cast
//			sendCmdtoClient(CM_MCU_VERSION, 0,  1, sizeof(int), mcuversion);
			sendCmdtoClient2(CM_MCU_VERSION, 0,	1, sizeof(int), mcuversion,session);

			break;
		case CM_MCU_CUSTOMER:
			//writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_CUSTOMER);
			break;
		case CM_MCU_CHANNEL:
			//writeCommand(0, TYPE_MCU_COMMAND, CMD_GW_GET_CHANNEL);
			break;
		case CM_MCU_DEFAULT_SYNCWORD:
			//writeCommand(0, TYPE_MCU_COMMAND, CMD_GW_GET_DEFAULT_SYNCWORD);
			break;
		case CM_MCU_DID:
			// Jeff define to change get command resp to uni-cast
			//sendCmdtoClient(CM_MCU_DID, 0, 1, sizeof(int), &g_DID);
			sendCmdtoClient2(CM_MCU_DID, 0, 1, sizeof(int), &g_DID,session);
			break;
		case CM_TESTSD:
			sdtest(session);
			break;
		case CM_LED:
			ledtest();
			break;

		case CM_CAMERA_ON:
			startRec();
			break;
		case CM_CAMERA_SINGLE:
			break;

		case CM_RESETDEFAULT: //clear everything except jswitem (all device)
			factoryReset(1);
			break;
		case CM_QUICKPAIR:
			break;
		case CM_RFTX:
			rftxTest();

			break;
		case CM_WRITEDID:
			writegatewayinfo(inbuf);
			break;

		case CM_WRITEMAC:
			printf("write MAC %s\n", inbuf );
			if (strlen(inbuf) > 32 || strlen(inbuf) == 0 )
			{


				sendCmdtoClient(CM_WRITEMAC, 4, 0, 0, NULL);

			}else
			{
				r = changeMac(inbuf);
				if ( r == 0)
					sendCmdtoClient(CM_WRITEMAC, 4, 0, 0, NULL);
				else
				{
					sendCmdtoClient(CM_WRITEMAC, 0, 0, 0, NULL);
					memset(macaddr, 0, sizeof(macaddr));
					strcpy(macaddr, inbuf);
					savemac();
				}
			}

			break;

		case CM_WRITE_RTC:
			if ( hdr.payload_length == sizeof(int) )
			{
			    struct timeval tv;
			    struct timezone tz;
			    if(gettimeofday(&tv, &tz) == 0)
			    {
                    memcpy(&tv.tv_sec , inbuf, sizeof(int));
                    //tv.tv_sec -= (g_setting.timezone*60*60); //adjust for timezone
                    if(settimeofday(&tv, &tz) == 0)
                    {
                        system("hwclock -w");
                    }else
                    {
                        printf("Error: settimeofday() failed!!!\n");
                    }
			    }else
			    {
                    printf("Error: gettimeofday() failed!!!\n");
			    }
			}
            break;

		case CM_GET_BATT_LOW_LIST://get g_battlowlist
			sendCmdtoClient2(CM_GET_BATT_LOW_LIST, 0, g_battlowcount, sizeof(g_battlowlist), (char*) &g_battlowlist, session);
			break;
		case CM_DEL_ONE_BATT_LOW://del one list from g_battlowlist
			if ( hdr.payload_length == sizeof(jswbattlow) )
			{
			    jswbattlow bl;
                memcpy(&bl , inbuf, sizeof(jswbattlow));
                if(remove_battlow(bl.did) > 0)
                {//remove successfully
					sendCmdtoClient(CM_DEL_ONE_BATT_LOW, 0, 0, 0, NULL);
                }else
                {//remove failed
					sendCmdtoClient2(CM_DEL_ONE_BATT_LOW, 1, 0, 0, NULL,session);
                }
			}
			break;

		case CM_GET_PUSH_STR_LANG:
            sendCmdtoClient2(CM_GET_PUSH_STR_LANG, 0, 1, sizeof(int), &g_setting4.extra.push_string_language_id, session);
            break;
		case CM_SET_PUSH_STR_LANG:
			if ( hdr.payload_length == sizeof(int) )
			{//valid
			    memcpy(&g_setting4.extra.push_string_language_id, inbuf, sizeof(int));
                if( (g_setting4.extra.push_string_language_id < LANG_ID_GERMAN) || (g_setting4.extra.push_string_language_id >= LANG_ID_LAST) )
                    g_setting4.extra.push_string_language_id = DEF_PUSH_LANGUAGE;
			    savesetting4();
                sendCmdtoClient(CM_SET_PUSH_STR_LANG, 0, 1, sizeof(int), (char*)&g_setting4.extra.push_string_language_id);
			}else
			{//invalid
				// Jeff define to change get command resp to uni-cast
//                sendCmdtoClient(CM_SET_PUSH_STR_LANG, 1, 0, 0, NULL);
                sendCmdtoClient2(CM_SET_PUSH_STR_LANG, 1, 0, 0, NULL,session);
			}
            break;

		case CM_GET_PUSH_SELECTION1:
            sendCmdtoClient2(CM_GET_PUSH_SELECTION1, 0, 1, sizeof(int), &g_setting4.extra.push_selection_1, session);
            break;
		case CM_SET_PUSH_SELECTION1:
			if ( hdr.payload_length == sizeof(int) )
			{//valid
			    memcpy(&g_setting4.extra.push_selection_1, inbuf, sizeof(int));
			    savesetting4();
                sendCmdtoClient(CM_SET_PUSH_SELECTION1, 0, 1, sizeof(int), (char*)&g_setting4.extra.push_selection_1);
			}else
			{//invalid
                sendCmdtoClient2(CM_SET_PUSH_SELECTION1, 1, 0, 0, NULL,session);
			}
            break;

		case CM_ALARM_AS_DOORBELL:
            call_buzzer_doorbell();
            break;

		case CM_GET_CITY_LATLNG:
            sendCmdtoClient2(CM_GET_CITY_LATLNG, 0, 1, sizeof(jswlatlng), (char*)&g_setting4.latlng, session);
            break;
		case CM_SET_CITY_LATLNG:
			if ( hdr.payload_length == sizeof(jswlatlng) )
			{//valid
#if 1
				jswlatlng temp_jswlatlng ;
				memcpy(&temp_jswlatlng, inbuf, sizeof(jswlatlng));
				if ((g_setting4.latlng.lat == 0.0) && (g_setting4.latlng.lng == 0.0))
				{
					printf("---- ReadIn latlng ----, lat=%f, lng=%f!!!\n", g_setting4.latlng.lat, g_setting4.latlng.lng);
				if ( (temp_jswlatlng.lat < -90) || (temp_jswlatlng.lat > 90) ||
					(temp_jswlatlng.lng < -180) || (temp_jswlatlng.lng > 180) )
				{
					printf("Invalid latlng, lat=%f, lng=%f!!!\n", g_setting4.latlng.lat, g_setting4.latlng.lng);
					sendCmdtoClient2(CM_SET_CITY_LATLNG, 1, 0, 0, NULL,session);
					break;
				}
				else
					{
					int need_upgrade_info = 0;
					if(g_setting4.latlng.lat != temp_jswlatlng.lat)
						{
						g_setting4.latlng.lat = temp_jswlatlng.lat;
						need_upgrade_info = 1;
						}
					if(g_setting4.latlng.lng !=  temp_jswlatlng.lng)
						{
						g_setting4.latlng.lng =  temp_jswlatlng.lng;
						need_upgrade_info = 1;
						}
					if(g_setting4.latlng.DST != temp_jswlatlng.DST)
						{
						g_setting4.latlng.DST = temp_jswlatlng.DST;
						need_upgrade_info = 1;
						}
					if(g_setting4.latlng.woeid != temp_jswlatlng.woeid)
						{
						g_setting4.latlng.woeid = temp_jswlatlng.woeid;
						need_upgrade_info = 1;
						}
					if(g_setting4.latlng.timezone_offset != temp_jswlatlng.timezone_offset)
						{
						g_setting4.latlng.timezone_offset = temp_jswlatlng.timezone_offset;
						need_upgrade_info = 1;
						}
					if(need_upgrade_info )
						{
						savesetting4();
						sendCmdtoClient2(CM_SET_CITY_LATLNG, 0, 1, sizeof(jswlatlng), (char*)&g_setting4.latlng);
						memset(g_sunrisesunset, 0, sizeof(g_sunrisesunset));
						get_sunrise_sunset_data();
						}
					else
						{
						sendCmdtoClient2(CM_SET_CITY_LATLNG, 1, 0, 0, NULL,session);
						}

					}
				}
				else
				{
				sendCmdtoClient2(CM_SET_CITY_LATLNG, 1, 0, 0, NULL,session);
				}
#else
				if ((g_setting4.latlng.lat == 0) && (g_setting4.latlng.lng == 0))
				{
					printf("---- ReadIn latlng ----, lat=%f, lng=%f!!!\n", g_setting4.latlng.lat, g_setting4.latlng.lng);
					memcpy(&g_setting4.latlng, inbuf, sizeof(jswlatlng));
				}

				if ( (g_setting4.latlng.lat < -360) || (g_setting4.latlng.lat > 360) ||
					(g_setting4.latlng.lng < -360) || (g_setting4.latlng.lng > 360) )
				{
					printf("Invalid latlng, lat=%f, lng=%f!!!\n", g_setting4.latlng.lat, g_setting4.latlng.lng);
					break;
				}

				savesetting4();
				sendCmdtoClient2(CM_SET_CITY_LATLNG, 0, 1, sizeof(jswlatlng), (char*)&g_setting4.latlng);

				memset(g_sunrisesunset, 0, sizeof(g_sunrisesunset));
				get_sunrise_sunset_data();
#endif
			} else {//invalid
				sendCmdtoClient2(CM_SET_CITY_LATLNG, 1, 0, 0, NULL,session);
			}
			break;
		case CM_SET_REMOTE_ACCESS:
			if ( hdr.payload_length == sizeof(int) )
			{//valid
			    memcpy(&g_setting4.extra.remote_access, inbuf, sizeof(int));
			    savesetting4();
			    if(g_setting4.extra.remote_access <= 0)
			    {
			        g_setting4.extra.remote_access = 0;
                    g_setting4.extra.random_code = 0;
			    }
                sendCmdtoClient(CM_SET_REMOTE_ACCESS, 0, 1, sizeof(int), (char*)&g_setting4.extra.remote_access);

                if(g_setting4.extra.remote_access == 0)
                    log_remote_access("Disable Remote Access.");
                else
                    log_remote_access("Enable Remote Access.");
			}else
			{//invalid
                sendCmdtoClient(CM_SET_REMOTE_ACCESS, 1, 0, 0, NULL);
			}
            break;
		case CM_GENERATE_RANDOM_CODE:
		{
            srand(time(NULL)-73018262);
            int code = 0;
            code = rand() % 10000;
            while( (code < 1000) || (code >= 10000) )
            {
                code = rand() % 10000;
            }
            g_setting4.extra.random_code = code;
            sendCmdtoClient2(CM_GENERATE_RANDOM_CODE, 0, 1, sizeof(int), (char*)&g_setting4.extra.random_code, session);

            log_remote_access("Generate Random Code.");
		}
            break;

		case CM_TEST_PUSH_NOTIFICATION:
		{
            SetRunningLog_str_int("g_selftest_flag=", g_selftest_flag);
            if(g_selftest_flag > 0)
            {
                SetRunningLog_str("--Push Test Start----");
            }

		    char msg[256];
            struct tm *utctm;
            time_t now = time(NULL);
            if( (g_setting.timezone != 0) || (g_setting4.extra.DST_enable != 0) )
            {
                now += (g_setting.timezone*60*60);
                if(g_setting4.extra.DST_enable != 0)
                    now += (1*60*60);
            }
            utctm = gmtime(&now);
            sprintf(msg, "Push Notification test at %04d/%02d/%02d %02d:%02d:%02d.",
                utctm->tm_year+1900, utctm->tm_mon+1, utctm->tm_mday, utctm->tm_hour, utctm->tm_min, utctm->tm_sec);
            pushEvent(msg, strlen(msg));

            log_remote_access("Push Notification Test.");

            if(g_selftest_flag > 0)
            {
                SetRunningLog_str("--Push Test End----");
            }
		}
            break;

		case CM_GET_G_SENSOR_LEVEL:
            sendCmdtoClient2(CM_GET_G_SENSOR_LEVEL, 0, 1, sizeof(int), &g_setting4.extra.g_sensor_level, session);
            break;
		case CM_SET_G_SENSOR_LEVEL:
			if ( hdr.payload_length == sizeof(int) )
			{//valid
			    memcpy(&g_setting4.extra.g_sensor_level, inbuf, sizeof(int));
			    if(g_setting4.extra.g_sensor_level < JSW_GSENSOR_LOW)
                    g_setting4.extra.g_sensor_level = JSW_GSENSOR_LOW;
			    if(g_setting4.extra.g_sensor_level > JSW_GSENSOR_HIGH)
                    g_setting4.extra.g_sensor_level = JSW_GSENSOR_HIGH;
			    savesetting4();
                sendCmdtoClient(CM_SET_G_SENSOR_LEVEL, 0, 1, sizeof(int), (char*)&g_setting4.extra.g_sensor_level);
			}else
			{//invalid
                sendCmdtoClient(CM_SET_G_SENSOR_LEVEL, 1, 0, 0, NULL);
			}
            break;
		case CM_GET_G_SENSOR_ONOFF:
            sendCmdtoClient2(CM_GET_G_SENSOR_ONOFF, 0, 1, sizeof(int), &g_setting4.extra.g_sensor_onoff, session);
            break;
		case CM_SET_G_SENSOR_ONOFF:
			if ( hdr.payload_length == sizeof(int) )
			{//valid
			    memcpy(&g_setting4.extra.g_sensor_onoff, inbuf, sizeof(int));
			    if(g_setting4.extra.g_sensor_onoff < 0)
                    g_setting4.extra.g_sensor_onoff = 0;
			    if(g_setting4.extra.g_sensor_onoff > 1)
                    g_setting4.extra.g_sensor_onoff = 1;
			    savesetting4();
                sendCmdtoClient(CM_SET_G_SENSOR_ONOFF, 0, 1, sizeof(int), (char*)&g_setting4.extra.g_sensor_onoff);
			}else
			{//invalid
                sendCmdtoClient(CM_SET_G_SENSOR_ONOFF, 1, 0, 0, NULL);
			}
            break;
		case CM_GET_NEST_PARM:
			sendCmdtoClient2(CM_GET_NEST_PARM, 0, 1, sizeof(stNEST_parm), (char*)&g_stNEST_parm,session);
			break;
		case CM_SET_NEST_PARM:
			if ( hdr.payload_length == sizeof(stNEST_parm) )
			{//valid
				printf("CMD-CM_SET_NEST_PARM\n");
				memcpy(&g_stNEST_parm, inbuf, sizeof(stNEST_parm));
				save_Nest_parm();
				sendCmdtoClient(CM_SET_NEST_PARM, 0, 1, sizeof(stNEST_parm), (char*)&g_stNEST_parm);
				update_NEST_devicelist();
			}else
			{//invalid
				sendCmdtoClient(CM_SET_NEST_PARM, 1, 0, 0, NULL);
			}
			break;
		//apple
		case CM_ACTIVATE:
			{
				printf("CMD-CM_ACTIVATE size = %d (%d) B\n",hdr.payload_length,  sizeof(stUSER_parm));
			 	if ( hdr.payload_length == sizeof(stUSER_parm) )
		 		{
			 		printf("CMD-CM_ACTIVATE A \n");
					stUSER_parm* pSetParam =(stUSER_parm*) inbuf;
					//strcpy(g_stUserParam.szEntryId, "405N98GO-AM2ZSTPQ-FYU0V407");
									
					if ( strcmp( g_stUserParam.szEntryId, pSetParam->szEntryId ) == 0)
					{
						printf("CM_ACTIVATE EntryId the same \n");
						sendCmdtoClient(CM_ACTIVATE , 0, 1, 0, NULL);
					}
					else
					{
					 	printf("CM_ACTIVATE entryid = %s, Username = %s \n", pSetParam->szEntryId, pSetParam->szUsername);
						if(strlen(pSetParam->szEntryId) > 10  && strlen(pSetParam->szUsername) > 10 )
						{					
							memcpy(&g_stUserParam, pSetParam, sizeof(stUSER_parm));
							save_cloud_user_parm();
							startCloud();
							sendCmdtoClient(CM_ACTIVATE , 0, 1, sizeof(stUSER_parm), (char*)&g_stUserParam);
							printf("CM_ACTIVATE success \n");
						}
						else
						{
							printf("CM_ACTIVATE failed \n");
							sendCmdtoClient(CM_ACTIVATE , 1, 0, 0, NULL);
						}
					}
			    }
			 	else
				{
					printf("CM_ACTIVATE failed \n");
					sendCmdtoClient(CM_ACTIVATE , 1, 0, 0, NULL);
				}
			}
		break;				

        //for tool remote viewer
        case 9000://system command
            {
                char cmd2[64], cmd3[128];
                memset(cmd2, 0, sizeof(cmd2));
                memset(cmd3, 0, sizeof(cmd3));
                memcpy(cmd2, inbuf, sizeof(cmd2));

                if( (g_setting4.extra.remote_access > 0) && (g_setting4.extra.random_code > 0) )
                {
                    if(strcmp(cmd2, "reboot") == 0)
                    {
                        log_remote_access("Reboot system by remote side.");
                    }
                }

                sprintf(cmd3, "%s > ./outputtext", cmd2);
                system(cmd3);
            }
            break;
        case 9001://show file
            {
                char cmd2[1024], cmd3[256];
                memset(cmd2, 0, sizeof(cmd2));
                memset(cmd3, 0, sizeof(cmd3));
                memcpy(cmd2, inbuf, sizeof(cmd2));
                if(strstr(cmd2, "/") == NULL)
                    sprintf(cmd3, "./%s", cmd2);
                else
                    strcpy(cmd3, cmd2);
                FILE *fp;
                int bread = 0;
                struct stat buf2;
                if(stat(cmd3, &buf2) == -1)
                {
                    sendCmdtoClient2(9001, 0, 0, 0, 0, session);
                    break;
                }
                if(buf2.st_size <= 0)
                {
                    sendCmdtoClient2(9001, 0, 0, 0, 0, session);
                    break;
                }
                int len = buf2.st_size;
                //int len2 = len;

                unsigned short checksum2 = 0;
                int ii;
                unsigned char bb = 0;
                fp = fopen(cmd3, "rb");

                if (fp == NULL)
                {
                    sendCmdtoClient2(9001, 0, 0, 0, 0, session);
                    break;
                }
                memset(cmd2, 0, sizeof(cmd2));
                while(len > 0)
                {
                    memset(cmd2, 0, sizeof(cmd2));
                    bread = fread(cmd2, 1, sizeof(cmd2), fp);
                    if(bread > 0)
                    {
                        //calculate checksum
                        for(ii=0;ii<bread;ii++)
                        {
                            bb = cmd2[ii];
                            checksum2 += bb;
                        }

                        sendCmdtoClient2(9001, 0, 1, bread, cmd2, session);
                        usleep(10*1000);
                    }else
                        break;
                    len -= bread;
                }
                sendCmdtoClient2(9001, checksum2, 2, 0, 0, session);
                fclose(fp);
            }
            break;
        case 9002://put file
            {
                FILE *fp;
                static char putfile[256];
                static unsigned short checksum1 = 0;
                char cmd2[256];//, cmd3[256];

                switch(hdr.count)
                {
                case 0://start
                    memset(cmd2, 0, sizeof(cmd2));
                    memcpy(cmd2, inbuf, sizeof(cmd2));
                    sprintf(putfile, "./%s", cmd2);
                    sprintf(cmd2, "rm %s.rv", putfile);
                    system(cmd2);
                    sprintf(cmd2, "cp %s %s.rv", putfile, putfile);
                    system(cmd2);
                    sprintf(cmd2, "rm %s", putfile);
                    system(cmd2);
                    sync();
                    checksum1 = 0;
                    break;
                case 1://put file
                    fp = fopen(putfile, "ab");
                    if (fp == NULL)
                    {
                        break;
                    }
                    fwrite(inbuf, hdr.payload_length, 1, fp);
                    fclose(fp);

                    //calculate checksum
                    int ii;
                    unsigned char bb = 0;
                    for(ii=0;ii<hdr.payload_length;ii++)
                    {
                        bb = inbuf[ii];
                        checksum1 += bb;
                    }
                    break;
                case 2://end
                    sync();
                    if(checksum1 != hdr.errCode)
                    {//checksum error, delete file
                        printf("Error: checksum failed!!! checksum1=%d, checksum2=%d\n", checksum1, hdr.errCode);
                        char cmd22[256];
                        sprintf(cmd22, "rm ./%s", putfile);
                        system(cmd22);
                        usleep(100*1000);
                        sync();
                        //Send back failed
                        sendCmdtoClient2(9002, 1, 3, 0, 0, session);
                    }else
                    {
                        //printf("9002 end\n");
                        //Send back successful
                        sendCmdtoClient2(9002, 0, 3, 0, 0, session);
                    }
                    break;
                }
            }
            break;
        case 9003://send RF command
            {
                //ReportRF *rrf = (ReportRF*)inbuf;
                mcuCallBack(inbuf);
                usleep(1000);
            }
            break;
        case 9004://pairing fake sensor
            {
                int did = 0;
                memcpy(&did, &inbuf[0], sizeof(int));
                int type = 0;
                memcpy(&type, &inbuf[4], sizeof(int));
                g_isPairMode = 1;
                memset(&g_pairDevinfo, 0, sizeof(g_pairDevinfo));
                g_pairDevinfo.did = did;
                g_pairDevinfo.model = type;
                int time_now = time(NULL);
                char name2[256];
                sprintf(name2, "%d_%d", type, time_now);
                strcpy(g_pairDevinfo.name, name2);
                strcpy(g_pairDevinfo.location, "Office");
                newDevFromMcu(did, type);
            }
            break;
        case 9005://Get MCU version
            {
                writeCommand( 0, TYPE_MCU_COMMAND, CMD_GW_GET_VERSION);
                usleep(1000);
            }
            break;
        case 9006://Get running log
            {
                sendCmdtoClient2(9006, 0, 1, LEN_RUNNING_LOG, gRunningLog, session);
                usleep(1000);
            }
            break;
        case 9007://GW self-test, write in running log
            {
                SetRunningLog_str("Running: Start GW self-test");

                if(g_DID <= 0)
                    SetRunningLog_str_int("ERROR: invaild g_DID", g_DID);
                if(strlen(gDID) <= 0)
                    SetRunningLog_str_int("ERROR: invaild gDID length", strlen(gDID));
                if(gCurl == NULL)
                    SetRunningLog_str_int("ERROR: invaild gCurl", 0);
                if(p2p_thread == NULL)
                    SetRunningLog_str_int("ERROR: invaild p2p_thread", 0);
                if(hardwarereset_thread == NULL)
                    SetRunningLog_str_int("ERROR: invaild hardwarereset_thread", 0);
                if(search_bc_thread == NULL)
                    SetRunningLog_str_int("ERROR: invaild search_bc_thread", 0);
                if(ipc_thread == NULL)
                    SetRunningLog_str_int("ERROR: invaild ipc_thread", 0);
                if(threadID == NULL)
                    SetRunningLog_str_int("ERROR: invaild threadID", 0);

                SetRunningLog_str("Running: End GW self-test");
            }
            break;
        case 9008://switch self-test mode, write to running log
            {
                SetRunningLog_str_int("Running: self-test flag switch to", hdr.count);
                g_selftest_flag = hdr.count;
            }
            break;
        case 9009://reset running log
            {
                memset(gRunningLog, 0, LEN_RUNNING_LOG);
                SetRunningLog_str("Running: Reset running log");
            }
            break;

		default:
            SetRunningLog_str_int("Error: p2p_process_cmd() unknown command", hdr.cmd);
			break;

	}
	return 0;
}


unsigned int checksum(void *buffer, size_t len, unsigned int seed)
{
      unsigned char *buf = (unsigned char *)buffer;
      size_t i;

      for (i = 0; i < len; ++i)
            seed += (unsigned int)(*buf++);
      return seed;
}



void savefw(int len, char* buf)
{
	if (g_fwupgrade !=1)
		return;

	if (fpfileupdate == NULL)
	{
		printf("file update null \n");
		return;
	}

	memcpy(fwbuffer +  g_fwinfo.bufferlen, buf, len);
	g_fwinfo.bufferlen += len;

	if (g_fwinfo.bufferlen > 85000)
	{

		fwrite(fwbuffer, g_fwinfo.bufferlen, 1, fpfileupdate);
		g_fwinfo.bufferlen = 0;
	}

}

//#define FW_UPDATE_MAX_LENGTH    800000
//char newfw[FW_UPDATE_MAX_LENGTH];

void stopfwupdate(int session)
{
	char path[1024];
	char tool[1024];
	FILE *fd;
	unsigned int cs =0;
	if (g_fwupgrade !=1)
	{
        g_armstate = st_disarm;
        SetRunningLog_str_int("Error: stopfwupdate() not in FW upgrade", g_fwupgrade);
		return;
	}

	if (g_fwinfo.bufferlen > 0)
	{

		fwrite(fwbuffer, g_fwinfo.bufferlen, 1, fpfileupdate);
		g_fwinfo.bufferlen = 0;
	}
	fclose(fpfileupdate);

	system("sync");

	fpfileupdate = NULL;

	//checksum
	sprintf(path, "%sceresnew", sdpath);
	sprintf(tool, "%supdate &", sdpath);
	if ( 0 !=  access(path, 0) )
	{
		g_armstate = st_disarm;
		sendCmdtoClient2(CM_FW_STOP, 4,  0, 0, NULL, session);
		g_fwupgrade = 0;

        SetRunningLog_str("Error: stopfwupdate() no FW file found");
		return;

	}

	//allocate memory for fw update buffer
	struct stat buf2;
	if(stat(path, &buf2) == -1)
	{
        printf("Error, no FW update file! %s \n", path);
        SetRunningLog_str("Error: no FW update file!");
        //resume_fw_update(session);
        return;
	}
	if( (buf2.st_size < 400000) || (buf2.st_size > 4000000) )
	{
        printf("Error, invalid FW file size %d !\n", buf2.st_size);
        SetRunningLog_str_int("Error: invalid FW file size!", buf2.st_size);
        //resume_fw_update(session);
        return;//file size error
	}
	char *newfw = NULL;
	int alloc_size = buf2.st_size + 100000;
	newfw = malloc(alloc_size);
	if(!newfw)
	{
        printf("Error, allocate memory failed, alocate size %d !\n", alloc_size);
        SetRunningLog_str_int("Error: allocate memory failed, alocate size", alloc_size);
        //resume_fw_update(session);
        return;//malloc error
	}

	fd = fopen(path, "rb");
	if (fd == NULL)
	{
		printf("Update file not found\n");
		g_armstate = st_disarm;
        SetRunningLog_str("Error: stopfwupdate() open FW file failed");
        free(newfw);
		return;
	}
	int total = fread(newfw, 1, alloc_size, fd);
	fclose(fd);
	cs = checksum(newfw, total, cs);
	if (cs != g_fwinfo.checksum )
	{
		g_armstate = st_disarm;
		sendCmdtoClient2(CM_FW_STOP, 4,  0, 0, NULL, session);
		printf("\n\n### fw update checksum not correct %d, %d\n", g_fwinfo.checksum, checksum);
        SetRunningLog_str_int("Error: stopfwupdate() checksum failed", cs);
	}else
	{
		g_armstate = st_disarm;
		saveTmpEvdata(1);
		system("killall -9 gateway_wdt");
        previous_net_status = 2;
		system(LED_SETTING_P2P_NG2);
		sendCmdtoClient2(CM_FW_STOP, 0,  0, 0, NULL, session);
		usleep(5 * 1000);

		printf("\n### fw update complete \n\n\n");
		system(tool);
		printf("\n## ceres closing \n\n\n");

        if( (g_setting4.extra.remote_access > 0) && (g_setting4.extra.random_code > 0) )
        {
            log_remote_access("FW upgrade by remote side.");
        }

        free(newfw);
		exit(0);
	}

	g_fwupgrade = 0;
	g_armstate = st_disarm;

    free(newfw);
}

void startfwupdate(char *inbuf, int session)
{
	char path[1024];

	if (g_fwupgrade ==1)
	{
		fclose(fpfileupdate);
		fpfileupdate = NULL;

		printf("already in process fw update\n");
		//return;
	}
	memcpy( &g_fwinfo, inbuf, sizeof(struct fwupdateinfo));
	g_fwinfo.bufferlen = 0;

	memset(path, 0, sizeof(path));
	printf("fw update info %d, %d\n", g_fwinfo.version, g_fwinfo.checksum);
	g_armstate = st_maintain;

	sprintf(path, "%sceresnew", sdpath);
	if ( 0 ==  access(path, 0) )
		remove(path);

	fpfileupdate = fopen(path, "wb+");
	if ( fpfileupdate ==NULL)
	{
        g_armstate = st_disarm;
		sendCmdtoClient2(CM_FW,4,  0, 0, NULL, session);
		return;
	}
	g_fwupgrade = 1;
	sendCmdtoClient2(CM_FW, 0,  0, 0, NULL, session);

}

char g_entrydelay_msg[4000];

static void timerHandler( int sig, siginfo_t *si, void *uc )
{
    timer_t *tidp;
	//DBG_PRINT("####TIMER CALL\n");
    tidp = si->si_value.sival_ptr;

	if (*tidp == 0 )
	{
		//DBG_PRINT("#timerHandler Null value\n");
		return;

	}
    if ( *tidp == pairing_t )
	{
		DBG_PRINT("####pairing time out is_inpairmode %d pair_t %d\n",g_isPairMode, pairing_t );
		if (g_isPairMode ==1)
		{
			g_isPairMode =0;
			if ( pairing_t != 0)
				makeTimer( &pairing_t, 0, 0);
			pairing_t = pairing_current_t =0;

			sendCmdtoClient(CM_DISCARDPAIRING, 4,  1,  sizeof(jswdev),(char*) &g_pairDevinfo) ;
			memset(&g_pairDevinfo, 0, sizeof(jswdev));

			if ( uartbusy ==1)
				uartbusy = 0;
			writePairing(g_DID, 0, 0xFF);
		}


	}else if ( *tidp == gettemp_t )
	{
		DBG_PRINT("####Gettemp\n " );
		g_get_temp++;
//printf("g_get_temp=%d\n", g_get_temp);
		if(g_get_temp >= 6) //4
		{//three times no receive temp, reboot
		    log_and_reboot_no_beep("Reboot by missing temp 5 times", 1);
		}

		g_get_temp2++;
printf("g_get_temp2=%d\n", g_get_temp2);
		if(g_get_temp2 >= 8) //6
		{//5 times no timeout of p2p listen, reboot
		    log_and_reboot_no_beep("Reboot by missing p2p timeout 7 times", 1);
		}

        //if(txCount <= 0)
        //{//get temp when uart not busy
            writeCommand( temp_did, TYPE_ONBOARD_TEMP, CMD_ONBOARD_TEMP_GETALL);
        //}
	}else if ( *tidp == exitdelay_t )
	{
		DBG_PRINT("####exitdelay time up (%ld)\n ",(long) exitdelay_t);
		exitdelay_t = 0;
		systemArm();
	}else if ( *tidp == entrydelay_t )
	{
		DBG_PRINT("####entry time up \n " );
		g_armstate = entrydelayState; //revert back to current state.

		if(strlen(g_entrydelay_msg) > 0)
            pushnotification(NT_TRIGGER, g_entrydelay_msg, STR_ID_ALARM);//trigger

		triggerAlarm(0, 0);
		entrydelay_t = 0;
	}else if ( *tidp == writeack_t )
	{
		if (g_isPairMode ==0) //in pairing. stop other command.
		 checkAckfromWrite();

		//printf("!!!! tidp == writeack_t !!!!!\n");
	}
	else if (*tidp == schedule_t)
	{
		arrangeSchedule();

	}else if (*tidp == keepalive_t)
	{
		checkkeepalive();
	}else if ( *tidp == rftx_t )
	{
		rftx_t = 0;
		writeCommand(0x12345678, TYPE_POWER_ADAPTER, CMD_ADAPTER_TURNON);

	//usleep(1000);

		makeTimer(&rftest_t, 2, 0);


	}else if (*tidp == siren_t)//siren time up
	{
		printf("siren timeout \n");
		g_sirenstate = 0;
		siren_t = 0;
		g_smokeOn = 0;
		#ifndef MELODY_IC
		//ledoff();
		pthread_create(&alarmstop_th, NULL, stopgatewayalarm_thread, 0);
		pthread_join(alarmstop_th, NULL);
		//sendCmdtoClient(CM_SIREN_OFF, 0, 0,  0,NULL);
		#endif
	}else if (*tidp == service_t )
	{
		//shutdown service mode.
		if ( g_armstate == st_maintain)
		{
			service_t = 0;
			g_armstate = st_disarm;
			sirenmaintain(1);
			ledoff(); //turn off trouble led
			newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
		}
	}
	else if (*tidp == smoke_t)//siren time up
	{
		g_smokeOn = 0;
		g_smokeOnLastTime = 0;
	}else if (*tidp == ipcreconnect_t)
	{
		ipc_reconnect();
	}else if (*tidp == acctreset_t)
	{
		//checkfactoryreset();
	}else if (*tidp == rftest_t)
	{
			RFModuleWrite( g_DID, 0, 0xFF);
	////if(PLATFORM == "SN98601")		//SN98601's UART is queer that need write 2 times....
	#ifndef SERIAL_RAW
			RFModuleWrite( g_DID, 0, 0xFF);
	#endif
			uartbusy = 0;

			//makeTimer( &writeack_t, 4, 2);//renable this
	}else if (*tidp == testmode_t)
	{
        timer_delete(testmode_t);
        systemDisarm();
        sendCmdtoClient(CM_GWSTATE, 0, 1, sizeof(int), &g_armstate);
	}
	else if (*tidp == NEST_device_update_t)
	{
		if(NEST_device_update == 1)
		{
			NEST_device_update = 0;
			save_Nest_parm();
			sendCmdtoClient(CM_SET_NEST_PARM, 0, 1, sizeof(stNEST_parm), (char*)&g_stNEST_parm);
		}

	}
	else //all others
	{
		dopaAutooff(*tidp);
	}

}


int killTimer(timer_t timerID)
{
	int ret = 0;
	if(timerID == 0)
		return -2;
	ret = timer_delete(timerID);
	printf("killTimer=%d\n",ret);
	return ret;
}

int makeTimer( timer_t *timerID, int expire_sec, int interval_sec )
	{
		struct sigevent te;
		struct itimerspec its;
		struct sigaction sa;
		int sigNo = SIGUSR1;

		/* Set up signal handler. */
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = timerHandler;
		sigemptyset(&sa.sa_mask);
		if (sigaction(sigNo, &sa, NULL) == -1) {
		  //  perror("sigaction");
			DBG_PRINT("##sigaction fail\n");
		}
		// Dummy timer
		if( expire_sec == 0 && interval_sec == 0)
		{
			te.sigev_notify = SIGEV_NONE;
			te.sigev_signo = sigNo;
			te.sigev_value.sival_ptr = timerID;
			timer_create(CLOCK_REALTIME, &te, timerID);

			memset(&its, 0, sizeof( struct itimerspec));
			its.it_interval.tv_sec = 0; //0;
			its.it_interval.tv_nsec = 0; //intervalMS * 1000000;
			its.it_value.tv_sec = 0;
			its.it_value.tv_nsec = 0; // expireMS * 1000000;

			timer_settime(*timerID, 0, &its, NULL);
		}
		else // Add New timer
		{
			/* Set and enable alarm */

			te.sigev_notify = SIGEV_SIGNAL;
			te.sigev_signo = sigNo;
			te.sigev_value.sival_ptr = timerID;
			timer_create(CLOCK_REALTIME, &te, timerID);

			memset(&its, 0, sizeof( struct itimerspec));
			its.it_interval.tv_sec = interval_sec; //0;
			its.it_interval.tv_nsec = 0; //intervalMS * 1000000;
			its.it_value.tv_sec = expire_sec;
			its.it_value.tv_nsec = 0; // expireMS * 1000000;

			timer_settime(*timerID, 0, &its, NULL);
		//DBG_PRINT("##timer_settime %d \n" , *timerID);
			return 1;
			}
	}


