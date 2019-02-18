#ifndef _IPC_HANDLER_H_
#define _IPC_HANDLER_H_

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mosquitto.h"
#include "sysdeps.h"
#include "camera.h"


#define MQTT_HOST     "127.0.0.1"
#define MQTT_PORT   1883

#define CLIENT_NAME 		"IPCam_Handler"

// ceres 发送指令给ipc_handler
#define TOPIC_IPC_HANDLER_ADD			"ipc_handler_add"
#define TOPIC_IPC_HANDLER_REMOVE		"ipc_handler_remove"
#define TOPIC_IPC_HANDLER_RECORD		"ipc_handler_record"
#define TOPIC_IPC_HANDLER_RECORD_STOP	"ipc_handler_record_stop"

// ipc_handler 发送指令给 ceres
#define TOPIC_IPC_HANDLER_MOTION		"ipc_handler_motion"

#define QOS 2

static volatile int run = 1;

//static int mqttRunning = 0;
static struct mosquitto *connection;
static int mMQTTConnected = 0;
static pthread_t mqtt_sub_pid;

// for camera
pthread_mutex_t ipc_mutex;

#define CAM_MAX_NUMBER			4		// 做多支持的IPC数目

#define CAM_DID_LENGTH			32
#define CAM_PASSWD_LENGTH		32
#define CAM_CHANNEL_LENGTH		32

typedef struct _camera_info {
	volatile int valid;			// 是否有效
	volatile int sendRecord;		// 是否需要发送 record 指令
	volatile int toPresetPoint;
	char did[CAM_DID_LENGTH];
	char passwd[CAM_PASSWD_LENGTH];

	volatile int running;
	pthread_t pthread_connect;
	pthread_t pthread_record;

	Camera *hCamera;		// 处理 Camera P2P 连接
} st_camera_info;

// 定义一个数组来存储所有的IPC连接
st_camera_info **gAllIPCams;

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
