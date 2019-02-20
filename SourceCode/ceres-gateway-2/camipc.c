#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <crypt.h>
#include "ceres.h"
#include "jsw_protocol.h"
#include "AES/AES_utils.h"
#include "PPPP/PPPP_API.h"
#include <signal.h>

#include "jsw_rf_api.h"
#include "ipcam.h"

typedef struct _IOCTRLAVStream
{
  uint8_t channel;
  uint8_t reserve[7];
} IOCTRLAVStream;


struct camobject{
    unsigned int did;
    char ipcdid[32];
    char ipcpwd[16];
    unsigned char AESKey[32];
    pthread_t handle;
    int connState;
    int session;
    int enableAES;
    int nextXorKeyForIO; // = 2
    unsigned int last_record_time;//last record time by time()
};

//void *ipcam_thread(void *arg);
void *ipcam_thread2(void *arg);
void Camera_DoIOCtrl(struct camobject* cam,    uint8_t IOData[]);
int Camera_DoAuth( struct camobject *cam, char * IOData) ;
void EncryptPacket(struct camobject *pCamera, uint8_t Data[], int nDataSize);
void deleteCamlist2(int id);

struct camobject camlist[MAXIPCAM];
int camcount=0;

int initipcam()
{

    int x;
    for (  x=0;x<devcount;x++)
    {
        if (devlist[x].model != TYPE_CAMERA )
            continue;

        //printf("find ipcam device in devlist\n");
        memset( &camlist[camcount], 0 , sizeof(struct camobject) );
        camlist[camcount].did =devlist[x].did;
        camlist[camcount].session = -1;
        camlist[camcount].enableAES = 1;
        camlist[camcount].nextXorKeyForIO = 2;
        strcpy(camlist[camcount].ipcdid, devlist[x].ipcdid);
        strcpy(camlist[camcount].ipcpwd, devlist[x].ipcpwd);
        camlist[camcount].handle = NULL;

        //printf("start thread for cam %u\n", camlist[camcount].did);
        pthread_create(&camlist[camcount].handle, NULL, ipcam_thread2, (void*) &camlist[camcount]);

        camcount++;
    }

}

void ipcReconnect()
{

}


void addipc(unsigned int did,char * ipcdid, char* ipcpwd )
{
    if (MAXIPCAM == camcount)
    {
        printf("max camobject\n");
        return;
    }

    //printf("add ipcam device camobject\n");
    memset( &camlist[camcount], 0 , sizeof(struct camobject) );
    camlist[camcount].did = did;
    camlist[camcount].session = -1;
    camlist[camcount].enableAES = 1;
    camlist[camcount].nextXorKeyForIO = 2;
    strcpy(camlist[camcount].ipcdid, ipcdid);
    strcpy(camlist[camcount].ipcpwd, ipcpwd);
    camlist[camcount].handle = NULL;

    //printf("start thread for cam %u\n", camlist[camcount].did);
    pthread_create(&camlist[camcount].handle, NULL, ipcam_thread2, (void*) &camlist[camcount]);

    camcount++;

}

void doCamRecord(struct camobject *pCam) 
{
   IOCTRLAVStream      IOData;
   memset(&IOData, 0, sizeof(IOCTRLAVStream));
   unsigned char packet[1500];

 	char *pOut = (char*)&IOData;

   int nIOCtrlType = IOCTRL_TYPE_MANU_REC_START;
   int nIODataSize = sizeof(IOCTRLAVStream);
   int nStreamIOType = SIO_TYPE_IOCTRL;

   int nSize = sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead) + nIODataSize;
   int packetsize = sizeof(st_AVIOCtrlHead) + nIODataSize;

   //printf("Camera_SendIOCtrl nSize %d, packet size %d\n", nSize, packetsize);

   //packetsize = __cpu_to_le32(packetsize);
   memset(packet, 0, sizeof(packet));
   packet[0] = (uint8_t) packetsize;
   packet[1] = (uint8_t) (packetsize >> 8);
   packet[3] = (uint8_t) nStreamIOType;
   packet[4] = (uint8_t) nIOCtrlType;
   packet[5] = (uint8_t) (nIOCtrlType >> 8);

   char *ptr = (char*)packet;
   if (pOut != NULL) {

	   packet[6] = (uint8_t) nIODataSize;
	   packet[7] = (uint8_t)(nIODataSize >> 8);
	   memcpy(ptr + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),   pOut, nIODataSize);
   }
