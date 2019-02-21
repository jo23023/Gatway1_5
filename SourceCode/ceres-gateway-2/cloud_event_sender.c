#include "cloud_event_sender.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include "jsw_rf_api.h"
#include <cJSON.h>
#include "jsw_protocol.h"
#include "cloudDef.h"


//structure

CURL* m_pcurl;
bool m_bSendDataToCloud = false;
pthread_t m_send_thread;
pthread_t m_check_thread;

bool m_bStartDA = false;


CHAR g_gwDID[32] = { 0 };

double  m_lLastKeepAliveTime =0 ;

//queue
jswcloudevent m_EventAry[CLOUD_MAX_EVENT_CNT];
short m_nEvtReadpos =0;
short m_nEvtWritepos = 0;
short m_nCount=0;
char g_szGWEntityID[60];

//------------------------------------------------------------------------------

const CloudEventTable g_stEventMap[] =	{
											{ EVENT_PANIC, "Panic"	},
											{ EVENT_SENSOR_ON, "Sensor On "	},
											{ EVENT_SENSOR_OFF, "Sensor Off "  },
											{ EVENT_THERMOSTAT_MODE, "Thermostat Mode"  },
											{ EVENT_THERMOSTAT_TMP, "Thermostat Temp"  },
											{ EVENT_THERMOSTAT_FAN, "Thermostat Fan"  },
											{ EVENT_BATTERY_REPORT, "Battery Reports"  },
											{ EVENT_SMOKE_DETECT, "Smoke Detect"  },
											{ EVENT_HAET_DETECT, "Heat Detect "  },
											{ EVENT_WATER_DETECT, "Water Detect "  },
											{ EVENT_FREEZE_DETECT, "Freeze Detect "  },
											{ EVENT_TAMPER_DETECT, "Tamper Detect "  },
											{ EVENT_MOTION_DETECT, "Motion detect "  },
											{ EVENT_DOOR_CLOSE, "Door Close"  },											
											{ EVENT_VIBRATION_DETECT, "Vibration Detected "  },
											{ EVENT_MOTION_OFF, "Motion Off"  },
											{ EVENT_GATEWAY_ARM, "Arm State On" },
											{ EVENT_GATEWAY_DISARM, "Disarm State On" },
											{ EVENT_BATTERY_LOW, "Battery Low"  },
											{ EVENT_GATEWAY_ARM, "Arm State On" },
											{ EVENT_SWITCH_ON, "Switch On" },
											{ EVENT_SWITCH_OFF, "Switch Off" },
											{ EVENT_PIR_MOTION, "PIR Motion" },
											{ EVENT_PIR_TEMPER, "PIR Temper" },
											{ EVENT_DOOR_OPEN, "Door Open"},
											{ EVENT_MAG_ON, "Mag On"},
											{ EVENT_MAG_OFF, "Mag Close"}	
										 };



//---------------------------------------------------------------------------------
// http header callback
static size_t header_this_callback(char  *ptr, size_t size, size_t nmemb, void *data)  
{  
	DBG_PRINT("header_this_callback msg = %s \n!!!", ptr);

	return 0;
}


//------------------------------------------------------------------------------
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t sz = (size * nmemb);
	char *begin = ptr;
	char *end = begin + sz;
	int len = 0;

	DBG_PRINT("write_callback msg  \n!!!");

	if(size > 0)
	  DBG_PRINT("write_callback msg = %s \n!!!", begin);

	return len;
}

double getTimeTick()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	//return tv.tv_usec/ 1000;
	return tv.tv_sec + tv.tv_usec * 1e-6;

}

