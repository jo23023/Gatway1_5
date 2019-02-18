/*
 * ceres.h
 *
 *  Created on: Feb 28, 2014
 *      Author: yulin724
 */

#ifndef CERES_H_
#define CERES_H_

//#include "lua.h"
//#include "lualib.h"
//#include "lauxlib.h"

//#include "mosquitto.h"

#define DEBUG 1
#include "PPPP/PPPP_API.h"

#include "nest_realtime.h"


//#include "cJSON.h"
//#include "printbuf.h"

#define SEARCH_BROADCAST_ENABLE 1
#define SCHEDULE_ENABLE 1
#define AUTOOFF_SCHEDULE_ENABLE 1
#define SENSOR_ALIVE_SCHEDULE_ENABLE 1
#define TEMPERATURE_SCHEDULE_ENABLE 1
#define AES_ENABLE 1
#define STARX_DEBUG 1

#define CERES_VERSION "0.8"

//#define MQTT_HOST     "127.0.0.1"
//#define MQTT_PORT   1883
//#define CLIENTID    "itemrepo"
//#define TOPIC       "itemrepo/#"
//#define QOS         2
#define TIMEOUT     10000L
#define MAX_P2P_BUFFER (16*1024)

#define MD5_LEN 32

//#define CERES_UTIL

#ifdef CERES_UTIL
#define TOPIC_GCM_PUSHEVENT     "gcm_pushevent"
#define TOPIC_GCM_REGISTER      "gcm_register"
#define TOPIC_SEND_EMIAL        "send_email"
#define TOPIC_CHECK_CONNECT     "check_connect"

//// ceres 发送指令给ipc_handler
//#define TOPIC_IPC_HANDLER_ADD         "ipc_handler_add"
//#define TOPIC_IPC_HANDLER_REMOVE      "ipc_handler_remove"
//#define TOPIC_IPC_HANDLER_RECORD      "ipc_handler_record"
//#define TOPIC_IPC_HANDLER_RECORD_STOP "ipc_handler_record_stop"
//
//// ipc_handler 发送指令给 ceres
//#define TOPIC_IPC_HANDLER_MOTION      "ipc_handler_motion"
//#define TOPIC_SEND_EMAIL_NOTIFICATION     "send_email_notification"
#endif

#define TOPIC_SMS_PUSHEVENT     "sms_pushevent"

//#define TOPIC_START_CAM_RECORD            "start_cam_record"
//#define TOPIC_STOP_CAM_RECORD             "start_cam_record"

#define LEN_RUNNING_LOG 1800

#define DEF_SECURITY_CODE       "123456"

//P2P Definition
typedef enum {
    //// Command Message
    CMD_AUTH = 0x01,
    CMD_AUTH_RESP = 0x02,
    CMD_AUTH_REQ = 0x03,
    CMD_AUTH_OK = 0x04,
    CMD_AUTH_NOTOK = 0x05,
    CMD_NEXTACTION = 0x06,
    CMD_STATUS = 0x07,
    CMD_COMMAND = 0x11,
    CMD_DEVICEJSONTREE = 0x12,

    CMD_AESKEY = 0x13,  // chenjian
    CMD_AESKEY_RSP = 0x14,

    CMD_UPGRADE_REQ = 0x15,
    CMD_UPGRADE_RSP = 0X16,
    CMD_UPGRADE_DOWNLOAD_COMPLETED = 0x17,
    CMD_UPGRADE_DOWNLOAD_ERROR = 0x18,
    CMD_UPGRADE_START = 0x19,
    CMD_UPGRADE_START_RSP = 0x1A,   /* 20150305_victorwu: notify mcu and app for update */

    //// Data Message
    PROTO_DATA = 0x41
} ENUM_P2PControl_Proto_Type;

typedef struct {
    UINT32 info; //// ((info & 0xFF000000) >> 24) --> ENUM_P2PControl_Proto_Type
//// (info & 0xFFFFFF) --> DataSize;
} st_Control_Proto_Header;

