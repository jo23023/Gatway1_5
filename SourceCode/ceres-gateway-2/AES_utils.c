#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AES/AES_EnDe.h"

//static unsigned char AES_KEY_DEFAULT[] = "0123456789ABCDEF";
static int AES_KEY_LENGTH = 16;
static int AES_BLOCK_SIZE = 16;

unsigned char *tailString[] =
               {(unsigned char*)"1",
            		   (unsigned char*)"22",
            		   (unsigned char*)"333",
            		   (unsigned char*)"4444",
            		   (unsigned char*)"55555",
            		   (unsigned char*)"666666",
            		   (unsigned char*)"7777777",
            		   (unsigned char*)"88888888",
            		   (unsigned char*)"999999999",
            		   (unsigned char*)"AAAAAAAAAA",
            		   (unsigned char*)"BBBBBBBBBBB",
            		   (unsigned char*)"CCCCCCCCCCCC",
            		   (unsigned char*)"DDDDDDDDDDDDD",
            		   (unsigned char*)"EEEEEEEEEEEEEE",
            		   (unsigned char*)"FFFFFFFFFFFFFFF",};

void printStr(unsigned char *pBuf, int size);
void printBuf(unsigned char *pBuf, int size);
unsigned char* getTailStr(int buSize);
unsigned char *myStrrstr(const char *haystack, const char *needle);

unsigned char encdata[72000];
unsigned char encOut[72000];
unsigned char decOut[8000];

unsigned char *encryptAES(unsigned char *srcStr, int *outSize, unsigned char *aesKey)
{
   int allLen = -1;
   unsigned char *allStr;
   unsigned char *enStr;

   if (NULL == srcStr)
      return NULL;

   AES_Init();

   int srcLen = strlen((const char*)srcStr);
//   printf("原始数据\n length=%d %s\n\n", srcLen, srcStr);

   int blockSize = srcLen / AES_BLOCK_SIZE;
   int tailSize = srcLen % AES_BLOCK_SIZE;
   if (0 == tailSize)
   {
	   allLen = srcLen;
   }
   else
   {
      blockSize += 1;
      allLen = srcLen + (AES_BLOCK_SIZE - tailSize);
   }

   //allStr = (unsigned char*)malloc((allLen + 1) * sizeof(char));
   //memset(allStr, 0, allLen + 1);

   if (allLen > 72000)
	   printf("encryptAES error, overflow : %d", allLen );

   memset(encdata, 0, sizeof(encdata));
   memset(encOut, 0, sizeof(encOut));

  // memcpy((char *)allStr, (char *)srcStr, srcLen);
	 memcpy((char *)encdata, (char *)srcStr, srcLen);
	allStr = encdata;
	enStr = encOut;

   if (tailSize > 0)
	   memcpy((char *)(allStr + srcLen), (char *)getTailStr(AES_BLOCK_SIZE-tailSize), AES_BLOCK_SIZE-tailSize);
//   printf("补全后数据\n length=%d %s\n\n", allLen, allStr);
	
   // 加密
   // 保存所有加密后数据
   //unsigned char *enStr = (unsigned char*)malloc((allLen + 1) * sizeof(char));
   //memset(enStr, 0, allLen + 1);

   int nSize = 0;
   unsigned char s_in[16] = {0};
   unsigned char s_out[16] = {0};
   int i = 0;
   for (i = 0; i < blockSize; i++)
   {
      memset(s_out, 0, 16);
      memcpy((unsigned char *)s_in, (unsigned char *)(allStr + AES_BLOCK_SIZE * i), AES_BLOCK_SIZE);
      nSize = AES_Encrypt(128, s_in, AES_BLOCK_SIZE, aesKey, AES_KEY_LENGTH, s_out);
      memcpy((unsigned char *)(enStr + AES_BLOCK_SIZE * i), (unsigned char *)s_out, AES_BLOCK_SIZE);
   }

  // free(allStr);

   AES_Deinit();

   *outSize = allLen;
   return enStr;
}

