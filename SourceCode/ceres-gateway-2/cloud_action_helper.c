#include "cloud_action_helper.h"
#include <cJSON.h>
#include "jsw_rf_api.h"
#include <sys/time.h>
#include "cloudDef.h"


#define WARN_MSG(_STR_, args...) printf("[%s] "_STR_, __FILE__, ##args);


#define GATEWAY_ARM  			"gateway_arm"
#define GATEWAY_PARTARM  		"gateway_partArm"
#define GATEWAY_DISARM  		"gateway_disarm"
#define GATEWAY_PANIC			"gateway_panic"

#define START_RECORDING_ALL 	"start_recording_all"
#define START_RECORDING_CAM  	"start_recording_cam"
#define START_SNAPSHOT_ALL 		"start_snapshot_all"
#define START_SNAPSHOT 			"start_snapshot"
#define START_GROUP  			"start_group"
#define STOP_GROUP  			"stop_group"
#define START_SIERN  			"start_siren"
#define STOP_SIERN  			"stop_siren"
#define SET_SWITCH_ON  			"set_switch_on"
#define SET_SWITCH_OFF       	"set_switch_off"

#define IGNIRE_MQTT_TIME		   2000	

static volatile int g_cloudMQTTRun = 1;
  
static struct mosquitto *m_pMQTTConn = NULL;
static int m_CloudMQTTConnect = 0;
static pthread_t mqtt_cloud_pid;
char  g_szTopicAlexa[600];
char  g_szTopicIFTTT[600];
char  g_szTopicRvDA[600];
char  g_szTopicSenDA[600];

char m_szQureyId[40];  //Token

stUSER_parm m_stUserParam;

extern CHAR g_gwDID[32];
extern struct googleSetting g_stDAParam;

int   m_nSyncID = 0;
char  m_deviceAry[10][32];
int   m_bWaitAck = false;
bool  m_bFirstAlexAction = true;
bool  m_bFirstDAAction = true;

typedef struct _tagCheckPower
{
	int 		nPowerIndex;
	unsigned int 	nPowerDID;
	int 		lLastTime;
	int 		bOnline;
	int 		bOnOff;
	
}CheckPowerStatus;


//Check Power status
CheckPowerStatus m_checkPowerList[10];
int m_nPowerCnt = 0;
int m_nDevListCnt = 0;
int m_nPowerDevCnt = 0;
pthread_t m_check_thread = NULL;


typedef struct EXECUTE_PARAM
{
  char szAPPId[32]; 	 //AppId 
  char szAPPToken[256];  //Token
  char szUsername[256];  //email
  char szEntryId[32];	 //entry id.gateway 
  
}stEXCUTE_parm;


bool get_sensor_alive(unsigned int nNodeID)
{
	bool bOn = false;
	int nType;
	bool bAlive = false;

	printf("get_sensor_status nNodeID =  %d \n", nNodeID);

	jswdev DevInfo;

	bAlive = getSensorstate(nNodeID, &DevInfo);

	return bAlive;


}


bool get_sensor_status(unsigned int nNodeID)
{
	bool bOn = false;
	int nType;
	bool bAlive = false;

	//printf("get_sensor_status nNodeID =  %d \n", nNodeID);

	jswdev DevInfo;

	getSensorstate(nNodeID, &DevInfo);

	int nDevStatus = DevInfo.status;

	//printf("TYPE_POWER_ADAPTER model = %d, state = %d \n",DevInfo.model, nDevStatus);
	
	switch(DevInfo.model)
	{
		case TYPE_SIREN_INDOOR:
		case TYPE_SIREN_OUTDOOR:
			if(1 == nDevStatus || 3 == nDevStatus )
				bOn = true;
			else if(2 == nDevStatus )
				bOn = false;
			break;
		case TYPE_POWER_ADAPTER:
			//printf("TYPE_POWER_ADAPTER state = %d \n", nDevStatus);
			if(1 == nDevStatus )
				bOn = true;
			else if(2 == nDevStatus || 0 == nDevStatus)
				bOn = false;
			break;
		case TYPE_PIR:
			if(1 == nDevStatus  || 2 == nDevStatus )
				bOn = true;
			else  
				bOn = false;
			break;
		case TYPE_MAGNETIC:
			if(1 == nDevStatus || 3 == nDevStatus)
				bOn = false; //lock
			else if(2 == nDevStatus )
				bOn = true;
			break;
			
		case TYPE_KEYPAD_JSW:
		case TYPE_REMOTE_CONTROL_NEW:
			if(2 == nDevStatus )
				bOn = false;  //disarm
			else 
				bOn = true;
			break;
		case TYPE_SMOKE:
			if( 1 == nDevStatus || 2 == nDevStatus) // 1 (0x01) means trigger, 2 (0x02) means smoke overheat.
				bOn = true;
			else
				bOn = false;  
			break;
		case TYPE_WATERLEVEL:
			if (2 == nDevStatus || 3 == nDevStatus)
				bOn = true;
			else
				bOn = false;  
			break;
		case TYPE_GATEWAY:
			if(0 == nDevStatus )
				bOn = false; //disarm
			else
				bOn = true;  
			break;
		case TYPE_CAMERA:
			bOn = true;  
			break;
		case TYPE_NEST_SMOKE:
		case TYPE_NEST_THERMO:
			bOn = false;  
		default:
			break;
		}


	return bOn;
}


