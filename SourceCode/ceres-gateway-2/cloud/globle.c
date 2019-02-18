# include <stdio.h>
# include <unistd.h>
# include <stdlib.h> 
# include <termios.h>
# include <fcntl.h> 
# include <string.h>
# include <sys/time.h>
# include <sys/types.h>
# include <pthread.h>
#include "p2pconfig.h"
#include <stdarg.h>

pthread_mutex_t  p2pctrcmd_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  comctrcmd_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  pair_timecount_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  gtimecount_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  nodeinfo_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  ledhandle_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  devinfo_mlock=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t  send_mlock=PTHREAD_MUTEX_INITIALIZER; 

pthread_mutex_t  send_data_mlock=PTHREAD_MUTEX_INITIALIZER; 
// static struct urdata purdata[LISTSIZE];
pthread_mutex_t  rs232_lock   = PTHREAD_MUTEX_INITIALIZER;
static struct devicestatus   devinfo[10];
int comfd,gpiofd;
static  uint p2pctrcmd1;
static uint comctrcmd1;
static uint pair_timecount1;
static uint gtimecount1;
static uint send_is_finished=0;
static char send_data_from_mcu[20]={0};


pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

sem_t oncmdsem;
sem_t onsetsem;

sem_t queuesem;


sem_t jsoncmd_iscome;

sem_t send_is_over;
uint rt5350ID;
 static struct nodeinfo_ceres nodeinfo1;
static struct led_display ledhandle1={
    .ledstatus=0,
    .pgpio_open=gpio_open,
    .plightled_74ch164=lightled_74ch164,
	.pgetledstatus=getledstatus,
    .pset_led_status=set_led_status,
  
};
void initial_var(void)
{

  p2pctrcmd1=0;
  comctrcmd1=0;
  pair_timecount1=0;
  
  gtimecount1=0;
  rt5350ID=0xf28eb4c0;
  initstack();
  inital_status_byid();

}


void set_send_data_from_mcu(char* value,uint len)
{
 
 	pthread_mutex_lock(&send_data_mlock);   
       if(value!=NULL)
       	{
       	   memcpy(send_data_from_mcu,value,len);
       	}
      pthread_mutex_unlock(&send_data_mlock);
}
uint get_send_data_from_mcu(char *value,uint len)
{
 	pthread_mutex_lock(&send_data_mlock);   
	  
     if(value!=NULL)
       	{
       	   memcpy(value,send_data_from_mcu,len);
       	}
      pthread_mutex_unlock(&send_data_mlock);
	  
}
void set_send_is_finished(uint value)
{
 
 	 pthread_mutex_lock(&send_mlock);   
      send_is_finished=value;
      pthread_mutex_unlock(&send_mlock);
}
uint get_send_is_finished(void)
{
      uint value=0;
 	  pthread_mutex_lock(&send_mlock);   
      value=send_is_finished;
      pthread_mutex_unlock(&send_mlock);
	  return value;
}
void set_p2pctrcmd(uint value)
{
	  pthread_mutex_lock(&p2pctrcmd_mlock);   
    p2pctrcmd1=value;
      pthread_mutex_unlock(&p2pctrcmd_mlock);
	  
}
void  get_p2pctrcmd(uint *value)
{
     pthread_mutex_lock(&p2pctrcmd_mlock);   
      *value=p2pctrcmd1 ;
      pthread_mutex_unlock(&p2pctrcmd_mlock);
}
void set_comctrcmd(uint value)
{
    pthread_mutex_lock(&comctrcmd_mlock);   
    comctrcmd1=value;
     pthread_mutex_unlock(&comctrcmd_mlock);
  
}
void get_comctrcmd(uint *value)
{
     pthread_mutex_lock(&comctrcmd_mlock);   
      *value=comctrcmd1;
      pthread_mutex_unlock(&comctrcmd_mlock);
}
void set_pair_timecount(uint value)
{
      pthread_mutex_lock(&pair_timecount_mlock);   
     pair_timecount1=value;
	 
      pthread_mutex_unlock(&pair_timecount_mlock);
}
void get_pair_timecount(uint *value)
{
      pthread_mutex_lock(&pair_timecount_mlock);   
      *value=pair_timecount1 ;
      pthread_mutex_unlock(&pair_timecount_mlock);
}
void set_gtimecount(uint value)
{
        pthread_mutex_lock(&gtimecount_mlock);   
       gtimecount1=value;
       pthread_mutex_unlock(&gtimecount_mlock);
}
void get_gtimecount(uint *value)
{
      pthread_mutex_lock(&gtimecount_mlock);   
        *value=gtimecount1 ;
       pthread_mutex_unlock(&gtimecount_mlock);
}

