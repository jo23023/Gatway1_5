#ifndef		__JSWPROTO_H__
#define 	__JSWPROTO_H__

#include "ceres.h"
#include <time.h>

//Feature Macro
#define DEF_FEATURE_MULTI_PUSH  1 //if defined, gateway will send out multi-language push string, or default push string

#define GATEWAY_VERSION 174

#define DBVERSION 0x10
#define MAXDEVICENUM 48

#define MAXEVENT 100
#define MAXEVENTCOUNT 10000
#define MAXEVENTFILES (MAXEVENTCOUNT/MAXEVENT)
#define MAXTMPEVENT 10
#define TMPEVENT_NAME   "tmpevent"
#define TMPEVENT_NAME2  "tmpevent2"
#define MAXRETURNEVENT  400
#define MCU_REBOOT_FILE "./mcureboot"
#define REMOTE_ACCESS_LOG_FILE "./RemoteAccess.log"

#define WATCH_TEMP_FILENAME "./watchtemp"
//#define ARM_ABNORMAL "(Sensor is abnormal!!)"
#define ARM_ABNORMAL " But you may have a few doors / windows open."
#define ARM_ABNORMAL2 "Setting %s Arm failed because the door is not closed."
#define CAT_WATCH_TEMP_DUR  7200 //120 mins

#define PRO_VERSION 10
#define MAXIPCAM 32

#define UART_WRITE_DELAY 10000
#define SERIAL_RAW
#define READ_BUF_SIZE 32

// Jeff define 
#define FORCE_UPDATE_SIREN_STATE 

#define MAX_NEST_DEVICE 10

#define SIREN_TIMER
#define MELODY_IC

enum CUSTOMER_CODE{
	CUSTOMER_UNKNOW,
	CUSTOMER_JSW,
	CUSTOMER_ABUS,
	CUSTOMER_ALC,
	CUSTOMER_LOSADA,
	CUSTOMER_HomeON,
	CUSTOMER_RAYLOIOS,
	CUSTOMER_INDEXA,
	CUSTOMER_JunLian,
	CUSTOMER_PWS,
	CUSTOMER_Nedis,
	CUSTOMER_SecuFirst,
	CUSTOMER_Uniden_USA,
	CUSTOMER_Uniden_AUS,
	CUSTOMER_Uniden_JP,
	CUSTOMER_Y3K,
	CUSTOMER_Konelco,
	CUSTOMER_Extel,
	CUSTOMER_Smartwares,
	CUSTOMER_Optex,
	CUSTOMER_ARCHTRON,
	CUSTOMER_JimsSecurity,
	CUSTOMER_NewDeal,
	CUSTOMER_MAXWELL,
	CUSTOMER_ALCBV,
	CUSTOMER_SYSLINK,
};

int Customer_code;

//
//#define DID_TEMP 0xC8
//#define DID_HUMI 0xC9
#if 0
#define STRING_TRIGGER_ALARM            "Your component %s %s of your Smartvest system %s has triggered the Alarm!"
#define STRING_TRIGGER_TAMPER_ALARM     "Your component %s %s of your Smartvest system %s has triggered the Tamper-Alarm!"
#define STRING_TRIGGER_BATTLOW_ALARM    "Your component %s %s of your Smartvest system %s is running low on battery!"
#define STRING_TRIGGER_NO_SIGNAL        "Your component %s %s of your Smartvest system %s has no signal."
#else
#define STRING_TRIGGER_ALARM            "Your component %s %s of your SHC_pro system %s has triggered the Alarm!"
#define STRING_TRIGGER_TAMPER_ALARM     "Your component %s %s of your SHC_pro system %s has triggered the Tamper-Alarm!"
#define STRING_TRIGGER_BATTLOW_ALARM    "Your component %s %s of your SHC_pro system %s is running low on battery!"
#define STRING_TRIGGER_NO_SIGNAL        "Your component %s %s of your SHC_pro system %s has no signal."

#endif

#define DEF_SUPERVISION_TIMER_DUR       4 //the supervision timer duration, in hour

#define DEF_CURL_OUTPUT_BUFFER_LEN  1024
#define DEF_BEEP_SOURCE_SPEAKER         2 //0:Speaker, 1:Buzzer, 2:Melody IC

