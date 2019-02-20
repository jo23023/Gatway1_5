#ifndef CLOUD_EVENT_SENDER_H
#define CLOUD_EVENT_SENDER_H

#include "jsw_protocol.h"


typedef struct _tagEventTable
{
	int		nCloudType;  
	char    szEventName[100];
	
} CloudEventTable;

typedef enum 
{
  EVENT_PANIC = 2,
  EVENT_ALARM_REPORT = 20001,
  EVENT_SENSOR_ON = 20004,
  EVENT_SENSOR_OFF = 20005,
  EVENT_THERMOSTAT_MODE = 20006,
  EVENT_THERMOSTAT_TMP = 20007,
  EVENT_THERMOSTAT_FAN = 20008,
  EVENT_BATTERY_REPORT = 20009,
  EVENT_SMOKE_DETECT = 20023,
  EVENT_HAET_DETECT = 20026,
  EVENT_WATER_DETECT = 20027,
  EVENT_FREEZE_DETECT = 20028,
  EVENT_TAMPER_DETECT = 20029,
  EVENT_MOTION_DETECT = 20033,
  EVENT_DOOR_CLOSE = 20035,
  EVENT_VIBRATION_DETECT = 20036,
  EVENT_MOTION_OFF = 20038,  
  EVENT_GATEWAY_ARM = 20041,
  EVENT_GATEWAY_DISARM = 20042,
  EVENT_DOOR_BELLING = 20067,
  EVENT_DOOR_OPEN = 20073,
  //new Event
  EVENT_BATTERY_LOW = 30001,
  EVENT_SWITCH_ON = 30002,
  EVENT_SWITCH_OFF = 30003,
  EVENT_PIR_MOTION = 30004,
  EVENT_PIR_TEMPER = 30005,
  EVENT_MAG_ON = 30006,
  EVENT_MAG_OFF = 30007,
  EVENT_SMOKE_OVERHEAT = 30008,
  EVENT_GATEWAY_PARTARM = 30009

  
}CLOUD_EVNET_TYPE;


////////////////////////////////////////////////////////////////////////////////////////////
// Function


int 	cloud_event_init();	
int  	cloud_event_start(char* pGwDID, char* pGWEntityID);
void  	cloud_event_stop();
int 	cloud_event_addEevent(unsigned int did, int nType, int nStatus, char* pName);

void ReportAllDeviceToDA();


#endif
		