// if (pCamera->enableAES) {
   if (nStreamIOType != SIO_TYPE_AUTH)
   {
	   //printf("encrypt data\n");
	   EncryptPacket(pCam, packet, nSize);			  // ?��?
   }

   int nRet = PPPP_Write(pCam->session, CHANNEL_IOCTRL, (char *)packet, nSize);
   pCam->last_record_time = time(NULL);

   printf("return from ipc record nRet %d\n", nRet);
}

void doRec(unsigned int did) // cam id (not did)
{
    int x;
    struct camobject *cam =NULL;

    printf("doRec \n");

    for (x= 0;x< camcount;x++)
    {
        if (camlist[x].did == did)
        {
            if (camlist[x].session < 0)
                return;

            cam = &camlist[x];
            break;
        }
    }
    if (cam == NULL)
        return;

	doCamRecord(cam);
  
}

void doRecByDID(char* pDID) 
{
    int x;
    struct camobject *cam =NULL;

    printf("doRecByDID %s \n", pDID);

    for (x= 0;x< camcount;x++)
    {
        if ( strcmp( camlist[x].ipcdid, pDID) == 0)
        {
            if (camlist[x].session < 0)
                return;

            cam = &camlist[x];
            break;
        }
    }
    if (cam == NULL)
        return;

	printf("doRecByDID find %s \n", pDID);

	doCamRecord(cam);
  
}



void EncryptPacket(struct camobject *pCamera, uint8_t Data[], int     nDataSize)
{


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
  uint8_t pOut[32]; // = (uint8_t *)malloc(32);
  memset(pOut, 0, sizeof(pOut));

  int nSize = AES_Encrypt(128,
                          Data + sizeof(st_AVStreamIOHead),
                          16, pCamera->AESKey, 16, pOut);
  if (nSize > 0)
  {
    //printf("encrypt done size %d\n",nSize);
    memcpy(Data + sizeof(st_AVStreamIOHead), pOut, 16);
  }
 // free(pOut);
}


int readDataFromRemote(  struct camobject * cam,    uint8_t Channel,    unsigned char *pData,    int nSize,    int timeout_ms,     int nBufferSize)
{
  int nRet = -1;
  int nReadSize = 0;
  int nExitCount = 0;
  int nTotalRead = 0;
  char *ptr;

  printf("start readDataFromRemote totalread %d, require size %d \n", nTotalRead,  nSize);
  while (nTotalRead < nSize) {
     printf("readDataFromRemote sess %d\n", cam->session );
    nReadSize = nSize - nTotalRead;
    ptr = (char *)(pData + nTotalRead);

    nRet = PPPP_Read(cam->session,Channel,
                      ptr, //(CHAR*)pIOData,
                      &nReadSize, timeout_ms);
    printf("read result %d , read size %d, totalread %d, require size %d\n", nRet, nReadSize, nTotalRead,  nSize);

    nTotalRead += nReadSize;
    if (nRet == ERROR_PPPP_TIME_OUT)
    {
        printf("read timeout \n");
        continue;
    }

    if (nRet == ERROR_PPPP_SUCCESSFUL) {
    // LOG_DEBUG(g_log, "1 nExitCount=%d", nExitCount);
        nExitCount = 0;
    } else
      nExitCount++;

    if (nExitCount > 3) {
        //printf("[ Camera ] Read from ipcam exit nExitCount=%d", nExitCount);
        nRet = ERROR_PPPP_TIME_OUT;
        break;
    }

  }

  printf("end readdata \n");
  return nRet;
}

