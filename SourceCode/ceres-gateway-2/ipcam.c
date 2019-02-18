#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <pthread.h>
//#include <crypt.h>
//#include "ceres.h"
//#include "jsw_protocol.h"
//#include "AES/AES_utils.h"
//#include "PPPP/PPPP_API.h"
//#include <signal.h>

int test()
{
	return 1;
}

//#include "ipcam.h"

//typedef struct _jswproto_devtree
//{		
//	unsigned int did;
//
//	char name[16];
//	char location[32];
//	char ipcdid[32];
//	char ipcpwd[16];	
//
//	short model;
//	unsigned short status;
//	unsigned short flag;
//	unsigned short ext1;
//
//}jswdev;
//
//struct camobject{
//	unsigned int did;
//	char ipcdid[32];
//	char ipcpwd[16];	
//	unsigned char AESKey[32];  
//	pthread_t handle;
//	int connState;
//	int session;
//	int enableAES;
//};
//
//void *ipcam_thread(void *arg);
//void Camera_DoIOCtrl();
//
//struct camobject camlist[MAXIPCAM];
//int camcount=0;
//
//int initipcam()
//{
//	int x;
//	for (  x=0;x<devcount;x++)
//	{
//		if (devlist[x].model != TYPE_CAMERA )
//			continue;
//
//		memset( &camlist[camcount], 0 , sizeof(struct camobject) );
//		camlist[camcount].did =devlist[x].did; 
//		camlist[camcount].session = -1;
//		camlist[camcount].enableAES = 1;
//		strcpy(camlist[camcount].ipcdid, devlist[x].ipcdid);
//		strcpy(camlist[camcount].ipcpwd, devlist[x].ipcpwd);
//
//		//pthread_create(&camlist[camcount].handle, NULL, ipcam_thread, (void*) &camlist[camcount]);
//
//		camcount++;			
//	}
//
//}
//
//
//int readDataFromRemote(  int session,    uint8_t Channel,    uint8_t *pData,    int nSize,    int timeout_ms,     int nBufferSize)
//{  
//  int nRet = -1;
//  int nReadSize = 0;
//  int nExitCount = 0;
//  int nTotalRead = 0;
//  
//  while (nTotalRead < nSize) {
//    nReadSize = nSize - nTotalRead;
//
//    nRet = PPPP_Read(session,
//                      Channel,
//                      pData + nTotalRead, //(CHAR*)pIOData,
//                      &nReadSize, timeout_ms);
//    
//    nTotalRead += nReadSize;
//	if (nRet == ERROR_PPPP_TIME_OUT)
//        continue;
//   
//	if (nRet == ERROR_PPPP_SUCCESSFUL) {
//	// LOG_DEBUG(g_log, "1 nExitCount=%d", nExitCount);
//		nExitCount = 0;
//	} else 
//	  nExitCount++;
//
//	if (nExitCount > 3) {
//		//printf("[ Camera ] Read from ipcam exit nExitCount=%d", nExitCount);
//		nRet = ERROR_PPPP_TIME_OUT;
//		break;
//	}
//    
//  }
//  return nRet;  
//}
//
//int Camera_SendIOCtrl(
//    int session,
//    int     nIOCtrlType,
//    char *IOData, //uint8_t IOData[],
//    int     nIODataSize,
//    int     nStreamIOType) {
//  
//  if (nIODataSize < 0 || nIOCtrlType < 0) {
//   // LOG_DEBUG(g_log, "Camera NULL");
//    return 0;
//  }  
//
//  //if (nStreamIOType != SIO_TYPE_AUTH && !pCamera->bSessionKeyOk) {
//  //  LOG_DEBUG(g_log, "Camera Wrong password!");
//  //  return ERROR_CAMERA_WRONG_PWD;
//  //}  
//  
//	int nSize = sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead) + nIODataSize;
//	unsigned char packet[2046];
//	memset(packet, 0, sizeof(packet) );
//
//	uint32_t packetsize = sizeof(st_AVIOCtrlHead) + nIODataSize;
//	packetsize = __cpu_to_le32(packetsize);
//	packet[0] = (uint8_t) packetsize;
//	packet[1] = (uint8_t) (packetsize >> 8);
//
//	packet[3] = (uint8_t) nStreamIOType;
//	packet[4] = (uint8_t) nIOCtrlType;
//	packet[5] = (uint8_t) (nIOCtrlType >> 8);
//
//	if (IOData != NULL) {
//		packet[6] = (uint8_t) nIODataSize;
//		packet[7] = (uint8_t)(nIODataSize >> 8);
//		memcpy(packet + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),   IOData, nIODataSize);
//	}
// // if (pCamera->enableAES) {
//    if (nStreamIOType != SIO_TYPE_AUTH)
//      EncryptPacket(pCamera, packet, nSize);             // 加密
// // }
//
//  int nRet = PPPP_Write(session, CHANNEL_IOCTRL, (CHAR *)packet, nSize);
//  //LOG_DEBUG(g_log, "sendIOCtrl(PPPP_Write) nRet=%d"
//  //          ", sessionHandle=%d"
//  //          ", nStreamIOType=%d"
//  //          ", nIOCtrlType=%d",
//  //          nRet, pCamera->hSession, nStreamIOType, nIOCtrlType);
//  
//  return nRet;
//  
//}

