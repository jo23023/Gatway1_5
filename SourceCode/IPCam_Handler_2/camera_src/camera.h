/*============================================================================
*
*  版权(R)所有  2003-2014 沈晓峰. All Rights Reserved.
*
*  【文件名】:          camera.h
*  【版本】：           （必需）
*  【功能模块和目的】:  （必需）
*				
*  【开发者】:          沈晓峰（gray.shen@starxnet.com）
*  【制作日期】:        2014/03/16 15:00:00 - 2014/03/16 15:00:00
*  【更改记录】:        （若修改过则必需注明）
*
*============================================================================*/

/* 防止头文件重复定义或包含 --------------------------------------------------*/
#ifndef _CAMERA_H
#define _CAMERA_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/* 包含头文件 --------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>

#include "AVSTREAM_IO_Proto.h"
#include "gsDef.h"

#include "config.h"
#include "AES_EnDe.h"
//#include "trace.h"
#include "sysdeps.h"
#include "fifo.h"
#include "logger.h"
#include "utility.h"
//#include "codec.h"
#include "PPPP_API.h"

/* 导出定义类型 ------------------------------------------------------------*/
typedef void*   HCAMERA;
 
typedef void (*PCAMERA_ROUTINE)(
    IN char *cam_did,
    IN uint8_t *bufdata,
    IN uint32_t bufsize,
    IN int param); 
  
#define TRIGGER_DEBUG		0

/* 导出定义常量 ------------------------------------------------------------*/
typedef enum _ENUM_CONNECTSTATUS
{
  CONN_INFO_SUCCESSED = 0,
  CONN_INFO_FAILED = 1,
  CONN_INFO_AUTHOK = 2,
  CONN_INFO_AUTHFAILED = 3,
  CONN_INFO_RUNNING = 4,
  CONN_INFO_CLOSED = 5
} ENUM_CONNECT_STATUS;
  
/* 私有定义常量 --------------------------------------------------------------*/
#define MAX_SIZE_BUF            512000  // 500*1024;
#define MAX_SIZE_IOCTRL_BUF     512

/* 私有定义类型 --------------------------------------------------------------*/

// 设备模式
enum DEV_MODE_TYPE
{
  DEV_MODE_UNKW	= 0,
  DEV_MODE_DWS,
  DEV_MODE_DWH,
  DEV_MODE_WAPP,
  DEV_MODE_GMAPP
};

enum AV_TYPE
{
  AV_TYPE_REALAV = 1,
  AV_TYPE_PLAYBACK,
  AV_TYPE_DOWNLOAD
};

enum PPPP_STATUS_TYPE
{
  STATUS_INFO_AV_ONLINENUM = 5111,
  STATUS_INFO_AV_RESOLUTION = 5112,
  STATUS_INFO_CAM_INDEX_FROM_DEV = 5113,
  STATUS_INFO_REMOTE_RECORDING = 5114
};

enum OM_TYPE
{
  OM_GET_ONE_PIC_FROM_STREAM = 5210,
  OM_IOCTRL_RECORD_PLAY_END	= 5211,
  OM_SHOW_DEVICE_VIDEO = 5212,
  OM_SHOW_OFFLINE_PIC = 5213,
  OM_GET_ONE_PIC_TIMEOUT = 5214,
};

// 连接状态
enum CONN_INFO_TYPE
{
  CONN_INFO_UNKNOWN = 5000,
  CONN_INFO_CONNECTING = 5001,
  CONN_INFO_NO_NETWORK = 5002,
  CONN_INFO_CONNECT_WRONG_DID = 5003,
  CONN_INFO_CONNECT_WRONG_PWD = 5004,
  CONN_INFO_CONNECT_FAIL = 5005,
  CONN_INFO_SESSION_CLOSED = 5006,
  CONN_INFO_CONNECTED = 5007
};

// 连接模式
enum CONN_MODE_TYPE
{
  CONN_MODE_UNKNWN = -1,
  CONN_MODE_P2P = 0,
  CONN_MODE_RLY = 1
};

// 通道定义
enum CHANNEL_TYPE
{
  CHANNEL_IOCTRL = 0,
  CHANNEL_AVDATA_DtoC = 1, // used to realtime av
  CHANNEL_AVDATA_CtoD = 2,
  CHANNEL_PLAYBACK_DtoC = 3,// used to playback av
  CHANNEL_DOWNLOAD_DtoC = 4 // used to file download
};