void set_nodeinfo(struct nodeinfo_ceres *value)
{
       pthread_mutex_lock(&nodeinfo_mlock);   
        memcpy(&nodeinfo1,value,sizeof(struct nodeinfo_ceres));
       pthread_mutex_unlock(&nodeinfo_mlock);
}
void get_nodeinfo(struct nodeinfo_ceres* value)
{
       pthread_mutex_lock(&nodeinfo_mlock);   
        memcpy(value,&nodeinfo1,sizeof(struct nodeinfo_ceres));
       pthread_mutex_unlock(&nodeinfo_mlock);
}

void set_ledhandle_status(int status)
{
       pthread_mutex_lock(&ledhandle_mlock);   
        ledhandle1.ledstatus=status;
       pthread_mutex_unlock(&ledhandle_mlock);
}
void get_ledhandle_status(int *status)
{
      pthread_mutex_lock(&ledhandle_mlock);   
      *status=ledhandle1.ledstatus;
       pthread_mutex_unlock(&ledhandle_mlock);
}
void inital_status_byid(void)
{
       pthread_mutex_lock(&devinfo_mlock);   
        memset(devinfo,-1,sizeof( struct devicestatus)*10);
       pthread_mutex_unlock(&devinfo_mlock);
}

int set_status_byid(uint ssyid,uint nodeid,int status)
{
  int i=0,j=0;
       pthread_mutex_lock(&devinfo_mlock);   
        for(i=0;i<100;i++)
        	{
        	  if(devinfo[i].status==-1)
        	  	{
        	  	        devinfo[i].nodeid=nodeid;
				   devinfo[i].sysid=ssyid;
				   devinfo[i].status=status;
				    j=1;
				   break;
        	  	}
			  else  if((devinfo[i].sysid==ssyid)&&
			  	(devinfo[i].nodeid==nodeid))
			  	{
			  	    devinfo[i].status=status;
				    j=1;
				   break;
			  	}
        	}
       pthread_mutex_unlock(&devinfo_mlock);
	   return j;
}
int get_status_byid( uint ssyid,uint nodeid)
{
    int i=0,j=0;
	j=-1;
      pthread_mutex_lock(&devinfo_mlock);   
      for(i=0;i<100;i++)
        	{
        	  if((devinfo[i].sysid==ssyid)&&
			  	(devinfo[i].nodeid==nodeid))
        	  	{
				   j=devinfo[i].status ;
				  
				   break;
        	  	}
        	}
       pthread_mutex_unlock(&devinfo_mlock);
	   return j;
}


int set_ticks_byid(uint ssyid,uint nodeid,unsigned long tic)
{
  int i=0,j=0;
       pthread_mutex_lock(&devinfo_mlock);   
        for(i=0;i<100;i++)
        	{
        	  if(devinfo[i].status==-1)
        	  	{
        	  	        devinfo[i].nodeid=nodeid;
				   devinfo[i].sysid=ssyid;
				   devinfo[i].lticks=tic;
				    j=1;
				   break;
        	  	}
			  else  if((devinfo[i].sysid==ssyid)&&
			  	(devinfo[i].nodeid==nodeid))
			  	{
			  	    devinfo[i].lticks=tic;
				    j=1;
				   break;
			  	}
        	}
       pthread_mutex_unlock(&devinfo_mlock);
	   return j;
}
unsigned long get_ticks_byid( uint ssyid,uint nodeid)
{
    unsigned long i=0,j=0;
	j=0;
      pthread_mutex_lock(&devinfo_mlock);   
      for(i=0;i<100;i++)
        	{
        	  if((devinfo[i].sysid==ssyid)&&
			  	(devinfo[i].nodeid==nodeid))
        	  	{
				   j=devinfo[i].lticks;
				  
				   break;
        	  	}
        	}
       pthread_mutex_unlock(&devinfo_mlock);
	   return j;
}

