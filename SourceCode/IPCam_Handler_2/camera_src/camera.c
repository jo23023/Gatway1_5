/*============================================================================
*
*  版权(R)所有  2003-2014 沈晓峰. All Rights Reserved.
*
*  【文件名】:          camera.c 
*  【版本】：           （必需）
*  【功能模块和目的】:  （必需）
*				
*  【开发者】:          沈晓峰（gray.shen@starxnet.com）
*  【制作日期】:        2014/03/16 14:59:40 - 2014/03/16 14:59:40
*  【更改记录】:        （若修改过则必需注明）
*
*============================================================================*/

/* 包含头文件 ----------------------------------------------------------------*/
#include "camera.h"

const char *PREFIX_DWS_AS = "AS";

/* C99 */
static const Camera defaultVal = {
  .p2pConnResult = TRIGGER_RECORDER_DEFAULT,
  .hSession = -1,               .nCurCamIndex = -1,
  .bRunning = false,            .bEmailAlert = false,
  .bEventNotify = false,        .bManuRecStatus = false,
  .bPTZ = false,                .bAutoDelRec = false,
  .bSoftAP = false,             .bRealStream = false,
  .bSessionKeyOk = false,       .bAuthRespFromDev = false,
  .nConnInfo = CONN_INFO_UNKNOWN,
  .nConnMode = CONN_MODE_UNKNWN,.nAVDataChannel = CHANNEL_AVDATA_DtoC,
  .enableAES = true,            .nextXorKeyForIO = 2,
  .nextXorKeyForAudio = 3};

// P2P error Table
static const char *P2PErrTab[] =
{
  "SUCCESSFUL",
  "Not Initialized",
  "Already Initialized",
  "Time Out",
  "Invalid ID",
  "Invalid Parameter",
  "Device Not Online",
  "Fail To Resolve Name",
  "invalid Prefix",
  "ID Out Of Date",
  "No Relay Server Available",
  "Invalid Session Handle",
  "Session Closed Remote",
  "Session Closed TimeOut"
  "Session Closed Called",
  "Remote Site Buffer Full",
  "User Listen Break",
  "Max Session",
  "UDP Port Bind Failed",
  "User Connect Break",
  "Closed Insufficient Memory",
  "Invalid APILicense"
};

const DEV_INFO stDevMode[] =
{
  {"\0",        DEV_MODE_UNKW},
  {"DWS",       DEV_MODE_DWS},
  {"DWH",       DEV_MODE_DWH},
  {"WAPP",      DEV_MODE_WAPP},
  {"GMAPP",     DEV_MODE_GMAPP},
};

const DEV_INFO stDevProductType[] =
{
  {"\0",        DEV_MODE_UNKW},
  {"DWS",       DEV_MODE_DWS},
  {"DWH3",      DEV_MODE_DWS},
  {"DWH5",      DEV_MODE_DWH},
  {"WAPP",      DEV_MODE_WAPP},
  {"GMAPP",     DEV_MODE_GMAPP},
};

  
LOGGER_DEFINE(g_log, "Camera");                            // 日志

/* 私有函数声明----------------------------------------------------------------*/
static void* RecvIOCtrlThread(IN void* args);

static void* RecvAVDataThread(IN void* args);

static void* FrameThread(IN void* args);

static __inline__
ENUM_CONNECT_STATUS ConnetStatusProc(IN Camera *pCamera);

void Camera_GetDeveveInfo(IN Camera *pCamera, ENUM_CONNECT_STATUS status);

static __inline__
int Camera_SendIOCtrl(
    IN Camera  *pCamera,
    IN int     nIOCtrlType,
    IN uint8_t pIOData[],
    IN int     nIODataSize,
    IN int     nStreamIOType);

void EncryptPacket(
    IN Camera *pCamera,
    IN uint8_t Data[],
    IN int     nDataSize);

int DecryptPacket(
    IN Camera *pCamera,
    IN int nStreamIOType,
    IN OUT uint8_t Data[],
    IN int nDataSize);

/* 私有函数 ------------------------------------------------------------------*/

#define CAMERA_VERSION "V1.0"

char* Camera_Version(void) {
  static char szVersion[] = CAMERA_VERSION;
  return szVersion;
}

/*
 * P2P Camera 初始化
 */
int Camera_Init(
    IN const char * const szParameter) {
  // TODO
  LOG_CREATE(g_log);
  
  //LOG_DEBUG(g_log, "CAMERA Size is:%d\n", sizeof(Camera));
  UINT32 APIVersion = PPPP_GetAPIVersion();
  printf("IPC_Handler PPPP_API Version: %d.%d.%d.%d \n",
         (APIVersion & 0xFF000000) >> 24,
         (APIVersion & 0x00FF0000) >> 16,
         (APIVersion & 0x0000FF00) >> 8,
         (APIVersion & 0x000000FF) >> 0);
  
  return PPPP_Initialize((char *)szParameter);
}

/*
 * 关闭 P2P Camera
 */
int Camera_Deinit(void) {
  // TODO
  LOG_INFO(g_log, "Session is closed");
  // 销毁 LOG
  LOG_DESTROY();
  PPPP_Connect_Break();
  return PPPP_DeInitialize();
}

/*
 * 连接到 P2P Camera
 */
int Camera_Connect(
    OUT HCAMERA   *hCamera,
    IN const char * const DevUID,
    IN const char * const SecurityCode,
    IN const unsigned short Port,
    IN bool EnableLan) {
  // TODO
  LOG_WARN(g_log, "Connecting to Camera: %s", DevUID);

  if (TRIGGER_DEBUG)
	  printf("=====> Camera_Connect\n");

  if ( isNullField(DevUID) )
    return ERROR_CAMERA_NULL;

  // 初始化 Camera 结构
  assert(hCamera);
  Camera *pCamera = (Camera *)malloc(sizeof(Camera));
  *hCamera = (Camera *)pCamera;

  memcpy(pCamera, &defaultVal, sizeof(Camera));
  strcpy(pCamera->DevUID, DevUID);
  pCamera->port = Port;
  pCamera->enableLan = EnableLan;
  if(SecurityCode)
    strcpy(pCamera->SecurityCode, SecurityCode);
  
  // 初始化 FIFO
   FIFO_Init(&pCamera->fifoVideo);
   FIFO_Init(&pCamera->fifoAudio);

  if (pCamera->hSession < 0)
  {
    pCamera->hSession = PPPP_Connect(DevUID, EnableLan, Port);
    if (pCamera->hSession < 0)
    {
        
      LOG_INFO(g_log, "DID:%s,Session %s:%d",
               DevUID,
               P2PErrTab[-pCamera->hSession],
               pCamera->hSession);
      return pCamera->hSession;
    }
  }

   st_PPPP_Session SInfo;
  if (ERROR_PPPP_SUCCESSFUL == PPPP_Check(pCamera->hSession, &SInfo)) {
    LOG_INFO(g_log, "-------Session(%d) Ready: -%s----",
           pCamera->hSession, (SInfo.bMode == 0) ? "P2P" : "RLY");
    LOG_INFO(g_log, "Socket : %d", SInfo.Skt);
    LOG_INFO(g_log, "Remote Addr : %s:%d",
             (char*)inet_ntoa(SInfo.RemoteAddr.sin_addr),
             ntohs(SInfo.RemoteAddr.sin_port));
    LOG_INFO(g_log, "My Lan Addr : %s:%d",
             (char*)inet_ntoa(SInfo.MyLocalAddr.sin_addr),
             ntohs(SInfo.MyLocalAddr.sin_port));
    LOG_INFO(g_log, "My Wan Addr : %s:%d",
             (char*)inet_ntoa(SInfo.MyWanAddr.sin_addr),
             ntohs(SInfo.MyWanAddr.sin_port));
    LOG_INFO(g_log, "Connection time : %d second before", SInfo.ConnectTime);
    LOG_INFO(g_log, "DID : %s", SInfo.DID);
    LOG_INFO(g_log, "I am %s", (SInfo.bCorD ==0) ? "Client" : "Device");
    LOG_INFO(g_log, "Connection mode: %s", (SInfo.bMode == 0) ? "P2P" : "RLY");
    LOG_INFO(g_log,"------------End of Session info :---------------");
  }
  
  // 互斥初始化
  gs_mutex_init(&pCamera->hMutex);


  // 建立工作线程
  if (!(pCamera->recvIOCtrlThread)) {
    gs_thread_create(&pCamera->recvIOCtrlThread,
                   RecvIOCtrlThread,
                   pCamera);
  }

/* chenjian
  if (!pCamera->recvAVDataThread) {
     gs_thread_create(&pCamera->recvAVDataThread,
                   RecvAVDataThread,
                   pCamera);
  }

  if (!pCamera->FrameThread) {
     gs_thread_create(&pCamera->FrameThread,
                   FrameThread,
                   pCamera);
  }
*/  
  return 0;  
}