/*IOCTL 类型 */
enum IOCTL_TYPE
{
  IOCTRL_TYPE_PUSH_CamIndex,
  IOCTRL_TYPE_VIDEO_START,
  IOCTRL_TYPE_VIDEO_STOP,
  IOCTRL_TYPE_AUDIO_START,
  IOCTRL_TYPE_AUDIO_STOP,

  // --- special -------------------
  IOCTRL_TYPE_DEVINFO_REQ,
  IOCTRL_TYPE_DEVINFO_RESP,
  IOCTRL_TYPE_RECORD_PLAYCONTROL_REQ,
  IOCTRL_TYPE_RECORD_PLAYCONTROL_RESP,
  IOCTRL_TYPE_PTZ_COMMAND,
  IOCTRL_TYPE_LISTEVENT_REQ,
  IOCTRL_TYPE_LISTEVENT_RESP,
  IOCTRL_TYPE_EVENT_NOTIFY,

  IOCTRL_TYPE_EMAIL_ON_OFF_REQ,         // Alarm Email enable / disable
  IOCTRL_TYPE_EMAIL_ON_OFF_RESP,

  IOCTRL_TYPE_EVENT_NOTIFY_ON_OFF_REQ,  // Device Event Notify enable / disable
  IOCTRL_TYPE_EVENT_NOTIFY_ON_OFF_RESP,

  IOCTRL_TYPE_GET_ON_OFF_VALUE_REQ,
  IOCTRL_TYPE_GET_ON_OFF_VALUE_RESP,

  IOCTRL_TYPE_SPEAKER_START,
  IOCTRL_TYPE_SPEAKER_STOP,

  IOCTRL_TYPE_SETPASSWORD_REQ,
  IOCTRL_TYPE_SETPASSWORD_RESP,

  IOCTRL_TYPE_SET_VIDEO_PARAMETER_REQ,
  IOCTRL_TYPE_SET_VIDEO_PARAMETER_RESP,
  IOCTRL_TYPE_GET_VIDEO_PARAMETER_REQ,
  IOCTRL_TYPE_GET_VIDEO_PARAMETER_RESP,

  IOCTRL_TYPE_LISTWIFIAP_REQ,
  IOCTRL_TYPE_LISTWIFIAP_RESP,
  IOCTRL_TYPE_SETWIFI_REQ,
  IOCTRL_TYPE_SETWIFI_RESP,

  IOCTRL_TYPE_SETMOTIONDETECT_REQ,
  IOCTRL_TYPE_SETMOTIONDETECT_RESP,
  IOCTRL_TYPE_GETMOTIONDETECT_REQ,
  IOCTRL_TYPE_GETMOTIONDETECT_RESP,

  IOCTRL_TYPE_SETRECORD_REQ,            // no use
  IOCTRL_TYPE_SETRECORD_RESP,           // no use
  IOCTRL_TYPE_GETRECORD_REQ,            // no use
  IOCTRL_TYPE_GETRECORD_RESP,           // no use

  IOCTRL_TYPE_FORMATEXTSTORAGE_REQ,     // Format external storage
  IOCTRL_TYPE_FORMATEXTSTORAGE_RESP,

  IOCTRL_TYPE_MANU_REC_START,           // start manual recording
  IOCTRL_TYPE_MANU_REC_STOP,            // stop manual recording

  IOCTRL_TYPE_SET_EMAIL_REQ,            // set alarm Email settings
  IOCTRL_TYPE_SET_EMAIL_RESP = 0x2C,
  IOCTRL_TYPE_GET_EMAIL_REQ,            // get alarm Email settings
  IOCTRL_TYPE_GET_EMAIL_RESP,

  IOCTRL_TYPE_AUTH_ADMIN_PASSWORD_REQ,  // authenticate admin password
  IOCTRL_TYPE_AUTH_ADMIN_PASSWORD_RESP,
  IOCTRL_TYPE_SET_ADMIN_PASSWORD_REQ,   // set admin password
  IOCTRL_TYPE_SET_ADMIN_PASSWORD_RESP,

  IOCTRL_TYPE_GETWIFI_REQ,
  IOCTRL_TYPE_GETWIFI_RESP,

  IOCTRL_TYPE_PUSH_APP_UTC_TIME,

  IOCTRL_TYPE_SET_TIMEZONE_REQ,
  IOCTRL_TYPE_SET_TIMEZONE_RESP,
  IOCTRL_TYPE_GET_TIMEZONE_REQ,
  IOCTRL_TYPE_GET_TIMEZONE_RESP,

  IOCTRL_TYPE_AUTO_DEL_REC_ON_OFF_REQ,