extern CHAR gDID[32];
extern char mcuversion[4];
extern int g_smokeOn ;
extern char macaddr[32];
extern char get_mcu_version;

enum NOTIFY_TYPE{
	NT_BATT_LOW,
	NT_ARM,
	NT_DISARM,
	NT_PARTARM,
	NT_TRIGGER,
	NT_TAMPER,
	NT_PANIC,
	NT_NO_SIGNAL

};
enum ALARM_STATE{
	ALARM_STATE_NO_ALARM,
	ALARM_STATE_TRIGGER,
	ALARM_STATE_TAMPER,
	ALARM_STATE_PANIC,
	ALARM_STATE_24H_ALARM,
};
enum CMD_LIST{
	CM_AUTH = 0x6,
	CM_AUTH_OK,
	CM_AUTH_FAIL,
	CM_DEV,
	CM_NEWDEVICE, //10
	CM_DISCARDPAIRING,
	CM_PAIRINGEXIST,
	CM_EDITDEV,
	CM_DELETEDEV,

	CM_SENSORUPDATE, //15
	CM_GETEVENT,
	CM_CLEAREVENT,

	CM_SETLINK, //18
	CM_GETLINKMENU,
	CM_SETLINKMENU,//20
	CM_GETSOURCELINK,
	CM_SETARMTABLE,
	CM_GETARMTABLE,

	CM_ARM,//24
	CM_DISARM,
	CM_PARTARM,
	CM_EXITDELAY,
	CM_ENTRYDELAY,
	CM_DOOROPEN,
	CM_PANIC, //30
	CM_SIREN_ON,
	CM_SIREN_OFF,
	CM_GWSTATE,
	CM_TESTMODE,
	CM_SERVICEMODE,

	CM_STARTZONE, //36
	CM_STOPZONE,
	CM_SETZONE,
	CM_GETALLZONE,

	CM_PASETAUTOOFF, //40
	CM_PAGETAUTOOFF,
	CM_PAON, //42
	CM_PAOFF,
	CM_SIRENON,
	CM_SIRENOFF,

	CM_ADDSCHEDULE, //46
	CM_DELETESCHEDULE,
	CM_GETSCHEDULE, //48

	CM_SETPUSH,
	CM_GETPUSH,//50

	CM_FACTORYRESET,
	CM_GETTEMP,
	CM_SETTEMP, //53

	CM_GETTIMEZONE,
	CM_SETTIMEZONE, //55

	CM_SETNETWORK,
	CM_GETNETWORK,
	CM_GETACCT, //58
	CM_SETACCT,
	CM_GETGWPROP, //60
	CM_SETGWPROP,

	CM_SETSIRENPROP,
	CM_GETSIRENPROP,

	CM_SETENTRYDELAY,
	CM_SETEXITDELAY, //65

	CM_ADMIN,

	CM_SETTIME,
	CM_GETTIME,

	CM_FW, //69
	CM_FW_START,
	CM_FW_STOP,
	CM_FW_DATA,

	//production test
	CM_MCU_VERSION,
	CM_MCU_CUSTOMER,
	CM_MCU_CHANNEL	,//75
	CM_MCU_DEFAULT_SYNCWORD,
	CM_MCU_DID,
	CM_TESTSD,
	CM_LED, //79

	CM_CAMERA_ON,
	CM_CAMERA_SINGLE,

	CM_RESETDEFAULT,
	CM_QUICKPAIR,
	CM_RFTX,
	CM_WRITEDID,
	CM_WRITEMAC,

	CM_AUTH_ENCRYPT,//87
	CM_AUTH_OK_ENCRYPT,
	CM_DATA_ENCRYPT,

    CM_ALARM_STATE,//90
    CM_WRITE_RTC,
	CM_FACTORYRESET2,
	CM_SEARCHEVENT,

	CM_GET_EXTRASETTING,//94
	CM_SET_EXTRASETTING,

	CM_GET_BATT_LOW_LIST,//96
	CM_DEL_ONE_BATT_LOW,

	CM_GET_PUSH_STR_LANG,//98
	CM_SET_PUSH_STR_LANG,

	CM_GET_PUSH_SELECTION1,//100
	CM_SET_PUSH_SELECTION1,

	CM_ALARM_AS_DOORBELL,//102

