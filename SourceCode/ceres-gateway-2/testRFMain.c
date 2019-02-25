#include "jsw_rf_api.h"
#include <pthread.h>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "rf_module.h"
#include "jsw_protocol.h"

#define TRUE	1
#define FALSE	0

#define PLATFORM		"SN98601"
#define COMPORT			"/dev/ttyS1"	//SN98601
//#define COMPORT			"/dev/ttyS0"	//RT5350

//#define WATCH_TEMP_FILENAME "./watchtemp"
//#define CAT_WATCH_TEMP_DUR  1800 //30 mins


pthread_t threadID;
fd_set set;
struct termios opt;
struct timeval timeout;
int g_temp_rec = 0;

unsigned char Rxbuf[20],Txbuf[20];
ReportRF Report;
int fd ;

int lastmcuinfocmd = 0;
int g_smokeBattLowLastTime = 0;//Last smoke battery low time
int g_scenario_start_by_mag = 0;//Start scenario by MAG

extern char g_entrydelay_msg[4000];
extern int g_get_temp;// = 0;//count for write get temp command to uart
extern int g_smokeOnLastTime;// = 0; //Last smoke trigger time
extern int g_battlowcount;// = 0;

void mcuinfo(char buf)
{


}

void mcuCallBack(void *data){

	int status;
	int k;
	char msg[4000]; //push
	jswdev* dev;
	int istrigger = 0;
	int update_vibration =0;
	static int s_last_vibration = 0;

	ReportRF *ptr = (ReportRF*)data;
	printf("---FromMCU ");
	printf("-ID: %08X  - Type: %2X ",ptr->DeviceID, ptr->DeviceType);
	printf("-Status: %02X %02X %02X %02X -\r\n", *(ptr->Status),   *(ptr->Status+1),
										   *(ptr->Status+2), *(ptr->Status+3));

	if(ptr->DeviceType == TYPE_UNKNOW)
	{
        SetRunningLog_str_int("Error: mcuCallBack() device type unknown", ptr->DeviceType);
		return;
	}

	if ( ptr->DeviceType == TYPE_MCU_COMMAND )
	{
//		memcpy(mcuversion, &ptr->Status, sizeof(mcuversion));
		//memcpy(mcuversion, "1411", sizeof(mcuversion)); //for MCU 1411 only
//		printf("recieve type_mcu ack %s\n", mcuversion);
		//if(g_setting.gwprop.mcu_version <= 0)
		{
            int vv = 0;
            unsigned char bb = ptr->Status[0]-'0';
            vv += (bb*1000);
            bb = ptr->Status[1]-'0';
            vv += (bb*100);
            bb = ptr->Status[2]-'0';
            vv += (bb*10);
            bb = ptr->Status[3]-'0';
            vv += (bb*1);
			printf("vv=%d\n", vv);
            if( (vv >= 1000) && (vv <= 9999) )
            {
            	get_mcu_version = 1;
                g_setting.gwprop.mcu_version = vv;
				memcpy(mcuversion, &ptr->Status, sizeof(mcuversion));
				//memcpy(mcuversion, "1411", sizeof(mcuversion)); //for MCU 1411 only
				printf("recieve type_mcu ack %s\n", mcuversion);
            }

            //g_setting.gwprop.mcu_version = 1411; //for MCU 1411 only
			printf("g_setting.gwprop.mcu_version=%d\n", g_setting.gwprop.mcu_version);
		}

		//sendCmdtoClient(0xAB, 0, 1, sizeof(int), &ptr->Status);

		/*if ( ptr->Status[0] == CMD_GW_GET_VERSION)
			sendCmdtoClient(CM_MCU_VERSION, 0, 1, sizeof(int), &ptr->Status);
		else if ( ptr->Status[0] == CMD_GW_GET_CUSTOMER)
			sendCmdtoClient(CM_MCU_CUSTOMER, 0, 1, sizeof(int), &ptr->Status);
		else if ( ptr->Status[0] == CMD_GW_GET_CHANNEL)
			sendCmdtoClient(CM_MCU_CHANNEL, 0, 1, sizeof(int), &ptr->Status);
		else if ( ptr->Status[0] == CMD_GW_GET_DEFAULT_SYNCWORD)
			sendCmdtoClient(CM_MCU_DEFAULT_SYNCWORD, 0, 1, sizeof(int), &ptr->Status);*/
		return;
	}
	if ( ptr->Status[0] == RE_SENSOR_PAIR)
	{
		if (g_isPairMode ==1)
		{
			if ( uartbusy ==1)
				uartbusy = 0;
			newDevFromMcu(ptr->DeviceID, ptr->DeviceType);
		}
		return;
	}

	if(ptr->DeviceType == TYPE_MCU)
	{
		status = ptr->Status[0];
		if( (status == RE_MCU_TEMP_TIMEOUT) || (status == RE_MCU_RCV_ERROR) || (status == RE_MCU_RESET_RF_IC) ||
            (status == RE_MCU_RESET_RF_IC2) )
		{//tag for reboot from MCU
			if(status == RE_MCU_TEMP_TIMEOUT)
			{
			    log_and_reboot_no_beep("MCU reboot by RE_MCU_TEMP_TIMEOUT", 0);
			}else if(status == RE_MCU_RCV_ERROR)
			{
			    log_and_reboot_no_beep("MCU reboot by RE_MCU_RCV_ERROR", 0);
			}else if(status == RE_MCU_RESET_RF_IC)
			{
			    //log_and_reboot_no_beep("MCU reset RF IC", 0);
                printf("RE_MCU_RESET_RF_IC\n");
			}else if(status == RE_MCU_RESET_RF_IC2)
			{
			    //log_and_reboot_no_beep("MCU reset RF IC2", 0);
                printf("RE_MCU_RESET_RF_IC2\n");
			}
			return;
		}

        //MCU update failed, update again
		if(status == RE_MCU_WAIT_FOR_UPDATE)
		{
            system("rm ./NewMCU1.sn8");
            usleep(100*1000);
            system("cp ./GatewayMCU.sn8 ./NewMCU1.sn8");
            sync();

            if(0 == access("./NewMCU1.sn8", 0))
            {//start MCU update (filename ./NewMCU2.sn8)
                system("reboot");
            }
            return;
		}
	}


	if ( ( ptr->DeviceType != TYPE_ONBOARD_TEMP ) && ( ptr->DeviceType != TYPE_MCU ) )
	{
		if (isDevRegister(ptr->DeviceID) == 0 )
		{
			printf("Device not registered, -Device Type:  %2X -\r\n",ptr->DeviceType);
            SetRunningLog_str_int("Error: mcuCallBack() did not found", ptr->DeviceID);
			return;
		}
	}
	dev = getDevbydid(ptr->DeviceID);

    //no smoke battery low in seconds
    if ( ptr->DeviceType == TYPE_SMOKE)
    {
        if(ptr->Status[0] == RE_SENSOR_BATLOW)
        {
            int lasttime2 = time(NULL);
            if( ((lasttime2 - g_smokeBattLowLastTime) <= 30) && ((lasttime2 - g_smokeBattLowLastTime) >= 0) )
                return;
            g_smokeBattLowLastTime = lasttime2;
        }
    }

    //Vibration ignore same cmd in 2 secs
    if(ptr->DeviceType == TYPE_VIBRATION)
    {
        int lasttime3 = time(NULL);
        int diff3 = lasttime3-s_last_vibration;
        if( (diff3 <= 2) && (diff3 >= 0) )
        {
            printf("----Ignore Vibration event, did=%u----\n", ptr->DeviceID);
            return;
        }
        s_last_vibration = lasttime3;
    }

	if ( ptr->DeviceType == TYPE_ONBOARD_TEMP )
	{
		//memcpy(&status ,ptr->Status, sizeof(int));
		short hum;
		short temp;
		char *pt;
		pt = (char*)&ptr->Status[1];

		hum = ptr->Status[0];
		memcpy(&temp, pt, sizeof(short));

		g_humid = hum;
		g_temp = temp / 10;

		//printf("temp event humidity = %d temp = %d\n", hum, temp);

        //watch temp (watch MCU)
        int time_now = time((time_t*)NULL);
        if( (time_now > 1400000000) && (time_now < 1800000000) )
        {//legal time duration, 2015-2026
            if(difftime(time_now, g_temp_rec) >= CAT_WATCH_TEMP_DUR)
            {
                g_temp_rec = time_now;
                char cmd2[256];
                sprintf(cmd2, "date > %s", WATCH_TEMP_FILENAME);
                system(cmd2);
                sync();
            }
        }

		g_get_temp = 0; //reset heartbeat for UART/RF/MCU blocking
	}else
	{
		status = ptr->Status[0];
		if (status ==RE_ABUS_AUTOREPORT)
		{
			//keep alive message
			for (k =0;k<devcount;k++)
			{
				if (keepalivelist[k].did == ptr->DeviceID)
					keepalivelist[k].lastcheckin = time(0);

				if (dev != NULL)
				{
					if (dev->status ==RE_ABUS_AUTOREPORT)//Make sure TX target is back to life or GW wont send command to it.
					{
						if ( isTarget(ptr->DeviceType ))
							dev->status = 0;
					}
				}
			}

			return;
		}

		//judge save event first or later
		unsigned char save_event_first = 0; //1: save event first, 0:save event later
		if( ((ptr->DeviceType == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_UNLOCK)) || //remote disarm
            ((ptr->DeviceType == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_PANIC)) || //remote panic
            ((ptr->DeviceType == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_UNLOCK)) || //remote disarm
            ((ptr->DeviceType == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_PANIC)) || //remote panic
            ((ptr->DeviceType == TYPE_GATEWAY) && (status == GW_PANIC)) || //system panic
            ((ptr->DeviceType == TYPE_GATEWAY) && (status == 0)) || //system disarm
            ((ptr->DeviceType == TYPE_PIR) && (status == RE_PIR_TEMPER)) || //PIR tamper
			//((ptr->DeviceType == TYPE_VIBRATION) && (status == RE_VIBRATION_TRIGGER)) || //Viberation trigger
			((ptr->DeviceType == TYPE_VIBRATION) && (status == RE_VIBRATION_TAMPER)) || //Viberation tempper
			((ptr->DeviceType == TYPE_BUTTON) && (status == RE_BUTTON_PRESS)) || //Button press
			((ptr->DeviceType == TYPE_BUTTON) && (status == RE_BUTTON_LONG_PRESS)) || //button long press
            ((ptr->DeviceType == TYPE_MAGNETIC) && (status == RE_MAG_TEMPER)) || //MAG tampe
            ((ptr->DeviceType == TYPE_SIREN_OUTDOOR) && (status == RE_SIREN_TEMPER)) || //Siren tampe
            ((ptr->DeviceType == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_TEMPER)) || //remote tamper
            ((ptr->DeviceType == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_TEMPER)) ) //remote tamper
		{
            save_event_first = 1;
		}
		//second judgement, for trigger only
		if(save_event_first == 0)
		{
		    if( (g_armstate == st_partarm) || (st_arm == g_armstate ) )
		    {
		        if( ((ptr->DeviceType == TYPE_MAGNETIC) && (status == RE_MAG_ISON)) || //Mag open trigger
                    ((ptr->DeviceType == TYPE_PIR) && (status == RE_PIR_MOTION)) || //PIR motion trigger
                    ((ptr->DeviceType == TYPE_VIBRATION) && (status == RE_VIBRATION_TRIGGER)) ) //Vibration trigger
		        {
					if(checkDevFlag(ptr->DeviceID))
					{
                        istrigger = 1;
                        save_event_first = 1;
					}
		        }
		    }
		}
		//save event first
		if(save_event_first == 1)
		{
		    //save event with trigger
		    if( (g_armstate == st_partarm) || (st_arm == g_armstate ) )
		    {
		        if( ((ptr->DeviceType == TYPE_MAGNETIC) && (status == RE_MAG_ISON)) || //Mag open trigger
                    ((ptr->DeviceType == TYPE_PIR) && (status == RE_PIR_MOTION)) || //PIR motion trigger
                    ((ptr->DeviceType == TYPE_VIBRATION) && (status == RE_VIBRATION_TRIGGER)) ) //Vibration trigger
		        {
					if(checkDevFlag(ptr->DeviceID))
					{
                        istrigger = 1;
					}
		        }
		    }
		    //always with trigger status
            if(dev)
            {
                if( (dev->model == TYPE_VIBRATION) && (dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE) )
                {//vibration tamper mode, must be trigger event
                    istrigger = 1;
                }
            }

            newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger);
		}

		//dispatch
		if( (ptr->DeviceType == TYPE_REMOTE_CONTROL) || (ptr->DeviceType ==TYPE_KEYPAD_JSW) ||
            (ptr->DeviceType == TYPE_REMOTE_CONTROL_NEW) || (ptr->DeviceType ==TYPE_KEYPAD_JSW_NEW) )
		{
			switch(status){
				case RE_REMOTE_CAMERA:
					startRec();
					break;
				case RE_REMOTE_PANIC:
					panic();
					break;
				case RE_REMOTE_UNLOCK:
					armCommand(c_disarm);
					break;
				case RE_REMOTE_LOCK:
					armCommand(c_arm);
					break;
				case RE_REMOTE_TEMPER:
					triggerAlarm(1, 0);
					sendTamperMsg(ptr->DeviceID);

					break;

				case RE_KEYPAD_PART_ARM:
					armCommand(c_partarm);
					break;
				default:
					break;
			}

		}
		if ( ptr->DeviceType == TYPE_PIR )
		{
			if (ptr->Status[0] == RE_PIR_MOTION)
			{
				if (g_armstate == st_partarm || st_arm == g_armstate )
				{
					if (checkDevFlag(ptr->DeviceID))
					{
						//newEvent(ptr->DeviceID, ptr->DeviceType, DEV_TRIGGER);
						memset(g_entrydelay_msg, 0, sizeof(g_entrydelay_msg));
						istrigger = 1;
						if (g_sirenstate==0)
							entryDelay(ptr->DeviceID);

						if (dev != NULL)
						{
#ifdef DEF_FEATURE_MULTI_PUSH
                            char msg2[256];
                            get_push_string(msg2, STR_ID_ALARM);
                            sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
                            sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
							if (g_setting.gwprop.entrydelay >0)
							{
								if(strlen(msg) > 0)
									strcpy(g_entrydelay_msg, msg);
							}else
							{
								if (strlen(msg) != 0)
                                    pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
							}
						}


					}
				}
				else if (g_armstate == st_disarm)
			//	{
					//do scenario.
					if(check_sensor_with_Scenario(ptr->DeviceID, ptr->Status[0],TYPE_PIR))
						{
						newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
						save_event_first = 1;
						}
					else
						{
						// ignore PIR in disarm mode w/o Scenario
						save_event_first = 1;
						}
					startScenario(ptr->DeviceID, ptr->Status[0] );
				//}
			}else if (ptr->Status[0] == RE_PIR_TEMPER)
			{
				newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
				save_event_first = 1;
				triggerAlarm(1, 0);
				sendTamperMsg(ptr->DeviceID);
			}
		}
		else if ( ptr->DeviceType == TYPE_VIBRATION)
		{
			if (ptr->Status[0] == RE_VIBRATION_TRIGGER)
			{
				update_vibration = 1;
				if( dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE)
                {
                    if(g_armstate != st_testmode)
                        g_armstate = st_arm;
				}
				if (g_armstate == st_partarm || st_arm == g_armstate )
				{
					if( (checkDevFlag(ptr->DeviceID)) || (dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE) )
					{
						//newEvent(ptr->DeviceID, ptr->DeviceType, DEV_TRIGGER);
						memset(g_entrydelay_msg, 0, sizeof(g_entrydelay_msg));
						istrigger = 1;
						if (g_sirenstate==0)
						{
                            if( dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE)
                            {//24H mode
                                triggerAlarm(0, 0);
                            }else
                            {//trigger mode
                                entryDelay(dev->did);
                            }
						}

						if (dev != NULL)
						{
#ifdef DEF_FEATURE_MULTI_PUSH
                            char msg2[256];
                            get_push_string(msg2, STR_ID_ALARM);
                            sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
                            sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
                            if( (strlen(msg) < 10) || (strlen(msg) >= sizeof(msg)) )
                                strcpy(msg, DEF_ERROR_PUSH_STRING);
                            if( dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE)
                            {//24H mode
                                if (strlen(msg) != 0)
                                    pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
                            }else
                            {//trigger mode
                                if (g_setting.gwprop.entrydelay > 0)
                                {
                                    if(strlen(msg) > 0)
                                        strcpy(g_entrydelay_msg, msg);
                                }else
                                {
                                    if (strlen(msg) != 0)
                                        pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
                                }
                            }
						}
					}
				}
				else if (g_armstate == st_disarm)
			//	{
					//do scenario.
					startScenario(ptr->DeviceID, ptr->Status[0] );
				//}
            }
			else if (ptr->Status[0] == RE_VIBRATION_TAMPER)
			{
				triggerAlarm(1, 0);
				sendTamperMsg(ptr->DeviceID);
			}
		}
        else if ( ptr->DeviceType == TYPE_BUTTON)
        {
            if (ptr->Status[0] == RE_BUTTON_PRESS || ptr->Status[0] == RE_BUTTON_LONG_PRESS)
            {
                if (g_armstate == st_partarm || st_arm == g_armstate )
                {
                    if (checkDevFlag(ptr->DeviceID))
                    {
                        //newEvent(ptr->DeviceID, ptr->DeviceType, DEV_TRIGGER);
                        memset(g_entrydelay_msg, 0, sizeof(g_entrydelay_msg));
                        istrigger = 1;
                        if (g_sirenstate==0)
                            entryDelay(dev->did);

                        if (dev != NULL)
                        {
#ifdef DEF_FEATURE_MULTI_PUSH
                            char msg2[256];
                            get_push_string(msg2, STR_ID_ALARM);
                            sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
                            sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
                            if( (strlen(msg) < 10) || (strlen(msg) >= sizeof(msg)) )
                                strcpy(msg, DEF_ERROR_PUSH_STRING);
                            if (g_setting.gwprop.entrydelay > 0)
                            {
                                if(strlen(msg) > 0)
                                    strcpy(g_entrydelay_msg, msg);
                            }else
                            {
                                if (strlen(msg) != 0)
                                    pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
                            }
                        }
                    }
                }
                else if (g_armstate == st_disarm)
            //	{
                    //do scenario.
                    startScenario(ptr->DeviceID, ptr->Status[0] );
                //}
            }
            /* else if (ptr->Status[0] == RE_PIR_TEMPER)
            {
                triggerAlarm(1, 0);
                sendTamperMsg(ptr->DeviceID);
            }
            */
        }
		else if ( ptr->DeviceType == TYPE_MAGNETIC )
		{
			if (ptr->Status[0] == RE_MAG_ISON)
			{
				if (g_armstate == st_partarm || st_arm == g_armstate )
				{
					//printf("MAG RE_MAG_ISON OPEN state = %d\n",g_armstate );
					//newEvent(ptr->DeviceID, ptr->DeviceType, DEV_TRIGGER);

					if (checkDevFlag(ptr->DeviceID))
					{
						//printf("MAG checkDevFlag pass\n");
                        istrigger = 1;
						memset(g_entrydelay_msg, 0, sizeof(g_entrydelay_msg));
						if (g_sirenstate == 0)
							entryDelay(ptr->DeviceID);
						 if (dev != NULL)
						 {
#ifdef DEF_FEATURE_MULTI_PUSH
                            char msg2[256];
                            get_push_string(msg2, STR_ID_ALARM);
                            sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
                            sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
							if (g_setting.gwprop.entrydelay >0)
							{
								if(strlen(msg) > 0)
									strcpy(g_entrydelay_msg, msg);
							}else
							{
								if (strlen(msg) != 0)
                                    pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
							}
						 }

					}
					startScenario(ptr->DeviceID, ptr->Status[0] );
				}
				else if (g_armstate == st_disarm)
				{
					//do scenario.
					if(check_sensor_with_Scenario(ptr->DeviceID, ptr->Status[0],TYPE_MAGNETIC))
                    {
						newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
						save_event_first = 1;
                    }
					startScenario(ptr->DeviceID, ptr->Status[0] );
				}
				if(g_armstate == st_testmode)
                    g_scenario_start_by_mag = 1;
			}
			else if (ptr->Status[0] == RE_MAG_TEMPER)
			{
				//printf("MAG TAMPER \n" );
				newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
				save_event_first = 1;
				triggerAlarm(1, 0);
				sendTamperMsg(ptr->DeviceID);
			}
			else if (ptr->Status[0] == RE_MAG_ISOFF)//CLOSE
			{
				//printf("MAG RE_MAG_ISOFF CLOSE state = %d\n",g_armstate );
//without checking for arm state here.

				if(check_sensor_with_Scenario(ptr->DeviceID, ptr->Status[0],TYPE_MAGNETIC))
					{
					newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
					save_event_first = 1;
					}
				startScenario(ptr->DeviceID, ptr->Status[0] );

				if(g_armstate == st_testmode)
                    g_scenario_start_by_mag = 1;

				if (g_armstate == st_disarm)
				{
					//int r = checkDoorOpen(c_disarm);
					//if (r == 0)
					if(getMAGOpenCount() <= 1)
                        if(g_battlowcount <= 0)
                            ledoff();
                    pushnotification2("{\"on\":false,\"alert\":\"none\"}");
				}


			}
			if( (status == RE_MAG_ISON) || (status == RE_MAG_ISOFF) )
			{//save MAG status to file
				if (dev != NULL)
				{
				    dev->status = status;
                    savedata();
				}
			}
		}
		if ( ptr->DeviceType == TYPE_SMOKE )
		{
			if ( (ptr->Status[0] == RE_SMOKE_TRIGGERED) || (ptr->Status[0] == RE_SMOKE_OVERHEAT) )
			{
				if (g_smokeOn == 0)
				{
					g_smokeOn = 1;
					if (smoke_t != 0)
					{
						makeTimer( &smoke_t, 0, 0);
						smoke_t = 0;
					}
					makeTimer( &smoke_t, 30/*g_setting.gwprop.duration*/, 0);
				}
				else
					return; //filter out.

				if(checkDevFlag(ptr->DeviceID))
				{
					//no smoke alarm again in 30 seconds
					int lasttime = time(NULL);
					if( ((lasttime - g_smokeOnLastTime) <= 30) && ((lasttime - g_smokeOnLastTime) >= 0) )
						return;
					g_smokeOnLastTime = lasttime;

					//printf("Smoke triggered g_sirenstate = %d\n", g_sirenstate);
					istrigger = 1;
					newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
					save_event_first = 1;
					triggerAlarm(1, 1);

					if (dev != NULL)
					{
#ifdef DEF_FEATURE_MULTI_PUSH
						char msg2[256];
						get_push_string(msg2, STR_ID_ALARM);
						sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
						if (strlen(msg) != 0)
							pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
					}
				}

				if(dev)
					startScenario(dev->did, status);
			}

		}
		if ( ptr->DeviceType ==TYPE_WATERLEVEL)
		{
		printf("TYPE_WATERLEVEL (%d)\n",(int)ptr->Status[0]);
		// Jeff remark  old code
		#if 1
			if (ptr->Status[0] == RE_WATERLEVEL_ARM1 || ptr->Status[0] == RE_WATERLEVEL_ARM2)
			{
//				newEvent(ptr->DeviceID, ptr->DeviceType, DEV_TRIGGER);
					if (dev != NULL)
					 {
					 	newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
						save_event_first = 1;
						triggerAlarm(1, 1);
#ifdef DEF_FEATURE_MULTI_PUSH
                        char msg2[256];
                        get_push_string(msg2, STR_ID_ALARM);
                        sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
                        sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
                        if( (strlen(msg) < 10) || (strlen(msg) >= sizeof(msg)) )
                            strcpy(msg, DEF_ERROR_PUSH_STRING);
                        if (strlen(msg) != 0)
							pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
					 }
			}
		#else
			printf("Water leak tigger %d\n",ptr->Status[0]);
			if (ptr->Status[0] == RE_WATERLEVEL_ARM1 || ptr->Status[0] == RE_WATERLEVEL_ARM2)
			{

				if (g_armstate == st_partarm || st_arm == g_armstate )
				{
					if (checkDevFlag(ptr->DeviceID))
					{
						//newEvent(ptr->DeviceID, ptr->DeviceType, DEV_TRIGGER);
						memset(g_entrydelay_msg, 0, sizeof(g_entrydelay_msg));
						istrigger = 1;
						if (g_sirenstate==0)
							entryDelay();

						if (dev != NULL)
						{
#ifdef DEF_FEATURE_MULTI_PUSH
                            char msg2[256];
                            get_push_string(msg2, STR_ID_ALARM);
                            sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
                            sprintf(msg, STRING_TRIGGER_ALARM, dev->name, dev->location, gDID );
#endif
							if (g_setting.gwprop.entrydelay >0)
							{
								if(strlen(msg) > 0)
									strcpy(g_entrydelay_msg, msg);
							}else
							{
								if (strlen(msg) != 0)
                                    pushnotification(NT_TRIGGER, msg, STR_ID_ALARM);//trigger
							}
						}


					}
				}
				//else if (g_armstate == st_disarm)
			//	{
					//do scenario.
					startScenario(ptr->DeviceID, ptr->Status[0] );
				//}
			}


		#endif

		}
		if ( ptr->DeviceType ==TYPE_SIREN_INDOOR || ptr->DeviceType ==TYPE_SIREN_OUTDOOR)
		{
			if (ptr->Status[0] == RE_SIREN_TEMPER)
			{
			    if(g_armstate != st_maintain)
			    {
			    	newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
					save_event_first = 1;
                    triggerAlarm(1, 0);
                    sendTamperMsg(ptr->DeviceID);
			    }
			set_sensor_alive(ptr->DeviceID);
			//apple 1226
			updateSensorStatus(ptr->DeviceType, ptr->DeviceID);
			}
		}
		if ( ptr->DeviceType == TYPE_POWER_ADAPTER)
		{

			if(status == RE_SENSOR_BATLOW)
				return;
			if( (status == RE_ADAPTER_ISON) || (status == RE_ADAPTER_ISOFF) )
			{//save power switch status to file
				if (dev != NULL)
				{
				    dev->status = status;
                    savedata();
			    	//newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
				}

				set_sensor_alive(ptr->DeviceID);
				//apple 1226
				updateSensorStatus(ptr->DeviceType,  ptr->DeviceID);
			}
		}

		//save event later
		if(save_event_first != 1)
		{
            newEvent(ptr->DeviceID, ptr->DeviceType,  status, istrigger );
		}
	}

	if(update_vibration)
    {
		writeCommand(ptr->DeviceID, ptr->DeviceType, dev->ext1);
	}
	update_vibration=0;

	g_scenario_start_by_mag = 0;
}