int Camera_SendIOCtrl( struct camobject* cam, int nIOCtrlType,char *IOData,  int     nIODataSize,    int     nStreamIOType)
{


    unsigned char packet[1500];
    //memset(packet, 0, sizeof(packet) );

    //printf("Camera_SendIOCtrl session start\n");

    int nSize = sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead) + nIODataSize;
    int packetsize = sizeof(st_AVIOCtrlHead) + nIODataSize;

    printf("Camera_SendIOCtrl nSize %d, packet size %d\n", nSize, packetsize);

    //packetsize = __cpu_to_le32(packetsize);
    memset(packet, 0, sizeof(packet));
    packet[0] = (uint8_t) packetsize;
    packet[1] = (uint8_t) (packetsize >> 8);
    packet[3] = (uint8_t) nStreamIOType;
    packet[4] = (uint8_t) nIOCtrlType;
    packet[5] = (uint8_t) (nIOCtrlType >> 8);

    char *ptr = (char*)packet;

    if (IOData != NULL) {
        packet[6] = (uint8_t) nIODataSize;
        packet[7] = (uint8_t)(nIODataSize >> 8);
        memcpy(ptr + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),   IOData, nIODataSize);
    }
 // if (pCamera->enableAES) {
    if (nStreamIOType != SIO_TYPE_AUTH)
    {
        printf("encrypt data\n");
        EncryptPacket(cam, packet, nSize);             // ?��?
    }
 // }

    printf("send to ipc %d\n", cam->session);
  int nRet = PPPP_Write(cam->session, CHANNEL_IOCTRL, (char *)packet, nSize);

  printf("return from ipc nRet %d\n", nRet);

  //LOG_DEBUG(g_log, "sendIOCtrl(PPPP_Write) nRet=%d"
  //          ", sessionHandle=%d"
  //          ", nStreamIOType=%d"
  //          ", nIOCtrlType=%d",
  //          nRet, pCamera->hSession, nStreamIOType, nIOCtrlType);

  return nRet;

}



int Camera_DoAuth( struct camobject *cam, char * IOData)
{
     unsigned char packet[1500];
    char *AES_KEY_DEFAULT = "0123456789ABCDEF";
    st_AuthHead *pstAuthHead = (st_AuthHead *)IOData;
    char pOut[32];

    switch (pstAuthHead->AuthType) {

        case AUTH_TYPE_REQ:
        // printf("auth AUTH_TYPE_REQ\n");
          memset(pOut, 0, sizeof(pOut));
          int nSize = AES_Encrypt(128, IOData + sizeof(st_AuthHead), 16,
                              (UCHAR *)cam->ipcpwd,
                              strlen(cam->ipcpwd), pOut);
        //  printf("auth AES_Encrypt %d\n", nSize);

          if (nSize > 0) {
    /*          Camera_SendIOCtrl(cam->session,
                              AUTH_TYPE_RESP,
                              pOut, 16, SIO_TYPE_AUTH);*/



    //memset(packet, 0, sizeof(packet) );
    int nIOCtrlType = AUTH_TYPE_RESP;
    int nIODataSize = 16;
    int nStreamIOType = SIO_TYPE_AUTH;

    //printf("Camera_SendIOCtrl session start\n");

    int nSize = sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead) + nIODataSize;
    int packetsize = sizeof(st_AVIOCtrlHead) + nIODataSize;

    //printf("Camera_SendIOCtrl nSize %d, packet size %d\n", nSize, packetsize);

    //packetsize = __cpu_to_le32(packetsize);
    memset(packet, 0, sizeof(packet));
    packet[0] = (uint8_t) packetsize;
    packet[1] = (uint8_t) (packetsize >> 8);
    packet[3] = (uint8_t) nStreamIOType;
    packet[4] = (uint8_t) nIOCtrlType;
    packet[5] = (uint8_t) (nIOCtrlType >> 8);

    char *ptr = (char*)packet;

    if (pOut != NULL) {
    //  printf("iodata not null\n");
        packet[6] = (uint8_t) nIODataSize;
        packet[7] = (uint8_t)(nIODataSize >> 8);
        memcpy(ptr + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),   pOut, nIODataSize);
    }
 // if (pCamera->enableAES) {
    if (nStreamIOType != SIO_TYPE_AUTH)
    {
        //printf("encrypt data\n");
        EncryptPacket(cam, packet, nSize);             // ?��?
    }
 // }

  int nRet = PPPP_Write(cam->session, CHANNEL_IOCTRL, (char *)packet, nSize);

  printf("login result from ipc nRet %d\n", nRet);








            }
            break;

      case AUTH_TYPE_OK:
        printf("auth AUTH_TYPE_OK\n");
        if (pstAuthHead->nAuthDataSize >= 16) {
          int nSize = AES_Decrypt(128, IOData + sizeof(st_AuthHead), 16,
                                  (UCHAR *)cam->ipcpwd,
                                  strlen(cam->ipcpwd),
                                  cam->AESKey);
          if (nSize <= 0) {
            return 0;//error
          }
        }
       // int nRet = Camera_SendIOCtrlToggle(pCamera, true, (uint8_t)0, (uint8_t)0);
        //LOG_DEBUG(g_log,
        //          "%s:sendIOCtrl_on_off(get,.)=%d",
        //          __FUNCTION__, nRet);

        cam->connState = 1;
        // chenjian
        // Camera_Start(pCamera);
