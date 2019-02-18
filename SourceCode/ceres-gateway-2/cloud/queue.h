#ifndef _QUEUE_H__
#define _QUEUE_H__

#include <stdio.h>
#include <stdlib.h>

#define QUEUE_DATA_BUF 1024
#define MAX_STACK      120
 
 #ifdef __cplusplus
 extern "C" {
 #endif

int isfull();
void push(char* src);
char* pop(void);
int is_empty(void);
void initstack();
int get_CurPos();
int get_pos();
void Reset_Queue();
int get_count();


#ifdef __cplusplus
}
#endif
#endif