	CM_GET_CITY_LATLNG,//103, city, latitude, longitude
	CM_SET_CITY_LATLNG,

	CM_SET_REMOTE_ACCESS, //105, set Remote Access, 1:enable, 0:disable
	CM_GET_REMOTE_ACCESS,

	CM_GENERATE_RANDOM_CODE,

	CM_TEST_PUSH_NOTIFICATION, //108

	CM_GET_G_SENSOR_LEVEL,//109
	CM_SET_G_SENSOR_LEVEL,
	CM_GET_G_SENSOR_ONOFF,
	CM_SET_G_SENSOR_ONOFF,

//	CM_STARTZONE2, //109 for Smartvest
//	CM_STOPZONE2,
//	CM_SETZONE2,
//	CM_GETALLZONE2,

	CM_PLAY_DOORCHIME,//113

	// Third-party API command set
	CM_GET_NEST_PARM = 0xB0,
	CM_SET_NEST_PARM, 

	//Cloud settings
	CM_ACTIVATE,
	CM_SET_MAG_ON_BEEP,
	CM_GET_MAG_ON_BEEP
};

typedef struct NEST_device
{
	int joined;
	int device_type;
	char device_ID[36];
	char device_name[36];
	char reserved[16];
}stNEST_device;


typedef struct NEST_PARM
{
	int nest_bind;
	char access_token[256];
	stNEST_device stNEST_device_list[MAX_NEST_DEVICE];
}stNEST_parm;



typedef struct ExitDelay_parm
{
	time_t avtive_time; // from 1970-01-01 00:00:00 sec , @ UTC time , if avtive_time == 0, or now > avtive_time means RIGHT NOW
	int toState ; // ref. to gateway_state
	char reserved[8];
} stExitDelay_parm;

typedef struct EntryDelay_parm
{
	time_t avtive_time; // from 1970-01-01 00:00:00 sec , @ UTC time , if avtive_time == 0, or now > avtive_time means RIGHT NOW
	unsigned int deviceID ; // ref. to jswdev
	char reserved[8];
} stEntryDelay_parm;



typedef struct _jswproto_hdr
{
	unsigned short version;
	unsigned short cmd;
	unsigned short payload_length;
	unsigned short count;
	unsigned short errCode;
	//unsigned short ext;
}jswhdr;

//payload
typedef struct _jswproto_devtree
{
	unsigned int did;

	char name[32];
	char location[32];
	char ipcdid[32];
	char ipcpwd[16];

	short model;
	unsigned short status;
	unsigned short flag;
	unsigned char ext1;
	unsigned char ext2;

}jswdev;

typedef struct _jswproto_devtree2
{
	unsigned int did;

	char name[16];
	char location[32];
	char ipcdid[32];
	char ipcpwd[16];

	short model;
	unsigned short status;
	unsigned short flag;
	//unsigned short ext1;
	// ext1: vibration sensitive 1:High 2:Middle 3:Low
	unsigned char ext1;
	unsigned char ext2;
}jswdev2;

typedef struct _jsw_event{
	unsigned int did;
	unsigned int time;
	int status;
	short model;
	short gwstate;
}jswevent;

typedef struct _jsw_alarm_event{
	unsigned int id;
	unsigned int time;
	int type;
	int status;
	char reserve[16];
}jsw_alarm_event;


typedef struct jsw_armtable{
	unsigned int did;
	short flag;
	short ext;
}jswarmtable;

typedef struct jsw_cloudevent
{
	unsigned int  	did;
	unsigned int 	time;
	int 		 	status;
	unsigned char	nType;
	char 		 	szName[100];
	
}jswcloudevent;

 typedef struct USER_PARM
{
  char szAPPId[32]; 	 //AppId 
  char szAPPToken[256];  //Token
  char szUsername[256];  //email
  char szEntryId[32];	 //entry id.gateway 
}stUSER_parm;

/*
"AppId" : "RWT3578Y-I2P8KZZH-HP54EJ9N",
"Token":"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJFbnRlcnByaXNlSWQiOiJGRFAwVDIxQi0yRERUVEw1Ty04TERQN1ZFMiJ9.G6O06dXCdJO75ydFov6paIEG91KnPQ-MFGbAA6tKaGE",
*/