unsigned char *encryptAES2(unsigned char *srcStr, int srcLen2, int *outSize, unsigned char *aesKey)
{
   int allLen = -1;
   unsigned char *allStr;
   unsigned char *enStr;

   if (NULL == srcStr)
      return NULL;

   AES_Init();

   int srcLen = srcLen2;//strlen((const char*)srcStr);
//   printf("原始数据\n length=%d %s\n\n", srcLen, srcStr);

   int blockSize = srcLen / AES_BLOCK_SIZE;
   int tailSize = srcLen % AES_BLOCK_SIZE;
   if (0 == tailSize)
   {
	   allLen = srcLen;
   }
   else
   {
      blockSize += 1;
      allLen = srcLen + (AES_BLOCK_SIZE - tailSize);
   }

   //allStr = (unsigned char*)malloc((allLen + 1) * sizeof(char));
   //memset(allStr, 0, allLen + 1);

   if (allLen > 72000)
	   printf("encryptAES error, overflow : %d", allLen );

   memset(encdata, 0, sizeof(encdata));
   memset(encOut, 0, sizeof(encOut));

  // memcpy((char *)allStr, (char *)srcStr, srcLen);
	 memcpy((char *)encdata, (char *)srcStr, srcLen);
	allStr = encdata;
	enStr = encOut;

   if (tailSize > 0)
	   memcpy((char *)(allStr + srcLen), (char *)getTailStr(AES_BLOCK_SIZE-tailSize), AES_BLOCK_SIZE-tailSize);
//   printf("补全后数据\n length=%d %s\n\n", allLen, allStr);

   // 加密
   // 保存所有加密后数据
   //unsigned char *enStr = (unsigned char*)malloc((allLen + 1) * sizeof(char));
   //memset(enStr, 0, allLen + 1);

   int nSize = 0;
   unsigned char s_in[16] = {0};
   unsigned char s_out[16] = {0};
   int i = 0;
   for (i = 0; i < blockSize; i++)
   {
      memset(s_out, 0, 16);
      memcpy((unsigned char *)s_in, (unsigned char *)(allStr + AES_BLOCK_SIZE * i), AES_BLOCK_SIZE);
      nSize = AES_Encrypt(128, s_in, AES_BLOCK_SIZE, aesKey, AES_KEY_LENGTH, s_out);
      memcpy((unsigned char *)(enStr + AES_BLOCK_SIZE * i), (unsigned char *)s_out, AES_BLOCK_SIZE);
   }

  // free(allStr);

   AES_Deinit();

   *outSize = allLen;
   return enStr;
}

unsigned char *decryptAES(unsigned char *enStr, int size, unsigned char *aesKey)
{
   if (NULL == enStr)
      return NULL;

   int enLen = size;
   int blockSize = enLen / AES_BLOCK_SIZE;
   int tailSize = enLen % AES_BLOCK_SIZE;
   if (0 != tailSize)
   {
      printf("decryptAES() - data length error!\n");
      return NULL;
   }

   //printf("enLen: %d, blocksize: %d\n", enLen, blockSize);
   AES_Init();

   // 保存所有解密后数据
   unsigned char *deStr = decOut; //(unsigned char*)malloc((enLen + 1) * sizeof(char));
   memset(deStr, 0, sizeof(decOut)); //enLen + 1);

   int nSize = 0;
   unsigned char s_in[16] = {0};
   unsigned char s_out[16] = {0};
   int i = 0;

   for (i = 0; i < blockSize; i++)
   {
      memset(s_out, 0, 16);
      memcpy((unsigned char *)s_in, (unsigned char *)(enStr + AES_BLOCK_SIZE * i), AES_BLOCK_SIZE);
      nSize = AES_Decrypt(128, s_in, AES_BLOCK_SIZE, aesKey, AES_KEY_LENGTH, s_out);
      memcpy((unsigned char *)(deStr + AES_BLOCK_SIZE * i), (unsigned char *)s_out, AES_BLOCK_SIZE);
   }

   //printf("decryptAES() - decrypt finish！\n");
   AES_Deinit();

   return deStr;
}

unsigned char* getTailStr(int tailSize)
{
  return tailString[tailSize - 1];
}

