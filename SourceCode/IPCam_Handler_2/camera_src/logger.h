/*============================================================================
*
*  ��Ȩ(R)����  2003-2014 ������. All Rights Reserved.
*
*  ���ļ���:          logger.h
*  ���汾����           v0.1
*  ������ģ���Ŀ�ġ�:  Log ϵͳʵ��
*				
*  �������ߡ�:          Xiaofeng Shen��gray.shen@starxnet.com��-i
*  ���������ڡ�:        2014/03/17 03:04:50 - 2014/03/17 03:04:50
*  ����ļ�¼��:        �����޸Ĺ������ע����
*
*============================================================================*/

/* ��ֹͷ�ļ��ظ������� --------------------------------------------------*/
#ifndef _LOGGER_H
#define _LOGGER_H
 
#pragma once

#if (defined(_WIN32) || defined(LINUX)) && (LOGGER == 1)

#include "log4c.h"

#define LOGGER_DEFINE(logger, tag)              \
  static void *logger;                          \
  static const char * const log_category_name = tag;

void log_init(void** category, const char * const tag);
void log_helper(void* category, int priority, const char* msg, ...);
void log_destroy(void);

#define LOG_CREATE(logger)                       \
  log_init(&logger, log_category_name);          \
  
#define LOG_ERROR(TAG, msg, args...)    \
  log_helper(TAG, LOG4C_PRIORITY_ERROR, msg, ##args)

#define LOG_WARN(TAG, msg, args...)     \
  intlog_helper(TAG, LOG4C_PRIORITY_ERROR, msg, ##args)

#define LOG_INFO(TAG, msg, args...)     \
  log_helper(TAG, LOG4C_PRIORITY_INFO, msg, ##args)

#define LOG_DEBUG(TAG, msg, args...)    \
  log_helper(TAG, LOG4C_PRIORITY_DEBUG, msg, ##args)

#define LOG_TRACE(TAG, msg, args...)    \
  log_helper(TAG, LOG4C_PRIORITY_TRACE, msg, ##args)

#define LOG_DESTROY     log_destroy

#elif defined(ANDROID) /* Android */

/* ��Ҫ���� -llog */
#include <android/log.h>

#define LOGGER_DEFINE(logger, tag) (const char* const(logger) = tag)

#define LOG_CREATE(logger)
#define LOG_DESTROY()

#define LOG_ERROR(TAG, msg, args...)     LOGE(TAG, msg, ##args)
#define LOG_WARN(TAG, msg, args...)      LOGW(TAG, msg, ##args)
#define LOG_INFO(TAG, msg, args...)      LOGI(TAG, msg, ##args)
#define LOG_DEBUG(TAG,msg, args...)      LOGD(TAG, msg, ##args)
#define LOG_TRACE(TAG, msg, args...)     LOGV(TAG, msg, ##args)

#else

#define LOGGER_DEFINE(logger, tag)
#define LOG_CREATE(logger)
#define LOG_DESTROY()

#define LOG_ERROR(TAG, msg, args...)     	printf(msg "\n", ##args);
#define LOG_WARN(TAG, msg, args...)   		printf(msg "\n", ##args);
#define LOG_INFO(TAG, msg, args...) 	//	printf(msg "\n", ##args);
#define LOG_DEBUG(TAG, msg, args...)	//	printf(msg "\n", ##args);
#define LOG_TRACE(TAG, msg, args...)  	//	printf(msg "\n", ##args);

#endif  /* !_WIN32 || !LINUX */


/* �������� ----------------------------------------------------------------*/

/* ��������ԭ�� ------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* _LOGGER_H */

/****************************** END OF FILE ********************************/