/*
 * 关闭 P2P Camera 连接
 */
void Camera_Disconnect(IN OUT HCAMERA hCamera) {
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  LOG_WARN(g_log, "Disconnecting the Camera: %s", pCamera->DevUID);
  
  pCamera->bRunning = false;

  if (pCamera->hSession >= 0) {
    gs_sleep_ms(20);
    PPPP_Close(pCamera->hSession);
    pCamera->hSession = -1;
  }

  // 等待线程结束
  if (pCamera->recvIOCtrlThread) {
    gs_thread_join(pCamera->recvIOCtrlThread);
  }

  /*if (pCamera->recvAVDataThread) {
    gs_thread_join(pCamera->recvAVDataThread);
  }*/

  /*if (pCamera->FrameThread) {
    gs_thread_join(pCamera->FrameThread);
  }*/

  // 销毁 FIFO
  FIFO_Destroy(&pCamera->fifoVideo);
  FIFO_Destroy(&pCamera->fifoAudio);

  // 销毁 Mutex
  gs_mutex_destroy(&pCamera->hMutex);
  
  pCamera->connState = CONN_INFO_CLOSED;
  // 连接回调函数
  int i = 0;
  int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
  while(i < num && pCamera->pfnConnectProc[i]) {
    pCamera->pfnConnectProc[i](pCamera->DevUID, NULL, 0, pCamera->connState);
    i++;
  }  
  // 释放 Camera
  free(pCamera);
}

/*
 * 开始接受视频和声频数据
 */
void Camera_Connect_CallBack_set(
    IN HCAMERA hCamera,
    IN PCAMERA_ROUTINE pStartAddress) {
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;

  if (pStartAddress) {
    int i = 0;
    int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
    while(i < num && pCamera->pfnConnectProc[i])
      i++;

    if (i >= num) return;
    pCamera->pfnConnectProc[i] = pStartAddress;
  }  
}

      
void Camera_Alarm_CallBack_set(
    IN HCAMERA hCamera,
    IN PCAMERA_ROUTINE pStartAddress) {
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;

  if (pStartAddress) {
    int i = 0;
    int num = SIZEOF_ARRAY(pCamera->pfnAlarmProc);
    while(i < num && pCamera->pfnAlarmProc[i])
      i++;
    
    if (i >= num) return;
    pCamera->pfnAlarmProc[i] = pStartAddress;
  }  
}

/*
 * 获取连接状态， 连接回调函数
 */
static __inline__
ENUM_CONNECT_STATUS ConnetStatusProc(IN Camera *pCamera)
{
  // TODO
  st_PPPP_Session SInfo;
  ENUM_CONNECT_STATUS result = CONN_INFO_SUCCESSED;

  // 获取连接状态， 连接回调函数
  int status = PPPP_Check(pCamera->hSession, &SInfo);
  if (status < 0)
    result = CONN_INFO_FAILED;
  else if (true == Camera_IsConnect(pCamera)
           && status == ERROR_PPPP_SUCCESSFUL)
    result = CONN_INFO_RUNNING;
  
  int i = 0;
  int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
  while(i < num && pCamera->pfnConnectProc[i]) {
    pCamera->pfnConnectProc[i](pCamera->DevUID, NULL, 0, result);
    i++;
  }

  // 身份认证
  if (status == ERROR_PPPP_SUCCESSFUL && pCamera->connState == CAMERA_CONN_CONNECTED) {
    result = (pCamera->bSessionKeyOk ? CONN_INFO_AUTHOK : CONN_INFO_AUTHFAILED);
    int i = 0;
    int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
    while(i < num && pCamera->pfnConnectProc[i]) {
      pCamera->pfnConnectProc[i](pCamera->DevUID, NULL, 0, result);
      i++;
    }
  }
  else	/* 20150119_victorwu: adjust ipcam connection status return */
  {
  	result = CONN_INFO_AUTHFAILED;
  }

  return result;
}

/*
 * 开始检测摄像头
 */
void Camera_Detection(IN OUT HCAMERA hCamera)
{
  // TODO;
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  LOG_WARN(g_log, "Camera Loop Detection: %s", pCamera->DevUID);

  gs_sleep_ms(500);
  
  ENUM_CONNECT_STATUS status = Camera_IsConnect(hCamera) ? CONN_INFO_SUCCESSED : CONN_INFO_FAILED;

  if (status == CONN_INFO_SUCCESSED)  /* 20150119_victorwu: adjust ipcam connection status return */
  {
  	status = ConnetStatusProc(pCamera);
  }
  
  int n = 0;
  int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
  while(n < num && pCamera->pfnConnectProc[n]) {
    pCamera->pfnConnectProc[n](pCamera->DevUID, NULL, 0, status);
    n++;
  }

  gs_sleep_ms(10000);
 
  Camera_GetDeveveInfo(pCamera, status);
  
  while (pCamera->bRunning) {
    // 连接状态处理
    status = ConnetStatusProc(pCamera);
    // 检测网络
    if (status == CONN_INFO_FAILED ||status == CONN_INFO_AUTHFAILED) {
	PPPP_ForceClose(pCamera->hSession);
	pCamera->hSession = -1;

	if (status == CONN_INFO_AUTHFAILED)	/* 20150119_victorwu: adjust ipcam connection status return */
	{
		printf("[ IPC_handler] Connect %s fail->password is wrong\n", pCamera->DevUID);
		pCamera->bRunning = false;
		break;
	}

	int i = 0;
      while (1) {
        LOG_DEBUG(g_log, "reconnect counter=%d", i + 1);
        pCamera->hSession = PPPP_Connect(pCamera->DevUID,
                                          pCamera->enableLan,
                                          pCamera->port);
        if (pCamera->hSession >= 0) {
          status = ConnetStatusProc(pCamera);
          Camera_GetDeveveInfo(pCamera, status);	// When reconnect, do get device info

          int n = 0;
          int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
          while(n < num && pCamera->pfnConnectProc[n]) {
            pCamera->pfnConnectProc[n](pCamera->DevUID, NULL, 0, CONN_INFO_SUCCESSED);
            n++;
          }
          // 等待线程结束
          if (pCamera->recvIOCtrlThread) {
            gs_thread_join(pCamera->recvIOCtrlThread);
            pCamera->recvIOCtrlThread = (int)NULL;
          }
          // 建立工作线程
          if (!(pCamera->recvIOCtrlThread)) {
            gs_thread_create(&pCamera->recvIOCtrlThread,
                             RecvIOCtrlThread,
                             pCamera);
          }
  
          break;
        } else if (i >= TIME_COUNT) {
          pCamera->bRunning = false;
          //  连接回调函数
          int n = 0;
          int num = SIZEOF_ARRAY(pCamera->pfnConnectProc);
          while(n < num && pCamera->pfnConnectProc[n]) {
            pCamera->pfnConnectProc[n](pCamera->DevUID, NULL, 0, CONN_INFO_CLOSED);
            n++;
          }
          i = 0;
          LOG_INFO(g_log, "Wait 1 minutes before trying again...");
          sleep(60);
          continue;
        }

        gs_sleep_ms(500);
        i++;

      }	// while (1)

    }	// if (status == CONN_INFO_FAILED)

    gs_sleep_ms(1000);	// while (pCamera->bRunning)
  }  

  gs_sleep_ms(1000);
}

/*
 * 开始接受视频和声频数据
 */
static __inline__
int Camera_Start(IN HCAMERA hCamera) {
  // TODO
  int Ret = 1;
  Camera *pCamera = (Camera *)hCamera;
  
  if (pCamera->nAVDataChannel == CHANNEL_AVDATA_DtoC) {
    LOG_INFO(g_log, "==== startAV send VIDEO_START");
  
    assert(pCamera);
  
    IOCTRLAVStream      IOData;
    memset(&IOData, 0, sizeof(IOCTRLAVStream));

    Ret = Camera_SendIOCtrl(
        pCamera,
        IOCTRL_TYPE_VIDEO_START,
        (uint8_t*)&IOData,
        sizeof(IOCTRLAVStream),
        SIO_TYPE_IOCTRL);

    if (Ret < 0) LOG_ERROR(g_log, "P2P Connect fail.");
  }
  return Ret;
}