//        Camera_manuRecStart(pCamera);

        break;

        case AUTH_TYPE_FAILED:
            printf("[ Camera ] AUTH_TYPE_FAILED\n");
            return 0;
          break;
  }
    return 1;
}

int DecryptPacket(
    struct camobject *pCamera,
      int nStreamIOType,
      uint8_t Data[],
     int      nDataSize) {


  //uint8_t *pOut = NULL;
  uint8_t pOut[512];
  memset(pOut, 0 , sizeof(pOut));

    int nSize;
     uint8_t XorKey;
      int index = 0;

  switch (nStreamIOType) {
    case SIO_TYPE_AUDIO:
    case SIO_TYPE_VIDEO:
        printf("DecryptPacket SIO_TYPE_VIDEO\n");
    /*  if (nDataSize < sizeof(st_AVFrameHead))
        return -2;

      int nSize = AES_Decrypt(128, Data, 16,
                  pCamera->AESKey, 16, pOut);
      if (nSize <= 0)
          goto error;

      memcpy(Data, pOut, sizeof(st_AVFrameHead));
      st_AVFrameHead *pstFrameHead = (st_AVFrameHead *)pOut;
      int index = 0;
      uint8_t XorKey = pstFrameHead->XorKey;
      if ( (pstFrameHead->nDataSize + sizeof(st_AVFrameHead)) == nDataSize) {
        for (index = sizeof(st_AVFrameHead); index < nDataSize; index++) {
          Data[index] = (uint8_t)(Data[index] ^ XorKey);
        }
      } else
          goto error;*/
      //free(pOut);
      break;

    case SIO_TYPE_IOCTRL:
        if (nDataSize < sizeof(st_AVIOCtrlHead))
            return -2;

        //pOut = (uint8_t *) malloc(sizeof(st_AVIOCtrlHead));
        //memset(pOut, 0, sizeof(st_AVIOCtrlHead));

        printf("SIO_TYPE_IOCTRL AES_Decrypt\n");
         nSize = AES_Decrypt(128, Data, 16, pCamera->AESKey, 16, pOut);
        if (nSize <= 0)
            goto error;

        memcpy(Data, pOut, sizeof(st_AVIOCtrlHead));
        st_AVIOCtrlHead *pstCtrlHead = (st_AVIOCtrlHead *) pOut;
        XorKey = pstCtrlHead->XorKey;
        if ((pstCtrlHead->nIOCtrlDataSize + sizeof(st_AVIOCtrlHead)) == nDataSize)
        {
            for (index = sizeof(st_AVIOCtrlHead); index < nDataSize; index++) {
                Data[index] = (uint8_t) (Data[index] ^ XorKey);
            }
        } else
            goto error;
        //free(pOut);
        break;
  }

  return 0;
error:
  //free(pOut);
  return -2;
}