unsigned char Init_UART(int fd_ComPort,int BaudRate){

	tcgetattr(fd_ComPort, &opt);
	tcflush(fd_ComPort, TCIOFLUSH);
	cfsetispeed(&opt, BaudRate);
	cfsetospeed(&opt, BaudRate);
	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;				//bit 8
	opt.c_cflag &= ~PARENB;			//
	opt.c_iflag &= ~INPCK;			//priority none
	opt.c_cflag &= ~CSTOPB;			//stop bit 1

	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);			/*Input*/
	opt.c_oflag &= ~OPOST;									/*Output*/
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);  //
    opt.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    opt.c_oflag &= ~(ONLCR | OCRNL);

	opt.c_cc[VTIME] = (PLATFORM == "SN98601")? 150 :100;//150; //15 seconds 150x100ms = 15s
	opt.c_cc[VMIN] = 12;

	tcflush(fd_ComPort, TCIFLUSH); //update the options and do it now

	tcsetattr(fd_ComPort,TCSANOW,&opt);	//TCSANOW :restart terminal now.


	if(fcntl(fd_ComPort, F_SETFL, 0)<0){
		printf("fcntl failed!\n");
		return FALSE;
	}
    else
        printf("fcntl=%d\n",fcntl(fd_ComPort, F_SETFL,0));
  /*  if(isatty(STDIN_FILENO)==0){
        printf("standard input is not a terminal device\n");
		return FALSE;
	}
    else
        printf("isatty success!\n");  */

  //  printf("fd-open=%d\n",fd_ComPort);
	return TRUE;

}