/*
 * 暂停接受视频和声频数据
 */
static __inline__
bool Camera_Stop(IN HCAMERA hCamera) {
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  
  if (pCamera->hSession < 0)
    return true;
  return false;
}

/************************************************************************/

/*
 * 查询是否 EMail 警告通知
 */
bool Camera_IsEmailAlert(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  return pCamera->bEmailAlert;
}

/*
 * 查询事件通知
 */
bool Camera_IsEventNotify(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  return pCamera->bEventNotify;
}

/*
 * 手动接受状态
 */
bool Camera_IsManuRecable(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  return pCamera->bManuRecStatus;
}

/*
 * 自动删除接受
 */
bool Camera_IsAutoDelRec(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  return pCamera->bAutoDelRec;
}

/*
 * 判断 P2P Camera 是否连接
 */
bool Camera_IsConnect(IN HCAMERA hCamera) {
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  
  return (pCamera->hSession >= 0);
}

/*
 * 查询是否开始摄像
 */
bool Camera_IsStart(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  return pCamera->bRunning;
}

/*
 * 查询是否在下载
 */
bool Camera_IsDownload(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  return true;
}

/*
 * 查询摄像头是（Pan/Tilt/Zoom）
 */
bool Camera_IsPTZ(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera *pCamera = (Camera *)hCamera;
  return pCamera->bPTZ;
}

/*
 * 查询是实时流
 */
bool Camera_IsRealStream(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  Camera * pCamera = (Camera *)hCamera;
  return pCamera->bRealStream;
}

/*
 * 认证密码是否准确
 */
bool Camera_IsAuthOk(IN HCAMERA hCamera)
{
  // TODO
  assert(hCamera);
  bool nRet = false;
  Camera *pCamera = (Camera *)hCamera;

/*  if (pCamera->bAuthRespFromDev) {
    IIF(pCamera->bSessionKeyOk, nRet = 0, nRet = 1);
  } else {
    IIF(pCamera->bSessionKeyOk, nRet = -1, nRet = -2);
  }*/

  if (pCamera->bAuthRespFromDev && pCamera->bSessionKeyOk)
	  nRet = true;

  return nRet;
}

/************************************************************************/
/*
 * 获取摄像头的运行模式
 */
static __inline__
int Camera_GetDevMode(IN Camera *pCamera) {
  // TODO
  int result = 0;
  int nSize = sizeof(stDevProductType) / sizeof(DEV_INFO);

  result = gs_strnicmp(pCamera->DevUID,
                       PREFIX_DWS_AS,
                       strlen(PREFIX_DWS_AS));
  
  if (result)
      return DEV_MODE_DWS;
      
  for (int n = 0; n < nSize; n++) {
    result = gs_strnicmp(stDevProductType[n].devPref,
                         pCamera->DevModeInfo,
                         strlen(stDevProductType[n].devPref));
    if (!result)        return stDevProductType[n].devMode;
    
  }
  return DEV_MODE_UNKW;
}


static __inline__
int Camera_GetProductType(IN Camera *pCamera) {
  // TODO
  int result = 0;
  int nSize = sizeof(stDevMode) / sizeof(DEV_INFO);

  result = gs_strnicmp(pCamera->DevUID,
                       PREFIX_DWS_AS,
                       strlen(PREFIX_DWS_AS));
  
  if (result)
      return DEV_MODE_DWS;
      
  for (int n = 0; n < nSize; n++) {
    result = gs_strnicmp(stDevMode[n].devPref,
                         pCamera->DevModeInfo,
                         strlen(stDevMode[n].devPref));
    if (!result)        return stDevMode[n].devMode;
    
  }
  return DEV_MODE_UNKW;
}

  
/************************************************************************/
/*
 * 发送 IO 控制
 */
static __inline__
int Camera_SendIOCtrl(
    IN Camera  *pCamera,
    IN int     nIOCtrlType,
    IN uint8_t IOData[],
    IN int     nIODataSize,
    IN int     nStreamIOType) {
  // TODO
  assert(pCamera);
  
  LOG_DEBUG(g_log, "%s(..) 1, handleSession=%d,"
           " nStreamIOType=%d, nIOCtrlType=%d",
            __FUNCTION__, pCamera->hSession,
            nStreamIOType, nIOCtrlType);
  
  if (nIODataSize < 0 || nIOCtrlType < 0) {
    LOG_DEBUG(g_log, "Camera NULL");
    return ERROR_CAMERA_NULL;
  }
  

  if (nStreamIOType != SIO_TYPE_AUTH && !pCamera->bSessionKeyOk) {
    LOG_DEBUG(g_log, "Camera Wrong password!");
    return ERROR_CAMERA_WRONG_PWD;
  }
  
  
  int nSize = sizeof(st_AVStreamIOHead)
      + sizeof(st_AVIOCtrlHead) + nIODataSize;
  
  uint8_t *packet = (uint8_t *)malloc(nSize);
  memset(packet, 0, nSize);

  uint32_t packetsize = sizeof(st_AVIOCtrlHead) + nIODataSize;
  packetsize = __cpu_to_le32(packetsize);
  packet[0] = (uint8_t) packetsize;
  packet[1] = (uint8_t) (packetsize >> 8);

  packet[3] = (uint8_t) nStreamIOType;
  packet[4] = (uint8_t) nIOCtrlType;
  packet[5] = (uint8_t) (nIOCtrlType >> 8);

  if (IOData != NULL) {
    packet[6] = (uint8_t) nIODataSize;
    packet[7] = (uint8_t)(nIODataSize >> 8);
    memcpy(packet + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),
           IOData, nIODataSize);
  }
  
  if (pCamera->enableAES) {
    if (nStreamIOType != SIO_TYPE_AUTH)
      EncryptPacket(pCamera, packet, nSize);             // 加密
  }

  int nRet = PPPP_Write(pCamera->hSession, CHANNEL_IOCTRL, (CHAR *)packet, nSize);
  LOG_DEBUG(g_log, "sendIOCtrl(PPPP_Write) nRet=%d"
            ", sessionHandle=%d"
            ", nStreamIOType=%d"
            ", nIOCtrlType=%d",
            nRet, pCamera->hSession, nStreamIOType, nIOCtrlType);
  
  free(packet);
  return nRet;
  
}

/*
 *
 */    
static __inline__
void Camera_SendStopCurCam(
    IN Camera   *pCamera,
    IN bool     bIncludeAudio) {
  // TODO
  assert(pCamera);
  
  if (pCamera->hSession < 0 || !pCamera->bSessionKeyOk)
    return;

  if (pCamera->nCurCamIndex >= 0) {
    IOCTRLAVStream stIOCtrlAV = {pCamera->nCurCamIndex};
    Camera_SendIOCtrl(pCamera, IOCTRL_TYPE_VIDEO_STOP,
                      (uint8_t *)&stIOCtrlAV, sizeof(IOCTRLAVStream),
                      SIO_TYPE_IOCTRL);
    gs_sleep_ms(10);
    
    if(bIncludeAudio) {
      Camera_SendIOCtrl(pCamera, IOCTRL_TYPE_AUDIO_STOP,
                        (uint8_t *)&stIOCtrlAV, sizeof(IOCTRLAVStream),
                        SIO_TYPE_IOCTRL);
      gs_sleep_ms(10);
    }
  }
  LOG_DEBUG(g_log, "sendIOCtrl_stopCurCam, m_nCurCamIndex=%d", pCamera->nCurCamIndex);
}

static __inline__
void Camera_ChgCamIndex(
    IN Camera   *pCamera,
    IN int      camIndex,
    bool        bIncludeAudio)
{
  // TODO
  assert(pCamera);
  
  if (pCamera->hSession < 0 || !pCamera->bSessionKeyOk)
    return;

  LOG_DEBUG(g_log,
            "sendIOCtrl_chgCamIndex, nCurCamIndex=%d, camIndexNew=%d",
            pCamera->nCurCamIndex, camIndex);
  
  if (pCamera->nCurCamIndex == camIndex)        return;

  Camera_SendStopCurCam(pCamera, bIncludeAudio);

  IOCTRLAVStream stIOCtrlAV = {pCamera->nCurCamIndex};
  if (camIndex >= 0) {
    pCamera->nCurCamIndex = camIndex;
    Camera_SendIOCtrl(pCamera, IOCTRL_TYPE_VIDEO_START,
                      (uint8_t *)&stIOCtrlAV, sizeof(IOCTRLAVStream),
                      SIO_TYPE_IOCTRL);
    if(bIncludeAudio) {
      Camera_SendIOCtrl(pCamera, IOCTRL_TYPE_AUDIO_START,
                        (uint8_t *)&stIOCtrlAV, sizeof(IOCTRLAVStream),
                        SIO_TYPE_IOCTRL);
      gs_sleep_ms(10);
    }
  }
  LOG_DEBUG(g_log, "sendIOCtrl_chgCamIndex, m_nCurCamIndex = %d", pCamera->nCurCamIndex);
}