void send_Qurey_Rsp()
{
	
	cJSON *root;
	char* pOut = NULL;
	int nIndex = 0;
		 
	DBG_PRINT("send_Qurey_Rsp @@@ \n");
		 
	 root = cJSON_CreateObject();	 
	 char szTmep[100];

	//printf("handle_DAQuery Device ID %s \n", &m_deviceAry[nIndex][0]);
	//printf("handle_DAQuery query id %s \n", m_szQureyId);
		
	 cJSON_AddStringToObject(root, "queryId", m_szQureyId);

	 //cJSON_AddStringToObject(root, "devices", "{\"855882716\": {\"on\": false, \"online\":true }}");
	 bool bAlive = true;
	 bool bOnoff = false;
	 
	 for (nIndex = 0; nIndex < 1; nIndex++)
	 {
		 int 	nNodeId = 0;
		 char 	szTag[32];
		 char  	szLevel[32];
		 bool 	bGw = false;

		  
		 printf("handle_DAQuery Device ID %s \n", &m_deviceAry[nIndex][0]);
		 memset(szLevel, 0 , 32);
		 
		 if(strcmp(&m_deviceAry[nIndex][0], g_gwDID) ==0)
		 {
			 bGw = true;
			 bAlive = true;
			 strcpy(szTag,"arm");
			 if(g_armstate == st_arm )
			 {
				 bOnoff = true;
				 strcpy(szLevel, "L2");
			 }
			 else if( g_armstate == st_partarm )
			 {
				 bOnoff = true;
				 strcpy(szLevel, "L1");
			 }
			 else
			 {
				 bOnoff = false;
			 }
	
		 }
		 else
		 { 
			bGw = false;
			strcpy(szTag,"on");
			nNodeId = atoi(&m_deviceAry[nIndex][0]);
			bOnoff = get_sensor_status(nNodeId);
			bAlive = get_sensor_alive(nNodeId);
		 }
 

		 printf("handle_DAQuery @@ alive %d \n", bAlive);
		
		cJSON* pStatus = cJSON_CreateObject();
		
		
		if(bAlive)
			 cJSON_AddTrueToObject(pStatus, "online");
		 else
			 cJSON_AddFalseToObject(pStatus, "online");
		
		 if(bGw)
		 {
			 if (bOnoff)
				 cJSON_AddTrueToObject(pStatus, "isArmed");
			 else
				 cJSON_AddFalseToObject(pStatus, "isArmed");
			 if(strlen(szLevel) >0)
				 cJSON_AddStringToObject(pStatus, "currentArmLevel", szLevel);
		 }
		 else
		 {
		 	if (bOnoff)
				cJSON_AddTrueToObject(pStatus, szTag);
			else
		 		cJSON_AddFalseToObject(pStatus, szTag);
		 }
	
		 cJSON* pDevice = cJSON_CreateObject();
		 cJSON_AddItemToObject(pDevice, &m_deviceAry[nIndex][0], pStatus);
		
		 cJSON_AddItemToObject(root, "devices", pDevice);
	
	 }
	
	 cJSON_AddStringToObject(root, "entityId", m_stUserParam.szEntryId);
	
	 pOut = cJSON_Print(root);
	 printf("handle_DAQuery response %s \n", pOut);
	 int nLen = strlen(pOut);
	
	 int nRet = mosquitto_publish(m_pMQTTConn, 0, g_szTopicSenDA, nLen,pOut, 0,0);
	 printf("handle_DAQuery response mqtt send Ret= %d \n", nRet);
	 free(pOut);

	 if(bAlive == false)
	 	ReportGWStateStateToDA(&m_deviceAry[nIndex][0], bOnoff, false);
}