#define SCENARIO_GATEWAY_DID        13572468
#define SCENARIO_SOURCE_SUNRISE     1
#define SCENARIO_SOURCE_SUNSET      2

#define SCENARIO_ONOFF				0x01
#define SCENARIO_CAMERA_RECORD		0x02
#define SCENARIO_CAMERA_PREVIEW		0x04
#define SCENARIO_CAMERA_PRESET_1	0x08
#define SCENARIO_CAMERA_PRESET_2	0x10
#define SCENARIO_CAMERA_PRESET_3	0x20
#define SCENARIO_GATEWAY_DOORBELL   0x40

#define SCENARIO_MAX_TARGET         16

#define SCENARIO_ONOFF_MAG_CLOSE                0x00
#define SCENARIO_ONOFF_MAG_OPEN                 0x01
#define SCENARIO_ONOFF_BUTTON_SHORT_PRESS       0x00
#define SCENARIO_ONOFF_BUTTON_LONG_PRESS        0x01
#define SCENARIO_ONOFF_CAMERA_MOTION_TRIGGER    0x00
#define SCENARIO_ONOFF_CAMERA_DOORKEY           0x01
#define SCENARIO_ONOFF_DOORLOCK_CLOSE               0x00
#define SCENARIO_ONOFF_DOORLOCK_CARD_UNLOCK         0x01
#define SCENARIO_ONOFF_DOORLOCK_FINGERPRINT_UNLOCK  0x02
#define SCENARIO_ONOFF_DOORLOCK_PASSWORD_UNLOCK     0x03
#define SCENARIO_ONOFF_DOORLOCK_KEY_UNLOCK          0x04


typedef struct jsw_linktarget{
	unsigned int did;
	int enable;//bit field
}jswlinktarget;


typedef struct jsw_link{
	unsigned int did;
	short enable; //scenario enable/disable
	short onoff; //scenario trigger state, ex. mag open/close, for sunrise/sunset, high byte meaning time shift, low byte meaning sunrise/sunset
	int target_num;
}jswlink;

//local
typedef struct jsw_scenario
{
	jswlink src;
	jswlinktarget target[SCENARIO_MAX_TARGET];

}jswscenario;

typedef struct devstatus{
	unsigned int did;
	jswevent evt;
};

#define MAXZONE 32
#define MAXSENSOR_ZONE 32
typedef struct _zone{
	char name[32];
	short num;
	short count;
}jswzone;

typedef struct _zonelist{
	jswzone zone;
	unsigned int didlist[MAXSENSOR_ZONE];
};



#define SCHEDULE_MAX_TARGET         16
typedef struct _schedule{
    unsigned int did;
	short num; //id into the list.
	short random;
	short week;
	short starttime;
	short endtime;
	short onoff;
}jswschedule;

struct pa_prop{
	unsigned int did;
	timer_t timerid;
	short duration;
	short elapse;
	short schedule_count;
	jswschedule schedlist[SCHEDULE_MAX_TARGET];
};
struct em{
	char email[64];
};


struct _eventinfo{
	int savepos; //0~9
	int count; //<= 10
	char filename[10][64]; //MAXEVENTFILES
};



extern struct generalinfo g_setting; //all misc setting.

typedef struct pushmsg{
	char key[64];
	char smsnumber[32];
	short smson;
	short route; //0, 1, 2 basic, gold, direct
	short pushon;
	short emailon;
	short emailcount;
}jswpush;


typedef struct _network{
	char ip[32];
	char mask[32];
	char gateway[32];
	char dns[32];
	short dhcp;
}jswnetwork;

typedef struct _acct{
	char loginpwd[32];
	char adminpwd[32];
}jswacct;

struct _acct_client{
	char oldlogin[32];
	char loginpwd[32];
	char oldadmin[32];
	char adminpwd[32];
};

#define JSW_VOLUME_HIGH     3
#define JSW_VOLUME_MEDIUM   2
#define JSW_VOLUME_LOW      1
#define JSW_VOLUME_MUTE     0

typedef struct _gwprop{
	char name[64];
	short alarmOn;//default 1. 0-3
	short version;//
	short ledon;
	short mcu_version;
	short entrydelay;
	short exitdelay;
	short duration;
	short timezonechanged;
}jswgwprop;

