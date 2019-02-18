#include "queue.h"

#define true 1
#define false 0
/************************************************************************/
/*                                */
/************************************************************************/
 static  int index_s= 0;
 static  int nCurListIndex = 0;
 static char stack[MAX_STACK][QUEUE_DATA_BUF];
 static char (*p)[QUEUE_DATA_BUF];
 int     nCnt =0;

pthread_mutex_t  qlock=PTHREAD_MUTEX_INITIALIZER;
void initstack()
{
 	index_s=0;
	//printf("first top=%d\n",index_s );
 	p=stack;
 	 memset(stack,0,QUEUE_DATA_BUF*MAX_STACK);
 	// printf("first top=%d\n",index_s );
}
int  isfull()
{
	printf("first top=%d\n",index_s );
  	if(index_s> MAX_STACK-1)
 	 return true;
  
  	return false;
}
void push(char* src)
{
  	pthread_mutex_lock(&qlock);
  	printf("first top=%d\n",index_s );

	if(index_s >= MAX_STACK)
		index_s = 0;
	
	char *des=(char*)&(p[index_s]);
   	memset(des,0,QUEUE_DATA_BUF);
   	if(strlen(src)< QUEUE_DATA_BUF)
    	memcpy(des,src,strlen(src));

 	index_s++;
	nCnt ++;
  	printf("top=%d,srclen=%d \n",index_s ,strlen(src));
  	pthread_mutex_unlock(&qlock);
	
}


char* pop(void)
{

 	pthread_mutex_lock(&qlock);

   	char * msg=NULL;
	if(nCurListIndex < 0 || nCnt == 0)
	{
		nCurListIndex = 0;
	}

	if(nCurListIndex >= MAX_STACK)
		nCurListIndex = 0;
	
   	if(is_empty()== false)
	{
  		//  printf("pop top=%d\n",index_s );
 		msg= (char*)&(p[nCurListIndex]);
		nCurListIndex ++;
		nCnt --;
   	}
   pthread_mutex_unlock(&qlock);
   
 	return msg;
}

int is_empty(void)
{
 	//printf("----------------top=%d\n",index_s);
  	if(nCnt == 0)
  		return true;
	
 	return false;

}

int get_CurPos()
{
//  printf("first top=%d\n",index_s );
    return nCurListIndex;
}

int get_pos()
{
//  printf("first top=%d\n",index_s );
    return index_s;
}

void Reset_Queue()
{
	index_s = 0;
	nCurListIndex = 0;
	nCnt =0;
}

int get_count()
{
	return nCnt;
}



