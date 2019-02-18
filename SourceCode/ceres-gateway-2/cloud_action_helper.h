#ifndef _CLOUD_ACTION_HELPER_H_
#define _CLOUD_ACTION_HELPER_H_

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "jsw_protocol.h"
#include "curl.h"
#include "mosquitto.h"


#define CLOUD_CLIENT_NAME 		"JCloud"

int 	cloud_action_init();
int 	cloud_action_stop();
void 	cloud_action_start(stUSER_parm* pParam);
void 	cloud_action_mcu_ack(int nDID);
bool 	cloud_action_DAExcuse();
void 	cloud_action_update_dev();


#endif //_CLOUD_ACTION_HELPER_H_



