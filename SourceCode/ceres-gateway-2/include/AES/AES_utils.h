/* AES_API.h --------------------------------------------------*/
#ifndef _AES_UTILS_H
#define _AES_UTILS_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
  
unsigned char *encryptAES(unsigned char *srcStr, int *outSize, unsigned char *aesKey);
unsigned char *decryptAES(unsigned char *enStr, int size, unsigned char *aesKey);

#ifdef __cplusplus
}
#endif

#endif /* _AES_UTILS_H */

/****************************** END OF FILE ********************************/