void *ipcam_thread2(void *arg)
{
    struct camobject* cam = (struct camobject*) arg;

    int recont =0;
    int timeout1 = 0;
    int thread_continue = 1;
    while(thread_continue > 0)
    {
        int delay_sec = 30;
        recont = 0;
        //connect
        while (cam->session < 0)
        {
//printf("cam start loop1, session=%d, ipcdid=%s, ipcpwd=%s, recont=%d\n", cam->session, cam->ipcdid, cam->ipcpwd, recont);
printf("cam start loop1, session=%d, ipcdid=%s, recont=%d\n", cam->session, cam->ipcdid, recont);
            if(cam->session == -999)
            {//exit thread
                printf("Error: cam->session == -999, exit thread, session=%d\n", cam->session);
                thread_continue = 0;
                break;
            }

            delay_sec = 30;

            //int EnableLan =1;
            //int Port = 0;
            cam->session = PPPP_Connect(cam->ipcdid, 1, 0); //EnableLan, Port);//cam->ipcdid, devlist[2].did
            if ( cam->session < 0)
            {
              recont++;
              usleep(2000 * 1000);
              if (recont >= 5)//10)
              {
                    printf("connect fail for ipc %s, errCode=%d\n", cam->ipcdid, cam->session);
                    delay_sec = 10;
                    break;//pthread_exit(0);//return;
              }
            }
        }

        //keep connection
        if(cam->session > 0)
        {//with connection
            int nRet = 0;
            int nRecvSize = 0;
            int nCurStreamIOType = 0;
            st_AVStreamIOHead *pStreamIOHead = NULL;
            unsigned char pIOData[2046] = { 0 };

            recont = 0;
            timeout1 = 0;

            while (1) {
//printf("cam start loop2, session=%d, %s, timeout1=%d\n", cam->session, cam->ipcdid, timeout1);

                if(cam->session == -999)
                {//exit thread
                    printf("Error: cam->session ==-999, exit thread, session=%d\n", cam->session);
                    thread_continue = 0;
                    break;
                }

                if(cam->session < 0)
                {//maybe closed by other process, like deletedev()
                    printf("Error: cam->session < 0, exit, session=%d\n", cam->session);
                    break;
                }

                delay_sec = 30;

                memset(pIOData, 0, sizeof(pIOData));
                nRecvSize = sizeof(st_AVStreamIOHead);

                nRet = PPPP_Read(cam->session,
                          CHANNEL_IOCTRL,
                          pIOData,
                          &nRecvSize, 3000);

                if (ERROR_PPPP_TIME_OUT == nRet)
                {
                    timeout1++;

                    if((timeout1 % 70) == 0)
                    {//every 210 secs to reconnect camera if no response
                        timeout1 = 0;
                        if(cam->session > 0)
                            PPPP_Close(cam->session);
                        cam->session = -1;
                        printf("Error: timeout1 >= 70, renew p2p thread\n");
                        thread_continue = 0;
                        break;
                    }

                    if((timeout1 % 30) == 0)
                    {//every 90 secs to get info once
                        //ipcam_get_info(cam);
                        unsigned char packet2[1500];
                        IOCTRLAVStream      IOData2;
                        int nIOCtrlType2 = IOCTRL_TYPE_DEVINFO_REQ; //IOCTRL_TYPE_MANU_REC_START;
                        int nIODataSize2 = sizeof(IOCTRLAVStream);
                        int nStreamIOType2 = SIO_TYPE_IOCTRL;
                        char *pOut2 = (char*)&IOData2;

                        int nSize2 = sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead) + nIODataSize2;
                        int packetsize2 = sizeof(st_AVIOCtrlHead) + nIODataSize2;
                        memset(packet2, 0, sizeof(packet2));
                        packet2[0] = (uint8_t) packetsize2;
                        packet2[1] = (uint8_t) (packetsize2 >> 8);
                        packet2[3] = (uint8_t) nStreamIOType2;
                        packet2[4] = (uint8_t) nIOCtrlType2;
                        packet2[5] = (uint8_t) (nIOCtrlType2 >> 8);
                        char *ptr2 = (char*)packet2;
                        if (pOut2 != NULL) {
                            packet2[6] = (uint8_t) nIODataSize2;
                            packet2[7] = (uint8_t)(nIODataSize2 >> 8);
                            memcpy(ptr2 + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),   pOut2, nIODataSize2);
                        }

                        if (nStreamIOType2 != SIO_TYPE_AUTH)
                        {
                            //printf("encrypt data\n");
                            EncryptPacket(cam, packet2, nSize2);
                        }

                        int nRet22 = PPPP_Write(cam->session, CHANNEL_IOCTRL, (char *)packet2, nSize2);

                        //printf("return from ipc get info nRet22=%d\n", nRet22);
                    }

                    continue;
                }

                if (ERROR_PPPP_SUCCESSFUL != nRet)
                {//disconnect
                    recont++;
                    if(recont > 1)
                    {
                        if(cam->session > 0)
                            PPPP_Close(cam->session);
                        //printf("[ Camera ] IPC disconnect header reading(2), did = %s, errcode = %d, recont=%d, session close\n", cam->ipcdid, nRet, recont);
                        cam->session = -1;
                        //thread_continue = 0;
                        break;//pthread_exit(0);//return;
                    }else
                    {
                        usleep(2*1000*1000);
                        continue;
                    }
                }

                pStreamIOHead = (st_AVStreamIOHead *)pIOData;
                nCurStreamIOType = pStreamIOHead->uionStreamIOHead.nStreamIOType;
                nRecvSize = (pStreamIOHead->uionStreamIOHead.nDataSize[2] & 0xFF) << 16
                  | (pStreamIOHead->uionStreamIOHead.nDataSize[1] & 0xFF) << 8
                  | (pStreamIOHead->uionStreamIOHead.nDataSize[0] & 0xFF);

                //nRet = readDataFromRemote(cam, CHANNEL_IOCTRL,
                //                        pIOData, nRecvSize, 500,
                //                        sizeof(pIOData));
                nRet = PPPP_Read(cam->session,
                         CHANNEL_IOCTRL,
                          pIOData,
                          &nRecvSize, 3000);

                //printf("[ Camera ] IPC PPPP_READ data res %d, readsize %d\n", nRet,nRecvSize);

                if (ERROR_PPPP_TIME_OUT == nRet)
                    continue;
                if (ERROR_PPPP_SUCCESSFUL != nRet)
                {//disconnect
                    recont++;
                    if(recont > 1)
                    {
                        if(cam->session > 0)
                            PPPP_Close(cam->session);
                        //printf("[ Camera ] IPC disconnect data reading(2), did = %s, errcode = %d, recont=%d, session close\n", cam->ipcdid, nRet, recont);
                        cam->session = -1;
                        //thread_continue = 0;
                        break;
                                //pthread_exit(0);//return;
                    }else
                    {
                        usleep(2*1000*1000);
                        continue;
                    }
                }

                 if (nCurStreamIOType == SIO_TYPE_AUTH) {
                     //printf("[ Camera ] IPC do Camera_DoAuth\n");
                     int isok = Camera_DoAuth(cam, pIOData);

                    // printf("[ Camera ] IPC do Camera_DoAuth result %d\n", isok);
                     if (isok ==0)
                     {//auth failed, close session
//printf("[ Camera ] Error: AUTH fail, ipcdid=%s, ipcpwd=%s, close session\n", cam->ipcdid, cam->ipcpwd);
printf("[ Camera ] Error: AUTH fail, ipcdid=%s, close session\n", cam->ipcdid);
                        if(cam->session > 0)
                            PPPP_Close(cam->session);
                        cam->session = -1;
                        //thread_continue = 0;
                        break;
                     }
                  }else
                  {
                    if (cam->enableAES)
                    {
                        int n =  DecryptPacket(cam, nCurStreamIOType, pIOData, nRecvSize);
                        if (n < 0)
                        {
                            printf("[ Camera ] Error: Decrypt fail\n");
                            break;
                        }else
                        {
                            timeout1 = 0;
                            Camera_DoIOCtrl(cam, pIOData);
                        }
                    }else
                    {
                        timeout1 = 0;
                        Camera_DoIOCtrl(cam, pIOData);
                    }
                 }
            }//while
        }

        //wait and reconnect
        if(cam->session > 0)
            PPPP_Close(cam->session);
        cam->session = -1;

        //delete camobject from camlist if not exist in device list
        if (isDevRegister(cam->did) == 0)
        {//not exist in devlist
            deleteCamlist2(cam->did);
            thread_continue = 0;
            break;
        }

        if(thread_continue > 0)
            usleep(delay_sec*1000*1000);//wait 10 seconds for reconnect
    }//while

    //exit thread
    if(cam->session > 0)
        PPPP_Close(cam->session);
    cam->session = -1;

    //delete camobject from camlist if not exist in device list
    if (isDevRegister(cam->did) == 0)
    {//not exist in devlist
        deleteCamlist2(cam->did);
    }

    printf("camera %s thread exit.\n", cam->ipcdid);
    cam->handle = NULL;
    pthread_exit(0);//return 0;
}

