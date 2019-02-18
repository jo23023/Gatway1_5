
#ifndef __GLOBLE_H__
#define __GLOBLE_H__
 
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "semaphore.h"
#define TX 				1
#define RX 				2
#define PAIRMODE        3
#define UNPARIMODE      4
#define NORMALMODE 		5
#define  f(x,K) =(x*P+K)%65536 
#define LISTSIZE 1024

 struct urdata{
     unsigned char registered;
	 unsigned char  deviceid[3];
     unsigned short rollingcode;
    
      
   // struct list_head  i_list;   
   // struct hlist_node i_hash; 
}  ;

 struct devicestatus{
         uint sysid;
         uint  nodeid;
	  int  status;	
	  unsigned long lticks;
    
      
   // struct list_head  i_list;   
   // struct hlist_node i_hash; 
}  ;
struct  nodeinfo_ceres {
	 uint  systemid;
     char  nodeid;
	 char  producttype;
	 char  misc;
     uint  cmd;
	 char  checksum;	
	 char  mode;
	 char  status;
	 char  protocal_name[10];
};

#define LOCK_COMAND_INIT 
#define LOCK_COMAND {sem_wait(&oncmdsem);}
#define UNLOCK_COMAND {sem_post(&oncmdsem);}


#define LOCK_QUEUE {sem_wait(&queuesem);}
#define UNLOCK_QUEUE {sem_post(&queuesem);}

#define LOCK_SETTING_INIT {int ret=sem_init(&onsetsem,0,0);}
#define LOCK_SETTING {sem_wait(&onsetsem);}
#define UNLOCK_SETTING {sem_post(&onsetsem);}
extern  struct urdata  purdata[];
extern int comfd,gpiofd;

extern struct ARC433M_sensorinfo my433ID;

extern sem_t oncmdsem;
extern sem_t jsoncmd_iscome;

extern sem_t onsetsem;
extern sem_t queuesem;

extern sem_t send_is_over;
extern pthread_mutex_t count_mutex ;
extern pthread_cond_t  condition_var ;
extern pthread_mutex_t mutex;
extern pthread_mutex_t cmdmutex;
extern pthread_mutex_t  rs232_lock    ;
extern uint rt5350ID;
extern char resultbuff[255];
extern lua_State * lua_state1;
extern uint gtimecount;
extern void initial_var(void);
extern void init_lua_env1(void *);

extern int lua_execute_shell(lua_State* L);
extern int execute(const char* command,char* buf,int bufmax) ;

extern void *cmd_deal(void*param);
extern void * setting_deal(void*param);
void set_send_data_from_mcu(char* value,uint len);
uint get_send_data_from_mcu(char *value,uint len);

extern void set_send_is_finished(uint value);

extern uint get_send_is_finished(void);


extern void set_p2pctrcmd(uint value);

extern void  get_p2pctrcmd(uint *value);

extern void set_comctrcmd(uint value);

extern void get_comctrcmd(uint *value);

extern void set_pair_timecount(uint value);

extern void get_pair_timecount(uint *value);

extern void set_gtimecount(uint value);

extern void get_gtimecount(uint *value);


extern void set_nodeinfo(struct nodeinfo_ceres *value);

extern void get_nodeinfo(struct nodeinfo_ceres* value);


extern void set_ledhandle_status(int status);

extern void get_ledhandle_status(int *statis);

extern void inital_status_byid(void);

extern int set_status_byid(uint ssyid,uint nodeid,int status);

extern int get_status_byid( uint ssyid,uint nodeid);

extern int set_ticks_byid(uint ssyid,uint nodeid,unsigned long tic);

extern unsigned long get_ticks_byid( uint ssyid,uint nodeid);

extern void Handle_Magic_Send_cmd(struct nodeinfo_ceres *pnodeinfo);

extern void Handle_RemoteControl_Send_cmd(struct nodeinfo_ceres *pnodeinfo);

extern void Handle_LICENSE_Key( struct nodeinfo_ceres* pNodeinfo);


#endif  