//
//
//int Camera_DoAuth( struct camobject *cam,char *pwd, char * IOData) 
//{
//
//	char *AES_KEY_DEFAULT = "0123456789ABCDEF";
//	st_AuthHead *pstAuthHead = (st_AuthHead *)IOData;
//
//	switch (pstAuthHead->AuthType) {
//
//		case AUTH_TYPE_REQ:
//
//		  int length = 32 * sizeof(uint8_t);
//		  uint8_t pOut[32]; // = (uint8_t *)malloc(length);
//		  memset(pOut, 0, length);
//		  int nSize = AES_Encrypt(128, IOData + sizeof(st_AuthHead), 16,
//							  (UCHAR *)pwd,
//							  strlen(pwd), pOut);
//		  if (nSize > 0) {
//				Camera_SendIOCtrl(cam->session,
//							  AUTH_TYPE_RESP,
//							  pOut, 16, SIO_TYPE_AUTH);
//			}
//			break;
//
//      case AUTH_TYPE_OK:
//
//        if (pstAuthHead->nAuthDataSize >= 16) {
//          int nSize = AES_Decrypt(128, IOData + sizeof(st_AuthHead), 16,
//                                  (UCHAR *)pwd,
//                                  strlen(pwd),
//                                  cam->AESKey);
//          if (nSize <= 0) {
//            return 0;//error
//          }
//        }
//       // int nRet = Camera_SendIOCtrlToggle(pCamera, true, (uint8_t)0, (uint8_t)0);
//        //LOG_DEBUG(g_log,
//        //          "%s:sendIOCtrl_on_off(get,.)=%d",
//        //          __FUNCTION__, nRet);
//
//        cam->connState = 1;
//        // chenjian
//        // Camera_Start(pCamera);
////        Camera_manuRecStart(pCamera);
//
//        break;
//
//        case AUTH_TYPE_FAILED:
//        	printf("[ Camera ] AUTH_TYPE_FAILED\n");
//			return 0;
//          break;
//  }
//	return 1;
//}
//
//int DecryptPacket(
//    struct camobject *pCamera,
//      int nStreamIOType,
//      uint8_t Data[],
//     int      nDataSize) {
//
//  
//  //uint8_t *pOut = NULL;
//  uint8_t pOut[512];
//  memset(pOut, 0 , sizeof(pOut));
//  
//  switch (nStreamIOType) {
//    case SIO_TYPE_AUDIO:
//    case SIO_TYPE_VIDEO:
//      if (nDataSize < sizeof(st_AVFrameHead))
//        return -2;
//
//      int nSize = AES_Decrypt(128, Data, 16,
//                  pCamera->AESKey, 16, pOut);
//      if (nSize <= 0)
//		  goto error;
//
//      memcpy(Data, pOut, sizeof(st_AVFrameHead));
//      st_AVFrameHead *pstFrameHead = (st_AVFrameHead *)pOut;
//      int index = 0;
//      uint8_t XorKey = pstFrameHead->XorKey;
//      if ( (pstFrameHead->nDataSize + sizeof(st_AVFrameHead)) == nDataSize) {
//        for (index = sizeof(st_AVFrameHead); index < nDataSize; index++) {
//          Data[index] = (uint8_t)(Data[index] ^ XorKey);
//        }
//      } else
//		  goto error;
//      //free(pOut);
//      break;
//
//    case SIO_TYPE_IOCTRL:
//		if (nDataSize < sizeof(st_AVIOCtrlHead))
//			return -2;
//
//		//pOut = (uint8_t *) malloc(sizeof(st_AVIOCtrlHead));
//		//memset(pOut, 0, sizeof(st_AVIOCtrlHead));
//
//		nSize = AES_Decrypt(128, Data, 16, pCamera->AESKey, 16, pOut);
//		if (nSize <= 0)
//			goto error;
//
//		memcpy(Data, pOut, sizeof(st_AVIOCtrlHead));
//		st_AVIOCtrlHead *pstCtrlHead = (st_AVIOCtrlHead *) pOut;
//		XorKey = pstCtrlHead->XorKey;
//		if ((pstCtrlHead->nIOCtrlDataSize + sizeof(st_AVIOCtrlHead))
//				== nDataSize) {
//			for (index = sizeof(st_AVIOCtrlHead); index < nDataSize; index++) {
//				Data[index] = (uint8_t) (Data[index] ^ XorKey);
//			}
//		} else
//			goto error;
//		//free(pOut);
//		break;
//  }
//  
//  return 0;
//error:
//  //free(pOut);
//  return -2;        
//}
// 
//void *ipcam_thread(void *arg) {
//	struct camobject* cam = (struct camobject*) arg;
//
//  //memcpy(pCamera, &defaultVal, sizeof(Camera));
//  //strcpy(pCamera->DevUID, DevUID);
//  //pCamera->port = Port;
//  //pCamera->enableLan = EnableLan;
//  //if(SecurityCode)
//  //  strcpy(pCamera->SecurityCode, SecurityCode);
//  //
//  int recont =0;
//
//  while (cam->session < 0)
//  {
//    int EnableLan =1;
//	int Port = 0;
//    cam->session = PPPP_Connect(cam->ipcdid, EnableLan, Port);
//    if ( cam->session < 0)
//    {
//        
//      printf("[Camere] DID:%s,Session code%d",
//               cam->ipcdid,
//               cam->session);
//      recont++;
//	  usleep(2000 * 1000);
//    }
//	
//  }
//
//  // st_PPPP_Session SInfo;
//  //if (ERROR_PPPP_SUCCESSFUL == PPPP_Check(cam->session, &SInfo)) {
//  // 
//  //}
// 
//
//  int nRet = 0;
//  int nRecvSize = 0;
//  int nCurStreamIOType = 0;
//  st_AVStreamIOHead *pStreamIOHead = NULL;
//  uint8_t pIOData[2046] = { 0 };
//
//  while (1) {
//		memset(pIOData, 0, sizeof(pIOData));    
//		nRecvSize = sizeof(st_AVStreamIOHead);    
//		nRet = readDataFromRemote(cam->session , CHANNEL_IOCTRL,
//								  pIOData, nRecvSize, 500,
//								  sizeof(pIOData));
//
//		if (ERROR_PPPP_TIME_OUT == nRet)
//			continue;
//		if (ERROR_PPPP_SUCCESSFUL != nRet) 
//		{//disconnect 
//			printf("[ Camera ] IPC disconnect\n");
//			cam->session = -1;
//			return;
//		}
//
//		pStreamIOHead = (st_AVStreamIOHead *)pIOData;
//		nCurStreamIOType = pStreamIOHead->uionStreamIOHead.nStreamIOType;
//		nRecvSize = (pStreamIOHead->uionStreamIOHead.nDataSize[2] & 0xFF) << 16
//		  | (pStreamIOHead->uionStreamIOHead.nDataSize[1] & 0xFF) << 8
//		  | (pStreamIOHead->uionStreamIOHead.nDataSize[0] & 0xFF);
//
//        nRet = readDataFromRemote(pCamera, CHANNEL_IOCTRL,
//                                pIOData, nRecvSize, 500,
//                                sizeof(pIOData));
//
//
//		if (ERROR_PPPP_TIME_OUT == nRet)
//			continue;
//		if (ERROR_PPPP_SUCCESSFUL != nRet) 
//		{//disconnect 
//			printf("[ Camera ] IPC disconnect\n");
//			cam->session = -1;
//			break; //disconnect
//		}
//     
//          if (nCurStreamIOType == SIO_TYPE_AUTH) {
//             int isok = Camera_DoAuth(cam, pIOData);
//			 if (isok ==0)
//				 break;
//          }
//          else
//          {
//            if (cam->enableAES) {
//               int n =  DecryptPacket(pCamera, nCurStreamIOType, pIOData, nRecvSize);
//               if (n < 0)
//                  printf("[ Camera ] Decrypt fail\n");
//               else
//            	  Camera_DoIOCtrl(pCamera, pIOData);
//               
//            }
//            else {
//              Camera_DoIOCtrl(pCamera, pIOData);
//            }
//         }
//        
//      
//    }//while
//   // gs_sleep_ms(200);
//
//  //do reconnect here
//
//
//  printf("camera thread exit.\n");
//  return 0;
//
//}
//
//
//void Camera_DoIOCtrl(struct camobject* cam,    uint8_t IOData[])
//{
//
//  
//  st_AVIOCtrlHead *pstIOCtrlHead = (st_AVIOCtrlHead *) IOData;
//  
//  //LOG_DEBUG(g_log,
//  //          "RecvIOCtrl DoIOCtrl(..) IOCtrlType=%d, pIODataSize=%d",
//  //          pstIOCtrlHead->nIOCtrlType, pstIOCtrlHead->nIOCtrlDataSize);
//
//  char data[1024];
//  switch (pstIOCtrlHead->nIOCtrlType) {
//  	  // 2014-10-21
//  	  case IOCTRL_TYPE_EVENT_NOTIFY:
//  		//LOG_DEBUG(g_log, "get IOCTRL_TYPE_EVENT_NOTIFY");
//
//  		//char *data = NULL;
//  	//	data = (char *)malloc(pstIOCtrlHead->nIOCtrlDataSize + 1);
//		memset(data,0,sizeof(data));
//		if (pstIOCtrlHead->nIOCtrlDataSize + 1 > 1024)
//		{
//			//LOG_DEBUG(g_log, "IOCTRL_TYPE_EVENT_NOTIFY  mem overflow");
//		}
//
//  		memcpy((void *)data, (void *)IOData + 16, pstIOCtrlHead->nIOCtrlDataSize + 1);
//
//  		int camIndex = (0xff & data[0]) | (0xff & data[1]) << 8
//  				| (0xff & data[2]) << 16 | (0xff & data[3]) << 24;
//  		int eventType = (0xff & data[4]) | (0xff & data[5]) << 8
//  				| (0xff & data[6]) << 16 | (0xff & data[7]) << 24;
//  		int eventTime = (0xff & data[16]) | (0xff & data[17]) << 8
//  				| (0xff & data[18]) << 16 | (0xff & data[19]) << 24;
//
//  	//	free(data);
//
//  		//LOG_DEBUG(g_log, "camIndex = %d, eventType = %d, eventTime = %d", camIndex,		eventType, eventTime);
//
//  		int i = 0;
//		//int num = SIZEOF_ARRAY(pCamera->pfnAlarmProc);
//
//		char msg[128] = { 0 };
//	/*	sprintf(msg, "%s;%d;%d;%d", pCamera->DevUID, camIndex, eventType, eventTime);
//		int len = strlen(msg);
//
//		while (i < num && pCamera->pfnAlarmProc[i]) {
//			pCamera->pfnAlarmProc[i]((uint8_t *)pCamera->DevUID, msg, len, 0);
//			i++;
//		}*/
//
//  		break;
//    case IOCTRL_TYPE_PUSH_CamIndex:
//      if (pstIOCtrlHead->nIOCtrlDataSize >= 4) {
//        int nCamIndex = __cpu_to_le32(*(uint8_t*)IOData);
//      /*  LOG_DEBUG(g_log,
//                  "%s:IOCTRL_TYPE_PUSH_CamIndex, nCamIndex=%d",
//                  __FUNCTION__, nCamIndex);*/
//      }
//      break;
//
//      case IOCTRL_TYPE_GET_VIDEO_PARAMETER_RESP:
//        //if (pstIOCtrlHead->nIOCtrlDataSize >= sizeof(IOCTRLGetVideoParameterResp)) {
//          //st_AVIOCtrlHead *pstIOCtrlHead =
//              //(st_AVIOCtrlHead *)IOData + sizeof(st_AVIOCtrlHead);
//        //}
//        break;
//
//      case IOCTRL_TYPE_RECORD_PLAYCONTROL_RESP:
//    	  break;
//		case IOCTRL_TYPE_DEVINFO_RESP:
//		{
//			//printf("[ IPC_handler] IOCTRL_TYPE_DEVINFO_RESP\n");
//			//bzero(pCamera->DevModeInfo, sizeof(pCamera->DevModeInfo));
//			//memcpy((void *)pCamera->DevModeInfo, (void *)IOData + 16, 16);	// Catch device model name
//
//			//printf("[ IPC_handler] Devive Model_name   : %s(%d)\n", pCamera->DevModeInfo, strlen(pCamera->DevModeInfo));
//			break;
//		}
//  }
//}