//connect
void connect_callback(struct mosquitto *mosq, void *obj, int rc) 
{
	int r = 0;
	if (rc == 0)
	{
		
		m_CloudMQTTConnect = 1;

		r = mosquitto_subscribe(m_pMQTTConn, NULL, g_szTopicRvDA, 0);		
		if( r != 0)
			WARN_MSG(" Subscribe %s connect to cloud mqtt bus Failed !\n", g_szTopicRvDA);
		
    	r = mosquitto_subscribe(m_pMQTTConn, NULL, g_szTopicAlexa, 0); // 0: send one times
     	if( r != 0)
			WARN_MSG(" Subscribe %s connect to cloud mqtt bus Failed !\n", g_szTopicAlexa);		

		r = mosquitto_subscribe(m_pMQTTConn, NULL, g_szTopicIFTTT, 0);
		if( r != 0)
			WARN_MSG(" Subscribe %s connect to cloud mqtt bus Failed !\n", g_szTopicIFTTT);

		WARN_MSG(" %s connect to cloud mqtt bus, succeed!\n", CLOUD_CLIENT_NAME);

		m_bFirstAlexAction = true;
		m_bFirstDAAction = true;

	}
	else
	{
		m_CloudMQTTConnect = 0;
		WARN_MSG(" %s connect to mqtt bus, failed!\n", CLOUD_CLIENT_NAME);
	}
}

void ExecuteAction(bool bGateway, int nNodeID, bool bOnOff, char* pExtra)
{
	int nType;

	printf("ExecuteAction is gateway(%d) nodid = %d, on(%d), extra = %s \n", bGateway, nNodeID, bOnOff, pExtra);

	jswdev DevInfo;

	if( bGateway)
	{
		
		if(bOnOff == true)
		{
			if(pExtra)
			{
				if(strcmp(pExtra, "L1") == 0)
				{
				    if( g_armstate == c_partarm )
				    	ReportGWStateStateToDA(g_gwDID, true, true);
					else
						armCommand(c_partarm);
				}
			}
			else
			{
			 	if( g_armstate == c_arm )
				    ReportGWStateStateToDA(g_gwDID, true, true);
				else
					armCommand(c_arm);
			}
		}
		else 
		{
			if( g_armstate == c_disarm )
			    ReportGWStateStateToDA(g_gwDID, false, true);
			else
				armCommand(c_disarm);
		}

	}
	else
	{

		DevInfo.did = 0;

		bool bRet = getSensorstate(nNodeID, &DevInfo);

		if(DevInfo.did == 0)
		{		
			printf("ExecuteAction() get sensor info failed \n");
			return ;
		}

		
		switch(DevInfo.model)
		{
		case TYPE_SIREN_INDOOR:
		case TYPE_SIREN_OUTDOOR:
			{
				//testSiren((char*)&nNodeID, bOnOff);	
				setSiren((char*)&nNodeID, bOnOff);	
				printf("ExecuteAction setSiren on %d \n", bOnOff);
			}
			break;
		case TYPE_POWER_ADAPTER:
			{
				//testPAon((char*)&nNodeID, bOnOff);
				setPAon((char*)&nNodeID, bOnOff); 
				printf("ExecuteAction setPAon on %d \n", bOnOff);
			}
			break;
		}
	}
	
}