  IOCTRL_TYPE_SETDETECTMODE_REQ,
  IOCTRL_TYPE_SETDETECTMODE_RESP,
  IOCTRL_TYPE_GETDETECTMODE_REQ,
  IOCTRL_TYPE_GETDETECTMODE_RESP,

  /***************** Extra Control Cmd defined for Onet *******************/
  IOCTRL_TYPE_GET_ONET_DEVINFO_REQ,
  IOCTRL_TYPE_GET_ONET_DEVINFO_RESP,
  IOCTRL_TYPE_SET_ONET_DEVINFO_REQ,
  IOCTRL_TYPE_SET_ONET_DEVINFO_RESP,
  IOCTRL_TYPE_SET_ONET_STATUS_REQ,
  IOCTRL_TYPE_SET_ONET_STATUS_RESP,

  IOCTRL_TYPE_REMOVE_EVENTLIST_REQ,     // Remove event list
  IOCTRL_TYPE_REMOVE_EVENTLIST_RESP,
  IOCTRL_TYPE_REMOVE_EVENT_REQ,         // Remove one event
  IOCTRL_TYPE_REMOVE_EVENT_RESP,

  IOCTRL_TYPE_UPGRADE_FIRMWARE_REQ,
  IOCTRL_TYPE_UPGRADE_FIRMWARE_RESP = 0x6F
};

enum PTZ_COMMAND
{
	IOCTRL_PTZ_STOP =0x00,
	IOCTRL_PTZ_UP,
	IOCTRL_PTZ_DOWN ,
	IOCTRL_PTZ_LEFT ,
	IOCTRL_PTZ_RIGHT,
	IOCTRL_PTZ_LEFT_UP,
	IOCTRL_PTZ_LEFT_DOWN ,
	IOCTRL_PTZ_RIGHT_UP,
	IOCTRL_PTZ_RIGHT_DOWN,
	IOCTRL_LENS_ZOOM_IN ,
	IOCTRL_LENS_ZOOM_OUT,
	IOCTRL_PTZ_AUTO_SCAN,
	IOCTRL_PTZ_PRESET_POINT,
	IOCTRL_PTZ_SET_PRESET_POINT,
};

/* 授权类型 */
enum AUTH_TYPE
{
  AUTH_TYPE_UNKN = 0,
  AUTH_TYPE_REQ = 0x1,
  AUTH_TYPE_RESP,
  AUTH_TYPE_OK,
  AUTH_TYPE_FAILED
};

/* Camera 状态 */
enum CAMERA_STATUS
{
  ERROR_CAMERA_NULL   = -5000,
  ERROR_CAMERA_WRONG_PWD = -5001
};

/* Camera Trigger Record 结果 */
enum CAMERA_TRIGGER_RESULT
{
	TRIGGER_RECORDER_SUCCESSFUL		= 1,
	TRIGGER_RECORDER_FAILED			= 2,
	TRIGGER_RECORDER_WRONG_PASSWROD	= 3,

	TRIGGER_RECORDER_DEFAULT		= 100
};

#define CAMERA_CONN_IDLE			0
#define CAMERA_CONN_CONNECTING		1
#define CAMERA_CONN_DISCONNECTING	2
#define CAMERA_CONN_CONNECTED		3

// P2P Camera 结构 344byte
typedef struct _Camera {
  int					connState;				// P2P 连接状态
  int                   cbSize;
  volatile int			p2pConnResult;

  // P2P
  char                  DevUID[32];             // P2P DID
  char                  SecurityCode[32];       // P2P CAMERA 密码
  int                   hSession;               // P2P 会话值
  uint16_t              port;                   // UDP 端口
  bool                  enableLan : 1;          // 启动\关闭开启局域网搜索
  // P2P camera status
  int                   nCurCamIndex;           // 当前摄像头索引
  char                  DevModeInfo[16];        // 摄像头模式信息
  long                  total_bytes;            // 总计数据大小
  long                  total_frames;           // 总计帧数
  volatile bool         bRunning : 1;           // 运行标记
  bool                  bEmailAlert : 1;        // EMail 通知警告
  bool                  bEventNotify : 1;       // 事件通知
  bool                  bManuRecStatus : 1;     // 手动接受状态
  bool                  bPTZ : 1;               // 摄像头类型（Pan/Tilt/Zoom）
  bool                  bAutoDelRec : 1;        // 自动输出删除接受
  bool                  bSoftAP : 1;            // 使用软 AP
  volatile bool         bRealStream : 1;        // 实时流
  volatile bool         bSessionKeyOk : 1;      // P2P 会话密匙准确标记
  volatile bool         bAuthRespFromDev : 1;   // 设备身份验证响应
  enum CONN_MODE_TYPE   nConnMode : 2;          // 连接模拟
  enum CONN_INFO_TYPE   nConnInfo;              // 连接信息
  enum CHANNEL_TYPE     nAVDataChannel : 3;     // 指定AV数据通道

  /* encrypt */
  bool                  enableAES : 1;          // 使用 AES 加密
  uint8_t               nextXorKeyForIO;        // 控制数据 Xor key
  uint8_t               nextXorKeyForAudio;     // 视频数据 Xor Key
  uint8_t               AESKey[32];             // AES Key
  /* Thread */
  //  gs_thread_t       recvAVDataThread;       // 接受 AV 数据线程
  gs_thread_t           recvIOCtrlThread;       // 接受 IO 控制线程
  //  gs_thread_t       FrameThread;            // 帧封包线程
  gs_mutex_t            hMutex;                 // 线程互斥
  /* FIFO */
  FIFO                  fifoVideo;              // 视频 FIFO
  FIFO                  fifoAudio;              // 音频 FIFO
  PCAMERA_ROUTINE       pfnConnectProc[16];     // 连接回调函数
  PCAMERA_ROUTINE       pfnAlarmProc[16];       // 警告回调函数
} Camera;