int endsWith(unsigned char *srcStr, unsigned char *child);
unsigned char *delTailStr(unsigned char *decryptStr)
{
   if (NULL == decryptStr)
      return NULL;

   unsigned char *plainStr = NULL;
   int arrSize = sizeof(tailString) / sizeof(tailString[0]);
   int len = strlen((const char *)decryptStr);
   int i = 0;
   int res = -1;
   for (; i < arrSize; i++)
   {
      res = endsWith(decryptStr, tailString[i]);
      if (0 == res)
      {
         // 匹配到
         //printf("res = %d\n", res);
	      
         plainStr = (unsigned char *)malloc(len - i);
         memset(plainStr, 0, len - i);
         memcpy((char *)plainStr, decryptStr, len - i - 1);
         break;
      }
   }


   if (NULL == plainStr)
   {
      plainStr = (unsigned char *)malloc(len + 1);
      memset(plainStr, 0, len + 1);
      memcpy((char *)plainStr, decryptStr, len);
   }

   return plainStr;
}

/**
在字符串 srcStr 中，是否以字符串 child 结束
*/
int endsWith(unsigned char *srcStr, unsigned char *child)
{
   int res = 0;
   int srcLen = strlen((char *)srcStr);
   int chLen = strlen((char *) child);
   
   unsigned char *srcPos = srcStr + srcLen - 1;
   unsigned char *chPos = child + chLen - 1;

   while (chPos >= child)
   {
      if (*chPos == *srcPos)
      {
         chPos--;
         srcPos--;
      }
      else
      {
         res = -1;
         break;
      }
   }

   return res;
}


// ================================ 以下是测试代码 ====================================

/*void testAES(unsigned char *srcStr)
{
   // 加密
   unsigned char *encryptStr = encryptAES(srcStr);
   if (NULL == encryptStr)
   {
      printf("\n加密数据错误！\n");
   }
   else
   {
      printf("\n加密后数据：\n");
      printBuf(encryptStr, strlen((const char*)encryptStr));
   }

   // 解密
   unsigned char *decryptStr = decryptAES(encryptStr);
   if (NULL == decryptStr)
   {
      printf("\n解密数据错误！\n");
   }
   else
   {
      printf("\n解密后数据：\n");
      printStr(decryptStr, strlen((const char*)decryptStr));
   }

   // 删除末尾补充的字符串
   unsigned char *plainStr = delTailStr(decryptStr);
  if (NULL == plainStr)
   {
      printf("\n解密后数据错误！\n");
   }
   else
   {
      printf("\n删除末尾补充的数据后，得到数据：\n");
      printStr(plainStr, strlen((const char*)plainStr));
   }

 
   free(encryptStr);
   free(decryptStr);
}*/


void printBuf(unsigned char *pBuf, int size)
{
  int n = 0;
  while (n < size) {
    printf("%02X ", pBuf[n]);
    n++;
  }
  printf("\n\n");
}

void printStr(unsigned char *pBuf, int size)
{
   int n = 0;
   while (n < size) {
    printf("%c", pBuf[n]);
    n++;
  }
  printf("\n\n");	
}


/*

int main(int argc, char *argv[])
{
   if (argc < 2)
   {
      printf("缺少测试字符串 \n\n");
      return -1;
   }
   else
   {
      testAES((unsigned char *)argv[1]);
   }

  int nSize = 0;
  unsigned char s_out[16] = "\0";

   AES_Init();

  printf("原始数据：\n");
  printStr(Data, 16);

  nSize = AES_Encrypt(128, Data, 16, AES_KEY, AES_KEY_LENGTH, s_out);
  printf("加密后的数据：\n");
  if (nSize > 0) {
      memcpy(Data, s_out, 16);
      printBuf(Data, 16);
  }

  nSize = AES_Decrypt(128, Data, 16, AES_KEY, AES_KEY_LENGTH, s_out);
  printf("解密后的数据：\n");
  if (nSize > 0)
      printStr(s_out, 16);

  printf("");

  AES_Deinit();

  return EXIT_SUCCESS;
}

*/