void handle_DAExecute(char *pData)
{	
	int i = 0;
	char deviceAry[10][32];
	int nDevCnt = 0;
	char* pArmLevel = NULL;
	bool bGateway = false;
	bool bOnoff = false;
	
	printf("handle_DAExecute\n");

	cJSON *obj = cJSON_Parse(pData);
	if(!obj)
	{
		printf("handle_DAExecute failed \n");
		return ;
	}

	char* pExecId = cJSON_GetStringItem(obj, "execId");
	if(pExecId == NULL)
	{	
		printf("handle_DARequest execId  = NULL\n");
		return;
	}
	
	cJSON* cDevice = cJSON_GetObjectItem(obj, "devices");
	if( cDevice != NULL)
	{
	 	for (  i = 0 ; i < cJSON_GetArraySize(cDevice) ; i++)
	 	{
	    	cJSON* pItem = cJSON_GetArrayItem(cDevice, i);
			if(pItem)
			{
				char* pID = cJSON_GetStringItem(pItem, "id");
				printf("cJSON_GetArrayItem name = %s \n", pID);
			 	if(strlen(pID) > 0)
				{
			 		strcpy( &deviceAry[i][0], pID); 
					nDevCnt++;
			 	}
	 		 }  
	 	}
	}
	else
	{
		DBG_PRINT(" handle_DAExecute content error \n");
		return;
	}

	bool bArm = false;
	cJSON* cExecution = cJSON_GetObjectItem(obj, "execution");
	if( cExecution != NULL)
	{
 		cJSON* pItem = cJSON_GetArrayItem(cExecution, 0);
		if(pItem)
		{
			cJSON* cArm = cJSON_GetObjectItem(pItem, "arm");
			if(cArm != NULL)
			{					
				DBG_PRINT(" handle_DAExecute arm(%d) \n", cArm->type);	
				 if(cArm->type  == cJSON_False)
			    	  bOnoff = false;
				 else 
	                bOnoff = true;

				pArmLevel = cJSON_GetStringItem(pItem, "armLevel");
				DBG_PRINT(" handle_DAExecute gateway = %d, level = %s \n", bOnoff, pArmLevel);				
				bGateway = true;
			} 
			else
			{					
				cJSON* cOnoff = cJSON_GetObjectItem(pItem, "on");
				DBG_PRINT(" handle_DAExecute sensor on/off(%d)\n", cOnoff->type);
				
				if(cOnoff->type  == cJSON_False)
			    	  bOnoff = false;
				 else 
	                bOnoff = true;
				bGateway = false;
					
				DBG_PRINT(" handle_DAExecute sensor = %d\n", bOnoff);				
			}//if
		}//item
	}//cExecution
	else
	{
		DBG_PRINT(" handle_DAExecute execution content error \n");
		return;
	}
	
  ////////////////////////////////////////////////////////////////////
  // Packet Execute response
/*
  {
	 "execId": "zqoEbIvGn8C7DvRY2F1wvABj0sMOAoiX",
	 "commands": [
		 {
			 "ids": ["855882716"],
			 "status": "SUCCESS",
			 "states": {
				 "isArmed": true,
				 "online": true,
				 "currentArmLevel": "L2"
			 }
		 }
	 ],
	 "entityId": "K40R5DDK-UPCAGNJ8-VCQW5S3F"
  }
  */


    cJSON *root;
	char* pOut = NULL;
	int nIndex = 0;

	printf("handle_DAExecute packet response ¡Asensor cnt = %d \n", nDevCnt);

	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "execId", pExecId);
	cJSON* cmdList = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "commands", cmdList);


	for (nIndex = 0; nIndex < nDevCnt; nIndex++)
	{
		cJSON* pDevice = cJSON_CreateObject();

		int nNodeId = 0;
		char szTag[32];
		char  szLevel[32];
		bool bGw = false;

		printf("handle_DAQurey Device ID %s \n", &deviceAry[nIndex][0]);
		memset(szLevel, 0, 32);


		cJSON* pIdList = cJSON_CreateArray();
		cJSON_AddItemToObject(pDevice, "ids", pIdList);
		cJSON_AddStringToObject(pDevice, "status", "SUCCESS");
		cJSON_AddItemToArray(pIdList, cJSON_CreateString(&deviceAry[nIndex][0]));

		//cJSON* pStateList = cJSON_CreateObject();
		cJSON* pState = cJSON_CreateObject();

		if (bGateway)
		{
			bool bSuccess = false;
						
			//execute
			ExecuteAction(bGateway, -1, bOnoff, pArmLevel);

			if (bOnoff)
				cJSON_AddTrueToObject(pState, "isArmed");
			else
				cJSON_AddFalseToObject(pState, "isArmed");

			//cJSON_AddTrueToObject(pState, "online");


			if(pArmLevel)
				cJSON_AddStringToObject(pState, "currentArmLevel", pArmLevel);


			//cJSON_AddNumberToObject(root, "exitAllowance", 120);

			bGw = true;
		}
		else
		{
			int nNodeId = atoi(&deviceAry[nIndex][0]);		
			//execute
			ExecuteAction(bGateway, nNodeId, bOnoff, pArmLevel);
			
			if (bOnoff)
				cJSON_AddTrueToObject(pState, "on");
			else
				cJSON_AddFalseToObject(pState, "on");

			cJSON_AddTrueToObject(pState, "online");


		}


		cJSON_AddItemToObject(pDevice, "states", pState);
		cJSON_AddItemToArray(cmdList, pDevice);
	}

	cJSON_AddStringToObject(root, "entityId", m_stUserParam.szEntryId);

	pOut = cJSON_Print(root);
	printf("handle_DAExecute response %s \n", pOut);
	int nLen = strlen(pOut);

	int nRet = mosquitto_publish(m_pMQTTConn, 0, g_szTopicSenDA, nLen,pOut, 0,0);
	printf("handle_DAExecute response mqtt send Ret= %d \n", nRet);
	free(pOut);
	

}


