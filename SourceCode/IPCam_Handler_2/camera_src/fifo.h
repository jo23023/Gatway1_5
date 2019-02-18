/*============================================================================
*
*  版权(R)所有  2003-2014 沈晓峰. All Rights Reserved.
*
*  【文件名】:          fifo.h 
*  【版本】：           v0.1
*  【功能模块和目的】:  FIFO 实现代码
*				
*  【开发者】:          Xiaofeng Shen（gray.shen@starxnet.com）-i
*  【制作日期】:        2014/03/13 08:26:24 - 2014/03/13 08:26:24
*  【更改记录】:
*   2014/03/13 用 C 实现：
*        ceres-android-phone-jsw/src/com/p2pcamera/app01hd/FIFO.java
*
*============================================================================*/

/* 防止头文件重复定义或包含 --------------------------------------------------*/
#ifndef _FIFO_H
#define _FIFO_H

#pragma once

/* 包含头文件 --------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "gsDef.h"

/* 导出定义类型 ------------------------------------------------------------*/
// 定义 FIFO 节点
typedef struct _Frame {
  int           nStreamType;    // 帧类型（视频/声频/控制）
  int           nDateSize;      // 帧数据大小
  uint8_t       *pData;         // 帧数据指针
  struct _Frame *pNext;         // 指向下一个帧
} Frame;

// 定义 FIFO 头结构
typedef struct _FIFO {
  Frame *front;         // 首指针
  Frame *rear;          // 尾指针
  int   nTotalSize;     // 累计帧分配大小
  int   nNum;           // FIFO 有个节点数
} FIFO;

/* 导出定义常量 ------------------------------------------------------------*/

/* 导出定义宏 --------------------------------------------------------------*/
#define FIFO_Destroy(fifo) FIFO_removeALL(fifo);

/* 导出变量 ----------------------------------------------------------------*/

/* 导出函数原型 ------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

  void FIFO_Init(IN FIFO *fifo);
  const int FIFO_getMinBufNum_20per(void);
  const int FIFO_getMinBufNum_80per(void);
  const int FIFO_getSize(IN FIFO *fifo);
  const int FIFO_getHeadSize(IN FIFO *fifo);
  const int FIFO_getTailSize(IN FIFO *fifo);
  const int FIFO_getNum(IN FIFO *fifo);
  const bool FIFO_isEmpy(IN FIFO *fifo);
  const bool FIFO_addLast(IN FIFO       *fifo,
                         IN int         nStreamType,
                         IN uint8_t     Data[],
                         IN int         nDateSize);
  const Frame *FIFO_getHead(IN FIFO *fifo);
  const bool FIFO_removeHead(IN FIFO    *fifo,
                             OUT uint8_t *pData);
  void FIFO_removeALL(IN FIFO *fifo);
  

#ifdef __cplusplus
}
#endif

#endif /* _FIFO_H */

/****************************** END OF FILE ********************************/