void Camera_DoIOCtrl(struct camobject* cam,    uint8_t IOData[])
{


  st_AVIOCtrlHead *pstIOCtrlHead = (st_AVIOCtrlHead *) IOData;

  //LOG_DEBUG(g_log,
  //          "RecvIOCtrl DoIOCtrl(..) IOCtrlType=%d, pIODataSize=%d",
  //          pstIOCtrlHead->nIOCtrlType, pstIOCtrlHead->nIOCtrlDataSize);

  char data[1024];
  switch (pstIOCtrlHead->nIOCtrlType) {
      // 2014-10-21
      case IOCTRL_TYPE_EVENT_NOTIFY:
        //LOG_DEBUG(g_log, "get IOCTRL_TYPE_EVENT_NOTIFY");

          printf("IOCTRL_TYPE_EVENT_NOTIFY\n");
        //char *data = NULL;
    //  data = (char *)malloc(pstIOCtrlHead->nIOCtrlDataSize + 1);
        memset(data,0,sizeof(data));
        if (pstIOCtrlHead->nIOCtrlDataSize + 1 > 1024)
        {
            printf("IOCTRL_TYPE_EVENT_NOTIFY  mem overflow\n");
            return;
        }

        memcpy((void *)data, (void *)IOData + 16, pstIOCtrlHead->nIOCtrlDataSize + 1);
        int camIndex = (0xff & data[0]) | (0xff & data[1]) << 8
                | (0xff & data[2]) << 16 | (0xff & data[3]) << 24;
        int eventType = (0xff & data[4]) | (0xff & data[5]) << 8
                | (0xff & data[6]) << 16 | (0xff & data[7]) << 24;
        int eventTime = (0xff & data[16]) | (0xff & data[17]) << 8
                | (0xff & data[18]) << 16 | (0xff & data[19]) << 24;

        //newEvent(cam->did, TYPE_CAMERA, 0x01, 0);
        if(eventType == SHC_ARM)
		{
		}else if(eventType == SHC_DISARM)
		{
		}else if(eventType == REC_BY_DOORKEY)
		{
		}else
		{
			ipc_motionEvent(cam->did);
		}


    //  free(data);

        //LOG_DEBUG(g_log, "camIndex = %d, eventType = %d, eventTime = %d", camIndex,       eventType, eventTime);

//        int i = 0;
        //int num = SIZEOF_ARRAY(pCamera->pfnAlarmProc);

        //char msg[128] = { 0 };
    /*  sprintf(msg, "%s;%d;%d;%d", pCamera->DevUID, camIndex, eventType, eventTime);
        int len = strlen(msg);

        while (i < num && pCamera->pfnAlarmProc[i]) {
            pCamera->pfnAlarmProc[i]((uint8_t *)pCamera->DevUID, msg, len, 0);
            i++;
        }*/

        break;
    case IOCTRL_TYPE_PUSH_CamIndex:
          printf("IOCTRL_TYPE_PUSH_CamIndex\n");
      if (pstIOCtrlHead->nIOCtrlDataSize >= 4) {
      //  int nCamIndex = __cpu_to_le32(*(uint8_t*)IOData);
      /*  LOG_DEBUG(g_log,
                  "%s:IOCTRL_TYPE_PUSH_CamIndex, nCamIndex=%d",
                  __FUNCTION__, nCamIndex);*/
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
            //bzero(pCamera->DevModeInfo, sizeof(pCamera->DevModeInfo));
            //memcpy((void *)pCamera->DevModeInfo, (void *)IOData + 16, 16);    // Catch device model name

            //printf("[ IPC_handler] Devive Model_name   : %s(%d)\n", pCamera->DevModeInfo, strlen(pCamera->DevModeInfo));

        }
        break;
  }
}

