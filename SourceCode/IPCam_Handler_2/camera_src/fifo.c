/*============================================================================
*
*  版权(R)所有  2003-2014 沈晓峰. All Rights Reserved.
*
*  【文件名】:          fifo.c 
*  【版本】：           v0.1
*  【功能模块和目的】:	FIFO 实现代码
*				
*  【开发者】:		Xiaofeng Shen（gray.shen@starxnet.com）
*  【制作日期】:	2014/03/13 08:26:32 - 2014/03/13 17:26:32
*  【更改记录】:
*   2014/03/13 用 C 实现：
*        ceres-android-phone-jsw/src/com/p2pcamera/app01hd/FIFO.java
*
*============================================================================*/

/* 包含头文件 ----------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fifo.h"
//#include "trace.h"

/* 私有定义常量 --------------------------------------------------------------*/
#define MIN_BUFFER_FRAME_NUM    10
#define MAXSIZE_CONTAIN         131072  // 128K

/* 私有定义类型 --------------------------------------------------------------*/
  
/* 私有定义宏 ----------------------------------------------------------------*/

/* 私有全局变量 --------------------------------------------------------------*/

/* 私有函数 ------------------------------------------------------------------*/

void FIFO_Init(IN FIFO *fifo) {
  // TODO
  assert(fifo);
  fifo->front = NULL;
  fifo->rear = NULL;
  fifo->nTotalSize = 0;
  fifo->nNum = 0;
}

const int FIFO_getMinBufNum_20per(void) {
  // TODO
  return (int)(MIN_BUFFER_FRAME_NUM * 0.2);
}

const int FIFO_getMinBufNum_80per(void) {
  // TODO
  return (int)(MIN_BUFFER_FRAME_NUM * 0.8);
}

const int FIFO_getSize(IN FIFO *fifo) {
  // TODO
  return fifo->nTotalSize;
}

const int FIFO_getHeadSize(IN FIFO *fifo) {
  // TODO
  if (fifo->front)
    return fifo->front->nDateSize;
  return 0;
}

const int FIFO_getTailSize(IN FIFO *fifo) {
  // TODO
  return fifo->rear->nDateSize;
}


const int FIFO_getNum(IN FIFO *fifo) {
  // TODO
  return fifo->nNum;
}

const bool FIFO_isEmpy(IN FIFO *fifo) {
  // TODO
  if (fifo->front == NULL && fifo->rear == NULL && !FIFO_getNum(fifo))
    return true;
  
  return false;
}

const bool FIFO_addLast(
    IN FIFO     *fifo,
    IN int      nStreamType,
    IN uint8_t  Data[],
    IN int      nDateSize) {
  // TODO
  if (!fifo)
    return false;

  Frame *pFrame = (Frame *)malloc(sizeof(Frame));
  pFrame->pData = (uint8_t *)malloc(sizeof(uint8_t) * nDateSize);

  pFrame->nStreamType = nStreamType;
  pFrame->nDateSize = nDateSize;
  memcpy(pFrame->pData, Data, nDateSize);
  pFrame->pNext = NULL;
  if (FIFO_isEmpy(fifo)) {
    fifo->front = pFrame;
  } else {
    fifo->rear->pNext = pFrame;
    
  }
  fifo->rear = pFrame;
  fifo->nNum++;
  fifo->nTotalSize += nDateSize;
  return true;
}

const Frame *FIFO_getHead(IN FIFO *fifo) {
  // TODO
  if (FIFO_isEmpy(fifo))
    return NULL;
  else
    return fifo->front->pNext;
}

const bool FIFO_removeHead(
    IN  FIFO *fifo,
    OUT uint8_t *pData) {
  // TODO
  assert(fifo);
  
  if (FIFO_isEmpy(fifo)) {
    return false;
  }

  Frame *pFrame = fifo->front;
  fifo->front = pFrame->pNext;

  if (fifo->rear == pFrame)
    fifo->rear = fifo->front;
  
  fifo->nTotalSize -= pFrame->nDateSize;
  fifo->nNum--;

  if (pData)
    memcpy(pData, pFrame->pData, pFrame->nDateSize);
  
  free(pFrame->pData);
  free(pFrame);
  
  return true;
  
}

void FIFO_removeALL(IN FIFO *fifo) {
  // TODO
  while (FIFO_removeHead(fifo, NULL));
  
  fifo->nTotalSize = 0;
  fifo->nNum = 0;
}

/******************************* END OF FILE *********************************/