void handle_DAQuery(char *pData)
{	
	int i = 0;
	printf("handle_DAQuery %s\n", pData);

	cJSON *obj = cJSON_Parse(pData);
	if(!obj)
	{
		printf("handle_DAQuery failed\n");
		return ;
	}

	char* pQureyId = cJSON_GetStringItem(obj, "queryId");
	memset(m_szQureyId, 0, 32);
 	strcpy(m_szQureyId, pQureyId);
	m_szQureyId[32] = '\0';
	
	if(pQureyId == NULL)
	{	
		printf("handle_DAQuery queryId  = NULL\n");
		return;
	}

	cJSON* cDevice = cJSON_GetObjectItem(obj, "devices");
	if( cDevice != NULL)
	{
	 	for (  i = 0 ; i < cJSON_GetArraySize(cDevice) ; i++)
	 	{
	    	cJSON* pItem = cJSON_GetArrayItem(cDevice, i);
			if(pItem)
			{
				char* pID = cJSON_GetStringItem(pItem, "id");
				printf("cJSON_GetArrayItem name = %s \n", pID);
			 	if(strlen(pID) > 0)
				{
			 		strcpy( &m_deviceAry[i][0], pID); 
			 	}
	 		 }  
	 	}
	}
	else
	{
		DBG_PRINT(" handle_DAQuery content error \n");
	}

	printf("handle_DAQuery Device ID %s \n", &m_deviceAry[0][0]);
	printf("handle_DAQuery GW DID %s \n", g_gwDID);
	
	if(strcmp(&m_deviceAry[0][0], g_gwDID) >= 0)
	{			
		printf("handle_DAQuery GW %s \n", g_gwDID);
		send_Qurey_Rsp();
		return;
	}
	else
	{
		printf("handle_DAQuery not gw %s \n", g_gwDID);
		m_bWaitAck = true;
	}
  ////////////////////////////////////////////////////////////////////
  // Packet query response
  /*
   "queryId": "zqoEbIvGn8C7DvRY2F1wvABj0sMOAoiX",
	"devices": {
	"855882716":{"on": true, "online": true},
	"855882456":{"arm": true, "online" :true}
	},
	"entityId":"K40R5DDK-UPCAGNJ8-VCQW5S3F"
  */

	int nIndex = 0;

	for (nIndex = 0; nIndex < 1; nIndex++)
	{
		int nNodeId = 0;
		char szTag[32];
		bool bOnoff = false;
		char szLevel[32];
		bool bGw = false;
		int nStatus = 0;
		
	 	printf("handle_DAQuery Device ID %s \n", &m_deviceAry[nIndex][0]);
		
		if(strcmp(&m_deviceAry[nIndex][0], g_gwDID) < 0)
		{ 
			jswdev DevInfo;
			nNodeId = atoi(&m_deviceAry[nIndex][0]);
			//Do Test 
			bool bRet = getSensorstate(nNodeId, &DevInfo);
			bOnoff = get_sensor_status(nNodeId);		

			switch(DevInfo.model)	
				{		
				case TYPE_SIREN_INDOOR:		
				case TYPE_SIREN_OUTDOOR:		
					{				
						testSiren((char*)&nNodeId, bOnoff);		
						//usleep(UART_WRITE_DELAY*10);
						printf("test siren alive setSiren on %d \n", bOnoff);	
					}			
				break;			
				case TYPE_POWER_ADAPTER:			
					{			
						testPAon((char*)&nNodeId, bOnoff); 	
						printf("test plug alive setPAon on %d \n", bOnoff);			
					}			
				break;						
				}//swith
		}
	}
	

}

void waitSleepTime(int nSec)
{
	int i=0;
	
	for(i=0; i< nSec; i++)
	{
		if(g_cloudMQTTRun == true)
			sleep(1);
		else
		  break;

	}
}


//------------------------------------------------------------------------------
static void *thread_check_powerStatus(void *arg)
{

    while(1)
    {
    	if(g_cloudMQTTRun == false)
			break;
	
    	if(m_CloudMQTTConnect)
		{
		 	if(m_bWaitAck == false)
				check_power_status();	
			
			waitSleepTime(30);
			
		}
		else
		{
			waitSleepTime(10);
		}
	
    }
 }


/*
"queryId": "zqoEbIvGn8C7DvRY2F1wvABj0sMOAoiX",
"intent": :QUERY"
"devices": [{"id": "855882716"}, {"id":"855882456"}]
}
*/

void handle_DARequest(char* pdata)
{
	DBG_PRINT(" handle_DARequest, msg = %s \n", pdata);

	cJSON *obj = cJSON_Parse(pdata);
	if(!obj)
	{
		printf("handle_DARequest cJSON_Parse failed | %s\n", pdata);
		return ;
	}
	else
	{
		//printf("cJSON_Parse ok | %s\n", str);
	}

	
	char* pAction = cJSON_GetStringItem(obj, "intent");
	if(pAction == NULL)
	{	
		printf("handle_DARequest intent =  NULL\n");
		return;
	}

	printf("handle_DARequest  Action = %s \n", pAction);

	
	if(strcmp(pAction, "EXECUTE") == 0)
	{
		handle_DAExecute(pdata);
	} 
	else if(strcmp(pAction, "QUERY") == 0)
	{
		handle_DAQuery(pdata);
	}
	else if(strcmp(pAction, "SYNC") == 0)
	{
		if(m_check_thread == NULL)
		{		
		    //check Each power status
		   int err = pthread_create(&m_check_thread, NULL, thread_check_powerStatus, NULL);
		   if(err)
		   {	 
			   DBG_PRINT("cloud_event_init thread_check_powerStatus error = %d	\n", err);
			}
		
		   cloud_action_update_dev();

		   g_stDAParam.bEnableDA = true;
		   save_cloud_goolg_DA();

		}
		ReportAllDeviceToDA();
	}

}