static __inline__
int Camera_Playback(
    IN Camera *pCamera,
    IN int nPlaybackCmd) {
  // TODO
  assert(pCamera);

  if(pCamera->hSession < 0
     || !pCamera->bSessionKeyOk
     || pCamera->nAVDataChannel != CHANNEL_AVDATA_DtoC)
    return ERROR_CAMERA_NULL;

  // do!!!!!!!!!!!!!!!!!!!!!!!!!
}

static __inline__
int Camera_Download(
    IN Camera *pCamera,
    IN int nPlaybackCmd) {return  0;}

static __inline__
int Camera_Outer(
    IN Camera *pCamera,
    IN int nIOCtrlType,
    IN uint8_t IOData[],
    int nIODataSize) {
  // TODO
  assert(pCamera);
  assert(IOData);
  
  if (pCamera->hSession < 0 || !pCamera->bSessionKeyOk)
    return ERROR_CAMERA_NULL;

  return Camera_SendIOCtrl(pCamera,
                           nIOCtrlType,
                           IOData,
                           nIODataSize,
                           SIO_TYPE_IOCTRL);
}
/*
 * 发送控制 on/off
 */
static __inline__
int Camera_SendIOCtrlToggle(
    IN Camera   *pCamera,
    IN bool     bGet,
    IN uint8_t  setEmailValue,
    IN uint8_t  setEventNotifyValue) {
  // TODO
  assert(pCamera);
  
  if (pCamera->hSession < 0 || !pCamera->bSessionKeyOk)
    return ERROR_CAMERA_NULL;

  if (bGet) {
    pCamera->bEmailAlert = false;
    pCamera->bEventNotify = false;
    pCamera->bManuRecStatus = false;
    pCamera->bAutoDelRec = false;
    pCamera->bSoftAP = false;
    
    return Camera_SendIOCtrl(pCamera,
                      IOCTRL_TYPE_GET_ON_OFF_VALUE_REQ,
                      NULL,
                      0,
                      SIO_TYPE_IOCTRL);
  } else {     // set
    uint8_t IOData[4] = {0, 0, 0, 0};
    IOData[0] = setEmailValue;
    int nRet1 = Camera_SendIOCtrl(
        pCamera,
        IOCTRL_TYPE_EMAIL_ON_OFF_REQ,
        IOData,
        sizeof(IOData),
        SIO_TYPE_IOCTRL);
    
    gs_sleep_ms(4);

    memset(IOData, 0, sizeof(IOData));
    IOData[0] = setEventNotifyValue;
    int nRet2 = Camera_SendIOCtrl(
        pCamera,
        IOCTRL_TYPE_EVENT_NOTIFY_ON_OFF_REQ,
        IOData,
        sizeof(IOData),
        SIO_TYPE_IOCTRL);
    
    LOG_DEBUG(g_log,
              "sendIOCtrl_on_off(set, .) nRet1=%d, nRet2=%d",
              nRet1, nRet2);

    return (nRet1 & nRet2);
  }

  return 0;
}

// chenjian
int Camera_manuRecStart(IN HCAMERA hCamera) {
  if (TRIGGER_DEBUG)
	  printf("===============> Camera_manuRecStart \n");
  // TODO
  int Ret = 1;
  Camera *pCamera = (Camera *)hCamera;
  
  if (pCamera->nAVDataChannel == CHANNEL_AVDATA_DtoC) {
    //LOG_INFO(g_log, "==== startAV send VIDEO_START");
  
    assert(pCamera);
  
    IOCTRLAVStream      IOData;
    memset(&IOData, 0, sizeof(IOCTRLAVStream));

    Ret = Camera_SendIOCtrl(
        pCamera,
        IOCTRL_TYPE_MANU_REC_START,
        (uint8_t*)&IOData,
        sizeof(IOCTRLAVStream),
        SIO_TYPE_IOCTRL);

    if (TRIGGER_DEBUG)
    	printf("===============> Ret = %d \n", Ret);
    if (Ret < 0)
    {
    	if (TRIGGER_DEBUG)
    		printf("===============> TRIGGER_RECORDER_FAILED \n");
    	pCamera->p2pConnResult = TRIGGER_RECORDER_FAILED;
    }
    else
    {
    	if (TRIGGER_DEBUG)
    	    printf("===============> TRIGGER_RECORDER_SUCCESSFUL \n");
    	pCamera->p2pConnResult = TRIGGER_RECORDER_SUCCESSFUL;
    }
    pCamera->bRunning = 0;

  }
  return Ret;
}

int Camera_manuRecStop(IN HCAMERA hCamera) {
	if (TRIGGER_DEBUG)
		printf("===============> Camera_manuRecStop \n");
  // TODO
  int Ret = 1;

  Camera *pCamera = (Camera *)hCamera;

  if (pCamera->nAVDataChannel == CHANNEL_AVDATA_DtoC) {
    //LOG_INFO(g_log, "==== startAV send VIDEO_START");

    assert(pCamera);

    IOCTRLAVStream      IOData;
    memset(&IOData, 0, sizeof(IOCTRLAVStream));

    Ret = Camera_SendIOCtrl(
        pCamera,
        IOCTRL_TYPE_MANU_REC_STOP,
        (uint8_t*)&IOData,
        sizeof(IOCTRLAVStream),
        SIO_TYPE_IOCTRL);

    if (TRIGGER_DEBUG)
    	printf("===============> Ret = %d \n", Ret);
    if (Ret < 0) LOG_ERROR(g_log, "P2P Connect fail.");
  }
  return Ret;
}


/*
 * 处理 P2P 授权验证状态
 */
static __inline__
void Camera_DoAuth(
    IN HCAMERA const hCamera,
    IN uint8_t IOData[]) {
  // TODO
  assert(hCamera);
  assert(IOData);
  Camera *pCamera = (Camera *)hCamera;

  if (TRIGGER_DEBUG)
	  printf("=====> Camera_DoAuth\n");
  
  st_AuthHead *pstAuthHead = (st_AuthHead *)IOData;
  LOG_DEBUG(g_log,
            "pIOData[0]-[3]=%d,%d,%d,%d",
            IOData[0],IOData[1],IOData[2],IOData[3]);
  LOG_DEBUG(g_log,
            "%s:AuthType=%d",
            __FUNCTION__, pstAuthHead->AuthType);

  if (TRIGGER_DEBUG)
	  printf("pstAuthHead->AuthType = %d\n", pstAuthHead->AuthType);
  
  switch (pstAuthHead->AuthType) {
    case AUTH_TYPE_REQ:
      LOG_DEBUG(g_log, "%s:AUTH_TYPE_REQ", __FUNCTION__);
      int length = 32 * sizeof(uint8_t);
      uint8_t *pOut = (uint8_t *)malloc(length);
      memset(pOut, 0, length);
      int nSize = AES_Encrypt(128, IOData + sizeof(st_AuthHead), 16,
                          (UCHAR *)pCamera->SecurityCode,
                          strlen(pCamera->SecurityCode), pOut);

      if (nSize > 0) {
        Camera_SendIOCtrl(pCamera,
                          AUTH_TYPE_RESP,
                          pOut, 16, SIO_TYPE_AUTH);
      }
      free(pOut);
      break;

      case AUTH_TYPE_OK:
    	if (TRIGGER_DEBUG)
    		printf("=====> AUTH_TYPE_OK\n");

        LOG_DEBUG(g_log, "%s:AUTH_TYPE_OK", __FUNCTION__);
        if (pstAuthHead->nAuthDataSize >= 16) {
          int nSize = AES_Decrypt(128, IOData + sizeof(st_AuthHead), 16,
                                  (UCHAR *)pCamera->SecurityCode,
                                  strlen(pCamera->SecurityCode),
                                  pCamera->AESKey);
          if (nSize > 0) {
            pCamera->bSessionKeyOk = true;
          }
        }
        pCamera->bAuthRespFromDev = true;
        int nRet = Camera_SendIOCtrlToggle(pCamera, true, (uint8_t)0, (uint8_t)0);
        LOG_DEBUG(g_log,
                  "%s:sendIOCtrl_on_off(get,.)=%d",
                  __FUNCTION__, nRet);
        pCamera->connState = CAMERA_CONN_CONNECTED;
        // chenjian
        // Camera_Start(pCamera);
//        Camera_manuRecStart(pCamera);

        break;

        case AUTH_TYPE_FAILED:
        if (TRIGGER_DEBUG)
        	printf("=====> AUTH_TYPE_FAILED\n");
          LOG_DEBUG(g_log, "%s:AUTH_TYPE_FAILED", __FUNCTION__);
          pCamera->p2pConnResult = TRIGGER_RECORDER_WRONG_PASSWROD;

          pCamera->bAuthRespFromDev = true;

          break;
  }
}


