/*============================================================================
*
*  ��Ȩ(R)����  2003-2014 ������. All Rights Reserved.
*
*  ���ļ�����:          fifo.h 
*  ���汾����           v0.1
*  ������ģ���Ŀ�ġ�:  FIFO ʵ�ִ���
*				
*  �������ߡ�:          Xiaofeng Shen��gray.shen@starxnet.com��-i
*  ���������ڡ�:        2014/03/13 08:26:24 - 2014/03/13 08:26:24
*  �����ļ�¼��:
*   2014/03/13 �� C ʵ�֣�
*        ceres-android-phone-jsw/src/com/p2pcamera/app01hd/FIFO.java
*
*============================================================================*/

/* ��ֹͷ�ļ��ظ��������� --------------------------------------------------*/
#ifndef _FIFO_H
#define _FIFO_H

#pragma once

/* ����ͷ�ļ� --------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "gsDef.h"

/* ������������ ------------------------------------------------------------*/
// ���� FIFO �ڵ�
typedef struct _Frame {
  int           nStreamType;    // ֡���ͣ���Ƶ/��Ƶ/���ƣ�
  int           nDateSize;      // ֡���ݴ�С
  uint8_t       *pData;         // ֡����ָ��
  struct _Frame *pNext;         // ָ����һ��֡
} Frame;

// ���� FIFO ͷ�ṹ
typedef struct _FIFO {
  Frame *front;         // ��ָ��
  Frame *rear;          // βָ��
  int   nTotalSize;     // �ۼ�֡�����С
  int   nNum;           // FIFO �и��ڵ���
} FIFO;

/* �������峣�� ------------------------------------------------------------*/

/* ��������� --------------------------------------------------------------*/
#define FIFO_Destroy(fifo) FIFO_removeALL(fifo);

/* �������� ----------------------------------------------------------------*/

/* ��������ԭ�� ------------------------------------------------------------*/
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