void handle_action(char* pdata)
{
	DBG_PRINT(" handle_action, msg = %s \n", pdata);
	int nGroupID = -1;
	int nNodeID = -1;
	int nExcuseID = -1;

	cJSON *obj = cJSON_Parse(pdata);
	if(!obj)
	{
		printf("cJSON_Parse failed | %s\n", pdata);
		return ;
	}
	else
	{
		//printf("cJSON_Parse ok | %s\n", str);
	}


	char* pActionTime	= cJSON_GetStringItem(obj, "Time");
	int nActionTime = atoi(pActionTime);
	char* pActionName	= cJSON_GetStringItem(obj, "ActionName");
	char* pNodeID= cJSON_GetStringItem(obj, "NodeID");
	char* pGroupID = cJSON_GetStringItem(obj, "GroupID");

    if( pNodeID)
    {
		nNodeID = atoi(pNodeID);
		//printf("handle_action node 3 %d \n", nNodeID);
    }


    if(pGroupID)
    {
		nGroupID = atoi(pGroupID);
		//printf("handle_action group 3 %d  \n", nGroupID);
    }

	if(pActionName == NULL )
	{
		printf("can't find ActionName \n");
		cJSON_Delete(obj);
		return;
	
	}


	DBG_PRINT(" handle_action Action name = %s, NodeID = %s,GroupID = %s \n", 
		pActionName, pNodeID, pGroupID);


	if(strcmp(pActionName, GATEWAY_ARM) == 0)
	{
		armCommand(c_arm);
	}
	else if(strcmp(pActionName, GATEWAY_PARTARM) == 0)
	{
		armCommand(c_partarm);
	}
	else if(strcmp(pActionName, GATEWAY_DISARM) == 0)
	{
		armCommand(c_disarm);
	}
	else if(strcmp(pActionName, GATEWAY_PANIC) == 0)
	{
		panic();
	}
	else if(strcmp(pActionName, START_RECORDING_ALL) == 0)
	{
		startRec();
	}
	else if(strcmp(pActionName, START_RECORDING_CAM) == 0 && pNodeID != NULL)
	{
		doRecByDID(pNodeID);
	}
	else if(strcmp(pActionName, START_GROUP) == 0 && pGroupID > 0)
	{
		if(nGroupID == 0)
		{		
		  printf("group id is number \n");
		  nGroupID = cJSON_GetIntItem(obj, "GroupID");
		  printf("group id is number %d \n", nGroupID);
		}	
		startzone((char*)&nGroupID, CMD_ADAPTER_TURNON);
	}
	else if(strcmp(pActionName, STOP_GROUP) == 0 && pGroupID > 0)
	{
		if(nGroupID == 0)
		{		
		  printf("group id is number \n");
		  nGroupID = cJSON_GetIntItem(obj, "GroupID");
		  printf("group id is number %d \n", nGroupID);
		}	
		startzone((char*)&nGroupID, CMD_ADAPTER_TURNOFF);
	}
	else if(strcmp(pActionName, STOP_GROUP) == 0 && pGroupID > 0)
	{
		startzone((char*)&nGroupID, CMD_ADAPTER_TURNOFF);
	}
	else if(strcmp(pActionName, START_SIERN) == 0 && nNodeID > 0)
	{
		setSiren((char*)&nNodeID, 1);	
	}
	else if(strcmp(pActionName, STOP_SIERN) == 0 && nNodeID > 0)
	{
		setSiren((char*)&nNodeID, 0);	
	}
	else if(strcmp(pActionName, SET_SWITCH_ON) == 0 && nNodeID > 0)
	{
		setPAon((char*)&nNodeID, 1); 
	}	
	else if(strcmp(pActionName, SET_SWITCH_OFF) == 0 && nNodeID > 0)
	{
		setPAon((char*)&nNodeID, 0); 
	}	

    cJSON_Delete(obj);

}


void message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message) 
{
	DBG_PRINT(" cloud_action_helper message_callback(). topic = %s\n", message->topic);
	DBG_PRINT(" cloud_action_helper message_callback(). payload = %s, len = %d\n", 
		message->payload, message->payloadlen);



	if (strcmp(message->topic, g_szTopicAlexa) == 0 || strcmp(message->topic, g_szTopicIFTTT) == 0)
	{
		if ( m_bFirstAlexAction)
    	{  
    		m_bFirstAlexAction = false;
			DBG_PRINT(" First time mqtt Alexa Cmd topic = %s\n", 	message->topic);
			return;
		}

		//pushEvent(message->payload, message->payloadlen);
		DBG_PRINT(" Match event action topic = %s\n", 	message->topic);
		handle_action(message->payload);

	}
	else if (strcmp(message->topic, g_szTopicRvDA) == 0 )
	{
		DBG_PRINT(" Match event action topic = %s\n", 	message->topic);
		handle_DARequest(message->payload);

	}
	
	
}