//// CMD_AUTH=0x01,   (AES encrypted)
typedef struct {
    CHAR Cypher[16];    //// AES encrypted of "0123456789ABCDEF" by 'Key'
} st_Control_Proto_Auth;

//// CMD_AUTH_RESP=0x02,  (NOT AES encrypted)
typedef struct {
    CHAR AuthResult;    //// 0: Success, -1: Auth Failed!!
    CHAR Reserved[3];
} st_Control_Proto_Auth_Resp;


typedef struct
{
    CHAR  valid;                // 0: in-valid, 1:valid
    INT32 session;              // session of p2p connect
    unsigned char aeskey[16];   // aes key
    int use_AES_data;           //1:use AES data, 0:plain data
    int connect_time;           //connected time, for kick first one if connection full
    int reserve[16];            //reserve
}st_p2p_session_record;

#define P2P_SESSION_MAX 8
pthread_mutex_t p2psession_mutex;


//#define COMMAND_BUF_LEN 4096

// core.lua runtime thread

pthread_mutex_t aes_mutex;
//pthread_mutex_t event_session_mutex;

pthread_t gcm_thread;
pthread_t email_thread;
//pthread_mutex_t gcm_mutex;
//pthread_mutex_t email_mutex;


// p2p runtime thread
pthread_t p2p_thread;
void *p2p_sub_thread_result;


//hardware reset detect thread
pthread_t hardwarereset_thread;

//ping hinet thread
pthread_t ping_hinet_thread;

// RF_resp_check_therad
pthread_t RF_resp_check_therad;



// response mobile's search DID broadcast
#ifdef SEARCH_BROADCAST_ENABLE
pthread_t search_bc_thread;
void *search_bc_sub_thread_result;

#define SEARCH_BROADCAST_GATEWAY_PORT       12000
#define SEARCH_BROADCAST_ANDROID_PORT       13000
#define SEARCH_BROADCAST_IPV4           1
#define SEARCH_BROADCAST_IPV6           0
#endif

#ifdef SCHEDULE_ENABLE
pthread_t schedule_thread;
void *schedule_thread_result;
static volatile int schedule_running = 1;
#endif

#ifdef AUTOOFF_SCHEDULE_ENABLE
pthread_t autooff_schedule_thread;
void *autooff_schedule_thread_result;
static volatile int autooff_schedule_running = 1;
#endif

#ifdef SENSOR_ALIVE_SCHEDULE_ENABLE
pthread_t sensor_alive_schedule_thread;
void *sensor_alive_schedule_thread_result;
static volatile int sensor_alive_schedule_running = 1;
#endif

//char get_mcu_version = 0;


//#define IPC_HANDLER_ENABLE            1
pthread_t ipc_thread;
#ifdef IPC_HANDLER_ENABLE

#endif


// 用于ARM延时的线程
pthread_t delay_thread;

// 用于 reset 操作延时的线程
pthread_t reset_thread;

//#define LOG_FILE  1

#ifdef DEBUG
    #define DBG_PRINT(_STR_, args...) printf("[ Ceres   ] "_STR_, ##args);
#else
    #define DBG_PRINT(_STR_, args...) ;
#endif


// Thread of sending mqtt message of register gcm server
pthread_t gcm_reg_thread;
extern int gcm_reg_runung; // = 1;

extern st_p2p_session_record gP2PSession[P2P_SESSION_MAX];

//int globalSession = -1;

#define CH_CTRL     0
#define CH_DAT      2
#define CH_FILE     3

int makeTimer( timer_t *timerID, int expireMS, int intervalMS );
int killTimer(timer_t timerID);

void SetRunningLog_str(char *str);
void SetRunningLog_str_int(char *str, int ii);
char *load_NEST_Smoke_name(smoke_data_t *this_smoke_data);
char *load_NEST_Thermo_name(thermo_data_t *this_thermo_data);
unsigned int get_NEST_Smoke_index(smoke_data_t *this_smoke_data);
char *strupr(char *str);


#endif /* CERES_H_ */