void setuarWriteTimer(int mode)
{
	if (mode == 1)
	{

	}else
	{

	}
}
int writePairing(unsigned int deviceID, unsigned char type, unsigned char cmd)
{
	//printf("Write Pairing DID : %u,Device Type:  %2X\n", deviceID, type);
	if (uartbusy ==1)
	{
		printf("trying to write pairing but UART busy\n");
		return 0;
	}

	if (type > 0) //zero is cancel.
		uartbusy =1; //block other until 0xAA return , or time up , or cancel pairing

	RFModuleWrite( deviceID, type, cmd);
#ifndef SERIAL_RAW
	if(PLATFORM == "SN98601")		//SN98601's UART is queer that need write 2 times....
		RFModuleWrite( deviceID, type, cmd);
#endif
	return 1;
}

//apple
int writeCommand(unsigned int deviceID, unsigned char type, unsigned char cmd)
{
	jswdev *dev;
	printf("writeCommand to mcu with(%08X,%02X,%02X), UART Busy = %d\n",deviceID,type,cmd,uartbusy);
	if (TYPE_MCU_COMMAND == type)
	{
		RFModuleWrite( deviceID, type, cmd);
#ifndef SERIAL_RAW
		if(PLATFORM == "SN98601")		//SN98601's UART is queer that need write 2 times....
			RFModuleWrite( deviceID, type, cmd);
#endif

		return 1;
	}

	dev = getDevbydid(deviceID);
#if 0
	// Jeff mark for NoSignal retry

	if (dev != NULL)
	{

		if (dev->status == RE_ABUS_AUTOREPORT)
		{
			printf("writecommand to dev %u ignore because it is offline\n",dev->did );
			return 0;
		}
	}

#endif
	if (uartbusy ==1)
	{
		if (deviceID != 0x12345678 )
		{
			if ( checkRepeatCmd(deviceID, cmd) ==0)
				return 0;
		}
		//if still busy then put it into queue and leave
		printf("UART busy, put into queue did %u  type %02X, cmd  %02X\n", deviceID, type, cmd);

		txqueue[txwritepos].did = deviceID;
		txqueue[txwritepos].type = type;
		txqueue[txwritepos].cmd = cmd;
		txqueue[txwritepos].time = 0;
		txqueue[txwritepos].errCount = 0;
		txwritepos++;
		txCount++;
		if( (txwritepos >=TXITEMMAX) || (txwritepos < 0) )
			txwritepos = 0;

		return 1;
	}else
	{
		//first time
		currenttx.did = deviceID;
		currenttx.time = timegettime();
		currenttx.errCount = 0;
		currenttx.type = type;
		currenttx.cmd = cmd;

		uartbusy = 1;
	}

	RFModuleWrite( deviceID, type, cmd);
#ifndef SERIAL_RAW
	if(PLATFORM == "SN98601")		//SN98601's UART is queer that need write 2 times....
		RFModuleWrite( deviceID, type, cmd);
#endif
	usleep(10000);
	return 1;

}


int serial_main(void){

	fd = open(  COMPORT, O_RDWR); /* Get the Comport's address */
	//printf("@@@###@@@@serial open %d\n", fd);

	if(Init_UART(fd,B57600) == FALSE)
	{
		printf("#####Init_UART error\n");
		return -1;
	}

	//printf("###uart init\n");
	if(	RFModule_Init(fd,Rxbuf,Txbuf, mcuCallBack) == TRUE )
	{
		//printf("Init seuccess\r\n");
	}
	else
	{
		printf("#####RFModule_Init error\n");
		return -1;
	}
#ifdef SERIAL_RAW
	struct termios raw;
	cfmakeraw(&raw);
	raw.c_lflag &= ~ICANON;
	raw.c_cc[VMIN] = READ_BUF_SIZE;
	raw.c_cc[VTIME] = 1;
	raw.c_cflag = B57600|CS8|CLOCAL|CREAD;

/* put terminal in raw mode after flushing */
if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
{
	printf("tty set raw mode err: %s\n");
}



#endif

	if(pthread_create(&threadID, NULL,(void*)RFModuleRead, &Report) != 0)
		return -1;

	return 1;


}