struct sirenprop{
	unsigned int did;
	short ledon; //0 = mute + LED_ON, 1: soundon + LED_OFF , 2: soundon + LED_ON
	short soundon;//0=low, 1 = high
	short duration; //seconds
	short r1; //0 = low , 1= high.
	short r2;
};



struct generalinfo{
	struct _eventinfo ev;
	jswacct acct;
	jswnetwork network;
	jswgwprop gwprop;
	short timezone;
	struct sirenprop sirenlist[10];
	short sirencount;
};

//for sunrise/sunset
typedef struct _lat_lng
{
    double lat;
    double lng;
    unsigned int woeid; //client use this member to get city name
    int timezone_offset; //timezone offset, in secs
    int DST; //daylight saving time
    int sync_local_enable;
    char wlan_ip[24];
    unsigned char reserve[40];
}jswlatlng; //sizeof()=96, sizeof(double)=8

#define JSW_GSENSOR_LOW     3
#define JSW_GSENSOR_MIDDLE  6
#define JSW_GSENSOR_HIGH    10

//for setting4info p2p usage
typedef struct _ExtraSetting{
	int armstate;
	int ShowAlarmView;
	int AlarmState;
	int DST_enable;//1:with DST, 0:without DST
	int push_string_language_id; //push string language id, mapping to PSUH_STRING_LANGUAGE_LIST, 1->german....
	int push_selection_1; //define which push type will be sent, group1
	int push_selection_2; //define which push type will be sent, group2
	int remote_access; //1: can remote access, 0: can't remote access, default is 0
	int random_code; //for remote access code, between 1000-9999, default is 0
	int g_sensor_level; // g-sensor level, 0-15, low=3, middle=6, high=10
	int g_sensor_onoff; // g-sensor on/off, 0:off, 1:on
	int reserve[100-9-2];
}ExtraSetting; //sizeof()=400

struct setting4info{
    ExtraSetting extra; //for p2p transmit
	jswlatlng latlng;
	int reserve[1024-99-25];
}; //sizeof()=4096

struct setting5info{
	char filename[100][64]; //event filename, MAXEVENTFILES
	int reserve[1024];
};


struct gatewayinfodb{
	char did[32];
	char license[32];
	char checksum[32];
};

typedef struct otherSetting{
	bool bBeepInMagOn;  //in Disarm mode
}jswOther;


typedef struct googleSetting{
	
	bool bEnableDA;  
	char szUerAccount[32];
	
}jswGoogleSetting;


extern CHAR gPassword[32];
extern jswpush pushdata;
extern struct em emaillist[6];

void setpush(char *buf);
void getpush();
void loadpush();
void savepush();

void addPAproperties(unsigned int id);
void savePAprop();
void loadPAprop();
void setPAofftime( char*buf);
void getPAofftime( char*buf);
void setPAon(char *buf, int mode);
void delsched(char *buf);
void setsched(char *buf);
void getsched(char *buf);
extern struct pa_prop pa_proplist[MAXDEVICENUM];
extern int paCount;
/////

extern jswdev devlist[MAXDEVICENUM];
extern int devcount;


extern jswevent evtlist[MAXEVENT];
extern int evtcount;
#define SCENARIOSIZE 64 //32
extern jswscenario scenariolist[SCENARIOSIZE];
extern int srcCount;

extern jsw_alarm_event last_trigger;

extern  struct _zonelist zonelist[MAXZONE];
extern int zoneCount;
extern stExitDelay_parm g_ExitDelay_parm;
extern stEntryDelay_parm g_EntryDelay_parm;

void startzone(char *buf, int mode);
void setzone(char *buf);
void getzone(int cmd, int broadcast, int session);
void gettemperature();

int loaddata();
void loaddata2();
void packDevlist();
void addNewDevice(char *buf);
void deleteDev(unsigned int id );
void editDev(char *buf );


void loadconfig();
void saveconfig();

void loadScenario();
void saveScenario();

void loadzone();
void savezone();

void newDevFromMcu(unsigned int id, char type);
int isDevRegister(unsigned int id);
void newEvent(unsigned int id, unsigned char type, int status, int triggerAlarm);
int update_last_alarm(unsigned int id, unsigned char type, int status,unsigned int time_now, int triggerAlarm);
void newNEST_Event(unsigned int id , unsigned char type, int status, int triggerAlarm);