void* getCamListByDID(int did)
{
    int x;
    for (x =0;x<camcount;x++)
    {
        if (camlist[x].did != did)
             continue;

        return &camlist[x];
    }

    return NULL;
}

//index: 0->preset 1, 1->preset2, 2->preset 3
void cameraGotoPTPoint2(int did, int index)
{
    struct camobject* cam = (struct camobject*)getCamListByDID(did);
    if(cam == NULL)
    {
        printf("Error: Camera %d not found!\n", did);
        return;
    }

    IOCTRLPtzCmd req;
    memset(&req, 0, sizeof(req));
    req.channel=IOCTRL_PTZ_SET_PRESET_POINT;
    req.control=index;

    unsigned char packet2[1500];
    //IOCTRLAVStream      IOData2;
    int nIOCtrlType2 = IOCTRL_TYPE_PTZ_COMMAND; //IOCTRL_TYPE_MANU_REC_START;
    int nIODataSize2 = sizeof(IOCTRLPtzCmd);
    int nStreamIOType2 = SIO_TYPE_IOCTRL;
    char *pOut2 = (char*)&req;

    int nSize2 = sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead) + nIODataSize2;
    int packetsize2 = sizeof(st_AVIOCtrlHead) + nIODataSize2;
    memset(packet2, 0, sizeof(packet2));
    packet2[0] = (uint8_t) packetsize2;
    packet2[1] = (uint8_t) (packetsize2 >> 8);
    packet2[3] = (uint8_t) nStreamIOType2;
    packet2[4] = (uint8_t) nIOCtrlType2;
    packet2[5] = (uint8_t) (nIOCtrlType2 >> 8);
    char *ptr2 = (char*)packet2;
    if (pOut2 != NULL) {

        packet2[6] = (uint8_t) nIODataSize2;
        packet2[7] = (uint8_t)(nIODataSize2 >> 8);
        memcpy(ptr2 + sizeof(st_AVStreamIOHead) + sizeof(st_AVIOCtrlHead),   pOut2, nIODataSize2);
    }

    if (nStreamIOType2 != SIO_TYPE_AUTH)
    {
        //printf("encrypt data\n");
        EncryptPacket(cam, packet2, nSize2);
    }

    int nRet33 = PPPP_Write(cam->session, CHANNEL_IOCTRL, (char *)packet2, nSize2);

    printf("return from ipc preset %d nRet33=%d\n", index, nRet33);
}