/*
 * 处理控制消息
 */
static __inline__
void Camera_DoIOCtrl(
    IN HCAMERA hCamera,
    IN uint8_t IOData[]) {
  // TODO
  assert(hCamera);
  assert(IOData);
  Camera *pCamera = (Camera *)hCamera;
  
  st_AVIOCtrlHead *pstIOCtrlHead = (st_AVIOCtrlHead *) IOData;
  
  LOG_DEBUG(g_log,
            "RecvIOCtrl DoIOCtrl(..) IOCtrlType=%d, pIODataSize=%d",
            pstIOCtrlHead->nIOCtrlType, pstIOCtrlHead->nIOCtrlDataSize);

  switch (pstIOCtrlHead->nIOCtrlType) {
  	  // 2014-10-21
  	  case IOCTRL_TYPE_EVENT_NOTIFY:
  		LOG_DEBUG(g_log, "get IOCTRL_TYPE_EVENT_NOTIFY");

  		char *data = NULL;
  		data = (char *)malloc(pstIOCtrlHead->nIOCtrlDataSize + 1);
  		memcpy((void *)data, (void *)IOData + 16, pstIOCtrlHead->nIOCtrlDataSize + 1);

  		int camIndex = (0xff & data[0]) | (0xff & data[1]) << 8
  				| (0xff & data[2]) << 16 | (0xff & data[3]) << 24;
  		int eventType = (0xff & data[4]) | (0xff & data[5]) << 8
  				| (0xff & data[6]) << 16 | (0xff & data[7]) << 24;
  		int eventTime = (0xff & data[16]) | (0xff & data[17]) << 8
  				| (0xff & data[18]) << 16 | (0xff & data[19]) << 24;

  		free(data);

  		LOG_DEBUG(g_log, "camIndex = %d, eventType = %d, eventTime = %d", camIndex,
  				eventType, eventTime);
  		int i = 0;
		int num = SIZEOF_ARRAY(pCamera->pfnAlarmProc);

		char msg[128] = { 0 };
		sprintf(msg, "%s;%d;%d;%d", pCamera->DevUID, camIndex, eventType, eventTime);
		int len = strlen(msg);

		while (i < num && pCamera->pfnAlarmProc[i]) {
			pCamera->pfnAlarmProc[i]((uint8_t *)pCamera->DevUID, msg, len, 0);
			i++;
		}

  		break;
    case IOCTRL_TYPE_PUSH_CamIndex:
      if (pstIOCtrlHead->nIOCtrlDataSize >= 4) {
        int nCamIndex = __cpu_to_le32(*(uint8_t*)IOData);
        LOG_DEBUG(g_log,
                  "%s:IOCTRL_TYPE_PUSH_CamIndex, nCamIndex=%d",
                  __FUNCTION__, nCamIndex);
      }
      break;

      case IOCTRL_TYPE_GET_VIDEO_PARAMETER_RESP:
        //if (pstIOCtrlHead->nIOCtrlDataSize >= sizeof(IOCTRLGetVideoParameterResp)) {
          //st_AVIOCtrlHead *pstIOCtrlHead =
              //(st_AVIOCtrlHead *)IOData + sizeof(st_AVIOCtrlHead);
        //}
        break;

      case IOCTRL_TYPE_RECORD_PLAYCONTROL_RESP:
    	  break;
		case IOCTRL_TYPE_DEVINFO_RESP:
		{
			//printf("[ IPC_handler] IOCTRL_TYPE_DEVINFO_RESP\n");
			bzero(pCamera->DevModeInfo, sizeof(pCamera->DevModeInfo));
			memcpy((void *)pCamera->DevModeInfo, (void *)IOData + 16, 16);	// Catch device model name

			//printf("[ IPC_handler] Devive Model_name   : %s(%d)\n", pCamera->DevModeInfo, strlen(pCamera->DevModeInfo));
			break;
		}
  }
}

  long nFirstTickLocal_video = 0L, nTick2_video = 0L, nFirstTimestampDevice_video = 0L;
  long nLastDevTimeStamp = 0L;
/*
 * 处理收到的视频数据
 */
static __inline__
void Camera_DoVideoData(
    IN Camera *pCamera,
    IN uint8_t AVData[],
    IN long mTick1,
    IN uint8_t out_4para[4],
    IN volatile bool *bFirstFrame_video,
    IN volatile bool *bProcess) {
  // TODO
  assert(pCamera);
  assert(AVData);
  assert(out_4para); // out_4para:0, 1, 2-width, 3-height 

  long tick2 = GetCurrentTick();
  if ((tick2 - mTick1) > 3000 || *bFirstFrame_video) {
    mTick1 = tick2;
  }


  st_AVFrameHead *pstFrameHead = (st_AVFrameHead *)AVData;
  int nFrameSize = pstFrameHead->nDataSize;
  
  switch(pstFrameHead->nCodecID) {
    case CODECID_V_H264:
    //if(nInitH264Encoder < 0) break;
      if(*bFirstFrame_video && pstFrameHead->flag != VFRAME_FLAG_I) break;
      *bFirstFrame_video = false;
      int consumed_bytes = 0;
      AVData = AVData + sizeof(st_AVFrameHead);
      while (nFrameSize > 0) {
        //consumed_bytes=H264Codec.H264Decode(out_bmp565, pAVData, nFrameSize, out_4para);
        //consumed_bytes = Encode(pCamera->hCodec, AVData, nFrameSize);
        if(consumed_bytes < 0) {
          nFrameSize = 0;
          break;
        }
        //  Encode(pCamera->hCodec, AVData, nFrameSize);
              
        if(!pCamera->bRunning || !(*bProcess)) break;


          long nCurDevTimeStamp = 0L, nDiffTimeStamp = 0L;
          nCurDevTimeStamp = pstFrameHead->nTimeStamp;
          nTick2_video = GetCurrentTick();
          if(nFirstTimestampDevice_video == 0L || nFirstTickLocal_video == 0L) {
            nFirstTimestampDevice_video = nCurDevTimeStamp;
            nFirstTickLocal_video = nTick2_video;
          }
          if(nTick2_video < nFirstTickLocal_video ||
             nCurDevTimeStamp < nFirstTimestampDevice_video) {
            nFirstTimestampDevice_video = nCurDevTimeStamp;
            nFirstTickLocal_video = nTick2_video;
          }
          if(pCamera->nAVDataChannel == CHANNEL_PLAYBACK_DtoC) {
            if(nCurDevTimeStamp == nLastDevTimeStamp) nCurDevTimeStamp += 45;
          }

          nLastDevTimeStamp = nCurDevTimeStamp;
          nDiffTimeStamp = (nCurDevTimeStamp - nFirstTimestampDevice_video)
              - (nTick2_video - nFirstTickLocal_video);
          
          LOG_DEBUG(g_log, "ThreadPlayVideo, nDiffTimeStamp=%ld", nDiffTimeStamp);
          if(nDiffTimeStamp < 3000) { //bugbug
            for(int kk = 0;kk < nDiffTimeStamp; kk++) {
              if(!pCamera->bRunning || !(*bProcess)) break;
              gs_sleep_ms(1);
            }
          }
        /* bytBuffer=ByteBuffer.wrap(out_bmp565, 0, bmpSizeInBytes);
         * bmpLast.copyPixelsFromBuffer(bytBuffer);
         * updateAVListenerVFrame(bmpLast);*/
         /* else {
         * System.out.println(" FrmFlag="+stFrameHead.getFlag()+",Dec_Fail nCurDevTimeStamp="+stFrameHead.getTimeStamp()+",nDiffCurDevTimeStamp="+
         * (stFrameHead.getTimeStamp()-m_nFirstTimestampDevice_video)+",nDiffLocalTimeStamp="+
         * (m_nTick2_video-m_nFirstTickLocal_video)+", getNum="+m_fifoVideo.getNum());
         */
        nFrameSize -= consumed_bytes;
        if(nFrameSize > 0) AVData = AVData + consumed_bytes; // 需要验证 test
        else nFrameSize = 0;
        LOG_DEBUG(g_log, "FrameThread, nFrameSize=%d", nFrameSize);

        if (!pCamera->bRunning || !(*bProcess)) break;
      } // end while (nFrameSize > 0)
      LOG_DEBUG(g_log,
                "FrameThread, bRunning=%s, bPacket=%s",
                (pCamera->bRunning ? "true" : "false"),
                (bProcess ? "true": "flase"));
      break;
  } // end switch(pstFrameHead->nCodecID)
}

