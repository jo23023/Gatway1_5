/*============================================================================
*
*  版权(R)所有  2003-2014 沈晓峰. All Rights Reserved.
*
*  【文件名】:          sysdeps.h
*  【版本】：           v0.1
*  【功能模块和目的】:  系统依赖（operating system dependent）定义
*				
*  【开发者】:          沈晓峰（gray.shen@starxnet.com）
*  【制作日期】:        2014/03/16 14:18:18 - 2014/03/16 14:18:18
*  【更改记录】:        （若修改过则必需注明）
*
*============================================================================*/

/* 防止头文件重复定义或包含 --------------------------------------------------*/
#ifndef _SYSDEPS_H
#define _SYSDEPS_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gsDef.h"
  
#ifdef __CYGWIN__
#  undef _WIN32
#endif

#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>
#include <process.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>  

#define OS_CRLF_STR "\r\n"
#define OS_PATH_SEPARATOR '\\'
#define OS_PATH_SEPARATOR_STR "\\"

typedef CRITICAL_SECTION        gs_mutex_t;

typedef  unsigned long          gs_thread_t;
  
#define  GS_MUTEX_DEFINE(x)     static gs_mutex_t x

/* declare all mutexes */
/* For win32, gs_sysdeps_init() will do the mutex runtime initialization. */
#define  GS_MUTEX(x)   extern gs_mutex_t  x;


extern void  gs_sysdeps_init(void);

static __inline__
void gs_mutex_init(gs_mutex_t* lock)
{
  InitializeCriticalSection(lock);
}
                   
static __inline__
void gs_mutex_lock( gs_mutex_t* lock)
{
    EnterCriticalSection( lock );
}

static __inline__
void  gs_mutex_unlock( gs_mutex_t* lock)
{
    LeaveCriticalSection( lock );
}

static __inline__
void gs_mutex_destroy(gs_mutex_t* lock)
{
  DeleteCriticalSection(lock);
}

typedef  void*  (*gs_thread_func_t)(void*  arg);

typedef  unsigned (__stdcall *win_thread_func_t)(void*  arg);

static __inline__
int  gs_thread_create( gs_thread_t *pthread, gs_thread_func_t func, void* arg)
{
  *pthread = _beginthreadex(NULL, 0, (win_thread_func_t)func, arg, 0, NULL);
  if (!(*pthread)) {
        return -1;
    }
    return 0;
}

static __inline__
void gs_thread_join(gs_thread_t pthread)
{
  WaitForSingleObject((HANDLE)pthread, INFINITE);
}

static __inline__
void gs_thread_exit(unsigned int retval)
{
  ExitThread(retval);
}
  
static __inline__
void  gs_sleep_ms( int  mseconds )
{
    Sleep( mseconds );
}

#define gs_stricmp      stricmp
#define gs_strnicmp     strnicmp
  
/* 产生一个当前的毫秒 */
#define GetCurrentTick    GetTickCount
 
#else /* !_WIN32 a.k.a. Unix */
  
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>  
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif  

#define OS_CRLF_STR "\n"
#define OS_PATH_SEPARATOR '/'
#define OS_PATH_SEPARATOR_STR "/"

typedef  pthread_mutex_t         gs_mutex_t;
  
#define  gs_thread_join(thread)  pthread_join((thread), NULL)
#define  gs_thread_exit(retval)  pthread_exit((void *)(retval))
  
#define  GS_MUTEX_INITIALIZER    PTHREAD_MUTEX_INITIALIZER
#define  gs_mutex_init(lock)     pthread_mutex_init((lock), NULL)
#define  gs_mutex_lock           pthread_mutex_lock
#define  gs_mutex_unlock         pthread_mutex_unlock
#define  gs_mutex_destroy        pthread_mutex_destroy

#define  GS_MUTEX_DEFINE(m)      static gs_mutex_t m = PTHREAD_MUTEX_INITIALIZER
 
#define  gs_cond_t               pthread_cond_t
#define  gs_cond_init            pthread_cond_init
#define  gs_cond_wait            pthread_cond_wait
#define  gs_cond_broadcast       pthread_cond_broadcast
#define  gs_cond_signal          pthread_cond_signal
#define  gs_cond_destroy         pthread_cond_destroy

/* declare all mutexes */
#define  GS_MUTEX(x)             extern gs_mutex_t  x;
 
typedef  pthread_t                 gs_thread_t;

typedef void*  (*gs_thread_func_t)( void*  arg );

static __inline__
int  gs_thread_create(gs_thread_t *pthread, gs_thread_func_t start, void* arg)
{
  int ret = 0;
  pthread_attr_t  attr;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  //ret = pthread_create(pthread, &attr, start, arg);
  ret = pthread_create(pthread, NULL, start, arg);
  pthread_attr_destroy(&attr);
  return ret;
    
}

static __inline__
void  gs_sleep_ms( int  mseconds )
{
  usleep( mseconds * 1000 );
}

/* 产生一个当前的毫秒 */
static __inline__
long GetCurrentTick() {
  // TODO
  long           tick;
  struct timeval time_val;
  gettimeofday(&time_val , NULL);
  tick = time_val.tv_sec * 1000 + time_val.tv_usec / 1000;
  return tick; 
}

#define gs_stricmp      strcasecmp
#define gs_strnicmp     strncasecmp
  
#endif /* !_WIN32 */

  /* 获取程序的完整路径 */  
void GetAppPath(OUT char *exe,
               IN size_t maxLen);
  
  /* 获取程序所在的自身目录 */
static __inline__
void GetAppDir(OUT char *path, IN size_t maxLen)
{
  int length;
  GetAppPath(path, maxLen);

  length = strlen(path);
  if(length > 0) {
    path[length] = '\0';
    for (int i = length; i >= 0; --i)    
    {    
      if (path[i] == OS_PATH_SEPARATOR)    
      {    
        path[i + 1] = '\0';    
        break;    
      }    
    }      
  } 
}

  
#ifdef __cplusplus
}
#endif

#endif /* _SYSDEPS_H */

/****************************** END OF FILE ********************************/