static int post_data(char *buffer)
{
    CURLcode curl_res;
    struct curl_slist* headers = NULL;

	//DBG_PRINT("post_data = %s \n", buffer);

 	m_pcurl = curl_easy_init();

	if(m_pcurl == NULL)
    {
        DBG_PRINT("curl_easy_init failed\n!!!");
        return -1;
    }

	int nLen = strlen(buffer);

	curl_easy_setopt(m_pcurl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(m_pcurl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_URL, m_szServerURL);
	curl_easy_setopt(m_pcurl, CURLOPT_HEADERFUNCTION, header_this_callback );  
	curl_easy_setopt(m_pcurl, CURLOPT_FOLLOWLOCATION, 0);
	curl_easy_setopt(m_pcurl, CURLOPT_POST,1);
    curl_easy_setopt(m_pcurl, CURLOPT_HEADER, 1);  
    curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDS, buffer);
	curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDSIZE, nLen);
    headers = curl_slist_append(NULL, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Cookie: atthings-session=s%3AH_5fZesYzii_N2-MrwHjd6OsoH4RAGV0.uMWS3q8zRYrPycU8nAOLKGQeCdj2vba2rdIoEX%2FZy94");

	//headers = curl_slist_append(headers, "Cookie: atthings-session=s%3ANNBF4b9DRjbD5EYn4ejDMR-QYqE872pg.zqcyYkSnOvcBzVyPRKb7r8p8tpIQUBfLbscYBZ%2F2gHg");


    curl_easy_setopt(m_pcurl, CURLOPT_HTTPHEADER, headers);

	
    curl_res = curl_easy_perform(m_pcurl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(m_pcurl);
	
    return 0;
}

void addToQueue(jswcloudevent * pEvent)
{
	DBG_PRINT(" addToQueue did =%d, type = %d, state = %d \n", pEvent->did, pEvent->nType, pEvent->status);

	m_EventAry[m_nEvtWritepos].did = pEvent->did;
	m_EventAry[m_nEvtWritepos].nType= pEvent->nType;
	m_EventAry[m_nEvtWritepos].status= pEvent->status;
	m_EventAry[m_nEvtWritepos].time = pEvent->time;
	if(strlen(pEvent->szName) >0)
		strcpy(m_EventAry[m_nEvtWritepos].szName, pEvent->szName);

	m_nEvtWritepos++;
	m_nCount++;
	
	if( (m_nEvtWritepos >=CLOUD_MAX_EVENT_CNT) || (m_nEvtWritepos < 0) )
		m_nEvtWritepos = 0;
}

int popupFromQueue(jswcloudevent* pCurEvent)
{
	if (m_nCount < 0 || pCurEvent == NULL)
		return false;

	//DBG_PRINT("### popupFromQueue cnt =%d\n", m_nCount);

	memcpy(pCurEvent, &m_EventAry[m_nEvtReadpos], sizeof(jswcloudevent));

	//DBG_PRINT("popupFromQueue next did=%u type=%2d \n", pCurEvent->did, pCurEvent->nType);

 	if (pCurEvent->did != 0)
	{		
		m_nEvtReadpos++;
		m_nCount--;

		if( (m_nEvtReadpos >= CLOUD_MAX_EVENT_CNT) || (m_nEvtReadpos < 0) )
			m_nEvtReadpos = 0;

		if (m_nCount < 0)
			m_nCount = 0;
		
		return 1;
		
	}
	
}

//------------------------------------------------------------------------------
void handle_event_msg()
{
	char szEventName[100];
	time_t timer;
	char szTime[26];
	struct tm* tm_info;
	jswcloudevent jswEvent;

	if(m_nCount == 0)
		return;

	//DBG_PRINT("popupFromQueue count = %d \n", m_nCount);
	if( popupFromQueue(&jswEvent) == 1)
	{	
		DBG_PRINT("handle_event_msg cnt =%d, type = %d , nodename = %s \n", m_nCount, jswEvent.szName);
		int nEventID = getCloudEventID(jswEvent.nType, (int)jswEvent.status, szEventName);

		bool bOnOff = false;
		char szNodeID[32];

		if(jswEvent.nType == TYPE_GATEWAY)
		{		
			if( jswEvent.status == st_arm )
				bOnOff = true;

			ReportGWStateStateToDA(g_gwDID, bOnOff, true);

		}
		else if(jswEvent.nType == TYPE_POWER_ADAPTER)
		{
			bOnOff = get_sensor_status(jswEvent.did);
			sprintf(szNodeID, "%d", jswEvent.did);
			ReportGWStateStateToDA(szNodeID, bOnOff, true);
			updatePowerStatus(jswEvent.did, bOnOff, true);
		}
	 }

}


int GetEventName(int nType, char * pEventName)
{
	int j = 0;

	//DBG_PRINT("GetEventName	type =%d  \n", nType);

	for ( j = 0; j < (sizeof(g_stEventMap) / sizeof(CloudEventTable)); j++ )
	{
		if ( nType == g_stEventMap[j].nCloudType)
		{
			strcpy(pEventName, g_stEventMap[j].szEventName);
			return nType;
		}
	}

	return -1;
}


int getCloudEventID(int nType, int nState, char* pEventName)
{
	int nEvnetID = -1;

	//DBG_PRINT("getCloudEventID jsw Type(%d), state %d \n", (int)nType, nState);
	//if( nType == TYPE_GATEWAY)
	//	DBG_PRINT("event Type(%d), state %d is gateway \n", (int)nType, nState);

	switch(nType)
	{
		case TYPE_GATEWAY:
			{			
				DBG_PRINT("event  is gateway state %d\n", nState);
				if( nState == st_disarm)
					nEvnetID = EVENT_GATEWAY_DISARM;
				else if( nState == st_partarm)
					nEvnetID = EVENT_GATEWAY_PARTARM;
				else if( nState == st_arm)
					nEvnetID = EVENT_GATEWAY_ARM;
			}
		break;
		case TYPE_SIREN_INDOOR:
		case TYPE_SIREN_OUTDOOR:
			{
				if( nState == RE_SIREN_ISON )
					nEvnetID = EVENT_SENSOR_ON;
				else if( nState == RE_SIREN_ISOFF )
					nEvnetID = EVENT_SENSOR_OFF;
			}
			break;
		
		case TYPE_POWER_ADAPTER:
			{
				if( nState == RE_ADAPTER_ISON )
					nEvnetID = EVENT_SWITCH_ON;
				else if( nState == RE_ADAPTER_ISOFF )
					nEvnetID = EVENT_SWITCH_OFF;
			}
			break;	
		case TYPE_PIR:
			{
				nEvnetID = -1;
				if( nState == RE_PIR_MOTION )
				{
					nEvnetID = EVENT_MOTION_DETECT;
					strcpy(pEventName, "Motion detected");
				}else if( nState == RE_PIR_TEMPER )
					nEvnetID = EVENT_PIR_TEMPER;
			}
			break;	

		case TYPE_MAGNETIC:
			{	
				nEvnetID = -1;
				if( nState == RE_MAG_ISON )
				{
					nEvnetID = EVENT_ALARM_REPORT;
					strcpy(pEventName, "Door Opened");
				}
				else if( nState == RE_MAG_ISOFF )
				{
					strcpy(pEventName, "Door Closed");
					nEvnetID = EVENT_ALARM_REPORT;
				}
			}
			break;	

		case TYPE_REMOTE_CONTROL_NEW:
		case TYPE_REMOTE_CONTROL:
			{
			}
			break;	
		case TYPE_SMOKE:
		case TYPE_NEST_SMOKE:
			{
				nEvnetID = -1;
				//if( nState == RE_SMOKE_TRIGGERED )
				//	nEvnetID = EVENT_SMOKE_DETECT;
				//else if( nState == RE_SMOKE_OVERHEAT )
				//	nEvnetID = EVENT_SMOKE_OVERHEAT;
			}
			break;	
			
		case TYPE_WATERLEVEL:
			{
				nEvnetID = -1;
				//if( nState == RE_WATERLEVEL_ARM1 || nState == RE_WATERLEVEL_ARM2)
				//	nEvnetID = EVENT_WATER_DETECT;
			}
			break;
			
		case TYPE_KEYPAD_JSW:
		case TYPE_ACCELEROMETER:
			break;

		case TYPE_CAMERA:
			{
				nEvnetID = -1;// EVENT_MOTION_DETECT; //remove event too much
			}
			break;	
		case TYPE_MCU_COMMAND:
			{
			}
			break;

		case TYPE_NEST_THERMO:
			{
			}
			break;
		case TYPE_NEST_CAM:
			{
			}
			break;
 
	}

	if( nEvnetID > 0)
	{
		if(strlen(pEventName) == 0)
			GetEventName(nEvnetID, pEventName);

		DBG_PRINT("getCloudEventID jsw Type, cloud event id: %d (%s) \n",
			nEvnetID, pEventName );
	}

	return nEvnetID;
}



//------------------------------------------------------------------------------
static void *thread_proc_cmd(void *arg)
{

    while(m_bSendDataToCloud)
    {
		handle_event_msg();	
		sleep(3);
    }

 }



//------------------------------------------------------------------------------
static curl_socket_t socket_callback(void *p,
                                     curlsocktype t,
                                     struct curl_sockaddr *addr)
{
    //curl_socket_t fd = socket(addr->family, addr->socktype, addr->protocol);
    //DBG_PRINT("%s | fd=%d\n", __func__, fd);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//Funtions

int cloud_event_init() 
{
	
	m_lLastKeepAliveTime = getTimeTick();

    memset(m_szToken, 0, sizeof(m_szToken));
	memset(m_szAppID, 0, sizeof(m_szAppID));
	memset(m_szServerURL, 0, sizeof(m_szServerURL));
	memset(m_szEnterprise, 0, sizeof(m_szEnterprise));
	memset(m_szMQTTHost, 0, sizeof(m_szMQTTHost));
	

	return 1 ;
}


//------------------------------------------------------------------------------
int  cloud_event_start(char* pGwDID, char* pGWEntityID)
{
	int err = 0;
	DBG_PRINT("cloud_event_start DID = %s, Entity = %s\n", pGwDID, pGWEntityID );

	strcpy(g_gwDID, pGwDID);
	strcpy(g_szGWEntityID, pGWEntityID);

	DBG_PRINT(" cloud_event_start did = %s  \n", g_gwDID);
	
	char * pEUServer = NULL;
	pEUServer = strstr(g_gwDID, "WGAS");
    if(pEUServer)
    {
    	DBG_PRINT(" cloud_event_start EU svr \n");
    	strcpy(m_szToken, EU_CLOUD_TOKEN);
		strcpy(m_szAppID, EU_APP_ID);
		strcpy(m_szServerURL, EU_SERVER_URL);
		strcpy(m_szEnterprise, EU_ENTERPRISE_ID);
		strcpy(m_szDAReportURL, EU_CLOUD_TOKEN);
		strcpy(m_szMQTTHost, EU_MQTT_HOST);
    }
	else
    {
    	strcpy(m_szToken, UA_CLOUD_TOKEN);
		strcpy(m_szAppID, UA_APP_ID);
		strcpy(m_szServerURL, UA_SERVER_URL);
		strcpy(m_szEnterprise, UA_ENTERPRISE_ID);
		strcpy(m_szDAReportURL, UA_CLOUD_TOKEN);
		strcpy(m_szMQTTHost, UA_MQTT_HOST);
    }

		DBG_PRINT("token =  %s \n", m_szToken);
		DBG_PRINT("APP id =  %s \n", m_szAppID);
		DBG_PRINT("server url =  %s \n", m_szServerURL);
		DBG_PRINT("Enterprise =  %s \n", m_szEnterprise);
		DBG_PRINT("mqtt host =  %s \n", m_szMQTTHost);
		

	m_bSendDataToCloud = true;
	
	err = pthread_create(&m_send_thread, NULL, thread_proc_cmd, NULL);
	 if(err)
	 {	 	 
	 	DBG_PRINT("cloud_event_init thread_proc_cmd error = %d	\n", err);
		return errno;
	 }

     m_lLastKeepAliveTime= getTimeTick();
 

	return 0;
}

//------------------------------------------------------------------------------
void  cloud_event_stop()
{

	DBG_PRINT("####  cloud_event_stop \n");
	
	m_bSendDataToCloud = 0;

    if(m_send_thread)
    {
        pthread_join(m_send_thread, NULL);
        m_send_thread = 0;
    }

	if(m_check_thread)
	{
		pthread_join(m_check_thread, NULL);
		m_check_thread = 0;

	}
	

}

int cloud_event_addEevent(unsigned int did, int nType, int nStatus, char* pName)
{
	if(m_bSendDataToCloud == 0)
		return 0;

	DBG_PRINT("cloud_event_addEevent did = %d , type = %d,  status = %d ,name = %s\n", did, nType, nStatus, pName);

	jswcloudevent eventInfo;

	eventInfo.did = did;
	eventInfo.nType = nType;
	eventInfo.status = nStatus;
	strcpy(eventInfo.szName, pName);

	addToQueue(&eventInfo);
	
	return 1;
}

void ReportGWStateStateToDA(char* pNodeID, bool bOnOff, bool bOnline)
{	
	DBG_PRINT("ReportGWStateStateToDA nodeID = %s\n", pNodeID);

	if(pNodeID == NULL)
		return;
	
	struct curl_slist* headers = NULL;

	CURLcode curl_res;
	cJSON *root;
	char* pOut = NULL;
	char* pDevStatus = NULL;
	
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "AppId", m_szAppID);
	cJSON_AddStringToObject(root, "Token", m_szToken );
	cJSON_AddStringToObject(root, "EntityId", g_szGWEntityID);
	cJSON_AddStringToObject(root, "DeviceId", pNodeID);

	char szLevel[40];
	memset(szLevel, 0, 40);
	char szTag[16],szOnline[16],szOn[16];
	bool bGw = false; 

	memset(szTag, 0, 16);
	
	if(strcmp(pNodeID, g_gwDID) >= 0)
		strcpy(szTag, "isArmed");
	else
		strcpy(szTag, "on");

	if(bOnline)
		strcpy(szOnline, "true");
	else
		strcpy(szOnline, "false");

	if(bOnOff)
		strcpy(szOn, "true");
	else
		strcpy(szOn, "false");

	if(strcmp(pNodeID, g_gwDID) >= 0)
	{			
		if(g_armstate == st_arm )
		 {
			 strcpy(szLevel, "L2");
		 }
		 else if( g_armstate == st_partarm )
		 {
			 strcpy(szLevel, "L1");
		 }
	}
	
	char szData[200];
	if(strlen(szLevel) >0)
	{
		sprintf(szData, "{\"online\":%s,\"%s\":%s,\"currentArmLevel\":\"%s\"}", 
		szOnline, szTag, szOn, szLevel);
		
	}else{
	
		sprintf(szData, "{\"online\":%s,\"%s\":%s}", szOnline, szTag, szOn);		
	}

	DBG_PRINT("####  szData = %s \n", szData);

	
	cJSON* pStatus = cJSON_CreateObject();
		
	if(bOnline)
		 cJSON_AddTrueToObject(pStatus, "online");
	 else
		 cJSON_AddFalseToObject(pStatus, "online");
		
	if (bOnOff)
		cJSON_AddTrueToObject(pStatus, szTag);
	else
		cJSON_AddFalseToObject(pStatus, szTag);

	if(strlen(szLevel) >0)
		 cJSON_AddStringToObject(pStatus, "currentArmLevel", szLevel);

	pDevStatus = cJSON_Print(pStatus); 

    //cJSON_AddStringToObject(root, "DeviceStates", pDevStatus);
	cJSON_AddStringToObject(root, "DeviceStates", szData);

	
	pOut = cJSON_Print(root); 
		
	DBG_PRINT("####	ReportGWStateStateToDA = %s \n", pOut);
	int nLen = strlen(pOut);

	//http cmd
	m_pcurl = curl_easy_init();
				
	curl_easy_setopt(m_pcurl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(m_pcurl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_URL, m_szDAReportURL);
	curl_easy_setopt(m_pcurl, CURLOPT_HEADERFUNCTION, header_this_callback );  
	curl_easy_setopt(m_pcurl, CURLOPT_FOLLOWLOCATION, 0);
	curl_easy_setopt(m_pcurl, CURLOPT_POST,1);
	curl_easy_setopt(m_pcurl, CURLOPT_HEADER, 1);  
	curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDS, pOut );
	curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDSIZE, nLen);
	headers = curl_slist_append(NULL, "Content-Type: application/json");
	//headers = curl_slist_append(headers, "Cookie: atthings-session=s%3AH_5fZesYzii_N2-MrwHjd6OsoH4RAGV0.uMWS3q8zRYrPycU8nAOLKGQeCdj2vba2rdIoEX%2FZy94");
	headers = curl_slist_append(headers, "Cookie: _ga = GA1.1.616510436.1519265767; lifecare - session = s % 3AO8864NRk5 - lIRi755aOVGLMWqtdPB28X.QJjStYnt5OWrVL8AF42MtM1pg68rYOT3xsDf1%2BNGe % 2BY; language = eng");
			
	
	curl_easy_setopt(m_pcurl, CURLOPT_HTTPHEADER, headers);
					   
	curl_res = curl_easy_perform(m_pcurl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(m_pcurl);

	free(pOut);
	free(pStatus);
 			
}

/*
{
	"AppId": "R9200CBJ-SE19FLQW-5WA03KEH",
	"Token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJFbnRlcnByaXNlSWQiOiJIS0U4N081WC1YUDBINUtWTC02Mjk1S0VNRCJ9.oNsyr3tt8sdG1kxZqhlzKLjnCaakVpnOqX6IQ22pGxQ",
	"DeviceId": "WGXX-000281-SXEJS",
	"DeviceData": "V169",
	"Enterprise": "FDP0T21B-2DDTTL5O-8LDP7VE2"
}
*/

void sendKeepAliveToDA()
{
	CURLcode curl_res;
	struct curl_slist* headers = NULL;
		
	//DBG_PRINT("sendKeepAliveToDA \n");

	cJSON *root;
	char* pOut = NULL;
	root = cJSON_CreateObject();	
	
	cJSON_AddStringToObject(root, "AppId", m_szAppID );
	cJSON_AddStringToObject(root, "Token", m_szToken);
	cJSON_AddStringToObject(root, "DeviceId", g_gwDID);
	char szVersion[32];
	sprintf(szVersion, "%d", GATEWAY_VERSION);
	cJSON_AddStringToObject(root, "DeviceData", szVersion);
	cJSON_AddStringToObject(root, "Enterprise", m_szEnterprise);
	pOut = cJSON_Print(root); 

		
	//DBG_PRINT("sendKeepAliveToDA data = %s \n", pOut);
	//http cmd
	m_pcurl = curl_easy_init();
				
	if(m_pcurl == NULL)
	{
		DBG_PRINT("curl_easy_init failed\n!!!");
		 return ;
	}


	int nLen = strlen(pOut);
				
	curl_easy_setopt(m_pcurl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(m_pcurl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_URL, UA_KEEPALIVE_URL);
	curl_easy_setopt(m_pcurl, CURLOPT_HEADERFUNCTION, header_this_callback );  
	curl_easy_setopt(m_pcurl, CURLOPT_FOLLOWLOCATION, 0);
	curl_easy_setopt(m_pcurl, CURLOPT_POST,1);
	curl_easy_setopt(m_pcurl, CURLOPT_HEADER, 1);  
	curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDS, pOut);
	curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDSIZE, nLen);
	headers = curl_slist_append(NULL, "Content-Type: application/json");
	//headers = curl_slist_append(headers, "Cookie: atthings-session=s%3AH_5fZesYzii_N2-MrwHjd6OsoH4RAGV0.uMWS3q8zRYrPycU8nAOLKGQeCdj2vba2rdIoEX%2FZy94");
	headers = curl_slist_append(headers, "Cookie: _ga = GA1.1.616510436.1519265767; lifecare - session = s % 3AO8864NRk5 - lIRi755aOVGLMWqtdPB28X.QJjStYnt5OWrVL8AF42MtM1pg68rYOT3xsDf1%2BNGe % 2BY; language = eng");
			
	
	curl_easy_setopt(m_pcurl, CURLOPT_HTTPHEADER, headers);
					   
	curl_res = curl_easy_perform(m_pcurl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(m_pcurl);

	free(pOut);

}

void ReportAllDeviceToDA()
{
	jswdev devlist[MAXDEVICENUM];
	int nDevCnt = 0;
	bool bOnOff = false;

	printf("ReportAllDeviceToDA intent \n");
	
	nDevCnt = getSensorList((char *)&devlist);
	printf("ReportAllDeviceToDA sensor cnt = %d \n", nDevCnt);

	if( g_armstate == st_arm )
		bOnOff = true;

	ReportGWStateStateToDA(g_gwDID, bOnOff, true);


	int i = 0;

    for( i = 0; i< nDevCnt; i++)
    {      
  	  printf("ReportAllDeviceToDA index = %d, did = %d, type = %d \n", i,devlist[i].did, devlist[i].model);
	  if (devlist[i].model == TYPE_POWER_ADAPTER )
		{
			bOnOff = get_sensor_status(devlist[i].did);
			bool bOnline = get_sensor_alive(devlist[i].did);
			char szID[40];
			sprintf(szID, "%d", devlist[i].did);
			ReportGWStateStateToDA(szID, bOnOff, bOnline);
			updatePowerStatus(devlist[i].did, bOnOff, bOnline);
		}
    }
   
}


