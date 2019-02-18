#ifndef __INCLUDE_PPPP_COMMON_H__
#define __INCLUDE_PPPP_COMMON_H__

#include "PPPP_Type.h"
#ifdef _ARC_COMPILER
#include "sys_comlib.h"
#endif ////#ifdef _ARC_COMPILER
#define MAX_DATA_SIZE_IN_LINKLIST 1024

INT32 GetProfileItem(const CHAR * lpcstrFileName, const CHAR * lpcstrSectionName, const CHAR * lpcstrItemName, const CHAR * lpcstrFormat, ...); 
INT32 GetProfileSection(const CHAR * lpcstrFileName, const CHAR * lpcstrSectionName, const CHAR * lpcstrValue); 
INT32 WriteProfileItem(const CHAR * lpcstrFileName, const CHAR * lpcstrSectionName, const CHAR * lpcstrItemName, const CHAR * lpcstrFormat, ...); 
INT32 RemoveProfileItem(const CHAR * lpcstrFileName, const CHAR * lpcstrSectionName, const CHAR * lpcstrItemName); 
INT32 PPPP_DecodeString(CHAR *strSource, CHAR* strDest, INT32 size);
INT32 PPPP_CRCEnc(UCHAR *Src, INT32 SrcSize, UCHAR* Dest, INT32 DestSize, UCHAR* CRCKey);  //// ver 1.2
INT32 PPPP_CRCDec(UCHAR *Src, INT32 SrcSize, UCHAR* Dest, INT32 DestSize, UCHAR* CRCKey);  //// ver 1.2
#endif  //#ifndef __INCLUDE_PPPP_COMMON_H__