void* mqtt_cloud_thread_routine(void *ptr) 
{
	int rc = 0;
	int nFailCount = 0;

	sleep(3);

	mosquitto_lib_init();

	m_pMQTTConn = mosquitto_new(NULL, true, NULL);
	if (m_pMQTTConn) 
	{
	  //user name entity id(get it from login)
	 
		mosquitto_username_pw_set(m_pMQTTConn, m_stUserParam.szEntryId, m_szToken);
 		mosquitto_connect_callback_set(m_pMQTTConn, connect_callback);
		mosquitto_message_callback_set(m_pMQTTConn, message_callback);


		rc = mosquitto_connect(m_pMQTTConn, m_szMQTTHost, CLOUD_MQTT_PORT, 30);

		while (g_cloudMQTTRun)
		{
			rc = mosquitto_loop(m_pMQTTConn, -1, 1);
			if ( rc != MOSQ_ERR_SUCCESS)	
			{
				if (nFailCount == 5)	
				{
					WARN_MSG("%s is disconnected with mqtt, error = %d \n", CLOUD_CLIENT_NAME,rc);
					sleep(3);
					nFailCount = 0;
				}
				else
				{
					WARN_MSG("%s is disconnected with mqtt, reconnect err = %d \n", CLOUD_CLIENT_NAME, rc);
					sleep(3);
					nFailCount++;
					mosquitto_reconnect(m_pMQTTConn);
				}
			}
			sleep(1);
		}
		mosquitto_disconnect(m_pMQTTConn);
		mosquitto_destroy(m_pMQTTConn);
	}
	mosquitto_lib_cleanup();

	g_cloudMQTTRun = 0;	// End main thread
	DBG_PRINT("End thread\n");
	return NULL;
}


bool checkIfNeedReport(int nDID, bool bOnOff, bool bOnline)
{
	int i=0; 
	
	//printf("checkIfNeedReport ID = %d, bOnOff = %d, bOnline =%d \n", nDID, bOnOff, bOnline);

	for( i=0; i< m_nPowerDevCnt; i++)
	{
		if(m_checkPowerList[i].nPowerDID == nDID)
		{		
			//printf("@@ Online = %d, OnOff =%d \n", m_checkPowerList[i].bOnline, m_checkPowerList[i].bOnOff);
			if(m_checkPowerList[i].bOnline != bOnline )
				return true;

			//if(bOnline == true )
			//	return false;
				
		}
	}
	return false;
}

void updatePowerStatus(int nDID,int bOnOff, int bOnline )
{
	if(m_nPowerDevCnt == 0)
		return;

	//DBG_PRINT("updatePowerStatus DID = %d, online = %d \n", nDID, bOnline);

	int i =0 ;
	for(i =0; i< m_nPowerDevCnt; i++)
	{	
		if(m_checkPowerList[i].nPowerDID == nDID)
		{		
			//DBG_PRINT("updatePowerStatus nPowerDID = %d, online = %d \n", m_checkPowerList[i].nPowerDID,bOnline);
			m_checkPowerList[i].bOnline = bOnline;
			m_checkPowerList[i].bOnOff = bOnOff;
		}
	}
}



void cloud_action_mcu_ack(int nDID) 
{
	//printf("cloud_action_mcu_ack ID = %d, power cnt = %d \n", nDID, m_nPowerDevCnt);
	if(g_stDAParam.bEnableDA == false)
		return ;

	if(g_cloudMQTTRun == false)
		return;
	
	if(m_bWaitAck == true )
	{
		send_Qurey_Rsp();
		m_bWaitAck = false;
	}
	else
	{

		int i=0;

		//Send Report Status
		bool bOnoff = get_sensor_status(nDID);
		bool bOnline = get_sensor_alive(nDID);
				
		printf("cloud_action_mcu_ack ID = %d, onff= %d , online = %d\n",
				nDID, bOnoff, bOnline);
			
		if(checkIfNeedReport(nDID, bOnoff, bOnline))
		{		
			printf("checkIfNeedReport true \n");
			char szNodeID[40];
			sprintf(szNodeID, "%d", nDID);
			ReportGWStateStateToDA(szNodeID, bOnoff, bOnline);			
			updatePowerStatus(nDID, bOnoff, bOnline);
		}
		else
			printf("checkIfNeedReport false \n");

	}
	
}

int cloud_action_init() 
{
	DBG_PRINT("cloud_action_init \n");

	memset(&m_stUserParam, 0, sizeof(m_stUserParam));

	

	return 0;
}