/*
 * 加密 P2P 数据包
 */
void EncryptPacket(
    IN Camera *pCamera,
    IN uint8_t Data[],
    IN int     nDataSize) {
  // TODO
  assert(pCamera);
  assert(Data);
  
  int nHeadLen =  sizeof(st_AVIOCtrlHead) + sizeof(st_AVStreamIOHead);
  if (nDataSize < nHeadLen)
    return;

  st_AVIOCtrlHead *pstIOCtrlHead = NULL;
  st_AVFrameHead *pstFrameHead = NULL;
  st_AVStreamIOHead *pstStreamIOHead  = (st_AVStreamIOHead *) Data;

  switch(pstStreamIOHead->uionStreamIOHead.nStreamIOType) {
    case SIO_TYPE_IOCTRL:
      pstIOCtrlHead = (st_AVIOCtrlHead *) (Data + sizeof(st_AVStreamIOHead));
      if (pstIOCtrlHead->nIOCtrlDataSize > 0) {
        int index = 0;
        pstIOCtrlHead->XorKey = pCamera->nextXorKeyForIO;        // Xorkey
        pCamera->nextXorKeyForIO = (Data[index]) ^ pstIOCtrlHead->XorKey;
        for(index = nHeadLen;
            index < (pstIOCtrlHead->nIOCtrlDataSize + nHeadLen);
            index++) {
          Data[index] = Data[index] ^ pstIOCtrlHead->XorKey;
        }
      }
      break;
      
      case SIO_TYPE_AUDIO:
        pstFrameHead = (st_AVFrameHead *)Data + 4;
        if (pstFrameHead->nDataSize > 0) {
          int index= 0;
          pstFrameHead->XorKey = pCamera->nextXorKeyForIO; // XorKey
          pCamera->nextXorKeyForIO = Data[index] ^ pstFrameHead->XorKey;
          for (index = nHeadLen;
               index < (pstFrameHead->nDataSize + nHeadLen);
               index++) {
            Data[index] = Data[index] ^ pstFrameHead->XorKey;
          }
        }
        break;
  }
  uint8_t *pOut = (uint8_t *)malloc(32);
  int nSize = AES_Encrypt(128,
                          Data + sizeof(st_AVStreamIOHead),
                          16, pCamera->AESKey, 16, pOut);
  if (nSize > 0)
    memcpy(Data + sizeof(st_AVStreamIOHead), pOut, 16);
  free(pOut);
}

/*
 * 解密 P2P 数据包
 */
int DecryptPacket(
    IN Camera *pCamera,
    IN int nStreamIOType,
    IN OUT uint8_t Data[],
    IN int      nDataSize) {
  // TODO
  assert(pCamera);
  assert(Data);
  
  uint8_t *pOut = NULL;
  
  switch (nStreamIOType) {
    case SIO_TYPE_AUDIO:
    case SIO_TYPE_VIDEO:
      if (nDataSize < sizeof(st_AVFrameHead))
        return -2;
      
      pOut = (uint8_t *) malloc(sizeof(st_AVFrameHead));
      memset(pOut, 0, sizeof(st_AVFrameHead));

      int nSize = AES_Decrypt(128, Data, 16,
                  pCamera->AESKey, 16, pOut);
      if (nSize <= 0)   goto error;

      memcpy(Data, pOut, sizeof(st_AVFrameHead));
      st_AVFrameHead *pstFrameHead = (st_AVFrameHead *)pOut;
      int index = 0;
      uint8_t XorKey = pstFrameHead->XorKey;
      if ( (pstFrameHead->nDataSize + sizeof(st_AVFrameHead)) == nDataSize) {
        for (index = sizeof(st_AVFrameHead); index < nDataSize; index++) {
          Data[index] = (uint8_t)(Data[index] ^ XorKey);
        }
      } else goto error;
      free(pOut);
      break;

    case SIO_TYPE_IOCTRL:
		if (nDataSize < sizeof(st_AVIOCtrlHead))
			return -2;

		pOut = (uint8_t *) malloc(sizeof(st_AVIOCtrlHead));
		memset(pOut, 0, sizeof(st_AVIOCtrlHead));

		nSize = AES_Decrypt(128, Data, 16, pCamera->AESKey, 16, pOut);
		if (nSize <= 0)
			goto error;

		memcpy(Data, pOut, sizeof(st_AVIOCtrlHead));
		st_AVIOCtrlHead *pstCtrlHead = (st_AVIOCtrlHead *) pOut;
		XorKey = pstCtrlHead->XorKey;
		if ((pstCtrlHead->nIOCtrlDataSize + sizeof(st_AVIOCtrlHead))
				== nDataSize) {
			for (index = sizeof(st_AVIOCtrlHead); index < nDataSize; index++) {
				Data[index] = (uint8_t) (Data[index] ^ XorKey);
			}
		} else
			goto error;
		free(pOut);
		break;
  }
  
  return 0;
error:
  free(pOut);
  return -2;        
}

/*
 * 在线程中读取 P2P 数据
 * ret = ERROR_PPPP_TIME_OUT,nRecvSize = -2
 */
static __inline__
int  readDataFromRemote (
    IN Camera* const pCamera,
    IN uint8_t       Channel,
    IN uint8_t       *pData,
    IN int           nSize,
    IN int           timeout_ms,
    IN int           nBufferSize,
    IN volatile bool *bRecving)
{
  // TODO
  assert(pCamera);
  assert(bRecving);
  
  int nRet = -1;
  int nReadSize = 0;
  int nExitCount = 0;
  int nTotalRead = 0;
  
  while (nTotalRead < nSize) {
    nReadSize = nSize - nTotalRead;

    uint8_t *pIOData = (uint8_t *)malloc(nBufferSize);
    nRet = PPPP_Read(pCamera->hSession,
                      Channel,
                      (CHAR*)pIOData,
                      &nReadSize, timeout_ms);
    memcpy(pData + nTotalRead, pIOData, nReadSize);
    free(pIOData);
    
    nTotalRead += nReadSize;

    if (*bRecving) {
      if (nRet == ERROR_PPPP_TIME_OUT)
        continue;
      else      break;
    } else {
      if (nRet == ERROR_PPPP_SUCCESSFUL) {
        LOG_DEBUG(g_log, "1 nExitCount=%d", nExitCount);
        nExitCount = 0;
      } else nExitCount++;

      if (nExitCount >= 2) {
        LOG_DEBUG(g_log, "1 nExitCount=%d", nExitCount);
        nRet = ERROR_PPPP_TIME_OUT;
        break;
      }
    }
  }
  return nRet;  
}

/*
 * 接受 控制指令 线程
 */
