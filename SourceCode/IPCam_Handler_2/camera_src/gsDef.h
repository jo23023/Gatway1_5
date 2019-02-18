/*============================================================================
*
*  版权(R)所有  2003-2014 沈晓峰. All Rights Reserved.
*
*  【文件名】:          gsDef.h
*  【版本】：           （必需）
*  【功能模块和目的】:  （必需）
*				
*  【开发者】:          沈晓峰（gray.shen@starxnet.com）
*  【制作日期】:        2014/03/14 08:45:03 - 2014/03/16 07:45:03
*  【更改记录】:        （若修改过则必需注明）
*
*============================================================================*/

/* 防止头文件重复定义或包含 --------------------------------------------------*/
#ifndef _GSDEF_H
#define _GSDEF_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/* 包含头文件 --------------------------------------------------------------*/
  
/* 导出定义类型 ------------------------------------------------------------*/

/* 导出定义常量 ------------------------------------------------------------*/
#ifndef MAX_PATH
#	define MAX_PATH                260
 #endif
/* 导出定义宏 --------------------------------------------------------------*/
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)  /* C99 */
#	define __inline__     inline
//#	define __restrict__   restrict
#else
#	define __inline__
#	define __restrict__
#endif

/* 由a,b两个字节返回一个Word。*/
#ifdef MAKEWORD
#define MAKEWORD(a, b)          ((uint16_t)(((uint8_t)((uint32_t)(a) & 0xff)) \
                                | ((uint16_t)((uint8_t)((uint32_t)(b) & 0xff))) << 8))
#endif
 
/* 由a，b两个字返回LONG */
#ifdef MAKELONG  
#define MAKELONG(a, b)          ((long)(((uint16_t)((uint32_t)(a) & 0xffff)) \
                                | ((uint32_t)((uint16_t)((uint32_t)(b) & 0xffff))) << 16))
#endif
  
/* 获取一个双字的低位字*/
#ifndef LOWORD  
#define LOWORD(l)               ((uint16_t)((uint32_t)(l) & 0xffff))
#endif
  
/* 获取一个双字的高位字 */
#ifndef HIWORD  
#define HIWORD(l)               ((uint16_t)((uint32_t)(l) >> 16))
#endif
  
/* 获取一个字的低位字节 */
#ifndef LOBYTE 
#define LOBYTE(w)               ((uint8_t)((uint32_t)(w) & 0xff))
#endif
  
/* 获取一个字的低高字节 */
#ifndef HIBYTE 
#define HIBYTE(w)               ((uint8_t)((uint32_t)(w) >> 8))
#endif 

/* 交换两个变量的值 */
#define SWAP( x, y )	\
  		do {(x) ^= (y); (y) ^= (x); (x) ^= (y);} while (0)
  
/* 来返回两部分中的其中一个 */
#define	IIF( expr, truepart, falsepart ) \
	    ((expr) ? (truepart) : (falsepart))

/* c 为 TRUE 代表小端，FALSE 代表大端 */  
#define CheckCPUEndian(c)       \
  do{ (c) = (char)1; (c) = ((c) == 1); } while(0)
  
/* 将大端编码的 32 位数据和当前 CPU 体系构架所支持的数据字节顺序间进行转换
 * 在大端体系架构下，这个宏什么也不做。
 *
 * 注意：
 *
 * [输入] -x ：将这个 32 位数据进行字节顺序转换
 *
 * 返回值：
 * 修正后字节顺序的输入值
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)         /* C99 */
  
inline
static unsigned __swab32(unsigned x) {
  unsigned __x = (x);
  return (unsigned)(
		(((__x) >> 24) & (unsigned)0xffUL)      |
		(((__x) >> 8) & (unsigned)0xff00UL)     |
		(((__x) << 8) & (unsigned)0x00ff0000UL) |
		(((__x) << 24) & (unsigned)0xff000000UL));  
}
  
inline
static unsigned short __swab16(unsigned short x) {
  unsigned short __x = (x);
  return (unsigned short)(
      (((__x) >> 8) & (unsigned)0xffUL) |
      (((__x) << 8) & (unsigned)0xff00UL));
}
  
#else  
#define __swab32(x) (                   \
  (((x) >> 24) & (unsigned)0xffUL)      \
  |\
  (((x) >> 8) & (unsigned)0xff00UL)     \
  |\
  (((x) << 8) & (unsigned)0x00ff0000UL) \
  |\
  (((x) << 24) & (unsigned)0xff000000UL)\
)

#define  __swab16(x) (                  \
  (((_X) >> 8) & (unsigned)0xffUL)      \
  |\
  (((_x) << 8) & (unsigned)0xff00UL)   \
)

#endif

  
# if defined(_BIG_ENDIAN)                          // 定义大端
#	define __le32_to_cpu(x) __swab32(x)
#	define __le16_to_cpu(x) __swab16(x)
#	define __be32_to_cpu(x) ((unsigned)(x))
#	define __be16_to_cpu(x) ((unsigned short)(x))
#	define __cpu_to_le32(x) __swab32(x)
#	define __cpu_to_le16(x) __swab16(x)  
#	define __cpu_to_be32(x) ((unsigned)(x))
#	define __cpu_to_be16(x) ((unsigned short)(x))  
# elif defined(_LITTLE_ENDIAN)                     // 定义小端
#	define __le32_to_cpu(x) ((unsigned)(x))
#	define __le16_to_cpu(x) ((unsigned short)(x))
#	define __be32_to_cpu(x) __swab32(x)
#	define __be16_to_cpu(x) __swab16(x)
#	define __cpu_to_le32(x) ((unsigned)(x))
#	define __cpu_to_le16(x) ((unsigned short)(x))
#	define __cpu_to_be32(x) __swab32(x)
#	define __cpu_to_be16(x) __swab16(x)
# else          /* !__LITTLE_ENDIAN */
#	error "Could not determine byte order"
# endif         /* !__BIF_ENDIAN */

  /*
  *          #define DO_PRAGMA(x) _Pragma (#x)
          #define TODO(x) DO_PRAGMA(message ("TODO - " #x))
          
          TODO(Remember to fix this)
  */
/* 导出变量 ----------------------------------------------------------------*/

/* 导出函数原型 ------------------------------------------------------------*/

  


#ifdef __cplusplus
}
#endif

#endif /* _GSDEF_H */

/****************************** END OF FILE ********************************/