/////////////////////////

//void startRec(char *did, int id)
//{
//
//}

void deleteCamlist(int id)
{
    int x;
    for (x =0;x<camcount;x++)
    {
        if (camlist[x].did != id )
             continue;

        //found
//        if(camlist[x].handle)
//        {
//            pthread_cancel(camlist[x].handle);
//        }
        if(camlist[x].session > 0)
            PPPP_Close(camlist[x].session);
        camlist[x].session = -1;

//      if (camcount-1 != x) //not lastitem
//      {//move the last item here . to keep array continuous.
//          memcpy(&camlist[x], &camlist[camcount-1], sizeof(struct camobject));
//      }
//      camcount --;
//      if (camcount < 0)
//          camcount = 0;

        break;
    }
}

void deleteCamlist2(int id)
{
    int x;
    for (x =0;x<camcount;x++)
    {
        if (camlist[x].did != id )
             continue;

        if (camcount-1 != x) //not lastitem
        {//move the last item here . to keep array continuous.
            deleteCamlist(camlist[camcount-1].did);
            camlist[camcount-1].session = -999;
            memcpy(&camlist[x], &camlist[camcount-1], sizeof(struct camobject));
        }
        camcount --;
        if (camcount < 0)
            camcount = 0;

        break;
    }
}

void exitCamThread(int id)
{
    int x;
    for (x =0;x<camcount;x++)
    {
        if (camlist[x].did != id )
             continue;

        camlist[x].session = -999;

        break;
    }
}

void updateCamListPWD(jswdev *cam_dev)
{
    int x;
    for (x =0;x<camcount;x++)
    {
        if (camlist[x].did != cam_dev->did)
             continue;

        strcpy(camlist[x].ipcpwd, cam_dev->ipcpwd);

        break;
    }
}

void ipc_reconnect()
{
    int x;

    //reset supervision array
    int k;
    static int last_check_alive_time = 0;
    int timenow = time(0);
    if( ((timenow - last_check_alive_time) < 0) || ((timenow - last_check_alive_time) >= 180) )
    {//every 3 mins to check once
        last_check_alive_time = timenow;
        if((timenow > 1400000000) && (timenow < 1800000000))
        {
            for (k=0;k<devcount;k++)
            {
                if((keepalivelist[k].lastcheckin < 1400000000) || (keepalivelist[k].lastcheckin > 1800000000))
                {//invalid time
                    keepalivelist[k].lastcheckin = timenow;
                }
            }
        }
    }

    //reconnect ipcam
    for (  x=0;x<camcount;x++)
    {
        //camlist[camcount].did =devlist[x].did;
        //if ( camlist[x].session > 0) //!= -1)
        if ( camlist[x].handle != NULL)
            continue;

        camlist[x].nextXorKeyForIO = 2;

        //printf("start thread for cam %u\n", camlist[x].did);
        if(camlist[x].session == -999)
            camlist[x].session = -1;
        pthread_create(&camlist[x].handle, NULL, ipcam_thread2, (void*) &camlist[x]);
        pthread_join(camlist[x].handle, NULL);
    }

}

//return whether camera is recording or not
//return >0:camera recording, <=0:camera not recording
int getCameraRecording(unsigned int did)
{
    int x;
    for (x=0;x<camcount;x++)
    {
        if (camlist[x].did != did)
            continue;

        time_t time_now = time(NULL);
        if( ((time_now - camlist[x].last_record_time) >= 0) && ((time_now - camlist[x].last_record_time) <= 60) )
        {
            printf("Camera is recording(%d)\n", (time_now - camlist[x].last_record_time));
            return 1;
        }else
        {
            printf("Camera is not recording(%d)\n", (time_now - camlist[x].last_record_time));
            return 0;
        }
    }

    return 0;
}