/* 导出定义常量 ------------------------------------------------------------*/

typedef struct _IOCTRLAVStream
{
  uint8_t channel;
  uint8_t reserve[7];
} IOCTRLAVStream;

typedef struct _IOCTRLPlayRecord
{
  int           camIndex;
  int           playbackCmd;
  int           playbackPara;
  uint8_t       stUTCTime[8];
  uint8_t       utc;
  uint8_t       event_channel;
  uint8_t       reserved[2];
} IOCTRLPlayRecord;


typedef struct _IOCTRLParam
{
  uint8_t       Channel;
  uint8_t       *pData;
  int           nSize;
  int           timeout_ms;
  int           *nTotalRead;
  int           nBufferSize;
} IOCTRLParam;

typedef struct _DEV_INFO
{
  char                  *devPref;       // 设备名前缀
  enum DEV_MODE_TYPE    devMode;        // 设备工作模式
} DEV_INFO;

// Currently useless, we only need one parameter "Model_name"
// So, reserve for extension
typedef struct{
	char Model_name[16];
	char Vendor[16];
	UCHAR fw_version[4];
	UCHAR api_version[4];
	INT32 total_volume;
	INT32 free_volume;
	UCHAR reserve[8];
} Resp_device_info;

/* 私有定义宏 ----------------------------------------------------------------*/
#define SIZEOF_ARRAY(arr)       (sizeof(arr) / sizeof(arr[0]))

/* 私有全局变量 --------------------------------------------------------------*/

#define TIME_COUNT                      6

/* 导出变量 ----------------------------------------------------------------*/

/* 导出函数原型 ------------------------------------------------------------*/

  char* Camera_Version(void);
  int Camera_Init(IN const char * const szParameter);
  int Camera_Connect(
      OUT HCAMERA *hCamera,
      IN const char * const DevUID,
      IN const char * const SecurityCode,
      IN const unsigned short Port,
      IN bool LanSearch);
  bool Camera_IsEmailAlert(IN HCAMERA hCamera);
  bool Camera_IsEventNotify(IN HCAMERA hCamera);
  bool Camera_IsManuRecable(IN HCAMERA hCamera);
  bool Camera_IsAutoDelRec(IN HCAMERA hCamera);
  bool Camera_IsStart(IN HCAMERA hCamera);
  bool Camera_IsDownload(IN HCAMERA hCamera);
  bool Camera_IsPTZ(IN HCAMERA hCamera);
  bool Camera_IsRealStream(IN HCAMERA hCamera);
  bool Camera_IsAuthOk(IN HCAMERA hCamera);
  bool Camera_IsConnect(IN HCAMERA hCamera);
  void Camera_Disconnect(IN OUT HCAMERA hCamera);
  int Camera_Deinit(void);
  int Camera_manuRecStart(IN HCAMERA pCamera);
  int Camera_manuRecStop(IN HCAMERA pCamera);

  void Camera_Detection(IN OUT HCAMERA hCamera);
  void Camera_Connect_CallBack_set(
      IN HCAMERA hCamera,
      IN PCAMERA_ROUTINE pStartAddress);
  void Camera_Alarm_CallBack_set(
      IN HCAMERA hCamera,
      IN PCAMERA_ROUTINE pStartAddress);
  
#ifdef __cplusplus
}
#endif

#endif /* _CAMERA_H */

/****************************** END OF FILE ********************************/