//global

extern char sdpath[128];
//pairing
extern jswdev g_pairDevinfo;//current pairing dev
extern int g_isPairMode;

extern timer_t pairing_t;
extern timer_t pairing_current_t;
extern timer_t exitdelay_t;
extern timer_t exitdelay_current_t;
extern timer_t entrydelay_t;
extern timer_t entrydelay_current_t;
extern timer_t writeack_t;
extern timer_t siren_t;
extern timer_t siren_current_t;
extern timer_t service_t;
extern timer_t smoke_t;
extern timer_t rftest_t;
extern timer_t rftx_t;
//
enum arm_cmd{
	c_disarm,
	c_arm,
	c_partarm
};

enum gateway_state{
	st_disarm,
	st_arm,
	st_partarm,
	st_exitdelay,
	st_entrydelay,
	st_maintain,
	st_testmode
};

extern unsigned int g_DID;

extern int g_armstate; //0=disarm, 1 = exit delay, 2, entry delay, 3, arm.
extern int g_sirenstate; // 0=off, 1=on.
extern short entrydelayState;

void armCommand(int cmd);
void panic();
void triggerAlarm(int mode, int is_smoke);
void sendTamperMsg(unsigned int did);
void stopAlarm();
void Force_stopAlarm();
void systemArm();
void systemDisarm();
void entryDelay(unsigned int deviceDID);

int isSource(short model);
int isTarget(short model);
void addsource(unsigned int did);

void client_linkmenu(int session);
void setarmtable(int count, char* in);

void startScenario(unsigned int  id, char status);

int checkDevFlag(unsigned int did);

double timegettime();

extern short g_temp;
extern short g_humid;

void factoryReset(int mode);

void setnetwork(int len, char * buf);
void getnetwork();

void updatepwd(char *in, int session);
void setgatewayinfo(char *in);
void setsirenprop(char *in);
void getsirenprop(char *in);
int getEvfilelist();


void savesetting();
void loadsetting();
void *startgatewayalarm_thread(void *ptr );

int initled();
void yellowled(int mode);
void redled(int mode);
void greenled(int mode);
void blueled(int mode);
void ledoff();

int ledtest();
void yellowled(int mode);

struct fwupdateinfo{
	int version;
	int checksum;
	int reserve;
	unsigned int bufferlen;
};

//
void startfwupdate(char *inbuf, int session);
void stopfwupdate(int session);
void savefw(int len, char* buf);

void arrangeSchedule();
void checkkeepalive();

struct keepalive{
	unsigned int did;
	time_t lastcheckin;
	char sensor_version;
	unsigned char misss_supervision;
	char sent_push_event;
	char sensor_NO_resp ;
	char reserved[6];
};

extern struct keepalive keepalivelist[MAXDEVICENUM];

void getevent(int cnt, char* buf, int sess);

void sdtest(int sess);

int initipcam();
void paAutoOff(unsigned int did);
void dopaAutooff(timer_t id);

void doRec(unsigned int did);
void addipc(unsigned int did,char * ipcdid, char* ipcpwd );
void ipcReconnect();
void doRecByDID(char* pDID);

void startRec();

void sirenmaintain(int mode);

void ipc_reconnect();

void ipc_motionEvent(int did);

void checkfactoryreset();

void writegatewayinfo(char *buf);
void rftxTest();

int delEvfilelist();

void loadmac();
void savemac();

int PPPP_Write2(INT32 SessionHandle, UCHAR Channel, CHAR *DataBuf, INT32 DataSizeToWrite, int is_encrypt, char *aeskey);

typedef struct _jswbattlow_
{
	unsigned int did;
	unsigned int time;
	int model;
	int reserve[4];
}jswbattlow;

extern jswbattlow g_battlowlist[MAXDEVICENUM];
extern int g_battlowcount;

void recount_battlow();//recount battlowlist and set to g_battlowcount
void sort_out_battlow();//sort out battlowlist
void kill_oldest_battlow(void);//kill the oldest list from battlowlist
int remove_battlow(unsigned int did);//remove one list from battlowlist, remove successfully: return 1, otherwise return 0
void add_battlow(unsigned int did, int model);//add one list to battlowlist
void check_battlow(); //check the battery list is valid or not, and delete the ibvalid item
//void show_battlow();//show all battlowlist, just for test
int getMAGOpenCount(void);//return count of mag open
void call_buzzer_doorbell(void);//trigger GW to alarm as doorbell



