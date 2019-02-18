#ifndef _CERES_UTIL_H_
#define _CERES_UTIL_H_

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "curl.h"
//#include "mosquitto.h"

struct sms_info{
	char key[64];
	char to[64];
	char from[64];
	char route[16];
	char message[256];
};

#ifdef IPCAM_ENABLE
#include "sysdeps.h"
#include "camera.h"
#endif



#define MQTT_HOST     "127.0.0.1"
#define MQTT_PORT   1883

#define CLIENT_NAME 		"ceres_util"

#define TOPIC_GCM_PUSHEVENT 	"gcm_pushevent"
#define TOPIC_GCM_REGISTER		"gcm_register"
#define TOPIC_SEND_EMIAL		"send_email"
#define TOPIC_CHECK_CONNECT		"check_connect"
#define TOPIC_SMS_PUSHEVENT 	"sms_pushevent"
#define TOPIC_SEND_EMAIL_NOTIFICATION 	"send_email_notification"

#ifdef IPCAM_ENABLE
#define TOPIC_IPC_RECORD		"ipc_record"
#endif

#define QOS 2

static volatile int g_run = 1;

//static struct mosquitto *connection;
//static int mMQTTConnected = 0;
//static pthread_t mqtt_sub_pid;

extern CURL *gCurl;
extern CURLcode gCurlCode;

// 保存 gateway did
//char gDID[32] = { 0 };
extern char gDID[32];

pthread_t gcm_thread;
pthread_t sms_thread;
pthread_t email_thread;
pthread_t email_notification_thread;

pthread_mutex_t curl_mutex;
pthread_mutex_t email_mutex;

#ifdef IPCAM_ENABLE
pthread_t ipc_record_thread;


// for camera
#define CAM_DID_LENGTH			17+1
#define CAM_PASSWD_LENGTH		20+1
#define CAM_CHANNEL_LENGTH		16+1

typedef struct _camera_info {
	char did[CAM_DID_LENGTH];
	char passwd[CAM_PASSWD_LENGTH];
	char channel[CAM_CHANNEL_LENGTH];

	int valid;
	int touch;			// 记录此记录最后一次被更新时间

	Camera hCamera;		// 处理 Camera P2P 连接
} st_camera_info;


static st_camera_info gTriggerCamera;		// 触发的摄像头
static pthread_t trigger_camera_sub_pid;
const char *pszParam = "ECGBFFBJKAIEGHJAEBHLFGEMHLNBHCNIGEFCBNCIBIJALMLFCFAPCHODHOLCJNKIBIMCLDCNOBMOAKDMJGNMIJBJML";

// P2P connect error message
static const char *P2PErrorMsg[] =
{
  "SUCCESSFUL",					// 0
  "Not Initialized",
  "Already Initialized",
  "Time Out",
  "Invalid DID",					// 4
  "Invalid Parameter",			// 5
  "Device Not Online",			// 6
  "Fail To Resolve Name",
  "invalid Prefix",
  "ID Out Of Date",
  "No Relay Server Available",
  "Invalid Session Handle",
  "Closed Remote",				// 12
  "Closed TimeOut"
  "Closed Called",
  "Remote Site Buffer Full",
  "User Listen Break",
  "Max Session",				// 17
  "UDP Port Bind Failed",		// 18
  "User Connect Break",			// 19
  "Closed Insufficient Memory",	// 20
  "Invalid APILicense"			// 21
};

// Starx result message
static const char *StarxErrorMsg[] =
{
   "no use",					// 0
   "Trigger Record Successful",	// 1
   "Trigger Record Failed",		// 2
   "Wrong Password",			// 3
};
#endif

void gcmRegister(const char *did);

void pushEvent(const char *payload, int len);
void smsEvent(const char *payload, int len);
void sendEmailNotification(const char *payload, int len);
void *gcm_thread_rountine2(void *ptr);
void gcm_send_event_list(const char* text);
static size_t http_get_callback(void *contents, size_t size, size_t nmemb, void *userp);

#endif