void check_power_status()
{
	DBG_PRINT("Check_power_status \n");

	jswdev  devlist[MAXDEVICENUM];
	int 	nPowerCnt = 0;
	bool 	bOnOff = false;
	int 	nReportDID = -1;
	int 	nDevCnt =0;
	int		i=0;

	nDevCnt = getSensorList((char *)&devlist);

	if(nDevCnt != m_nDevListCnt)
		cloud_action_update_dev();
		

	for( i = 0; i< nDevCnt; i++)
    {      
	  if (devlist[i].model == TYPE_POWER_ADAPTER )
		{		
			printf("Check_power_status index = %d(%d), did = %d, name = %s \n",
				i, nDevCnt, devlist[i].did, devlist[i].name);
			
			bOnOff = get_sensor_status(devlist[i].did);
			testPAon((char*)&devlist[i].did, bOnOff);
			waitSleepTime(30);
		}
    }
	
}


void cloud_action_start(stUSER_parm* pParam)
{
	m_CloudMQTTConnect = 0;
	g_cloudMQTTRun = 1;


	memcpy(&m_stUserParam, pParam, sizeof(stUSER_parm));
	memset(g_szTopicAlexa, 0, sizeof(g_szTopicAlexa));
	memset(g_szTopicIFTTT, 0, sizeof(g_szTopicIFTTT));
	memset(g_szTopicRvDA, 0, sizeof(g_szTopicRvDA));
	memset(g_szTopicSenDA, 0, sizeof(g_szTopicSenDA));
	
	strcat(g_szTopicAlexa, m_stUserParam.szEntryId);
	strcat(g_szTopicAlexa, "/gateways/ALEXA");
	strcat(g_szTopicIFTTT, m_stUserParam.szEntryId);
	strcat(g_szTopicIFTTT, "/gateways/IFTTT");
	strcat(g_szTopicRvDA,  m_stUserParam.szEntryId);
	strcat(g_szTopicRvDA,  "/gateways/smarthome");
	strcat(g_szTopicSenDA, m_szEnterprise);
	strcat(g_szTopicSenDA, "/server/smarthome");

	pthread_create(&mqtt_cloud_pid, NULL, mqtt_cloud_thread_routine, NULL);

	if( g_stDAParam.bEnableDA)
	{		
		 //check Each power status
	  	int err = pthread_create(&m_check_thread, NULL, thread_check_powerStatus, NULL);
		 if(err)
		 {	 
		   DBG_PRINT("cloud_event_init thread_check_powerStatus error = %d	\n", err);
		}
		
	   cloud_action_update_dev();
	}

	
	DBG_PRINT(" start entry id = %s \n", m_stUserParam.szEntryId);
	DBG_PRINT(" topic = %s \n",  g_szTopicAlexa);
	DBG_PRINT(" DA topic = %s \n",  g_szTopicRvDA);

}

int cloud_action_stop()
{
	DBG_PRINT("cloud_action_stop Enter\n");

	g_cloudMQTTRun = 0;
	if(m_pMQTTConn)
	{
		mosquitto_disconnect(m_pMQTTConn);
		mosquitto_destroy(m_pMQTTConn);
		mosquitto_lib_cleanup();
	}

 	pthread_join(mqtt_cloud_pid, NULL);
	DBG_PRINT("cloud_action_stop mqtt_cloud_pid \n");
	pthread_join(m_check_thread, NULL);
	DBG_PRINT("cloud_action_stop m_check_thread \n");
	
	DBG_PRINT("cloud_action_stop Leave\n");
	
	return 1;
}

void cloud_action_update_dev()
{
	jswdev devlist[MAXDEVICENUM];
	int nPowerCnt = 0;
	bool bOnOff = false;
	int nReportDID = -1;

	int nDevCnt = getSensorList((char *)&devlist);

	int i=0;

	if(m_nDevListCnt != nDevCnt)
	{
		for(i=0; i < nDevCnt; i++)
		{
			if (devlist[i].model == TYPE_POWER_ADAPTER )
		  	{		  	
				m_checkPowerList[nPowerCnt].nPowerDID = devlist[i].did;	
				int bOnOff = get_sensor_status(devlist[i].did);
				m_checkPowerList[nPowerCnt].bOnOff= bOnOff;	
				m_checkPowerList[nPowerCnt].bOnline = true;
				m_checkPowerList[nPowerCnt].nPowerIndex= i;	
				nPowerCnt++;
				printf("cloud_action_update_dev index = %d, did = %d \n", i, m_checkPowerList[i].nPowerDID);
		  	}
		}
		m_nDevListCnt = nDevCnt;	
		m_nPowerDevCnt = nPowerCnt;
	}


	DBG_PRINT("@@@ cloud_action_update_dev power cnt = %d \n", m_nPowerDevCnt);

}