static void* RecvIOCtrlThread(IN void* args) {

	if (TRIGGER_DEBUG)
		printf("=====> RecvIOCtrlThread\n");

  Camera *pCamera = (Camera *)args;
  assert(pCamera);
  
  pCamera->bRunning = true;
  LOG_DEBUG(g_log, "----ThreadRecvIOCtrl going...");
  volatile bool bRecvingIO = true;

  int nRet = 0;
  int nRecvSize = 0;
  int nCurStreamIOType = 0;
  st_AVStreamIOHead *pStreamIOHead = NULL;
//  uint8_t *pIOData = (uint8_t *)malloc(MAX_SIZE_IOCTRL_BUF);

  uint8_t pIOData[MAX_SIZE_IOCTRL_BUF + 1] = { 0 };

  do {
    LOG_DEBUG(g_log, "----ThreadRecvIOCtrl looping 1...");
    memset(pIOData, 0, MAX_SIZE_IOCTRL_BUF + 1);
    
    nRecvSize = sizeof(st_AVStreamIOHead);
    
    nRet = readDataFromRemote(pCamera, CHANNEL_IOCTRL,
                              pIOData, nRecvSize, 500,
                              MAX_SIZE_IOCTRL_BUF, &bRecvingIO);

    if (TRIGGER_DEBUG)
    {
    	printf("1111 nRet = %d\n", nRet);
    	printf("1111 pCamera->p2pConnResult = %d\n", pCamera->p2pConnResult);
    }
    // chenjian
    if (0 == pCamera->p2pConnResult || TRIGGER_RECORDER_DEFAULT == pCamera->p2pConnResult)
    	pCamera->p2pConnResult = nRet;

    if (nRet == ERROR_PPPP_TIME_OUT)
      nRecvSize = -2;
    
    LOG_DEBUG(g_log, "----ThreadRecvIOCtrl looping 1...");
    LOG_DEBUG(g_log,
              "RecvIOCtrl Recv1 AVStreamIOHead: nRet=%d pIOData[0]-[3]=%d,%d,%d,%d",
              nRet,  pIOData[0],  pIOData[1],  pIOData[2],  pIOData[3]);

    LOG_DEBUG(g_log, "%s:Session %s!, %d", __FUNCTION__, P2PErrTab[-nRet], nRet);

    if (nRecvSize > 0) {
      pStreamIOHead = (st_AVStreamIOHead *)pIOData;
      nCurStreamIOType = pStreamIOHead->uionStreamIOHead.nStreamIOType;
      nRecvSize = (pStreamIOHead->uionStreamIOHead.nDataSize[2] & 0xFF) << 16
          | (pStreamIOHead->uionStreamIOHead.nDataSize[1] & 0xFF) << 8
          | (pStreamIOHead->uionStreamIOHead.nDataSize[0] & 0xFF);
      if (nRecvSize > MAX_SIZE_IOCTRL_BUF) {
        LOG_DEBUG(g_log,
                  "====ThreadRecvIOCtrl, 1 nRecvSize>1*1024, nCurStreamIOType=%d",
                  nCurStreamIOType);
        break;
      }


      nRet = readDataFromRemote(pCamera, CHANNEL_IOCTRL,
                                pIOData, nRecvSize, 500,
                                MAX_SIZE_IOCTRL_BUF, &bRecvingIO);
      if (nRet == ERROR_PPPP_TIME_OUT)
        nRecvSize = -2;
      
      LOG_DEBUG(g_log,
                "RecvIOCtrl Recv2 I/O Ctrl: nRet=%d nCurStreamIOType=%d nRecvSize=%d",
                nRet, nCurStreamIOType, nRecvSize);
      LOG_DEBUG(g_log, "%s:Session %s!, %d", __FUNCTION__, P2PErrTab[-nRet], nRet);

      // chenjian
      if (TRIGGER_DEBUG)
      {
    	  printf("2222 nRet = %d\n", nRet);
    	  printf("2222 pCamera->p2pConnResult = %d\n\n", pCamera->p2pConnResult);
      }
      if (0 == pCamera->p2pConnResult || TRIGGER_RECORDER_DEFAULT == pCamera->p2pConnResult)
    	  pCamera->p2pConnResult = nRet;

      if (nRecvSize > 0) {
        if (nRecvSize > MAX_SIZE_IOCTRL_BUF)
        {
          LOG_DEBUG(g_log,
                    "====ThreadRecvIOCtrl, 2 nRecvSize>1*1024, nCurStreamIOType=%d",
                    nCurStreamIOType);
          break;
        }
        else
        {
          if (nCurStreamIOType == SIO_TYPE_AUTH) {
            Camera_DoAuth(pCamera, pIOData);
          }
          else
          {
            if (pCamera->enableAES) {
               int n =  DecryptPacket(pCamera, nCurStreamIOType, pIOData, nRecvSize);
               LOG_DEBUG(g_log, "ThreadRecvIOCtrl DecryptPacket n=");
               if (n < 0)
               {
                  LOG_DEBUG(g_log, "ThreadRecvIOCtrl, n = %d", n);
               }
               else
               {
            	  Camera_DoIOCtrl(pCamera, pIOData);
               }
            }
            else {
              Camera_DoIOCtrl(pCamera, pIOData);
            }
          }
        }
      }
    }
    gs_sleep_ms(200);
  } while(pCamera->bRunning && bRecvingIO);

  LOG_DEBUG(g_log, "===ThreadRecvIOCtrl exit.");
  return 0;
}

/*
 * 接受视/声频数据线程
 */
static void* RecvAVDataThread(IN void* args) {
  // TODO
  
  Camera *pCamera = (Camera *)args;
  assert(pCamera);

  int           nRet = 0;
  int           nCurStreamIOType = 0;
  int           nRecvSize = 0;
  long          nExitTickSpan = 0;
  long          nExitTick0 = GetCurrentTick();
  volatile bool bRecving = true;
  st_AVStreamIOHead *pstStreamIOHead = NULL;
  st_AVFrameHead    *pstFrameHead = NULL;
  uint8_t *pAVData = (uint8_t *) malloc(MAX_SIZE_BUF * sizeof(uint8_t));

  LOG_DEBUG(g_log, "----ThreadRecvAVData going...");

  while (pCamera->bRunning) {
    LOG_DEBUG(g_log,
              "----ThreadRecvAVData going... 1, m_nAVDataChannel: %d",
              pCamera->nAVDataChannel);
    if (pCamera->nAVDataChannel == CHANNEL_PLAYBACK_DtoC
        && FIFO_getNum(&pCamera->fifoVideo) >= 50) {
      // playback
      LOG_DEBUG(g_log, "ThreadRecvAVData, CHANNEL_PLAYBACK_DtoC");
      gs_sleep_ms(45);
      continue;
    }
    
    nRecvSize = sizeof(st_AVStreamIOHead);
    nRet = readDataFromRemote(pCamera, pCamera->nAVDataChannel,
                              pAVData, nRecvSize, 500,
                              MAX_SIZE_BUF, &bRecving);
    if (nRet == ERROR_PPPP_TIME_OUT)
      nRecvSize = -2;

    if (nRecvSize == -1)
      break;// for playback

    LOG_DEBUG(g_log, "%s(%d):Session %s!, %d,%d", __FUNCTION__, __LINE__, P2PErrTab[-nRet], nRet, pCamera->hSession);

    if (nRecvSize > 0) {
      pstStreamIOHead = (st_AVStreamIOHead *) pAVData;
      nCurStreamIOType = pstStreamIOHead->uionStreamIOHead.nStreamIOType;
      nRecvSize = (pstStreamIOHead->uionStreamIOHead.nDataSize[2] & 0xFF) << 16
          | (pstStreamIOHead->uionStreamIOHead.nDataSize[1] & 0xFF) << 8
          | (pstStreamIOHead->uionStreamIOHead.nDataSize[0] & 0xFF);


      nRet = readDataFromRemote(pCamera, pCamera->nAVDataChannel, pAVData, nRecvSize, 500, MAX_SIZE_BUF, &bRecving);
      if (nRet == ERROR_PPPP_TIME_OUT)
        nRecvSize = -2;

      if (nRecvSize == -1)
        break;// for playback
      
      LOG_DEBUG(g_log,
               "%s(%d):Session %s!, %d",
               __FUNCTION__, __LINE__,  P2PErrTab[-nRet], nRet);

      if (nRecvSize > 0 && pCamera->bRunning && bRecving) {
        if (pCamera->enableAES) {
          DecryptPacket(pCamera,
                        nCurStreamIOType,
                        pAVData,
                        nRecvSize); // 解密 P2P 数据帧  
        }
        LOG_DEBUG(g_log,
                  "====ThreadRecvAVData, nCurStreamIOType:%d",
                  nCurStreamIOType);
        if (nRecvSize > MAX_SIZE_BUF) {
          pCamera->nConnInfo = CONN_INFO_SESSION_CLOSED;
          LOG_DEBUG(g_log,
                    "====ThreadRecvAVData, nRecvSize>256*1024, nCurStreamIOType=%d",
                    nCurStreamIOType);
          break;
        } else {
          if (nCurStreamIOType == SIO_TYPE_AUDIO) {
            FIFO_addLast(&pCamera->fifoAudio, nCurStreamIOType, pAVData, nRecvSize);
            LOG_DEBUG(g_log, "Audio:ThreadRecvAVData: Recvsize=%d", nRecvSize);
            pstFrameHead = (st_AVFrameHead *) pAVData;
            LOG_DEBUG(g_log, "Audio getTimeStamp()=%u", pstFrameHead->nTimeStamp);
          }
          if (nCurStreamIOType == SIO_TYPE_VIDEO) {
            FIFO_addLast(&pCamera->fifoVideo, nCurStreamIOType, pAVData, nRecvSize);
            LOG_DEBUG(g_log, "Video:ThreadRecvAVData: Recvsize=%d", nRecvSize);
            pstFrameHead = (st_AVFrameHead *) pAVData;
            LOG_DEBUG(g_log, "Video getTimeStamp()=%u", pstFrameHead->nTimeStamp);
          }
        }
      }
    }
    int WriteSize = 0;
    int ReadSize = 0;
    if (!bRecving && nRet < 0) {
      nRet = PPPP_Check_Buffer(pCamera->hSession,
                                pCamera->nAVDataChannel,
                                (UINT32 *)&WriteSize, (UINT32 *)&ReadSize);
      LOG_DEBUG(g_log,
                "ThreadRecvAVData: PPPP_Check_Buffer] ReadSize[0]=%d, nRet=%d",
                ReadSize, nRet);
      LOG_DEBUG(g_log, "----ThreadRecvAVData going... ");
      break;
    }
    if (!bRecving) {
      nExitTickSpan = GetCurrentTick() - nExitTick0;
      LOG_DEBUG(g_log,
                "....ThreadRecvAVData  nExitTickSpan=%ld",
                nExitTickSpan);
      if (nExitTickSpan >= 3500
          && pCamera->nAVDataChannel != CHANNEL_PLAYBACK_DtoC) {
        PPPP_ForceClose(pCamera->hSession);
        LOG_DEBUG(g_log, "....ThreadRecvAVData  ForceClose. channel=playback");
        break;
      }
      if (nExitTickSpan >= 3500
          && pCamera->nAVDataChannel != CHANNEL_DOWNLOAD_DtoC) {
        PPPP_ForceClose(pCamera->hSession);
        LOG_DEBUG(g_log, "....ThreadRecvAVData  ForceClose. channel=download");
        break;
      }
    }
    gs_sleep_ms(4);
  }
  free(pAVData);
  LOG_DEBUG(g_log,"===ThreadRecvAVData exit.");
  return 0;
}