//Multi-Language push string
#define PUSH_STRING_CFG_FILE    "push_string.cfg" //push string text file name
#define PUSH_STRING_GAP         "$$$" //gap for two push strings
#define DEF_ERROR_PUSH_STRING   "Error to get push string!"
#define DEF_SYSTEM_NAME         "SHC_pro"

#define DEF_PUSH_LANGUAGE       2 //Default push string language is english
#define DEF_RESET_SYSTEM_NAME   "13572468"

enum PSUH_STRING_LANGUAGE_LIST{
	LANG_ID_GERMAN = 0x01,
	LANG_ID_ENGLISH,
	LANG_ID_FRENCH,
	LANG_ID_DUTCH,
	LANG_ID_DANISH,
	LANG_ID_SWEDISH,
	LANG_ID_ITALIAN,
	LANG_ID_SPANISH,
	LANG_ID_LAST, //last language id
};

enum PSUH_STRING_ID_LIST{
	STR_ID_ARM = 0x01,
	STR_ID_PART_ARM,
	STR_ID_DISARM,
	STR_ID_PANIC,
	STR_ID_TAMPER,
	STR_ID_ALARM,
	STR_ID_SENSOR_BATTERY_LOW,
	STR_ID_GATEWAY_BATTERY_LOW,
	STR_ID_LOST_SIGNAL,
	STR_ID_LAST, //last string id
};

#define DEF_PUSH_SELECTION1_ARM             	0x00000001
#define DEF_PUSH_SELECTION1_PART_ARM        	0x00000002
#define DEF_PUSH_SELECTION1_DISARM          	0x00000004
#define DEF_PUSH_SELECTION1_PANIC           	0x00000008
#define DEF_PUSH_SELECTION1_TAMPER          	0x00000010
#define DEF_PUSH_SELECTION1_ALARM           	0x00000020
#define DEF_PUSH_SELECTION1_SENSOR_BATTLOW  	0x00000040
#define DEF_PUSH_SELECTION1_GATEWAY_BATTLOW     0x00000080
#define DEF_PUSH_SELECTION1_LOST_SIGNAL     	0x00000100

void get_push_string(char *msg, int str_id); //get push string by language id and string id


//for Sunrise/Sunset
typedef struct _sunrise_sunset
{
//    struct tm date;
    struct tm sunrise;
    struct tm sunset;
//    struct tm solar_noon;
//    int day_length;
//    struct tm civil_twilight_begin;
//    struct tm civil_twilight_end;
//    struct tm nautical_twilight_begin;
//    struct tm nautical_twilight_end;
//    struct tm astronomical_twilight_begin;
//    struct tm astronomical_twilight_end;
    //jswlatlng latlng;
	int reserve[4];
}jswsunrisesunset;

char *get_json_value(char *key, char* json, char *value);//get json value by key
char *get_json_value_int(char *key, char* json, char *value);//get json value(int) by key
int parse_json_value_to_tm(char *value, struct tm *t2);//parse json value to struct tm //return: parse successfully, else parse failed
void get_sunrise_sunset_data();//get and re-flash sunrise/sunset data
int add_push_alarm_sound(int str_id);

#define DEF_ALARM_51_SONG_NO     8 //default 51 alarm song no
#define DEF_DOORCHIME_SONG_MAX   8 //Max song no
void play_doorchime(jswdev *dev, int doorchime_track, int save_event); //play song of doorchime

//apple
bool getSensorstate(unsigned int nNodeID, jswdev* pDevIfno);


pthread_mutex_t uart_TX_mutex;


#endif


//int    tm_sec   Seconds [0,60].
//int    tm_min   Minutes [0,59].
//int    tm_hour  Hour [0,23].
//int    tm_mday  Day of month [1,31].
//int    tm_mon   Month of year [0,11].
//int    tm_year  Years since 1900.
//int    tm_wday  Day of week [0,6] (Sunday =0).
//int    tm_yday  Day of year [0,365].
//int    tm_isdst Daylight Savings flag.
