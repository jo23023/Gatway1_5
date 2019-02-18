#if !defined(__INCLUDED_TYPE_DEFINE_H)
#define __INCLUDED_TYPE_DEFINE_H

typedef int INT32;
typedef unsigned int UINT32;

typedef short INT16;
typedef unsigned short UINT16;

typedef char CHAR;
typedef unsigned char UCHAR;

typedef long LONG;
typedef unsigned long ULONG;

#ifndef bool
#define bool	CHAR
#endif

#endif  //__INCLUDED_TYPE_DEFINE_H