/*
 * 处理视/声频线程
 */
static void* FrameThread(IN void* args) {
  // TODO
  Camera *pCamera = (Camera *)args;
  assert(pCamera);

  long mTick1 = 0L;
  uint8_t *videoData = NULL;
  uint8_t out_4para[4] = "\0";
  volatile bool bProcess = false;
  volatile bool bFirstFrame_video = true;

  LOG_DEBUG(g_log, "----FrameThread going...");
  gs_mutex_lock(&pCamera->hMutex);
  
  bProcess = true;
  
  while (pCamera->bRunning && bProcess) {
    int nSize = FIFO_getHeadSize(&pCamera->fifoVideo);
    
    if (mTick1 == 0L) mTick1 = GetCurrentTick();

    if (nSize) {
      videoData = (uint8_t *)malloc(sizeof(uint8_t) * nSize);
      
      bool bRet = FIFO_removeHead(&pCamera->fifoVideo, videoData);
      if (bRet) {
      Camera_DoVideoData(pCamera, videoData,
                           mTick1, out_4para,
                           &bFirstFrame_video, &bProcess);
      }
    free(videoData);
    }
    gs_sleep_ms(4);
  }
  
  LOG_DEBUG(g_log, "===FrameThread exit. H264Codec");
  gs_mutex_unlock(&pCamera->hMutex);
  return 0;
}

//获取时间系数(目前没用)
static __inline__
int getTimeCoefficient(int size, int COE_BIG, int COE_SMALL) {
  // TODO
  int coef = 1000;
  if(size > COE_BIG) coef = 1000 - (size - COE_BIG) * 100;
  else if(size < COE_SMALL) coef = 1000 + (COE_SMALL-size) * 1000;

  return (coef > 0) ? coef : 0;
}

void Camera_GetDeveveInfo(IN Camera *pCamera, ENUM_CONNECT_STATUS status)
{
	//printf("[ IPC_handler] Camera_GetDeveveInfo\n");

	int nRet = 0;
	int nCount = 0;
	int nConnectStatus = 0;

	// Check connect staus
	while(status == CONN_INFO_FAILED || status == CONN_INFO_AUTHFAILED)	
	{
		if(++nCount < 10)
		{
			sleep(1);
			continue;
		}

		printf("[ IPC_handler] Connect %s fail, exit\n", pCamera->DevUID);
		return;
	}
	 
	printf("[ IPC_handler] %s Connect success, send IOCTRL_TYPE_DEVINFO_REQ\n", pCamera->DevUID);

	// Send p2p command to obtain device infomation
	st_AVIOCtrlHead IOData;

	memset(&IOData, 0, sizeof(st_AVIOCtrlHead));
	IOData.nIOCtrlType = IOCTRL_TYPE_DEVINFO_REQ;
	IOData.nIOCtrlDataSize = 8;
	nRet = Camera_SendIOCtrl(pCamera, IOCTRL_TYPE_DEVINFO_REQ, (uint8_t*)&IOData, sizeof(st_AVIOCtrlHead), SIO_TYPE_IOCTRL);

	if(nRet < 0)
	{
		LOG_ERROR(g_log, "GetDeveveInfo fail.");
	}
}

int Camera_ManuRecStart(IN Camera *pCamera) {
	assert(pCamera);
	LOG_WARN(g_log, "[ IPC_handler] Camera_ManuRecStart: %s", pCamera->DevUID);

	int ret = 1;
	int nRecChannel = 0;
	int nMaxRecChannel = 1;	// Default 1
	IOCTRLAVStream IOData;

	memset(&IOData, 0, sizeof(IOCTRLAVStream));

	// Mapping device information
	do
	{
		int n;
		int nResult;
		int nSize = sizeof(stDevProductType) / sizeof(DEV_INFO);

		// Unknow device info, use defalut value
		if(pCamera->DevModeInfo[0] == '\0')
		{
			break;
		}

		for(n=1; n<nSize; n++)
		{
			nResult = strncmp(stDevProductType[n].devPref, pCamera->DevModeInfo, strlen(stDevProductType[n].devPref));
			if(nResult == 0)
			{
				break;
			}
		}
		// For DWH series, trigger 4 channel recording
		nMaxRecChannel = (stDevProductType[n].devMode == DEV_MODE_DWH) ? 4 : 1;
		//printf("result: %d %d\n", stDevProductType[n].devMode, nMaxRecChannel);
	} while(0);

	for(nRecChannel=0; nRecChannel<nMaxRecChannel; nRecChannel++)
	{
		IOData.channel = nRecChannel;

		printf("[ IPC_handler] Send recording channel %d\n", IOData.channel);
		ret = Camera_SendIOCtrl(
			pCamera,
			IOCTRL_TYPE_MANU_REC_START,
			(uint8_t*)&IOData,
			sizeof(IOCTRLAVStream),
			SIO_TYPE_IOCTRL);

		gs_sleep_ms(50);
	}

	LOG_DEBUG(g_log, "ret = %d", ret);
	return ret;
}

int Camera_MoveToPresetPoint(IN Camera *pCamera, int presetPoint) {
	assert(pCamera);
	LOG_WARN(g_log, "[ IPC_handler] Camera_MoveToPresetPoint: %s", pCamera->DevUID);

	int ret = 1;
	int nRecChannel = 0;
	int nMaxRecChannel = 1;	// Default 1

	// Mapping device information
	do
	{
		int n;
		int nResult;
		int nSize = sizeof(stDevProductType) / sizeof(DEV_INFO);

		// Unknow device info, use defalut value
		if(pCamera->DevModeInfo[0] == '\0')
		{
			break;
		}

		for(n=1; n<nSize; n++)
		{
			nResult = strncmp(stDevProductType[n].devPref, pCamera->DevModeInfo, strlen(stDevProductType[n].devPref));
			if(nResult == 0)
			{
				break;
			}
		}
		// For DWH series, trigger 4 channel recording
		nMaxRecChannel = (stDevProductType[n].devMode == DEV_MODE_DWH) ? 4 : 1;
		//printf("result: %d %d\n", stDevProductType[n].devMode, nMaxRecChannel);
	} while(0);

	for(nRecChannel=0; nRecChannel<nMaxRecChannel; nRecChannel++)
	{
		char data[8] = {0};

		data[0] = IOCTRL_PTZ_PRESET_POINT;
		data[1] = presetPoint;

		ret = Camera_SendIOCtrl(
			pCamera,
			IOCTRL_TYPE_PTZ_COMMAND,
			(uint8_t*)&data,
			sizeof(data),
			SIO_TYPE_IOCTRL);

		gs_sleep_ms(50);
	}

	LOG_DEBUG(g_log, "ret = %d", ret);
	return ret;
}


/******************************* END OF FILE *********************************/
