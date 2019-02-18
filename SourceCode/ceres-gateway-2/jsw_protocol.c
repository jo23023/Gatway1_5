#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <crypt.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ceres.h"
#include "jsw_protocol.h"
#include "AES/AES_utils.h"
#include "jsw_rf_api.h"
#include <arpa/inet.h>
#include "ceres_util.h"

#include "snx_gpio.h"
#include "cloud_event_sender.h"

//apple
//#include "cloud_event_sender.h"

pthread_t alarm_th;
pthread_t alarmstop_th;

extern pthread_t philipt_hue_handle;// = NULL;

const short flagArmset = 0x1;
const short flagpartArmset = 0x2;
const short flagpanicset = 0x4;
const short flagcamrecord = 0x8;

int g_smokeOn = 0;
int g_smokeOnLastTime = 0; //Last smoke trigger time
struct generalinfo g_setting;
struct setting4info g_setting4;
struct setting5info g_setting5;

#define NEST_SUPPORT

stNEST_parm g_stNEST_parm;
int g_NEST_running =0;
char g_NEST_active_token[256];
stUSER_parm g_stUserParam;
int g_cloud_running = 0;
struct googleSetting g_stDAParam;

jswscenario scenariolist[SCENARIOSIZE];
int srcCount =0;

jswdev devlist[MAXDEVICENUM];
int devcount = 0;

jswevent o_evtlist[MAXEVENT];
jsw_alarm_event last_trigger;
jswevent evtlist[MAXEVENT];
int evtcount =0;
int is_oldevtexist = 0;

struct _zonelist zonelist[MAXZONE];
int zoneCount;

struct pa_prop pa_proplist[MAXDEVICENUM];
int paCount;

int g_armstate=st_disarm; //0=disarm, 1 = exit delay, 2, entry delay, 3, arm.
int g_sirenstate=0; // 0=off, 1=on.
int g_ShowAlarmView=0; //0=not show, 1=Show.
int g_AlarmState=ALARM_STATE_NO_ALARM;

short exitdelayTarget = st_arm;
short entrydelayState =0;

stExitDelay_parm g_ExitDelay_parm;
stEntryDelay_parm g_EntryDelay_parm;


jswpush pushdata;
struct em emaillist[6];
jswOther m_othersData;

char macaddr[32];

struct keepalive keepalivelist[MAXDEVICENUM];
char mag_open_list[MAXDEVICENUM][32];


//battery low list
jswbattlow g_battlowlist[MAXDEVICENUM];
int g_battlowcount = 0;

jswsunrisesunset g_sunrisesunset[2]; //0 for today, 1 for tomorrow

char g_curl_output[DEF_CURL_OUTPUT_BUFFER_LEN]; //curl output buffer length

pthread_mutex_t getevent_mutex;



extern char g_entrydelay_msg[4000];

extern int g_selftest_flag;// = 0;//self-test flag, write to running log

extern int g_scenario_start_by_mag;// = 0;//Start scenario by MAG

extern int previous_net_status;// = -1;//0:green(w network), 1:off(w/o network, w rj45), 2:flash red(w/o network, w/o rj45),

extern char g_curl_output[DEF_CURL_OUTPUT_BUFFER_LEN];

time_t wait_force_arm  = 0;
#define FORCE_ARM_TIME 30

unsigned short maxid;
struct itemdbhdr{
	unsigned int dbversion;
	int count;
};
//newdev
jswdev g_pairDevinfo;
int g_isPairMode=0;
//

double timegettime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	//return tv.tv_usec/ 1000;
	return tv.tv_sec + tv.tv_usec * 1e-6;

}


jswdev* getDevbydid(unsigned int did)
{
	int x;
	for (x=0;x<devcount;x++)
	{
		if ( devlist[x].did == did )
			return &devlist[x];
	}
	return NULL;

}

//char evfilelist[10][32];

int delEvfilelist()
{
	DIR *d;
	char filepath[1024];
	struct dirent *dir;
	char *path;
	int cnt =0;

	if (isMmcblk0Valid())
		path =  "/media/mmcblk0/sn98601/EventList/.";
	else if(isMmcblk0p1Valid())
		path = "/media/mmcblk0p1/sn98601/EventList/.";

	d = opendir(path);
	if (d == NULL)
      return 0;
    while ((dir = readdir(d)) != NULL)
    {
		if (dir->d_type == DT_REG)
		{
			sprintf(filepath, "%sEventList/%s", sdpath, dir->d_name);
		//if ( 0 ==  access(path, 0) )
			remove(filepath);

		//  sprintf(evfilelist[cnt], "%s", dir->d_name);
		 // printf("%s\n", evfilelist[cnt]);
		 /* cnt++;
		  if (cnt >=10 )
			  break;*/
		}
    }
    closedir(d);

	return cnt;


}



time_t prevpushtime= 0;
int prevpushtype = -1;

//get selection1 to judge send or not
int get_selection(int str_id)
{
    int ret = 0;
    switch(str_id)
    {
    case STR_ID_ARM:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_ARM;
        break;
    case STR_ID_PART_ARM:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_PART_ARM;
        break;
    case STR_ID_DISARM:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_DISARM;
        break;
    case STR_ID_PANIC:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_PANIC;
        break;
    case STR_ID_TAMPER:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_TAMPER;
        break;
    case STR_ID_ALARM:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_ALARM;
        break;
    case STR_ID_SENSOR_BATTERY_LOW:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_SENSOR_BATTLOW;
        break;
    case STR_ID_GATEWAY_BATTERY_LOW:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_GATEWAY_BATTLOW;
        break;
    case STR_ID_LOST_SIGNAL:
        ret = g_setting4.extra.push_selection_1 & DEF_PUSH_SELECTION1_LOST_SIGNAL;
        break;
    }

    return ret;
}

extern int g_network_detect;
//Send push notification
extern int gcm_sound_type;
void pushnotification(int id , char *msg, int str_id)
{
	// Jeff define for reject push by internet NOT ready
	if(g_network_detect ==0) return;

	time_t nw;
	double du;

    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("Running: pushnotification(), Test Push Start");
    }

	if( (g_armstate == st_maintain) && (id == NT_TAMPER) )
	{
	    if(g_selftest_flag > 0)
	    {
            SetRunningLog_str("Running: pushnotification(), return by maintain mode and tamper");
	    }
		return;
	}

	if (id == prevpushtype)
	{
		nw = time(0);
		du = difftime(nw, prevpushtime);
		if( (du <= 2) && (du >= 0) )
		{
            SetRunningLog_str_int("Error: pushnotification() reject same push in 2 seconds", id);
			return;
		}
	}

	prevpushtype = id;
	prevpushtime = time(0);


    if(has_philips_hue_config() <= 0)
    {//no philips hue demo
        if(get_selection(str_id))
        {
            if (pushdata.smson ==1)
            {
                if(g_selftest_flag > 0)
                {
                    SetRunningLog_str("Running: pushnotification(), send sms");
                }
                smsEvent(msg, strlen(msg));
            }

            if (pushdata.pushon ==1)
            {
                if(g_selftest_flag > 0)
                {
                    SetRunningLog_str("Running: pushnotification(), send push event");
                }
				gcm_sound_type = add_push_alarm_sound(str_id);
				//printf("PUSH MSG[%s]\n",msg);
                pushEvent(msg, strlen(msg));
            }

            if (pushdata.emailon == 1)
            {
                if(g_selftest_flag > 0)
                {
                    SetRunningLog_str("Running: pushnotification(), send email");
                }
				//char *title ="SHC_pro Message";
				char *title ="Your System has Message.";
                //char *title ="Smartvest Message";
                sendemail(title , msg);
            }
        }
    }else
    {//in philips hue demo mode
        if(g_selftest_flag > 0)
        {
            SetRunningLog_str("Running: pushnotification(), discard due to in philips hue demo mode");
        }
    }

    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("Running: pushnotification(), Test Push End");
    }
}

int add_push_alarm_sound(int str_id)
{
    switch(str_id)
    {
    case STR_ID_ARM:
	case STR_ID_PART_ARM:
	case STR_ID_DISARM:
	case STR_ID_SENSOR_BATTERY_LOW:
	case STR_ID_GATEWAY_BATTERY_LOW:
    case STR_ID_LOST_SIGNAL:
		return 0;
        break;
    case STR_ID_PANIC:
		return 1;
        break;
    case STR_ID_TAMPER:
       return 1;
        break;
    case STR_ID_ALARM:
        return 1;
        break;
	default:
		return 0;
		break;
    }

	return 0;
}
//check philips hue config file is exist or not
//return 1:has config file, 0:no config file
int has_philips_hue_config(void)
{
    struct stat buf1;
    if(stat("./philips_hue.db", &buf1) == 0)
    {//has file
        SetRunningLog_str("Mode: In philips hue mode");
        return 1;
    }else
    {//no file
        SetRunningLog_str("Mode: Non philips hue mode");
        return 0;
    }
    return 0;
}

//control philips hue lamp
void pushnotification2(char *msg)
{
    if(has_philips_hue_config() > 0)
        pushEvent2(msg, strlen(msg));
}

void loadmac()
{
	char buf[64];
	int len;
	int x;

	FILE *fp;
	fp = fopen("/mnt/mac.db", "rb");

	if (fp ==NULL)
	{
		DBG_PRINT("mac not exist\n");
        SetRunningLog_str("Error: no mac.db found");
		return;
	}
	int bread = fread(buf, 1, 64, fp);
	fclose(fp);

	if (bread >0)
	{

		memcpy(macaddr, buf, bread);
		printf("MAC %s\n", macaddr);

		changeMac(macaddr);

	}

}

void savemac()
{
	FILE *fp;
	fp = fopen("/mnt/mac.db", "wb");

	if (fp == NULL)
	{
		DBG_PRINT("mac data fail to open\n");
        SetRunningLog_str("Error: open mac.db failed");
		return;
	}
	fwrite(&macaddr, strlen(macaddr), 1, fp);

	fclose(fp);

}

void delTmpEvdata(int mode)//mode: 2->tmpevent2, 1->tmpevent
{
	char filename[64];
	char path[256], cmd[256];

	memset(filename, 0, sizeof(filename));
	memset(path, 0, sizeof(path));
	memset(cmd, 0, sizeof(cmd));

    if(mode == 2)
        strcpy(filename, TMPEVENT_NAME2);
    else
        strcpy(filename, TMPEVENT_NAME);

	memset(path, 0, sizeof(path));
	sprintf(path, "%sEventList/%s", sdpath, filename);
	sprintf(cmd, "rm %s", path);
	system(cmd);

	sync();
}
#if 1
void saveTmpEvdata(int mode)//mode: 2->tmpevent2, 1->tmpevent
{
	FILE *fp;
	char filename[64];
	char path[256], cmd[256];

	memset(filename, 0, sizeof(filename));
	memset(path, 0, sizeof(path));
	memset(cmd, 0, sizeof(cmd));

    if(mode == 2)
		sprintf(path, "%sEventList/%s", sdpath, TMPEVENT_NAME2);
//        strcpy(filename, TMPEVENT_NAME2);
    else
		sprintf(path, "%sEventList/%s", sdpath, TMPEVENT_NAME);
        //strcpy(filename, TMPEVENT_NAME);

//	memset(path, 0, sizeof(path));
//	sprintf(path, "%sEventList/%s", sdpath, filename);
//	sprintf(cmd, "rm %s", path);
//	system(cmd);

	//printf("\nsave event file %s total %d,  currentpos %d\n", filename, g_setting.ev.count, g_setting.ev.savepos);
	fp = fopen(path, "wb");

	if (fp == NULL)
	{
		DBG_PRINT("save tmp event data fail to open path : %s\n", path);
        SetRunningLog_str("Error: save file tmpevent failed");
		return;
	}

	fwrite(&evtcount, sizeof(int), 1, fp);
	fwrite(&evtlist, sizeof(struct _jsw_event) * MAXEVENT, 1, fp);

	fflush(fp);
	fclose(fp);
//	sync();
}



#else
void saveTmpEvdata(int mode)//mode: 2->tmpevent2, 1->tmpevent
{
	FILE *fp;
	char filename[64];
	char path[256], cmd[256];

	memset(filename, 0, sizeof(filename));
	memset(path, 0, sizeof(path));
	memset(cmd, 0, sizeof(cmd));

    if(mode == 2)
        strcpy(filename, TMPEVENT_NAME2);
    else
        strcpy(filename, TMPEVENT_NAME);

	memset(path, 0, sizeof(path));
	sprintf(path, "%sEventList/%s", sdpath, filename);
	sprintf(cmd, "rm %s", path);
	system(cmd);

	//printf("\nsave event file %s total %d,  currentpos %d\n", filename, g_setting.ev.count, g_setting.ev.savepos);
	fp = fopen(path, "wb");

	if (fp == NULL)
	{
		DBG_PRINT("save tmp event data fail to open path : %s\n", path);
        SetRunningLog_str("Error: save file tmpevent failed");
		return;
	}

	fwrite(&evtcount, sizeof(int), 1, fp);
	fwrite(&evtlist, sizeof(struct _jsw_event) * MAXEVENT, 1, fp);

	fflush(fp);
	fclose(fp);
	sync();
}
#endif
int loadTmpEvdata(int mode)//mode: 2->tmpevent2, 1->tmpevent
{
	FILE *fp;
	char filename[64];
	char path[256];

	memset(filename, 0, sizeof(filename));
	memset(path, 0, sizeof(path));

    if(mode == 2)
        strcpy(filename, TMPEVENT_NAME2);
    else
        strcpy(filename, TMPEVENT_NAME);

	memset(path, 0, sizeof(path));
	sprintf(path, "%sEventList/%s", sdpath, filename);

    evtcount = 0;

	//judge file size
	struct stat buf2;
	if(stat(path, &buf2) == -1)
	{
        printf("CMD stat() failed, path=%s, mode=%d!\n", path, mode);
        return 0;
	}
	if(buf2.st_size != (sizeof(struct _jsw_event)*MAXEVENT+sizeof(int)) )
	{
        printf("File size not equal, mode=%d, size=%d, %d!\n", mode, buf2.st_size, sizeof(struct _jsw_event)*MAXEVENT+sizeof(int));
        SetRunningLog_str_int("Error: loadTmpEvdata() file length not equal", buf2.st_size);
        return 0;//file size error
	}

	//printf("\nsave event file %s total %d,  currentpos %d\n", filename, g_setting.ev.count, g_setting.ev.savepos);
	fp = fopen(path, "rb");

	if (fp == NULL)
	{
		printf("load tmp event data fail to open path : %s\n", path);
        SetRunningLog_str("Error: loadTmpEvdata() open file failed");
		return 0;
	}

	fread(&evtcount, sizeof(int), 1, fp);
	fread(&evtlist, sizeof(struct _jsw_event) * MAXEVENT, 1, fp);

    printf("Load %d events from tmpevent%d!!!\n", evtcount, mode);

	fclose(fp);
	return 1;
}

int select_2(const struct dirent *dir2)
{
    if(strlen(dir2->d_name) == 10)
        return 1;
    else
        return 0;
}

void loadLastEvdata()
{
	FILE *fp;
	char filename[64];
	char path[256];
	struct dirent **namelist;
	int i, total, name, lastname=0;

	memset(filename, 0, sizeof(filename));
	memset(path, 0, sizeof(path));

	sprintf(path, "%sEventList/", sdpath);

    total = scandir(path, &namelist, select_2, 0);
    if(total > 0)
    {//files found
        for(i=0;i<total;i++)
        {
            name = atoi(namelist[i]->d_name);
            if( (name > 1400000000) && (name < 1800000000) )
            {//between 2015 - 2026
                if(name > lastname)
                    lastname = name;
            }
        }
    }

    if(lastname <= 0)
    {
        SetRunningLog_str_int("Error: loadLastEvdata() no last file found", lastname);
        return;
    }

	memset(path, 0, sizeof(path));
	sprintf(path, "%sEventList/%u", sdpath, lastname);

	//printf("\nsave event file %s total %d,  currentpos %d\n", filename, g_setting.ev.count, g_setting.ev.savepos);
	fp = fopen(path, "rb");

	if (fp == NULL)
	{
		DBG_PRINT("load last event data fail to open path : %s\n", path);
        SetRunningLog_str_int("Error: loadLastEvdata() open file failed", lastname);
		return;
	}

	fread(&i, sizeof(int), 1, fp);
	fread(&o_evtlist, sizeof(struct _jsw_event) * MAXEVENT, 1, fp);

	fclose(fp);
	is_oldevtexist = 1;
}

void delAllEvdata()
{
	char path[256], filename[256];
	struct dirent **namelist;
	int i, total;

	memset(path, 0, sizeof(path));

	sprintf(path, "%sEventList/", sdpath);

	total = scandir(path, &namelist, select_2, 0);
	if(total > 0)
	{//files found
		for(i=0;i<total;i++)
		{
			memset(filename, 0, sizeof(filename));
			sprintf(filename, "%sEventList/%s", sdpath, namelist[i]->d_name);
			remove(filename);
		}
	}

	sync();
}



//save evtlist to file
void saveEvdata()
{
	FILE *fp;
	char filename[64];
	char path[1024];
	char *nextfile;

	memset(filename, 0, sizeof(filename));
	memset(path, 0, sizeof(path));

	sprintf(filename, "%u", evtlist[0].time);

	//delete existing fiel
	int nextp = g_setting.ev.savepos;
	nextfile = g_setting5.filename[nextp];//g_setting.ev.filename[nextp];
	if ( strlen(nextfile) )
	{
		sprintf(path, "%sEventList/%s", sdpath, nextfile);
		if ( 0 ==  access(path, 0) )
			remove(path);
	}

	memset(g_setting5.filename[nextp], 0, sizeof(g_setting5.filename[nextp]));//g_setting.ev.filename[nextp]
	strcpy(g_setting5.filename[nextp], filename);//g_setting.ev.filename[nextp]
	g_setting.ev.savepos++;
	 if ( g_setting.ev.savepos >= MAXEVENTFILES)
		  g_setting.ev.savepos = 0;


	memset(path, 0, sizeof(path));
	sprintf(path, "%sEventList/%u", sdpath, evtlist[0].time);

	//printf("\nsave event file %s total %d,  currentpos %d\n", filename, g_setting.ev.count, g_setting.ev.savepos);
	fp = fopen(path, "wb+");

	if (fp == NULL)
	{
		DBG_PRINT("save event data fail to open path : %s\n", path);
        SetRunningLog_str("Error: saveEvdata() open file failed");
		return;
	}

	fwrite(&evtcount, sizeof(int), 1, fp);
	fwrite(&evtlist, sizeof(struct _jsw_event) * evtcount, 1, fp);

	g_setting.ev.count ++;
	if (g_setting.ev.count > MAXEVENTFILES)
		g_setting.ev.count = MAXEVENTFILES;

	evtcount = 0;

	fflush(fp);
	fclose(fp);

	savesetting();
	savesetting5();
}


void savedata()
{
	FILE *fp;
	fp = fopen("/mnt/jswitem2.db", "wb");

	if (fp == NULL)
	{
		DBG_PRINT("save data fail to open\n");
        SetRunningLog_str("Error: savedata() open file failed");
		return;
	}
	unsigned int v = DBVERSION;

	fwrite(&v, sizeof(int), 1, fp);
	fwrite(&devcount, sizeof(int), 1, fp);

	fwrite(&devlist, sizeof(jswdev) * devcount, 1, fp);

	fclose(fp);
	sync();

}

void saveScenario()
{
	FILE *fp;
		//scenario
	fp = fopen("/mnt/scenario.db", "wb+");
	if (fp == NULL)
	{
		//DBG_PRINT("scenario data fail to open\n");
        SetRunningLog_str("Error: saveScenario() open file failed");
		return;
	}
	fwrite(&srcCount, sizeof(int), 1, fp);
	fwrite(&scenariolist,  sizeof(jswscenario) * srcCount, 1, fp);

	fclose(fp);
    sync();
}

//must be call after devcount is known
void loadScenario()
{
	char buf[4096*3]; //sizeof(jswscenario)=140, 140*64=8960
	int len;
	int x;
	memset(&scenariolist, 0 , sizeof(scenariolist) );
	srcCount = 0;

	FILE *fp;
	fp = fopen("/mnt/scenario.db", "rb");

	if (fp ==NULL)
	{
		//DBG_PRINT("scenario fail to open\n");
        SetRunningLog_str("Error: loadScenario() open file failed");
		return;
	}
	int bread = fread(buf, 1, 4, fp);
	memcpy(&srcCount, buf, sizeof(int));

	bread = fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	if(bread > sizeof(buf))
	{
		printf("Error: Scenario buffer overflow, bread(%d) > sizeof(buf)(%d)\n", bread, sizeof(buf));
        SetRunningLog_str_int("Error: loadScenario() buffer overflow", bread);
		return;
	}

	if ( bread != (srcCount * sizeof(jswscenario)) )
	{
		printf("Load scenario size error\n");
        SetRunningLog_str_int("Error: loadScenario() size not equal", bread);
		return;
	}
	if (srcCount >0)
		memcpy(&scenariolist, buf,  sizeof(jswscenario) * srcCount);

}


void parsenetworkfile()
{



}

void loadsetting()
{
	char buf[2046];
	int len;
	int x;


	memset(&o_evtlist, 0 , sizeof(o_evtlist) );
	memset(&g_setting, 0 , sizeof(struct generalinfo) );
	//default item
	strcpy(g_setting.gwprop.name , "SHC-PRO");

	g_setting.gwprop.alarmOn = 1;
//	g_setting.gwprop.ledon =2; //0= no light, 1=low light, 2= high light.
	g_setting.gwprop.ledon =0; //SHC_PRO set to 0

	g_setting.gwprop.entrydelay = 0;
	g_setting.gwprop.exitdelay = 30;

	g_setting.gwprop.duration = 60;
	g_setting.gwprop.timezonechanged = 0;

	//network info.
	g_setting.network.dhcp = 1;

	strcpy( g_setting.acct.adminpwd, "123456");
	strcpy( g_setting.acct.loginpwd, gPassword);
	g_setting.gwprop.version = GATEWAY_VERSION;

	g_setting.sirencount = 0;

	FILE *fp;
	fp = fopen("/mnt/setting3.db", "rb");
	if (fp ==NULL)
	{
        strcpy(g_setting.gwprop.name , DEF_SYSTEM_NAME);
		DBG_PRINT("setting data fail to open\n");
        SetRunningLog_str("Error: loadsetting() open file failed");
		return;
	}
	int bread = fread(buf, 1, 2046, fp);
	fclose(fp);

	struct itemdbhdr *hdr;
	hdr = (struct itemdbhdr*)buf;
	if (hdr->dbversion != DBVERSION)
	{
		DBG_PRINT("setting.db file version not match\n");
        SetRunningLog_str("Error: loadsetting() DBVERSION failed");
		return;
	}
	char *p = buf + sizeof(struct itemdbhdr);
	if (bread > sizeof(struct generalinfo)+sizeof(struct itemdbhdr ) )
	{
		printf("setting db read error, read byte=%d, sizeof struct =%d\n", bread ,sizeof(struct generalinfo) +sizeof(struct itemdbhdr ));
        SetRunningLog_str_int("Error: loadsetting() buffer overflow", bread);
		return;
	}
	//sprintf("acct %s\n",g_setting.acct.loginpwd );
	memcpy(&g_setting, p,  sizeof(g_setting));//bread); //sizeof(struct generalinfo) );
//	strcpy(g_setting.gwprop.name , DEF_SYSTEM_NAME);
	g_setting.gwprop.version = GATEWAY_VERSION;
}

void load_Nest_parm(void)
{
	//printf("load_Nest_parm !!!!!!!!!\n");
	char buf[2046];
	int len;
	int x;


	memset(&g_stNEST_parm, 0 , sizeof(stNEST_parm) );

	FILE *fp;
	fp = fopen("/mnt/NEST_device.db", "rb");
	if (fp ==NULL)
	{
		DBG_PRINT("Load NEST_device failed\n");
		save_Nest_parm();
		return;
	}
	else
	{
		int bread = fread(buf, 1, 2046, fp);
		fclose(fp);

		if(bread != sizeof(stNEST_parm))
		{
			DBG_PRINT("Load NEST_device struct failed\n");
			// TODO create NEW NEST_DB
			 save_Nest_parm();
			return ;
		}

		memcpy(&g_stNEST_parm,buf,  sizeof(stNEST_parm));
		DBG_PRINT("Load NEST_device struct done\n");
	}

}

void save_Nest_parm()
{
	//printf("Save_Nest_parm !!!!!!!!!\n");


	FILE *fp;
	int n = 1;
	fp = fopen("/mnt/NEST_device.db", "wb");

	if (fp == NULL)
	{
		DBG_PRINT("save Nest_parm failed\n");
		return;
	}
// JEFF TEST ONLY
#if 0
	g_stNEST_parm.nest_bind = 1;
	strcpy(g_stNEST_parm.access_token,"c.wMildCe1nDhQrXhfTHfaJ5mwNQ3G0fYpwmdXbqIWI5uoC4MPzZ8MRHnW5h5uXZqgJwWewSDlaWDQTMliGDcoUZD29ECZS9LoPx2Xy5BDUCyn9B5Eo7S5yqSl7ENfr6cYleiP20y2rNLpR6MR");
	g_stNEST_parm.stNEST_device_list[0].joined =1;
	g_stNEST_parm.stNEST_device_list[0].device_type= TYPE_NEST_SMOKE;
	strcpy(g_stNEST_parm.stNEST_device_list[0].device_name,"TY Smoke#1");
	strcpy(g_stNEST_parm.stNEST_device_list[0].device_ID,"uNI9Pt4XDzPy5nCa9PnAo7n_6kUwZs0M");

	g_stNEST_parm.stNEST_device_list[1].joined =1;
	g_stNEST_parm.stNEST_device_list[1].device_type= TYPE_NEST_SMOKE;
	strcpy(g_stNEST_parm.stNEST_device_list[1].device_name,"TY Smoke#2");
	strcpy(g_stNEST_parm.stNEST_device_list[1].device_ID,"uNI9Pt4XDzMmGDM4Yfqn47n_6kUwZs0M");

	g_stNEST_parm.stNEST_device_list[2].joined =1;
	g_stNEST_parm.stNEST_device_list[2].device_type= TYPE_NEST_THERMO;
	strcpy(g_stNEST_parm.stNEST_device_list[2].device_name,"TY Thermo#1");
	strcpy(g_stNEST_parm.stNEST_device_list[2].device_ID,"acdbTaCI3GiG0Z0BjYf2FLn_6kUwZs0M");

	g_stNEST_parm.stNEST_device_list[3].joined =1;
	g_stNEST_parm.stNEST_device_list[3].device_type= TYPE_NEST_THERMO;
	strcpy(g_stNEST_parm.stNEST_device_list[3].device_name,"TY Thermo#2");
	strcpy(g_stNEST_parm.stNEST_device_list[3].device_ID,"acdbTaCI3GhLkl-bdUg3KLn_6kUwZs0M");

#endif
	fwrite(&g_stNEST_parm, sizeof(stNEST_parm) , 1, fp);
	fclose(fp);
    sync();
}

//apple
void load_cloud_user_parm(void)
{
	printf("load_user_parm !!!!!!!!!\n");
	char buf[2046];
	int len;
	int x;

	memset(&g_stUserParam, 0 , sizeof(stUSER_parm) );

	FILE *fp;
	fp = fopen("/mnt/user_info.db", "rb");
	if (fp ==NULL)
	{
		DBG_PRINT("Load user_device failed\n");
		return;
	}
	else
	{
		int bread = fread(buf, 1, 2046, fp);
		fclose(fp);

		if(bread != sizeof(stUSER_parm))
		{
			DBG_PRINT("Load user_device struct failed\n");
			return ;
		}
		
		memcpy(&g_stUserParam, buf, sizeof(stUSER_parm));
		DBG_PRINT("Load user_device struct done\n");

		printf("### load_cloud_user_parm appid[%s], Apptoken[%s], username[%s], entryid[%s] \n",g_stUserParam.szAPPId, g_stUserParam.szAPPToken
			,g_stUserParam.szUsername, g_stUserParam.szEntryId);
	}

	

}

void save_cloud_user_parm()
{
	printf("save_user_parm !!!!!!!!!\n");

	
	if(strlen(g_stUserParam.szEntryId) ==0 || strlen(g_stUserParam.szUsername) ==0)
	{
	
		printf("### save_cloud_user_parm username = null and entryid = null \n");
		return;
	}
	
	printf("### save_cloud_user_parm appid[%s], Apptoken[%s], username[%s], entryid[%s] \n",g_stUserParam.szAPPId, g_stUserParam.szAPPToken
		,g_stUserParam.szUsername, g_stUserParam.szEntryId);

	FILE *fp;
	int n = 1;
	fp = fopen("/mnt/user_info.db", "wb");

	if (fp == NULL)
	{
		DBG_PRINT("save user_device failed\n");
		return;
	}

	fwrite(&g_stUserParam, sizeof(stUSER_parm) , 1, fp);
	fclose(fp);
    sync();
}


//load DA setting
void load_cloud_goolg_DA(void)
{
	printf("load_cloud_goolg_DA !!!!!!!!!\n");
	char buf[2046];
	int len;
	int x;

	memset(&g_stDAParam, 0 , sizeof(struct googleSetting) );

	FILE *fp;
	fp = fopen("/mnt/DA_info.db", "rb");
	if (fp ==NULL)
	{
		DBG_PRINT("Load user_device failed\n");
		return;
	}
	else
	{
		int bread = fread(buf, 1, 2046, fp);
		fclose(fp);

		if(bread != sizeof(struct googleSetting))
		{
			DBG_PRINT("Load user_device struct failed\n");
			return ;
		}
		
		memcpy(&g_stDAParam, buf, sizeof(struct googleSetting));
		DBG_PRINT("Load user_device struct done\n");

		printf("### load_cloud_user_parm appid[%s], Apptoken[%s], username[%s], entryid[%s] \n",g_stUserParam.szAPPId, g_stUserParam.szAPPToken
			,g_stUserParam.szUsername, g_stUserParam.szEntryId);
	}

}

void save_cloud_goolg_DA()
{
	printf("save_cloud_goolg_DA !!!!!!!!!\n");
	

	printf("### save_cloud_user_parm enbale = [%d]\n", g_stDAParam.bEnableDA);

	FILE *fp;
	int n = 1;
	fp = fopen("/mnt/DA_info.db", "wb");

	if (fp == NULL)
	{
		DBG_PRINT("save DA_info failed\n");
		return;
	}

	fwrite(&g_stDAParam, sizeof(struct googleSetting) , 1, fp);
	fclose(fp);
    sync();
}




void savesetting()
{

	FILE *fp;
	int n = 1;
	fp = fopen("/mnt/setting3.db", "wb");

	if (fp == NULL)
	{
		//DBG_PRINT("save data fail to open\n");
        SetRunningLog_str("Error: savesetting() open file failed");
		return;
	}
	unsigned int v = DBVERSION;
	g_setting.gwprop.version = GATEWAY_VERSION;

	fwrite(&v, sizeof(int), 1, fp);
	fwrite(&n, sizeof(int), 1, fp);

	fwrite(&g_setting, sizeof(struct generalinfo) , 1, fp);

	fclose(fp);
    sync();
}

void save_alarm_info()
{

	FILE *fp;
	fp = fopen("/mnt/alarm_info.db", "wb");

	if (fp == NULL)
	{
		//DBG_PRINT("save data fail to open\n");
        SetRunningLog_str("Error: save_alarm_info() open file failed");
		return;
	}

	fwrite(&last_trigger, sizeof(jsw_alarm_event), 1, fp);
	fclose(fp);
    sync();
}


void load_alarm_info()
{

	FILE *fp;
	char buf[512];
	memset(&last_trigger, 0 , sizeof(jsw_alarm_event) );

	fp = fopen("/mnt/alarm_info.db", "rb");

	if (fp == NULL)
	{
        SetRunningLog_str("Error: load_alarm_info() open file failed");
		return;
	}
	int file_size = fread(&buf, 1,512, fp);
	fclose(fp);
	if(file_size != sizeof(jsw_alarm_event))
		{
		return ;
		}
	memcpy(&last_trigger,buf,	sizeof(jsw_alarm_event));
	DBG_PRINT("Load last_trigger struct done\n");

}



void loadsetting4()
{
	memset(&g_setting4, 0 , sizeof(struct setting4info) );
	//default item
	g_setting4.extra.armstate = st_disarm;
    g_setting4.extra.push_string_language_id = DEF_PUSH_LANGUAGE;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_ARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_PART_ARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_DISARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_PANIC;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_TAMPER;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_ALARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_SENSOR_BATTLOW;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_GATEWAY_BATTLOW;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_LOST_SIGNAL;

	FILE *fp;
	fp = fopen("/mnt/setting4.db", "rb");
	if (fp ==NULL)
	{
		DBG_PRINT("setting4 data fail to open\n");
        SetRunningLog_str("Error: loadsetting4() open file failed");
		return;
	}
	int bread = fread(&g_setting4, 1, sizeof(struct setting4info), fp);
	fclose(fp);

	if( (g_setting4.extra.armstate != st_disarm) && (g_setting4.extra.armstate != st_arm) &&
        (g_setting4.extra.armstate != st_partarm) )
	{
		g_setting4.extra.armstate = st_disarm;
	}
	g_armstate = g_setting4.extra.armstate;
	g_ShowAlarmView = g_setting4.extra.ShowAlarmView;
	g_AlarmState = g_setting4.extra.AlarmState;

    if( (g_setting4.extra.push_string_language_id < LANG_ID_GERMAN) || (g_setting4.extra.push_string_language_id >= LANG_ID_LAST) )
        g_setting4.extra.push_string_language_id = DEF_PUSH_LANGUAGE;
}

void savesetting4()
{
	FILE *fp;
	int n = 1;
	fp = fopen("/mnt/setting4.db", "wb");

	if (fp == NULL)
	{
		//DBG_PRINT("save data fail to open\n");
        SetRunningLog_str("Error: savesetting4() open file failed");
		return;
	}

    g_setting4.extra.armstate = g_armstate;
    g_setting4.extra.ShowAlarmView = g_ShowAlarmView;
	g_setting4.extra.AlarmState = g_AlarmState;
	fwrite(&g_setting4, sizeof(struct setting4info) , 1, fp);

	fclose(fp);
	sync();
}

void loadsetting5()
{
	memset(&g_setting5, 0 , sizeof(struct setting5info) );
	//default item

	FILE *fp;
	fp = fopen("/mnt/setting5.db", "rb");
	if (fp == NULL)
	{
		DBG_PRINT("setting5 data fail to open\n");
        SetRunningLog_str("Error: loadsetting5() open file failed");
		return;
	}
	int bread = fread(&g_setting5, 1, sizeof(struct setting5info), fp);
	fclose(fp);
	sync();
}

void savesetting5()
{
	FILE *fp;
	//int n = 1;
	fp = fopen("/mnt/setting5.db", "wb");

	if (fp == NULL)
	{
		//DBG_PRINT("save data fail to open\n");
        SetRunningLog_str("Error: savesetting5() open file failed");
		return;
	}

	fwrite(&g_setting5, sizeof(struct setting5info) , 1, fp);

	fclose(fp);
	sync();
}


//successful retuen 1, else return 0
int loaddata()
{//load new devlist
	char buf[4096*2]; //sizeof(jswdev)=124, 124*48=5952
	int len;
	int x;
	memset(&devlist, 0 , sizeof(devlist) );
	devcount = 0;

	//keepalive
	memset(&keepalivelist, 0, sizeof(keepalivelist));

	FILE *fp;
	fp = fopen("/mnt/jswitem2.db", "rb");

	if (fp ==NULL)
	{
		DBG_PRINT("loaddata fail to open\n");
        SetRunningLog_str("Error: loaddata() open file failed");
		return 0;
	}
	int bread = fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	struct itemdbhdr *hdr;
	hdr = (struct itemdbhdr*)buf;

	if (hdr->dbversion != DBVERSION)
	{
		DBG_PRINT("db file version not match\n");
        SetRunningLog_str("Error: loaddata() DBVERSION failed");
		return 0;
	}

	devcount = hdr->count;
	char *p = buf + sizeof(struct itemdbhdr);

	if (devcount >0)
	{
		//jswdev (*ptr)[];
		jswdev *ptr;
		ptr = (jswdev*)p;
		memcpy(&devlist, p,  sizeof(jswdev) * devcount);
		for(x=0;x<devcount;x++)
		{
            if( (devlist[x].model == TYPE_MAGNETIC) && (devlist[x].status == 0) )
                devlist[x].status = RE_MAG_ISOFF;
            if( (devlist[x].model == TYPE_POWER_ADAPTER) && (devlist[x].status == 0) )
                devlist[x].status = RE_ADAPTER_ISOFF;
            if( (devlist[x].model == TYPE_SIREN_OUTDOOR) && (devlist[x].status == 0) )
                devlist[x].status = RE_SIREN_ISOFF;
		}
	}
    return 1;
}

void loaddata2()
{//load old devlist, and copy to new devlist
	char buf[4096*2]; //sizeof(jswdev)=108, 108*48=5184
	int len;
	int x;
    jswdev2 devlist2[MAXDEVICENUM];
	memset(&devlist2, 0 , sizeof(devlist2) );
	int devcount2 = 0;

	//keepalive
	memset(&keepalivelist, 0, sizeof(keepalivelist));

	FILE *fp;
	fp = fopen("/mnt/jswitem.db", "rb");

	if (fp ==NULL)
	{
		DBG_PRINT("loaddata2 fail to open\n");
        SetRunningLog_str("Error: loaddata2() open file failed");
		return;
	}
	int bread = fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	struct itemdbhdr *hdr;
	hdr = (struct itemdbhdr*)buf;

	if (hdr->dbversion != DBVERSION)
	{
		DBG_PRINT("db file version not match\n");
        SetRunningLog_str("Error: loaddata2() DBVERSION failed");
		return;
	}

	devcount2 = hdr->count;
	char *p = buf + sizeof(struct itemdbhdr);

	if (devcount2 >0)
	{
		jswdev2 *ptr;
		ptr = (jswdev2*)p;
		memcpy(&devlist2, p,  sizeof(jswdev2) * devcount2);
		memset(&devlist, 0,  sizeof(devlist));
		for(x=0;x<devcount2;x++)
		{
		    devlist[x].did = devlist2[x].did;
		    strcpy(devlist[x].name, devlist2[x].name);
		    strcpy(devlist[x].location, devlist2[x].location);
		    strcpy(devlist[x].ipcdid, devlist2[x].ipcdid);
		    strcpy(devlist[x].ipcpwd, devlist2[x].ipcpwd);
		    devlist[x].model = devlist2[x].model;
		    devlist[x].status = devlist2[x].status;
		    devlist[x].flag = devlist2[x].flag;
		    devlist[x].ext1 = devlist2[x].ext1;
		}
		devcount = devcount2;

		savedata();
	}
}



void savepush()
{
	FILE *fp;
	fp = fopen("/mnt/push2.db", "wb+");
	if (fp == NULL)
	{
		DBG_PRINT("push data fail to open\n");
        SetRunningLog_str("Error: savepush() open file failed");
		return;
	}
	fwrite(&pushdata, sizeof(jswpush), 1, fp);
	fwrite(&emaillist,  sizeof(struct em) * 5, 1, fp);

	fclose(fp);

}



void loadpush()
{
	char buf[4096];
	int len;
	int x;
	memset(&pushdata, 0, sizeof(jswpush));

	pushdata.pushon =1;
	pushdata.route == 1; //gold


	FILE *fp;
	fp = fopen("/mnt/push2.db", "rb");

	if (fp ==NULL)
	{
		//DBG_PRINT("pushdata fail to open\n");
        SetRunningLog_str("Warning: loadpush() open file failed");
		return;
	}
	int bread = fread(buf, 1, sizeof(jswpush), fp);
	memcpy(&pushdata, buf, sizeof(jswpush));
	fread(buf, 1, sizeof(struct em) * 5, fp);
	memcpy(&emaillist, buf, sizeof(struct em) * 5);

}

void saveOthers()
{
	FILE *fp;
	fp = fopen("/mnt/others.db", "wb+");
	if (fp == NULL)
	{
		DBG_PRINT("push data fail to open\n");
        SetRunningLog_str("Error: saveOthers() open file failed");
		return;
	}
	fwrite(&pushdata, sizeof(jswOther), 1, fp);

	fclose(fp);

}



void loadOthers()
{
	char buf[4096];
	int len;
	int x;
	memset(&m_othersData, 0, sizeof(jswOther));

	m_othersData.bBeepInMagOn = 0;


	FILE *fp;
	fp = fopen("/mnt/others.db", "rb");

	if (fp ==NULL)
	{
        SetRunningLog_str("Warning: loadOthers() open file failed");
		return;
	}
	int bread = fread(buf, 1, sizeof(jswOther), fp);
	memcpy(&m_othersData, buf, sizeof(jswOther));

}


void savePAprop()
{
	FILE *fp;
	fp = fopen("/mnt/pa.db", "wb+");
	if (fp == NULL)
	{
		DBG_PRINT("pa data fail to open\n");
        SetRunningLog_str("Error: savePAprop() open file failed");
		return;
	}
	fwrite(&paCount, sizeof(int), 1, fp);
	fwrite(&pa_proplist,  sizeof(struct pa_prop) * paCount, 1, fp);

	fclose(fp);
}

void loadPAprop()
{
	char buf[4096*4]; //sizeof( struct pa_prop)=272, 272*48=13056
	int len;
	int x;
	memset(&pa_proplist, 0, sizeof(pa_proplist));
	paCount = 0;

	FILE *fp;
	fp = fopen("/mnt/pa.db", "rb");

	if (fp ==NULL)
	{
		//DBG_PRINT("pa fail to open\n");
        SetRunningLog_str("Error: loadPAprop() open file failed");
		return;
	}
	int bread = fread(buf, 1, 4, fp);
	memcpy(&paCount, buf, sizeof(int));

	bread = fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	if(bread > sizeof(buf))
	{
		printf("Error: Schedule buffer overflow, bread(%d) > sizeof(buf)(%d)\n", bread, sizeof(buf));
        SetRunningLog_str_int("Error: loadPAprop() buffer overflow", bread);
		return;
	}

	if ( bread != (paCount * sizeof( struct pa_prop)) )
	{
		printf("load scenario size error\n");
        SetRunningLog_str_int("Error: loadPAprop() size not equal", bread);
		return;
	}
	if (paCount >0)
		memcpy(&pa_proplist, buf,  sizeof( struct pa_prop) * paCount);

}

//////////

void savezone()
{
	FILE *fp;
	fp = fopen("/mnt/zone.db", "wb+");
	if (fp == NULL)
	{
		DBG_PRINT("zone data fail to open\n");
        SetRunningLog_str("Error: savezone() buffer overflow");
		return;
	}
	fwrite(&zoneCount, sizeof(int), 1, fp);
	fwrite(&zonelist,  sizeof( struct _zonelist) * zoneCount, 1, fp);

	fclose(fp);
}

void loadzone()
{
	char buf[4096*2];//sizeof( struct _zonelist)=164, 164*32=5248
	int len;
	int x;
	memset(&zonelist, 0, sizeof(struct _zonelist));
	zoneCount = 0;

	FILE *fp;
	fp = fopen("/mnt/zone.db", "rb");

	if (fp ==NULL)
	{
		DBG_PRINT("zone fail to open\n");
        SetRunningLog_str("Error: loadzone() buffer overflow");
		return;
	}
	int bread = fread(buf, 1, 4, fp);
	memcpy(&zoneCount, buf, sizeof(int));

	bread = fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	if ( bread != (zoneCount * sizeof( struct _zonelist)) )
	{
		printf("Error: load zone size error\n");
        SetRunningLog_str_int("Error: loadzone() size not equal", bread);
		return;
	}
	if (zoneCount >0)
	{
		memcpy(&zonelist, buf,  sizeof( struct _zonelist) * zoneCount);

        //delete hotkey with empty name
		int tmp = zoneCount;
		int y;
		for(x=0;x<tmp;x++)
		{
            if(strlen(zonelist[x].zone.name) <= 0)
            {//delete item with empty hotkey name
                printf("Delete hotkey with empty name, x=%d, len=%d, num=%d, count=%d\n", x, strlen(zonelist[x].zone.name),
                      zonelist[x].zone.num, zonelist[x].zone.count);
                for(y=x+1;y<zoneCount;y++)
                {
                    memcpy(&zonelist[y-1].zone, &zonelist[y].zone, sizeof(zonelist[y]));
                }
                tmp--;
                x--;
            }
		}
		if(zoneCount != tmp)
		{
            zoneCount = tmp;
            if( (zoneCount >= 0) && (zoneCount <= 32) )
            {
                savezone();
            }else
            {
                zoneCount = 0;
            }
		}
	}

}

int isDevRegister(unsigned int id)
{
	int x;

	for ( x=0;x<devcount;x++)
	{
		if ( devlist[x].did == id )
			return 1;
	}
	return 0; //not found
}

void deleteDev(unsigned int id )
{
	int x;
	int y;
	int k;
	int found = -1;
	//printf("@@client Delete device %d\n", id);

	//keep alive
	for (k=0;k<devcount;k++)
	{
		if ( keepalivelist[k].did == id)
		{
			if (devcount-1 != k) //not lastitem
			{//move the last item here . to keep array continuous.
				memcpy(&keepalivelist[k], &keepalivelist[devcount-1], sizeof(struct keepalive));
				keepalivelist[devcount-1].misss_supervision = 0;
				keepalivelist[devcount-1].sensor_version = 0;
				keepalivelist[devcount-1].sent_push_event = 0;
			}
			break;
		}
	}

	//device list
	for (x =0;x<devcount;x++)
	{
		if (devlist[x].did != id )
			 continue;
		found = x;
		if (devcount-1 != x) //not lastitem
		{//move the last item here . to keep array continuous.
			memcpy(&devlist[x], &devlist[devcount-1], sizeof(jswdev));
		}
		devcount --;
		if (devcount <0)
			devcount = 0;
		savedata();

		break;
	}

	//scenario
	for (  x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did != id)
			continue;
		if (srcCount-1 != x) //not lastitem
		{//move the last item here . to keep array continuous.
			memcpy(&scenariolist[x], &scenariolist[srcCount-1], sizeof(jswscenario));
		}
		srcCount --;
        x=0-1;
		if (srcCount<0)
			srcCount =0;
		saveScenario();

	}
	//scenario target
	int changed = 0;
	for (x=0;x<srcCount;x++)
	{
        for (y=0;y<scenariolist[x].src.target_num;y++)
        {
            if(scenariolist[x].target[y].did == id)
            {//found target
                //printf("REMOVE sensor [%d] scenario target [%d][%d]\n", scenariolist[x].target[y].did, x, y);
                if (scenariolist[x].src.target_num-1 != y) //not lastitem
                {
                    memcpy(&scenariolist[x].target[y], &scenariolist[x].target[scenariolist[x].src.target_num-1], sizeof(scenariolist[x].target[y]));
                }
                scenariolist[x].src.target_num--;
                if(scenariolist[x].src.target_num < 0)
                    scenariolist[x].src.target_num = 0;
                y=0-1;
                changed = 1;
            }
        }
	}
	if(changed > 0)
		saveScenario();

	//siren
	for (  x=0;x< g_setting.sirencount;x++)
	{
		if ( g_setting.sirenlist[x].did != id)
			continue;
		if (g_setting.sirencount-1 != x) //not lastitem
		{//move the last item here . to keep array continuous.
			memcpy(&g_setting.sirenlist[x], &g_setting.sirenlist[g_setting.sirencount-1], sizeof(struct sirenprop));
		}
		g_setting.sirencount --;
		if (g_setting.sirencount <0)
			g_setting.sirencount = 0;
		savesetting();

	}


	//PA (Schedule)
	for (  x=0;x<paCount;x++)
	{
		if (pa_proplist[x].did != id)
			continue;
		if (paCount-1 != x) //not lastitem
		{//move the last item here . to keep array continuous.
			memcpy(&pa_proplist[x], &pa_proplist[paCount-1], sizeof(struct pa_prop));
		}
		paCount --;
		if (paCount<0)
			paCount = 0;
		savePAprop();

	}

	//camera list
	deleteCamlist(id);

    //delete battery low list
    if(g_battlowcount > 0)
    {
        check_battlow();
        if(g_battlowcount == 0)
        {
            ledoff();
            set_alarm_LED();
        }
    }

	//hotkey target
	changed = 0;
	for (x=0;x<zoneCount;x++)
	{
        for (y=0;y<zonelist[x].zone.count;y++)
        {
            if(zonelist[x].didlist[y] == id)
            {//found target
                //printf("REMOVE sensor [%d] hotkey target [%d][%d]\n", zonelist[x].didlist[y], x, y);
                if (zonelist[x].zone.count-1 != y) //not lastitem
                {
                    zonelist[x].didlist[y] = zonelist[x].didlist[zonelist[x].zone.count-1];
                }
                zonelist[x].zone.count--;
                if(zonelist[x].zone.count < 0)
                    zonelist[x].zone.count = 0;
                y=0-1;
                changed = 1;
            }
        }
	}
	if(changed > 0)
		savezone();


	//do other things here
	if (found != -1 )
	{
		int payload_len =  sizeof(int);
		sendCmdtoClient(CM_DELETEDEV, 0,  1,  payload_len,(char*) &id );
	}else
		printf("@@ device delete not found \n");
}


void editDev(char *buf )
{
	int x;
	int found=-1;
	jswdev * dev = (jswdev*)buf;

	printf("@@client Edit device id %d, %s, %d \n", dev->did, dev->name, dev->model);


	for (x =0;x<devcount;x++)
	{
		if (devlist[x].did != dev->did )
		{
			continue;
		}
		//found

		found = x;
		if(dev->model == TYPE_CAMERA)
		{//Camera
            if(strcmp(dev->ipcpwd, devlist[x].ipcpwd) != 0)
            {//changed password
                updateCamListPWD(dev);
                exitCamThread(dev->did);
            }
		}
		if(dev->model == TYPE_VIBRATION)
		{//Vibration
			switch(dev->ext1)
            {
                case RE_VIBRATION_SET_HIGH:
                case RE_VIBRATION_SET_MIDDLE:
                case RE_VIBRATION_SET_LOW:
                case RE_VIBRATION_SET_H_HIGH:
                case RE_VIBRATION_SET_H_MIDDLE:
                case RE_VIBRATION_SET_H_LOW:
                    break;
                default:
                    dev->ext1 = RE_VIBRATION_SET_MIDDLE;
            }
			switch(dev->ext2)
            {
                case RE_VIBRATION_SET_TRIGGER_MODE:
                case RE_VIBRATION_SET_TAMPER_MODE:
                    break;
                default:
                    dev->ext2 = RE_VIBRATION_SET_TRIGGER_MODE;
            }
		}
		memcpy(&devlist[x], dev, sizeof(jswdev));

		savedata();
		break;
	}

	if (found != -1 )
	{
		int payload_len =  sizeof(jswdev);
		sendCmdtoClient(CM_EDITDEV, 0,  1,  payload_len, (char*) &devlist[found]);
	}else
		printf("@@ device edit not found \n");
}


//apple
void newDevFromMcu(unsigned int id, char type)
{
	int payload_len = sizeof(jswdev);

	if (g_isPairMode ==0)
	{
		printf("@@Pair info from MCU but gw Not in pairing mode\n");
		return;
	}
	if ( type != g_pairDevinfo.model )
	{
	    unsigned char type_equal = 0;
	    if( (g_pairDevinfo.model == TYPE_REMOTE_CONTROL) && (type == TYPE_REMOTE_CONTROL_NEW) )
            type_equal = 1;
        else if( (g_pairDevinfo.model == TYPE_KEYPAD_JSW) && (type == TYPE_KEYPAD_JSW_NEW) )
            type_equal = 1;
#ifdef JEFF_DEBUG
        else if( (g_pairDevinfo.model == TYPE_PIR) && (type == TYPE_WATERLEVEL) )
        	{
        	g_pairDevinfo.model == TYPE_PIR;
			type_equal = 1;
        	}
#endif
        if(type_equal == 0)
        {//device type is not equal
		printf("@@Not the same type type mcu=%02X  app =%02X \n", type, g_pairDevinfo.model);
		return;
	}
	}

	//check if registered  ;
	if (isDevRegister(id) ==1)
	{
		printf("@@ device already exist \n");
		g_isPairMode = 0;
		if ( pairing_t != 0)
			{
			killTimer(pairing_current_t);
			makeTimer( &pairing_t, 0, 0);
			pairing_t = pairing_current_t = 0;
			}


		sendCmdtoClient(CM_PAIRINGEXIST, 4,  1,  payload_len,(char*) &g_pairDevinfo) ;
		memset(&g_pairDevinfo, 0, sizeof(jswdev));
		return;
	}
	//kill 30 seconds timer.
	if ( pairing_t != 0)
	{
		killTimer(pairing_current_t);
		makeTimer( &pairing_t, 0, 0);
		pairing_t = pairing_current_t = 0;
	}

	//add to db
	if(devcount >= MAXDEVICENUM)
	{
	    printf("Device list is full: sort out it!\n");
	    freeoneDevice();//free one device list
	    if(devcount >= MAXDEVICENUM)
	    {
	        devcount = MAXDEVICENUM-1;
	    }
	}
	memset(&devlist[devcount], 0, sizeof(devlist[devcount]));
	devlist[devcount].did = id;
	devlist[devcount].model = type;
	strcpy(devlist[devcount].name, g_pairDevinfo.name);
	strcpy(devlist[devcount].location, g_pairDevinfo.location);
	// Jeff define for set default value to 0
	if(devlist[devcount].model == TYPE_SMOKE)
        devlist[devcount].status = 0;

	//need to add for both source and target
	if( (type != TYPE_DOORCHIME) && (type != TYPE_BUTTON) ) //Doorchime and Button Default are not including in arm event.
        devlist[devcount].flag = devlist[devcount].flag | 0x01;
	if( (type != TYPE_PIR) && (type != TYPE_DOORCHIME) && (type != TYPE_BUTTON) ) //PIR, Doorchime and Button Default are not including in part arm event.
		devlist[devcount].flag = devlist[devcount].flag | 0x02;
	if( (type != TYPE_DOORCHIME) && (type != TYPE_BUTTON) ) //Doorchime and Button Default are not including in panic event.
        devlist[devcount].flag = devlist[devcount].flag | 0x04;

	// Jeff define for set default vibration to middle=4
	if(devlist[devcount].model == TYPE_VIBRATION)
    {
        devlist[devcount].ext1 = RE_VIBRATION_SET_MIDDLE;
		devlist[devcount].ext2 = RE_VIBRATION_SET_TRIGGER_MODE;
    }

	if ( isTarget(type) )
	{
		if (type ==TYPE_POWER_ADAPTER )
			addPAproperties(id);

		if (type == TYPE_SIREN_INDOOR || type == TYPE_SIREN_OUTDOOR )
		{

			int cnt = g_setting.sirencount;
			//find existing
			int k=0;
			int exist = 0;
			for (k=0;k<cnt;k++)
			{
				if ( g_setting.sirenlist[k].did == id)
				{
					exist = 1;
					break;
				}
			}

			if ( (cnt < 10 ) && exist ==0)
			{
				  g_setting.sirenlist[cnt].did = id;
				  g_setting.sirenlist[cnt].ledon = 2;
				  g_setting.sirenlist[cnt].soundon = 1;
				  g_setting.sirenlist[cnt].duration = 180;
				  g_setting.sirencount++;
				  savesetting();
			}
		}//end siren
	}

	keepalivelist[devcount].did = id;
	keepalivelist[devcount].lastcheckin = time(0);
	keepalivelist[devcount].misss_supervision = 0;
	keepalivelist[devcount].sensor_version = 0;
	keepalivelist[devcount].sent_push_event = 0;

	devcount++;
	savedata();

	//add to source
	if ( isSource(type) ==1)
	{
		addsource(id);
	}

	payload_len = sizeof(jswdev);
	//DBG_PRINT("#### send to client CM_NEWDEVICE"); // %s,  %s, id %d\n", g_pairDevinfo.name, g_pairDevinfo.location, id);
	sendCmdtoClient(CM_NEWDEVICE, 0,  1,  payload_len,(char*) &devlist[devcount-1]) ;

	g_isPairMode = 0;
	memset(&g_pairDevinfo, 0, sizeof(jswdev));

}

void startRec()
{
	int x =0;
	for (x=0;x<devcount;x++)
	{
		if (devlist[x].model == TYPE_CAMERA )
		{
			if ( ( devlist[x].flag & 0x08) == 0x08 )
				doRec( devlist[x].did );
		}
	}
}


void addCamera(jswdev *dev)
{
	int max=1;
	int x;

	for (  x=0;x<devcount;x++)
	{
		if (devlist[x].model != TYPE_CAMERA )
			continue;
		if ( devlist[x].did >= max)
			max = devlist[x].did +1;

		if ( strcmp(devlist[x].ipcdid, dev->ipcdid) == 0)
		{
			printf("@@ camera already exist \n");
			sendCmdtoClient(CM_PAIRINGEXIST, 4,  0,  0, NULL);
			return;
		}
	}

	devlist[devcount].did = max;
	devlist[devcount].model = TYPE_CAMERA;
	strcpy(devlist[devcount].name, dev->name);
	strcpy(devlist[devcount].location,dev->location);
	strcpy(devlist[devcount].ipcdid, dev->ipcdid);
	strcpy(devlist[devcount].ipcpwd, dev->ipcpwd);

    char *cc = strstr(gDID, "WGAA-");
    if(cc)
    {//exclude arm table/part-arm table
        devlist[devcount].flag =( devlist[devcount].flag | 0x08 | 0x04); // rec+panic group
    }else
    {//original
        devlist[devcount].flag =( devlist[devcount].flag | 0x08 | 0x04 | 0x02 | 0x01); // rec+panic+paarm+arm group
    }

	addsource(max);
	//devstatlist[devcount].did = max;
	//memset(&devstatlist[devcount].evt, 0, sizeof(jswevent));

	addipc(max,dev->ipcdid, dev->ipcpwd );

	devcount++;
	savedata();

	//DBG_PRINT("#### send to client CM_NEWDEVICE %s,  %s, id %d\n", g_pairDevinfo.name, g_pairDevinfo.location, id);
	sendCmdtoClient(CM_NEWDEVICE, 0,  1,  sizeof(jswdev) ,(char*) &devlist[devcount-1]) ;

}

void addNewDevice(char *buf)
{
	jswdev *d;

	if (g_isPairMode ==1)
		return;

	d = (jswdev*)buf;
	if (d->model == TYPE_CAMERA )
	{
		addCamera(d);
		return ;
	}

	g_isPairMode = 1;
	memset(&g_pairDevinfo, 0, sizeof(jswdev));
	memcpy(&g_pairDevinfo, buf, sizeof(jswdev));

	DBG_PRINT("#### do pairing %s, %s, d->model=%d\n", g_pairDevinfo.name, g_pairDevinfo.location, d->model);

	if ( pairing_t != 0)
	{
		killTimer(pairing_current_t);
		makeTimer( &pairing_t, 0, 0);
		pairing_t = pairing_current_t = 0;
	}


	makeTimer( &pairing_t, 30, 0);
	pairing_current_t = pairing_t;
	writePairing(g_DID, d->model, 0xFF);
}



void sendCmdtoClient(int cmd, int errcode,  int count, int payload_len, char *payload) //without payload
{
	sendCmdtoClient2(cmd, errcode,  count, payload_len, payload, NULL);
}

void sendCmdtoClient2(int cmd, int errcode,  int count, int payload_len, char *payload, int sess) //without payload
{
	int datalen;
	char buff[MAX_P2P_BUFFER];
	jswhdr *h;
	int i=0;

    if((cmd >= 9000) && (cmd <= 9100))
    {
        DBG_PRINT("sendCmdtoClient(%d) cmd(xx), payload length %d, errCode %d\n", sess, payload_len, errcode);
    }else
    {
        DBG_PRINT("sendCmdtoClient(%d) cmd(%d) , payload length %d, errCode %d\n", sess, cmd, payload_len, errcode);
    }

	memset(buff, 0, sizeof(buff));
	h = (jswhdr*)buff;
	h->cmd = cmd;
	h->version = PRO_VERSION;
	h->count = count;
	h->payload_length = payload_len;
	h->errCode = errcode;

	/*pthread_mutex_lock(&aes_mutex);
	unsigned char *encrytStr = encryptAES((unsigned char *) devlist, &datalen, aeskey);
	pthread_mutex_unlock(&aes_mutex);*/

	if (payload_len >0 && payload != NULL)
	{
		char *p = buff + sizeof(jswhdr);
		memcpy(p, payload, payload_len);
	}
	datalen = h->payload_length+ sizeof(jswhdr);

    if(sess != 0)
//       ( (cmd == CM_GETACCT) || (cmd == CM_SETACCT) || (cmd == CM_ADMIN) || (cmd == CM_GETEVENT) || (cmd == CM_FW) ||
//         (cmd == CM_FW_START) || (cmd == CM_FW_STOP) || (cmd == CM_GET_BATT_LOW_LIST) || (cmd == CM_GET_PUSH_STR_LANG) ||
//         (cmd == CM_GET_PUSH_SELECTION1) || (cmd == CM_GETGWPROP) || (cmd == CM_GET_CITY_LATLNG) || (cmd == CM_GETALLZONE) ||
//         (cmd == CM_GETLINKMENU) || (cmd == CM_GETSOURCELINK) ||
//        ( (cmd >= 9000) && (cmd <= 9100) ) )
//        )
	{//Send to one session only
		i = getP2PSessionIndex(sess);
		if(i >= 0)
		{
			if ( gP2PSession[i].valid == 1)
			{
				int r = 0;
				if(gP2PSession[i].use_AES_data == 1)
				{
                    usleep(50*1000);
					r = PPPP_Write2(sess, CH_CTRL, (CHAR*) buff, datalen, 1, gP2PSession[i].aeskey);//getP2PSessionAesKey(sess));
                    usleep(50*1000);
				}else
				{
                    usleep(50*1000);
					r = PPPP_Write(sess, CH_CTRL, (CHAR*) buff, datalen);
                    usleep(50*1000);
				}
			}
		}
	}else
	{//Send to all sessions
		for (i = 0; i < P2P_SESSION_MAX; i++)
		{
			if ( gP2PSession[i].valid == 1)
			{
				int sess2 = gP2PSession[i].session ;
				int r = 0;
				if(gP2PSession[i].use_AES_data == 1)
				{
                    usleep(50*1000);
					r = PPPP_Write2(sess2, CH_CTRL, (CHAR*) buff, datalen, 1, gP2PSession[i].aeskey);//getP2PSessionAesKey(sess));
                    usleep(50*1000);
				}else
				{
                    usleep(50*1000);
					r = PPPP_Write(sess2, CH_CTRL, (CHAR*) buff, datalen);
                    usleep(50*1000);
				}
			}
		}
	}
}

void newNEST_Event(unsigned int id, unsigned char type, int status, int triggerAlarm)
{
	time_t tm = time(NULL);
//	if(type == TYPE_NEST_SMOKE)
//		type = TYPE_SMOKE;
	if(status == SMOKE_STATE_WARNING || status == SMOKE_STATE_EMERGENCY)
		status = RE_SMOKE_TRIGGERED;
	else
		return;
	newEvent(id,type,status,triggerAlarm);
	#if 0
	jswevent evt9;
	memset(&evt9, 0, sizeof(evt9));
	evt9.did = 0xff & id;
	evt9.time = tm;
	evt9.status = status;
	evt9.model = type;
	evt9.gwstate = DEV_TRIGGER;
	sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  sizeof(evt9),(char*) &evt9) ;
	#endif


#if 0
	//save to file
	//printf("devtype =%02X, status %d, event count %d\n" , type, status, evtcount);
	if (evtcount >= MAXEVENT)
	{
		memcpy(&o_evtlist, &evtlist, sizeof(evtlist)) ;
		is_oldevtexist =1;
		printf("max event, save data\n");
		saveEvdata();
		evtcount =0;

		delTmpEvdata(1);
	}else
	{
	    if( ((evtcount % MAXTMPEVENT) == 0) && (evtcount > 0) )
	    {//save every MAXTMPEVENT events for tmp event
	        saveTmpEvdata(1);
	    }
	}
    //send out to client

        sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  payload_len,(char*) &sendoutevt);
	#endif

}

int update_last_alarm(unsigned int id, unsigned char type, int status,unsigned int time_now ,int triggerAlarm)
{
	printf("!!!!! update_last_alarm[%d][%d][%d][%d]!!!!!!!!!!!\n",id,type,status,time_now);
	switch(type)
		{
		case TYPE_SIREN_INDOOR:
		case TYPE_SIREN_OUTDOOR:
			if(status == RE_SIREN_TEMPER)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_warning;
					return 1;
				}
			break;
		case TYPE_PIR:
			if(status == RE_PIR_TEMPER)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_warning;
					return 1;
				}
			else if(status == RE_PIR_MOTION)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_invasion;
					return 1;
				}
			break;
		case TYPE_MAGNETIC:
			if(status == RE_MAG_TEMPER)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_warning;
					return 1;
				}
			else if(status == RE_MAG_ISON)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_invasion;
					return 1;
				}
			break;
		case TYPE_REMOTE_CONTROL:
		case TYPE_REMOTE_CONTROL_NEW:
			if(status == RE_REMOTE_PANIC)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_SOS;
					return 1;
				}
			break;
		case TYPE_SMOKE:
		case TYPE_NEST_SMOKE:
			if(status == RE_SMOKE_TRIGGERED || status == RE_SMOKE_OVERHEAT)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_fire_emergency;
					return 1;
				}
			break;
		case TYPE_WATERLEVEL:
			if(status == RE_WATERLEVEL_ARM1 || status == RE_WATERLEVEL_ARM2)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_water_emergency;
					return 1;
				}
			break;
		case TYPE_KEYPAD_JSW:
		case TYPE_KEYPAD_JSW_NEW:
			if(status == RE_KEYPAD_PANIC)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_SOS;
					return 1;
				}
			else if(status == RE_KEYPAD_TEMPER)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_warning;
					return 1;
				}

			break;
		case TYPE_CAMERA:
			if(status == DEV_TRIGGER)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_invasion;
					return 1;
				}
			break;
		case TYPE_GATEWAY:
			if(status == GW_PANIC)
				{
					last_trigger.id = id;
					last_trigger.type = type;
					last_trigger.time = time_now;
					last_trigger.status = Alarm_SOS;
					return 1;
				}
			break;
		}
	return 0;
}

void newEvent(unsigned int id, unsigned char type, int status, int triggerAlarm)
{
	time_t tm = time(NULL);
	int x;
	int found =-1;
	static int last_event_time = 0;
	static int last_event_id = 0;
	static int last_event_type = 0;
	static int last_event_status = 0;
	int time_now = time(NULL);

	printf("### newEvent id = %d, type = %d status = %d \n",  id, (int)type, status);

    //reject same events in 2 secs
    if( ((time_now - last_event_time) >= 0) && ((time_now - last_event_time) <= 2) && (last_event_type == type) &&
       (last_event_status == status) && (last_event_id == id) )
    {
        SetRunningLog_str("Error: newEvent() reject same event in 2 seconds");
        return;
    }else
    {
        last_event_time = time_now;
        last_event_id = id;
        last_event_type = type;
        last_event_status = status;
    }

//add to current stat
	if (type == TYPE_GATEWAY)
	{
		found = 1;
	}else
	{
		for (x=0;x<devcount;x++)
		{
			if (devlist[x].did == id)
			{
				if( (status != GW_PANIC) && (status != RE_SENSOR_BATLOW) )
					devlist[x].status = status;
				found = x;
				break;
			}
		}
	}

	//check exception
	//if (type == TYPE_PIR)//only keep pir event at the following state.
	//{
	//	if (g_armstate != st_arm && g_armstate != st_partarm )
	//		return;
	//}

	if (status != RE_ABUS_AUTOREPORT && type != TYPE_GATEWAY )
	{
		for (x=0;x<devcount;x++)
		{
			if (keepalivelist[x].did == id)
			{
				keepalivelist[x].lastcheckin = time(0);
				keepalivelist[x].sent_push_event = 0;
				break;
			}
		}
	}


	if ( ( status == RE_SENSOR_BATLOW && found != -1) || ( status == RE_SENSOR_BATLOW && found == -1 && type == TYPE_MCU) )
	{
		if (g_armstate == st_disarm)
		{
			if (g_setting.gwprop.ledon == 1)
				yellowled(0);
			else if (g_setting.gwprop.ledon == 2)
				yellowled(1);
		}

		char msg[1024];
		memset(msg, 0, sizeof(msg));
		int str_id = 0;
		if( status == RE_SENSOR_BATLOW && found == -1 && type == TYPE_MCU)
		{
#ifdef DEF_FEATURE_MULTI_PUSH
            char msg2[256];
            get_push_string(msg2, STR_ID_GATEWAY_BATTERY_LOW);
            sprintf( msg, msg2, g_setting.gwprop.name);
#else
			sprintf(msg, "Your Gateway %s is running low on battery!", gDID);
#endif
			add_battlow(g_DID, type);
			str_id = STR_ID_GATEWAY_BATTERY_LOW;
		}else
		{
#ifdef DEF_FEATURE_MULTI_PUSH
            char msg2[256];
            get_push_string(msg2, STR_ID_SENSOR_BATTERY_LOW);
            sprintf( msg, msg2, devlist[found].name, devlist[found].location, g_setting.gwprop.name);
#else
			sprintf(msg, STRING_TRIGGER_BATTLOW_ALARM, devlist[found].name, devlist[found].location, gDID);
#endif
			add_battlow(devlist[found].did, devlist[found].model);
			str_id = STR_ID_SENSOR_BATTERY_LOW;
		}
		pushnotification(NT_BATT_LOW, msg, str_id);//battery low
	}


	//add to event db;
// Jeff Mark for SHC1.5

	int save_event = 1;//1:save event, 0:none save event
#if 0
	if( (type == TYPE_PIR) && (status == RE_PIR_MOTION) )
	{
		save_event = 0;
		if(g_armstate == st_testmode)
		{//test mode, save PIR event
			save_event = 1;
		}else
		{
            if( (g_armstate == st_partarm) || (st_arm == g_armstate ) || (st_entrydelay == g_armstate ) )
			{//trigger alarm, save PIR event
				if (checkDevFlag(id))
				{
					save_event = 1;
				}
            }else
            {//send PIR event for SFB in disarm mode
                jswevent evt9;
                memset(&evt9, 0, sizeof(evt9));
                evt9.did = id;
                evt9.time = tm;
                evt9.status = status;
                evt9.model = type;
                if ( triggerAlarm ==0)
                    evt9.gwstate = g_armstate;
                else
                {
                    evt9.gwstate = DEV_TRIGGER;
                }
                sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  sizeof(evt9),(char*) &evt9) ;
			}
		}
	}
#endif
	if( (type == TYPE_VIBRATION) && (status == RE_VIBRATION_TRIGGER))
	{
		save_event = 0;
		if(g_armstate == st_testmode)
		{//test mode, save Vibration event
			save_event = 1;
		}else
		{
            jswdev *dev;
            dev = getDevbydid(id);
            if(dev)
            {
                if( (g_armstate == st_partarm) || (st_arm == g_armstate ) || (st_entrydelay == g_armstate ) )
                {//trigger alarm, save Vibration event
                    if( checkDevFlag(id) || (dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE) )
                    {
                        save_event = 1;
                    }
                }else
                {//send Vibration event for SFB in disarm mode
                    if(dev->ext2 == RE_VIBRATION_SET_TAMPER_MODE)
                    {
                        save_event = 1;
                    }
                }
//                jswevent evt9;
//                memset(&evt9, 0, sizeof(evt9));
//                evt9.did = id;
//                evt9.time = tm;
//                evt9.status = status;
//                evt9.model = type;
//                if ( triggerAlarm ==0)
//                    evt9.gwstate = g_armstate;
//                else
//                {
//                    evt9.gwstate = DEV_TRIGGER;
//                }
//                sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  sizeof(evt9),(char*) &evt9) ;
			}
		}
	}
	if(save_event == 1)
	{
		evtlist[evtcount].did = id;
		evtlist[evtcount].time = tm;
		evtlist[evtcount].status = status;
		evtlist[evtcount].model = type;
		if ( triggerAlarm ==0)
			evtlist[evtcount].gwstate = g_armstate;
		else
        {
//            if( (type == TYPE_SMOKE) && (g_armstate == st_testmode) )
//                evtlist[evtcount].gwstate = g_armstate;
//            else
                evtlist[evtcount].gwstate = DEV_TRIGGER;
        }

		if( status == RE_SENSOR_BATLOW && found == -1 && type == TYPE_MCU)
		{
			evtlist[evtcount].did = g_DID;
			evtlist[evtcount].model = TYPE_GATEWAY;
		}

		evtcount++;

		char szName[100];
		if (type == TYPE_GATEWAY)
		{
			strcpy( szName, "Gateway");
		}
		else  if(found > 0)
		{
			strcpy(szName, devlist[found].name);

		}
		//printf("### Add event name = %s , type = %d \n", szName, type);
		cloud_event_addEevent(id, type ,  status, szName);
	}

	//send to client
	int payload_len = sizeof(jswevent);
	unsigned char sendout_flag = 0;
	jswevent sendoutevt;
	memset(&sendoutevt, 0, sizeof(jswevent));

	if ( (found != -1) || (found == -1 && type == TYPE_MCU && status == RE_SENSOR_BATLOW) || (type == TYPE_NEST_SMOKE))
	{
	    if(g_scenario_start_by_mag > 0)
	    {
	        int tt = evtlist[evtcount-1].time;
	        evtlist[evtcount-1].time -= 2;

            //sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  payload_len,(char*) &evtlist[evtcount-1]);
            sendout_flag = 1;
            memcpy(&sendoutevt, &evtlist[evtcount-1], sizeof(sendoutevt));

            evtlist[evtcount-1].time = tt;
            g_scenario_start_by_mag = 0;
	    }else
	    {
            //sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  payload_len,(char*) &evtlist[evtcount-1]);
            sendout_flag = 1;
            memcpy(&sendoutevt, &evtlist[evtcount-1], sizeof(sendoutevt));
	    }
	}else
	{
		DBG_PRINT("#### event did not found %d,  %d\n", id , type);
	}

    //no event for test mode
    if (g_armstate == st_testmode)
    {
        if(save_event == 1)
            evtcount--;
        //send out to client
        if(sendout_flag > 0)
        {
            sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  payload_len,(char*) &sendoutevt);
        }
        return;
    }

	if(save_event == 0)
	{
        //send out to client
        if(sendout_flag > 0)
        {
            sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  payload_len,(char*) &sendoutevt);
        }
        return;
	}
	if((type == TYPE_CAMERA) && (status == DEV_TRIGGER))
	{
	    if(update_last_alarm(id, type,status,time_now,triggerAlarm) == 1)
			save_alarm_info();
	}

	
	printf("### mag on beep, type = %d , status = %d \n", type, status);
	//if( type == TYPE_MAGNETIC && status == RE_MAG_ISON)
	//{
	//	printf("### mag on beep g_armstate = %d  \n", g_armstate);
	//	if(g_armstate == st_disarm)
	//		play_beep_disarm();
	//}

    //exchange events, e.g. MAG open <-> Siren On
    if( ((type == TYPE_MAGNETIC) && (status == RE_MAG_ISON)) ||
        ((type == TYPE_MAGNETIC) && (status == RE_MAG_TEMPER)) ||
        ((type == TYPE_PIR) && (status == RE_PIR_MOTION)) ||
        ((type == TYPE_PIR) && (status == RE_PIR_TEMPER)) ||
		((type == TYPE_VIBRATION) && (status == RE_VIBRATION_TRIGGER)) ||
		((type == TYPE_VIBRATION) && (status == RE_VIBRATION_TAMPER)) ||
		((type == TYPE_BUTTON) && (status == RE_BUTTON_PRESS)) ||
		((type == TYPE_BUTTON) && (status == RE_BUTTON_LONG_PRESS)) ||
        ((type == TYPE_SIREN_OUTDOOR) && (status == RE_SIREN_TEMPER)) ||
        ((type == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_TEMPER)) ||
        ((type == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_PANIC)) ||
        ((type == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_LOCK)) ||
        ((type == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_UNLOCK)) ||
        ((type == TYPE_REMOTE_CONTROL) && (status == RE_REMOTE_PART_ARM)) ||
        ((type == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_TEMPER)) ||
        ((type == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_PANIC)) ||
        ((type == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_LOCK)) ||
        ((type == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_UNLOCK)) ||
        ((type == TYPE_REMOTE_CONTROL_NEW) && (status == RE_REMOTE_PART_ARM)) ||
        ((type == TYPE_KEYPAD_JSW) && (status == RE_KEYPAD_TEMPER)) ||
        ((type == TYPE_KEYPAD_JSW_NEW) && (status == RE_KEYPAD_TEMPER)) ||
        ((type == TYPE_SMOKE) && (status == RE_SMOKE_OVERHEAT)) ||
        ((type == TYPE_SMOKE) && (status == RE_SMOKE_TRIGGERED)) ||
        ((type == TYPE_NEST_SMOKE) &&  (status == RE_SMOKE_TRIGGERED)) ||
        ((type == TYPE_WATERLEVEL) &&  (status == RE_WATERLEVEL_ARM1)) ||
        ((type == TYPE_WATERLEVEL) &&  (status == RE_WATERLEVEL_ARM2)) ||
        ((type == TYPE_GATEWAY) && (status == GW_PANIC)) ||
        ((type == TYPE_GATEWAY) && (status == 0)) ) //system disarm
    {//mag open, mag tamper, PIR Motion, PIR tamper, Siren tamper, Remote Tamper, Keypad tamper,
       if(update_last_alarm(id, type,status,time_now,triggerAlarm) == 1)
			save_alarm_info();
// Jeff Mark  for disable event sorting
#if 0
        if( (evtcount > 1) || (is_oldevtexist == 1) )
        {
            found = 0;
            jswevent jj;
            int tm1, tm2, tm3;
            jswevent kk[MAXEVENT*2];
            int count3 = evtcount+MAXEVENT;
            memset(kk, 0, sizeof(kk));
            memcpy(kk+MAXEVENT, evtlist, sizeof(jswevent)*evtcount);
            if(is_oldevtexist == 1)
            {
                memcpy(kk, o_evtlist, sizeof(o_evtlist));
            }

            int switch_event = 0;
            for(x=count3-1;x>=0;x--)
            {
                if(kk[x].did <= 0)
                    continue;
                if( ((tm - kk[x].time) <= 5) && ((tm - kk[x].time) >= 0) )
                {//in 5 secs
                    switch_event = 0;

                    //for 123 -> 321
                    if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_PANIC)) && //remote panic
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == GW_PANIC)) && //system panic
                        ((kk[x-2].model == TYPE_SIREN_OUTDOOR) && (kk[x-2].status == RE_SIREN_ISON)) //siren on
                       )
                    {//123 -> 321
                        printf("Event switch 1\n");
                        switch_event = 3;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_UNLOCK)) && // remote disarm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 0)) && //system disarm
                        ((kk[x-2].model == TYPE_SIREN_OUTDOOR) && (kk[x-2].status == RE_SIREN_ISOFF)) //siren off
                       )
                    {//123 -> 321
                        printf("Event switch 2\n");
                        switch_event = 3;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_PANIC)) && //remote panic
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == GW_PANIC)) && //system panic
                        ((kk[x-2].model == TYPE_SIREN_OUTDOOR) && (kk[x-2].status == RE_SIREN_ISON)) //siren on
                       )
                    {//123 -> 321
                        printf("Event switch 2_A\n");
                        switch_event = 3;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_UNLOCK)) && // remote disarm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 0)) && //system disarm
                        ((kk[x-2].model == TYPE_SIREN_OUTDOOR) && (kk[x-2].status == RE_SIREN_ISOFF)) //siren off
                       )
                    {//123 -> 321
                        printf("Event switch 2_B\n");
                        switch_event = 3;


                    //for 123 -> 312
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_UNLOCK)) && // remote disarm
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISOFF)) && //siren off
                        ((kk[x-2].model == TYPE_GATEWAY) && (kk[x-2].status == 0)) //system disarm
                       )
                    {//123 -> 312
                        printf("Event switch 2_1\n");
                        switch_event = 2;
                    }else if( ((kk[x].model == TYPE_GATEWAY) && (kk[x].status == 0)) && // system disarm
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISOFF)) && //siren off
                        ((kk[x-2].model == TYPE_SIREN_OUTDOOR) && (kk[x-2].status == RE_SIREN_ISOFF)) //siren off
                       )
                    {//123 -> 312
                        printf("Event switch 2_2\n");
                        switch_event = 2;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_UNLOCK)) && // remote disarm
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISOFF)) && //siren off
                        ((kk[x-2].model == TYPE_GATEWAY) && (kk[x-2].status == 0)) //system disarm
                       )
                    {//123 -> 312
                        printf("Event switch 2_3\n");
                        switch_event = 2;


                    //for 12 -> 21
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_LOCK)) && //remote arm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 1)) //system arm
                       )
                    {//12 -> 21
                        printf("Event switch 3\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_UNLOCK)) && //remote disarm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 0)) //system disarm
                       )
                    {//12 -> 21
                        printf("Event switch 4\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_PART_ARM)) && //remote part arm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 2)) //system part arm
                       )
                    {//12 -> 21
                        printf("Event switch 5\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL) && (kk[x].status == RE_REMOTE_PANIC)) && //remote panic
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == GW_PANIC)) //system panic
                       )
                    {//12 -> 21
                        printf("Event switch 6\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_MAGNETIC) && (kk[x].status == RE_MAG_ISON)) && //mag open
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 7\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_PIR) && (kk[x].status == RE_PIR_MOTION)) && //PIR motion
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 8\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_MAGNETIC) && (kk[x].status == RE_MAG_TEMPER)) && //MAG tamper
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 9\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_PIR) && (kk[x].status == RE_PIR_TEMPER)) && //PIR tamper
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 10\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_SMOKE) && (kk[x].status == RE_SMOKE_OVERHEAT)) && //smoke overheat
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 11\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_SMOKE) && (kk[x].status == RE_SMOKE_TRIGGERED)) && //smoke trigger
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 12\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_KEYPAD_JSW) && (kk[x].status == RE_KEYPAD_TEMPER)) && //keypad tamper
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 13\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_SIREN_OUTDOOR) && (kk[x].status == RE_SIREN_TEMPER)) && //siren tamper
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 14\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_GATEWAY) && (kk[x].status == 0)) && //system disarm
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISOFF)) //siren off
                       )
                    {//12 -> 21
                        printf("Event switch 15\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_LOCK)) && //remote arm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 1)) //system arm
                       )
                    {//12 -> 21
                        printf("Event switch 16\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_UNLOCK)) && //remote disarm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 0)) //system disarm
                       )
                    {//12 -> 21
                        printf("Event switch 17\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_PART_ARM)) && //remote part arm
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == 2)) //system part arm
                       )
                    {//12 -> 21
                        printf("Event switch 18\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_REMOTE_CONTROL_NEW) && (kk[x].status == RE_REMOTE_PANIC)) && //remote panic
                        ((kk[x-1].model == TYPE_GATEWAY) && (kk[x-1].status == GW_PANIC)) //system panic
                       )
                    {//12 -> 21
                        printf("Event switch 19\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_KEYPAD_JSW_NEW) && (kk[x].status == RE_KEYPAD_TEMPER)) && //keypad tamper
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 20\n");
                        switch_event = 1;
                    }else if( ((kk[x].model == TYPE_NEST_SMOKE) && (kk[x].status == RE_SMOKE_TRIGGERED)) && //smoke trigger
                        ((kk[x-1].model == TYPE_SIREN_OUTDOOR) && (kk[x-1].status == RE_SIREN_ISON)) //siren on
                       )
                    {//12 -> 21
                        printf("Event switch 21\n");
                        switch_event = 1;
                    }

                    //switch events
                    if(switch_event > 0)
                    {//Switch event
                        if(switch_event == 3)
                        {//123 -> 321
                            if(x > 1)
                            {
                                tm1 = kk[x].time;
                                tm2 = kk[x-1].time;
                                tm3 = kk[x-2].time;
                                memcpy(&jj, &kk[x], sizeof(jswevent));
                                memcpy(&kk[x], &kk[x-2], sizeof(jswevent));
                                memcpy(&kk[x-2], &jj, sizeof(jswevent));
                                kk[x].time = tm1;
                                kk[x-1].time = tm2;
                                kk[x-2].time = tm3;
                                found = 1;
                                break;
                            }
                        }else if(switch_event == 2)
                        {//123 -> 312
                            if(x > 1)
                            {
                                tm1 = kk[x].time;
                                tm2 = kk[x-1].time;
                                tm3 = kk[x-2].time;
                                memcpy(&jj, &kk[x], sizeof(jswevent));
                                memcpy(&kk[x], &kk[x-1], sizeof(jswevent));
                                memcpy(&kk[x-1], &kk[x-2], sizeof(jswevent));
                                memcpy(&kk[x-2], &jj, sizeof(jswevent));
                                kk[x].time = tm1;
                                kk[x-1].time = tm2;
                                kk[x-2].time = tm3;
                                found = 1;
                                break;
                            }
                        }else if(switch_event == 1)
                        {//12 -> 21
                            if(x > 0)
                            {
                                tm1 = kk[x].time;
                                tm2 = kk[x-1].time;
                                memcpy(&jj, &kk[x], sizeof(jswevent));
                                memcpy(&kk[x], &kk[x-1], sizeof(jswevent));
                                memcpy(&kk[x-1], &jj, sizeof(jswevent));
                                kk[x].time = tm1;
                                kk[x-1].time = tm2;
                                found = 1;
                                break;
                            }
                        }
                    }
                }else
                {
                    //quit for old events
                    break;
                }
            }

            //copy events back to evtlist and o_evtlist
            if(found == 1)
            {
                memcpy(evtlist, kk+MAXEVENT, sizeof(jswevent)*evtcount);
                if(is_oldevtexist == 1)
                {
                    memcpy(o_evtlist, kk, sizeof(o_evtlist));
                }
            }
        }
#endif
    }

    //send out to client
    if(sendout_flag > 0)
    {

        sendCmdtoClient(CM_SENSORUPDATE, 0,  1,  payload_len,(char*) &sendoutevt);
    }
	printf("evtcount=%d\n",evtcount);
	//save to file
	//printf("devtype =%02X, status %d, event count %d\n" , type, status, evtcount);
	if (evtcount >= MAXEVENT)
	{
		memcpy(&o_evtlist, &evtlist, sizeof(evtlist)) ;
		is_oldevtexist =1;
		printf("max event, save data\n");
		saveEvdata();
		evtcount =0;

		delTmpEvdata(1);
	}else
	{
	    if( ((evtcount % MAXTMPEVENT) == 0) && (evtcount > 0) )
	    {//save every MAXTMPEVENT events for tmp event
	        saveTmpEvdata(1);
	    }
	}

}


void sendemail(char *title, char *msg)
{
	char data[1500];
	int x=0;

	//char *title ="Your System has been tampered.";

	memset(data, 0, sizeof(data));

	/*pushdata.emailcount = 1;
	pushdata.emailon =1;
	strcpy( emaillist[0].email, "kafka2004@gmail.com");*/

	if (pushdata.emailcount <= 0 || pushdata.emailon == 0)
	{
		printf("email count zero while trying to send email\n");
        SetRunningLog_str("Running: sendemail() return by no emailcount or emailon");
		return;
	}

	for (x=0;x<pushdata.emailcount;x++)
	{
		if ( x == 0 )
			sprintf(data, "%s", emaillist[x].email);
		else
			sprintf(data, "%s,%s", data, emaillist[x].email);
	}
    //from address
	sprintf(data, "%s;donot-reply@omguard.com", data);
	//subject
	sprintf(data, "%s;%s", data, title);
    //body
	sprintf(data, "%s;%s;", data, msg);

	//printf(data);
	//printf("\n");

	sendEmailNotification(data, strlen(data));

	/*local to_address = mail_group
        local from_address = "donot-reply@omguard.com"
        local subject = title
        local content = msg
        local email_notification_command = to_address .. ";" .. from_address .. ";" .. subject .. ";" .. content .. ";"
        logDebug("email_notification_command: " .. email_notification_command)*/

}

///////////////////gw state

//clear all smokes' state to normal
void clear_all_smoke_state()
{
    int i;
    for(i=0;i<devcount;i++)
    {
        if( (devlist[i].model != TYPE_SMOKE) && (devlist[i].model != TYPE_WATERLEVEL) )
            continue;
        devlist[i].status = 0;
    }
}

void systemArm()
{
	int nttype = 0;
	char msg[1024];
	int MAGOpenCount= 0;
	memset(msg, 0, sizeof(msg));

	// Jeff define for SHC 1.5 Arm
	play_beep_armed();

    //for flash siren
	stopAlarm();
	//exitdelayTarget
	g_armstate = exitdelayTarget;

	newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
	//buzzer , light

    int str_id = 0;
	MAGOpenCount = getMAGOpenCount();
	if (g_armstate == st_arm)
	{
		sendCmdtoClient(CM_ARM, 0, 0,  0,NULL) ;
#ifdef DEF_FEATURE_MULTI_PUSH
        char msg2[256];

		if(MAGOpenCount > 0)
		{
			get_push_string(msg2, STR_ID_ARM);
			sprintf( msg, msg2, g_setting.gwprop.name);
			strcat(msg,ARM_ABNORMAL);
			int i;
			strcat(msg," [");
			for(i = 0 ; i < MAGOpenCount ; i++)
			{
				if(i > 4) {strcat(msg,"......"); break;}
				if(i > 0) strcat(msg,", ");
				strcat(msg,mag_open_list[i]);
			}
			strcat(msg," ]");
		}
		else
		{
        	get_push_string(msg2, STR_ID_ARM);
        	sprintf( msg, msg2, g_setting.gwprop.name);
		}

#else
		sprintf( msg, "Your SHC_pro System %s : is now Armed.", gDID);
#endif
		nttype = NT_ARM;
		str_id = STR_ID_ARM;
	}
	else if (g_armstate == st_partarm)
	{
		sendCmdtoClient(CM_PARTARM, 0, 0,  0,NULL) ;
#ifdef DEF_FEATURE_MULTI_PUSH
        char msg2[256];
		if(MAGOpenCount > 0)
		{
	        get_push_string(msg2, STR_ID_PART_ARM);
	        sprintf( msg, msg2, g_setting.gwprop.name);
			strcat(msg,ARM_ABNORMAL);
			int i;
			strcat(msg," [");
			for(i = 0 ; i < MAGOpenCount ; i++)
			{
				if(i > 4) {strcat(msg,"......"); break;}
				if(i > 0) strcat(msg,", ");
				strcat(msg,mag_open_list[i]);
			}
			strcat(msg," ]");

		}
		else
		{
	        get_push_string(msg2, STR_ID_PART_ARM);
	        sprintf( msg, msg2, g_setting.gwprop.name);
		}

#else
		sprintf( msg, "Your SHC_pro System %s : is now internal armed.", gDID);
#endif
		nttype = NT_PARTARM;
		str_id = STR_ID_PART_ARM;
	}
    if (strlen(msg) != 0)
		pushnotification(nttype, msg, str_id);//arm , part arm

	int n = g_setting.gwprop.ledon;
	if (n == 1)
		blueled(0);
	else if (n == 2)
		blueled(1);

	pushnotification2("{\"on\":true, \"sat\":254, \"bri\":254,\"hue\":46920,\"alert\":\"none\"}");

	savesetting4();
}

void systemDisarm()
{
	char msg[1024];
	memset(msg, 0, sizeof(msg));

	g_armstate = st_disarm;
	g_ShowAlarmView = 0;//APP not show alarm view
	g_AlarmState = ALARM_STATE_NO_ALARM;
	g_smokeOn = 0;

	printf("system disarm \n");
	// Jeff define for SHC 1.5 disArm
	play_beep_disarm();
	Force_stopAlarm();
#ifdef SIREN_TIMER
	//light , buzzer
	if (siren_current_t != 0)//disable siren timeout.
	{
		//printf("system disarm");
		//makeTimer( &siren_t, 0, 0);
		killTimer(siren_current_t);
		siren_current_t = siren_t = 0;
	}
#endif
	ledoff();
	sendCmdtoClient(CM_DISARM, 0, 0,  0,NULL) ;

	newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);

	set_alarm_LED();

    usleep(100*1000);
    if( (g_battlowcount > 0) || (getMAGOpenCount() > 0) )
        yellowled(1);

	clear_all_smoke_state();

#ifdef DEF_FEATURE_MULTI_PUSH
    char msg2[256];
    get_push_string(msg2, STR_ID_DISARM);
	sprintf( msg, msg2, g_setting.gwprop.name);
#else
	sprintf( msg, "Your SHC_pro System %s : is now disarmed.", gDID);
#endif
	//if (pushdata.pushon ==1)

    if (strlen(msg) != 0)
		pushnotification(NT_DISARM, msg, STR_ID_DISARM);//disarm

	pushnotification2("{\"on\":false,\"alert\":\"none\"}");

	savesetting4();
}

void systemArm2()
{
	int nttype = 0;
	char msg[1024];
	memset(msg, 0, sizeof(msg));


	//exitdelayTarget
	g_armstate = exitdelayTarget;
	//newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
	//buzzer , light

	if (g_armstate == st_arm)
	{
		//sendCmdtoClient(CM_ARM, 0, 0,  0,NULL) ;
#ifdef DEF_FEATURE_MULTI_PUSH
        char msg2[256];
		if(getMAGOpenCount() > 0)
		{
			get_push_string(msg2, STR_ID_ARM);
			sprintf( msg, msg2, g_setting.gwprop.name);
			strcat(msg,ARM_ABNORMAL);
		}
		else
		{
        	get_push_string(msg2, STR_ID_ARM);
        	sprintf( msg, msg2, g_setting.gwprop.name);
		}
#else
		sprintf( msg, "Your SHC_pro System %s : is now Armed.", gDID);
#endif
		nttype = NT_ARM;

	}
	else if (g_armstate == st_partarm)
	{
		//sendCmdtoClient(CM_PARTARM, 0, 0,  0,NULL) ;
#ifdef DEF_FEATURE_MULTI_PUSH
        char msg2[256];
		if(getMAGOpenCount() > 0)
		{
	        get_push_string(msg2, STR_ID_PART_ARM);
	        sprintf( msg, msg2, g_setting.gwprop.name);
			strcat(msg,ARM_ABNORMAL);
		}
		else
		{
	        get_push_string(msg2, STR_ID_PART_ARM);
	        sprintf( msg, msg2, g_setting.gwprop.name);
		}
#else
		sprintf( msg, "Your SHC_pro System %s : is now internal armed.", gDID);
#endif
		nttype = NT_PARTARM;
	}
    //if (strlen(msg) != 0)
		//pushnotification(nttype, msg);//arm , part arm

	int n = g_setting.gwprop.ledon;
	if (n == 1)
		blueled(0);
	else if (n == 2)
		blueled(1);

	pushnotification2("{\"on\":true, \"sat\":254, \"bri\":254,\"hue\":46920,\"alert\":\"none\"}");

	savesetting4();
}

void systemDisarm2()
{
	char msg[1024];
	memset(msg, 0, sizeof(msg));

	g_armstate = st_disarm;
	g_AlarmState = ALARM_STATE_NO_ALARM;

	printf("system disarm 2\n");
	//stopAlarm();

	//light , buzzer
#ifdef SIREN_TIMER
	if (siren_current_t!= 0)//disable siren timeout.
	{
		//printf("system disarm");
		//makeTimer( &siren_t, 0, 0);
		killTimer(siren_current_t);
		siren_current_t = siren_t = 0;
	}
#endif
	ledoff();
	//sendCmdtoClient(CM_DISARM, 0, 0,  0,NULL) ;

	//newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);

    usleep(100*1000);
    if( (g_battlowcount > 0) || (getMAGOpenCount() > 0) )
        yellowled(1);

	clear_all_smoke_state();

#ifdef DEF_FEATURE_MULTI_PUSH
    char msg2[256];
    get_push_string(msg2, STR_ID_DISARM);
	sprintf( msg, msg2, g_setting.gwprop.name);
#else
	sprintf( msg, "Your SHC_pro System %s : is now disarmed.", gDID);
#endif
		//if (pushdata.pushon ==1)

    //if (strlen(msg) != 0)
		//pushnotification(NT_DISARM, msg);//disarm

	pushnotification2("{\"on\":false,\"alert\":\"none\"}");

	savesetting4();
}


void exitDelay()
{
	//printf("Exit delay start... \n");
	g_ExitDelay_parm.avtive_time = time(0) +  g_setting.gwprop.exitdelay;
	printf("Exit delay start... until(%ld)\n",(long)g_ExitDelay_parm.avtive_time);
	g_ExitDelay_parm.toState = exitdelayTarget;
	//sendCmdtoClient(CM_EXITDELAY, 0, 0,  0,NULL) ;
	sendCmdtoClient(CM_EXITDELAY, 0,1 , sizeof(stExitDelay_parm),(char*)&g_ExitDelay_parm) ;
	makeTimer( &exitdelay_t, g_setting.gwprop.exitdelay, 0);
	// Jeff define for avoid quick switch ARM/DISARM
	exitdelay_current_t = exitdelay_t;
}

void stopExitDelay()
{
	//
	killTimer(exitdelay_current_t);
	g_ExitDelay_parm.avtive_time = 0;
	g_ExitDelay_parm.toState = 0;
	makeTimer( &exitdelay_t, 0, 0);
	exitdelay_current_t = exitdelay_t = 0;

}

void entryDelay(unsigned int deviceDID)
{
	printf("======================entryDelay(%d)\n",deviceDID);
	if (g_setting.gwprop.entrydelay >0)
	{
		entrydelayState = g_armstate;

		g_armstate = st_entrydelay;
		g_EntryDelay_parm.avtive_time = time(0) +  g_setting.gwprop.entrydelay -1;
		printf("start Entry delay... until(%ld) by[%ld]\n",(long)g_EntryDelay_parm.avtive_time,deviceDID);
		g_EntryDelay_parm.deviceID= deviceDID;


		makeTimer( &entrydelay_t, g_setting.gwprop.entrydelay, 0);
		sendCmdtoClient(CM_ENTRYDELAY, 0, 1, sizeof(stEntryDelay_parm),(char*)&g_EntryDelay_parm) ;
	}else
		triggerAlarm(0, 0);
}

void stopEntryDelay()
{
	printf("disarm stop Entry delay...\n");
//	g_EntryDelay_parm.avtive_time = 0;
//	g_EntryDelay_parm.deviceID= 0;
	makeTimer( &entrydelay_t, 0, 0);
	entrydelay_t = 0;
}

int checkDevFlag(unsigned int did)
{
	int x=0;
	int check_state = 0;

	for (x=0;x<devcount;x++)
	{
		if (devlist[x].did != did)
			continue;

        if(g_armstate == st_entrydelay)
        {
            check_state = entrydelayState;
        }else
        {
            check_state = g_armstate;
        }

	    if (check_state == st_arm)//arm
		{
			if ( ( devlist[x].flag & 0x01) == 0x01 )
				return 1;
			else
				return 0;
		}else if (check_state == st_partarm)//part arm
		{
			if ( ( devlist[x].flag & 0x02) == 0x02 )
				return 1;
			else
				return 0;
		}else
			return 0;
	}
	return 0;


}

int checkDoorOpen(int cmd)
{
	int x=0;
	for (x=0;x<devcount;x++)
	{
		if (devlist[x].model != TYPE_MAGNETIC)
			continue;
		if (devlist[x].status == RE_MAG_ISON ) // OLD 0x01)
		{
			if (cmd == c_partarm)
			{
				if ( ( devlist[x].flag & flagpartArmset )  ==flagpartArmset )
					return 1;
			}else if (cmd == c_arm)
			{
				if ( ( devlist[x].flag & flagArmset )  == flagArmset )
					return 1;
			}else if (cmd == c_disarm)
				return 1;

		}else
			break;
	}

	return 0;
}

void armCommand(int cmd)
{
	time_t now = time(NULL);

	printf("gateway change state, current %d, next %d \n" , g_armstate, cmd);
	if (g_armstate == st_disarm )
	{
		switch (cmd)
		{
			case c_disarm:
				systemDisarm();
				wait_force_arm = 0;
				break;

			case c_arm:
			case c_partarm:
				printf("Get ARM from client\n");
				if(Customer_code == CUSTOMER_ALC)
				{
					if ( getMAGOpenCount() > 0 && wait_force_arm ==0)
					{
						printf("SET ARM but MAG open\n");
						send_ARM_abnormal_notify();
						wait_force_arm = now + FORCE_ARM_TIME;
						sendCmdtoClient(CM_DISARM, 0, 0,  0,NULL) ;

					}
					else if(getMAGOpenCount() > 0 && (wait_force_arm > 0 && now <= wait_force_arm))
						{
						exitdelayTarget = cmd;
						printf("FORCE ARM\n");
						systemArm();
						}
					else if(getMAGOpenCount() > 0 && (wait_force_arm > 0 && now > wait_force_arm))
						{
						printf("wait FORCE ARM timeout\n");
						send_ARM_abnormal_notify();
						wait_force_arm = now + FORCE_ARM_TIME;
						sendCmdtoClient(CM_DISARM, 0, 0,  0,NULL) ;
						}
					else
					{

						exitdelayTarget = cmd;
						if (g_setting.gwprop.exitdelay > 0)
						{
							g_armstate = st_exitdelay;
							wait_force_arm = 0;
							play_beep_arm();
							exitDelay();
						}
						else
							systemArm();
					}
				}
				else
					{

					exitdelayTarget = cmd;
					if (g_setting.gwprop.exitdelay > 0)
					{
						g_armstate = st_exitdelay;
						wait_force_arm = 0;
						play_beep_arm();
						exitDelay();
					}
					else
						systemArm();
					}
				break;

			default:
				break;
		}
	}else if (g_armstate == st_arm )
	{
		wait_force_arm = 0;
		switch (cmd)
		{
		case c_disarm:
			wait_force_arm = 0;
			systemDisarm();
			break;
		case c_partarm:
			g_armstate = st_partarm;
			newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
			break;
		default:
			break;
		}
	}else if (g_armstate == st_partarm )
	{
		wait_force_arm = 0;
		switch (cmd)
		{
		case c_disarm:
			wait_force_arm = 0;
			systemDisarm();
			break;
		case c_arm:
			g_armstate = st_arm;
			newEvent(g_DID, TYPE_GATEWAY, g_armstate, 0);
			break;
		default:
			break;
		}
	}else if (g_armstate == st_exitdelay )
	{
		switch (cmd)
		{
		case c_disarm:
			wait_force_arm = 0;
			stopExitDelay();
			systemDisarm();
			break;
		case c_partarm:
		case c_arm:
			if(wait_force_arm > 0 && now <= wait_force_arm)
			{
				printf("FORCE ARM from st_exitdelay \n");
				exitdelayTarget = cmd;
				systemArm();
			}
			break;
		default:
			break;
		}
	}else if (g_armstate == st_entrydelay )
	{
		switch (cmd)
		{
		case c_disarm:
			wait_force_arm = 0;
			stopEntryDelay();
			systemDisarm();
			break;
		default:
			break;
		}
	}else if (g_armstate == st_maintain )
	{
		switch (cmd)
		{
		case c_disarm:
			wait_force_arm = 0;
			//systemDisarm();
			break;
		default:
			break;
		}
	}else if (g_armstate == st_testmode )
	{
		switch (cmd)
		{
		case c_disarm:
            Force_stopAlarm();
            ledoff();
			break;
		default:
			break;
		}
	}

}


//dev:sensor device, doorchime_track:song no, from 1-7, save_event:1->save event, 0->not save event
void play_doorchime(jswdev *dev, int doorchime_track, int save_event)
{
    int doorchime_track2 = 0;
    unsigned char doorchime_cmd = 0;

    if(doorchime_track <= 0)
        doorchime_cmd = 0;
    else if(doorchime_track <= DEF_DOORCHIME_SONG_MAX)
    {
        if(doorchime_track == DEF_ALARM_51_SONG_NO)
        {
            doorchime_cmd = CMD_DOORCHIME_ALARM;
        }else
        {
            doorchime_track2 = doorchime_track - 1;
            doorchime_cmd = ((doorchime_track2 << 4) & 0xf0) | 0x01;
        }
    }

    int ret = writeCommand( dev->did, dev->model, doorchime_cmd);
    if (ret == 1)
    {
        if(save_event == 1)
            newEvent(dev->did, dev->model, 0x01, 0);
    }
}

void panic()// 0 = keyfob ,  1 = app,  2 = keypad.
{
	int x;
	int found = 0;
	int ret = 0;

	if (g_armstate == st_maintain)
		return;

    if(g_armstate != st_testmode)
    {
        g_armstate = st_arm;
        exitdelayTarget = st_arm;
        g_ShowAlarmView = 1;
        g_AlarmState = ALARM_STATE_PANIC;
        systemArm2();
    }
#ifndef MELODY_IC
	//triggerAlarm();
	if (g_sirenstate ==0)
	{
		//system(". jsw_alarm.sh 180&");
		pthread_create(&alarm_th, NULL, startgatewayalarm_thread, 0);
		pthread_join(alarm_th, NULL);
	}
	//need to trigger gateway alarm and light too .
#else
	play_beep_alarm();
#endif
    //save event first
	newEvent(g_DID, TYPE_GATEWAY, GW_PANIC, 0);

	for (x=0;x<devcount;x++)
	{
		if ( isTarget(devlist[x].model ) ==1)
		{
			if ( devlist[x].model == TYPE_CAMERA )
			{
				if ( ( devlist[x].flag & 0x04) == 0x04 )
					doRec(devlist[x].did);
			}
			else
			{
				if ( ( devlist[x].flag & 0x04) == 0x04 )
				{
					if ( devlist[x].model == TYPE_POWER_ADAPTER  )
					{
						printf("send panic to adaptor %u", devlist[x].did);
						ret =writeCommand(devlist[x].did,devlist[x].model, CMD_ADAPTER_TURNON);
						if (ret ==1)
						{
							newEvent(devlist[x].did,devlist[x].model, CMD_ADAPTER_TURNON, 0);
							devlist[x].status = RE_ADAPTER_ISON;
							savedata();
						}

						//printf("[panic] wakeup power adaptor 1.2 sec\n");
						//	mSecSleep(400);
					}

					if ( devlist[x].model == TYPE_SIREN_INDOOR || devlist[x].model == TYPE_SIREN_OUTDOOR )
					{

						//printf("start siren\n");
						ret = writeCommand(devlist[x].did,devlist[x].model, CMD_SIREN_TURNON);
#ifndef FORCE_UPDATE_SIREN_STATE
						if (ret ==1)
#endif
							newEvent(devlist[x].did,devlist[x].model, CMD_SIREN_TURNON, 0);
						//printf("[panic] wakeup siren 4.0 sec\n");
						//mSecSleep(5000);
					}

					if(devlist[x].model == TYPE_DOORCHIME)
					{
					    play_doorchime(&devlist[x], DEF_ALARM_51_SONG_NO, 1);
					}

					usleep(1000);
				}
			}
		}
	}
	sendCmdtoClient(CM_PANIC, 0, 0,  0,NULL);
	changeSirenState(1);

	//push
	char msg[2046];
#ifdef DEF_FEATURE_MULTI_PUSH
    char msg2[256];
    get_push_string(msg2, STR_ID_PANIC);
	sprintf( msg, msg2, g_setting.gwprop.name);
#else
	sprintf(msg, "The Panic alarm has been triggered for your SHC_pro system %s",gDID );
#endif
	if (strlen(msg) != 0)
		pushnotification(NT_PANIC, msg, STR_ID_PANIC);//

	//newEvent(g_DID, TYPE_GATEWAY, GW_PANIC, 0);
	int n = g_setting.gwprop.ledon;
	if (n == 1)
		redled(0);
	else if (n == 2)
		redled(1);

	//can trigger alarm again
	//g_sirenstate = 0;
	//g_smokeOn = 0;

	pushnotification2("{\"on\":true, \"sat\":254, \"bri\":254,\"hue\":65535, \"alert\":\"lselect\"}");
}



void changeSirenState(int ison)
{
	if (ison ==1)
	{
		if (g_sirenstate ==0)
		{
			g_sirenstate = 1;
			printf("Trigger Alarm\n");
//			sendCmdtoClient(CM_SIREN_ON, 0, 0,  0,NULL);
			sendCmdtoClient(CM_SIREN_ON, 0, 1,	sizeof(jsw_alarm_event),&last_trigger);
		//printf("id=%d , time=%d , type=%d , status=%d\n",last_trigger.id,last_trigger.time,last_trigger.type,last_trigger.status);


		}

	}else
	{
		if (g_sirenstate ==1)
		{
			g_sirenstate = 0;
			printf("stop Alarm\n");
			sendCmdtoClient(CM_SIREN_OFF, 0, 0,  0,NULL);
		}
	}

}

void changeSirenState_inScenario(int ison)
{
	if (ison ==1)
	{
		if (g_sirenstate ==0)
		{
			g_sirenstate = 1;
//			printf("Trigger Alarm\n");
//			sendCmdtoClient(CM_SIREN_ON, 0, 0,  0,NULL);

		}

	}else
	{
		if (g_sirenstate ==1)
		{
			g_sirenstate = 0;
//			printf("stop Alarm\n");
//			sendCmdtoClient(CM_SIREN_OFF, 0, 0,  0,NULL);
		}
	}

}



void sendTamperMsg(unsigned int did)
{
	//item.name .. " at your " .. item.seat .. " has been tampered."
	char msg[1024];
	jswdev* dev;
	memset(msg, 0, sizeof(msg));

	dev = getDevbydid(did);
	if (dev == NULL)
	{
        SetRunningLog_str_int("Error: sendTamperMsg() no device found", did);
		return;
	}

#ifdef DEF_FEATURE_MULTI_PUSH
    char msg2[256];
    get_push_string(msg2, STR_ID_TAMPER);
	sprintf( msg, msg2, dev->name, dev->location, g_setting.gwprop.name);
#else
	sprintf(msg, STRING_TRIGGER_TAMPER_ALARM, dev->name, dev->location, gDID );
#endif

	 if (strlen(msg) != 0)
		pushnotification(NT_TAMPER, msg, STR_ID_TAMPER);
}


void *startgatewayalarm_thread(void *ptr )
{
	char cmd[1024];
	sprintf(cmd, "/usr/sbin/buzzer_alarm %d&", g_setting.gwprop.duration);
// Jeff remark if g_setting.gwprop.alarmOn = MUTE , keep BLUE LED
#if 0
	if (g_setting.gwprop.alarmOn == 0)
	{
		pthread_exit(0);
		return;
	}
#endif
	//system("killall -9 buzzer_beep");
	//usleep(100);
	//system(cmd); //"/usr/sbin/buzzer_alarm 180&");
	play_beep_alarm();
	usleep(100);
	pthread_exit(0);
}


void *stopgatewayalarm_thread(void *ptr )
{
#if 0
ndef MELODY_IC
	remove("/tmp/jsw_alarm");
	sync();
	system("rm /tmp/jsw_alarm");
	sync();
	usleep(2000);
#endif
	//if (g_setting.gwprop.alarmOn == 1)
	if( (g_sirenstate == 1) && (g_setting.gwprop.alarmOn > 0) )
	{
		//system("/usr/sbin/buzzer_beep 300 500 0 1&");
		stop_beep_alarm();
	}

		//rm /tmp/jsw_alarm && sync && usleep 100000
	//sh ${script_path}/jsw_beep.sh 300 500 0 1&

	usleep(100);

	pthread_exit(0);
}

int checkvalidflag(unsigned short flag)
{
	//printf("checkvalidflag , arm state %d\n", g_armstate);

	if (g_armstate == st_arm)//arm
	{
		if ( ( flag & 0x01) == 0x01 )
			return 1;
		else
			return 0;
	}else if (g_armstate == st_partarm)//part arm
	{
		if ( ( flag & 0x02) == 0x02 )
			return 1;
		else
			return 0;
	}

	return 0;
}

void triggerAlarm(int mode, int is_smoke) //mode: 0 = normal trigger, 1 = tamper; is_smoke: 1 = smoke, 0 = else
{
	printf("triggerAlarm(%d,%d)\n",mode,is_smoke);
	int x;

	if (g_armstate == st_maintain )
		return;

	if( (mode == 1) && (g_armstate == st_disarm) )
	{//tamper, switch to arm mode first
		g_armstate = st_arm;
		exitdelayTarget = st_arm;
		systemArm2();
	}
#ifndef MELODY_IC
	if (g_sirenstate ==0)
	{
		//system(". jsw_alarm.sh 180&");
		// Jeff remark if g_setting.gwprop.alarmOn = MUTE , keep BLUE LED
		//if (g_setting.gwprop.alarmOn > 0)
			pthread_create(&alarm_th, NULL, startgatewayalarm_thread, 0);


		if (siren_t != 0)//only do this the first time.
		{
			makeTimer( &siren_t, 0, 0);
			siren_t = 0;
		}
		printf("set siren time out , value, %d\n", g_setting.gwprop.duration - 5);
		if( g_setting.gwprop.duration > 6)
			makeTimer( &siren_t,( g_setting.gwprop.duration - 5), 0);
		else
			makeTimer( &siren_t, g_setting.gwprop.duration, 0);

		//makeTimer( &siren_t,3, 0);

	}
#else
#ifdef SIREN_TIMER
	if (siren_current_t != 0)//only do this the first time.
	{
		//makeTimer( &siren_t, 0, 0);
		killTimer(siren_current_t);
		siren_current_t = siren_t = 0;
	}
	if( g_setting.gwprop.duration > 0 && g_setting.gwprop.duration <= 180)
		makeTimer( &siren_t, g_setting.gwprop.duration , 0);
	else
		makeTimer( &siren_t, 180, 0);
	siren_current_t = siren_t;
#endif
	play_beep_alarm();
#endif
//	changeSirenState(1); // jeff move to end of function
	g_ShowAlarmView = 1;//APP show alarm view
	if(is_smoke == 1)
	{
        g_AlarmState = ALARM_STATE_24H_ALARM;
	}else
	{
        if(mode == 0)
            g_AlarmState = ALARM_STATE_TRIGGER;
        else
            g_AlarmState = ALARM_STATE_TAMPER;
	}
	savesetting4();

	//pushnotification2("{\"on\":true, \"sat\":254, \"bri\":254,\"hue\":65535, \"alert\":\"lselect\"}");

	for ( x=0;x<devcount;x++)
	{

		if (devlist[x].model == TYPE_SIREN_INDOOR ||  TYPE_SIREN_OUTDOOR == devlist[x].model )
		{
			printf("===== setting siren on devlist[%08X]\n",devlist[x].did);
			if ( ( checkvalidflag(devlist[x].flag) ==1) || ((mode == 1) && (g_armstate == st_disarm)) )
			{
				writeCommand( devlist[x].did, devlist[x].model, CMD_SIREN_TURNON);
				newEvent(devlist[x].did,devlist[x].model, CMD_SIREN_TURNON, 0);
				//printf("Siren wakeup for 4.0 sec\n");
				//mSecSleep(2000);
			}
			else
				{
				if(g_AlarmState == ALARM_STATE_24H_ALARM || g_AlarmState == ALARM_STATE_TAMPER)
					{
					writeCommand( devlist[x].did, devlist[x].model, CMD_SIREN_TURNON);
					newEvent(devlist[x].did,devlist[x].model, CMD_SIREN_TURNON, 0);
					}

				}

		}
		else if (devlist[x].model == TYPE_POWER_ADAPTER  )
		{
			if ( checkvalidflag(devlist[x].flag) ==1)
			{
				writeCommand( devlist[x].did, devlist[x].model, CMD_ADAPTER_TURNON);
				newEvent(devlist[x].did,devlist[x].model, CMD_ADAPTER_TURNON, 0);
				//printf("power adapter for 1.2 sec\n");
				//mSecSleep(800);
			}
		}else if ( devlist[x].model == TYPE_CAMERA  )
		{

			if ( checkvalidflag(devlist[x].flag) ==1 )
				doRec(devlist[x].did);
			else
            {
				if(g_AlarmState == ALARM_STATE_24H_ALARM || ALARM_STATE_TAMPER)
					doRec(devlist[x].did);
			}
		}else if(devlist[x].model == TYPE_DOORCHIME)
        {
            if ( checkvalidflag(devlist[x].flag) ==1 )
                play_doorchime(&devlist[x], DEF_ALARM_51_SONG_NO, 1);
		}
	}

	changeSirenState(1);


	int n = g_setting.gwprop.ledon;
	if (n == 1)
		redled(0);
	else if (n == 2)
		redled(1);

}

void Force_stopAlarm()
{
	int x;
	//if (g_sirenstate ==1)
	//{
		//g_sirenstate = 0;
		//system(". jsw_disarm.sh&");
	//pthread_create(&alarmstop_th, NULL, stopgatewayalarm_thread, 0);
	//pthread_join(alarmstop_th, NULL);
	//}
#ifdef MELODY_IC
//	stop_beep_alarm();
#endif
	changeSirenState(0);
	for ( x=0;x<devcount;x++)
	{

		if (devlist[x].model == TYPE_SIREN_INDOOR || TYPE_SIREN_OUTDOOR == devlist[x].model)
		{
		//	printf("===== setting siren off \n");
			writeCommand( devlist[x].did, devlist[x].model, CMD_SIREN_TURNOFF);
			usleep(200*1000);
			if(TYPE_SIREN_OUTDOOR == devlist[x].model)
			{
                if(devlist[x].status == CMD_SIREN_TURNON)
                {
                    newEvent(devlist[x].did, devlist[x].model, CMD_SIREN_TURNOFF, 0);
					devlist[x].status = CMD_SIREN_TURNOFF;
					savedata();
		        }
			}
		}

	}

}


void stopAlarm()
{
	int x;
	//if (g_sirenstate ==1)
	//{
		//g_sirenstate = 0;
		//system(". jsw_disarm.sh&");
	//pthread_create(&alarmstop_th, NULL, stopgatewayalarm_thread, 0);
	//pthread_join(alarmstop_th, NULL);
	//}
#ifdef MELODY_IC
//	stop_beep_alarm();
#endif
	changeSirenState(0);
	for ( x=0;x<devcount;x++)
	{

		if (devlist[x].model == TYPE_SIREN_INDOOR || TYPE_SIREN_OUTDOOR == devlist[x].model)
		{
            if(devlist[x].status == RE_SIREN_ISON)
            {
            	writeCommand( devlist[x].did, devlist[x].model, CMD_SIREN_TURNOFF);
				usleep(200*1000);
				devlist[x].status = RE_SIREN_ISOFF;
                newEvent(devlist[x].did, devlist[x].model, RE_SIREN_ISOFF, 0);
				savedata();
	        }
		}

	}

}
// 0: OFF 1: ON 3. flash
void set_SHC_PRO_alarm_LED(int state)
{
	printf("set_SHC_PRO_alarm_LED[%d]\n",state);
	switch(state)
		{
		case 0:
			McuGpio2(0xD3, 0, 0, 0, 0, 0);
		break;
		case 1:
			McuGpio2(0xD3, 1, 0, 0, 0, 0);
		break;
		case 2:
			McuGpio2(0xD3, 3, 60, 0, 0, 0);
		break;
		default:
			McuGpio2(0xD3, 0, 0, 0, 0, 0);
		break;
		}
}

void set_alarm_LED()
{//set LED by sensor status
	int k;
	if( (g_ShowAlarmView == 1) && ((g_armstate == st_arm) || (g_armstate == st_partarm)) )
	{//show alarm LED
		if (g_setting.gwprop.ledon == 1)
			redled(0);
		else if (g_setting.gwprop.ledon == 2)
			redled(1);
		return;
	}
#if 0
	if(g_armstate == st_disarm)
		set_SHC_PRO_alarm_LED(0);
	else
		set_SHC_PRO_alarm_LED(1);
#endif
	for (k=0;k<devcount;k++)
	{
		if( (devlist[k].model == TYPE_MAGNETIC) && (devlist[k].status == RE_MAG_ISON) )
		{//MAG opened
			if( (g_armstate == st_arm) || (g_armstate == st_partarm) )
			{//arm or part arm, show alarm LED
				if (g_setting.gwprop.ledon == 1)
					redled(0);
				else if (g_setting.gwprop.ledon == 2)
					redled(1);
				g_ShowAlarmView = 1;
				g_AlarmState = ALARM_STATE_TRIGGER;
			}else if(g_armstate == st_disarm)
			{//disarm, show yellow LED
				if (g_setting.gwprop.ledon == 1)
					yellowled(0);
				else if (g_setting.gwprop.ledon == 2)
					yellowled(1);
			}
			break;
		}
	}
}


////////////////////////////////////////////
////////////////////////////////////////////

int isSource(short model)
{
	if (model == TYPE_PIR || model == TYPE_MAGNETIC || model == TYPE_VIBRATION || model == TYPE_BUTTON|| model ==TYPE_CAMERA || \
		model == TYPE_SMOKE || model == TYPE_WATERLEVEL || model == TYPE_DOOR_LOCK )
		return 1;

	return 0;
}

int isTarget(short model)
{
	if (model == TYPE_SIREN_INDOOR || model == TYPE_SIREN_OUTDOOR || model == TYPE_DOORCHIME|| model ==TYPE_POWER_ADAPTER || model == TYPE_CAMERA )
		return 1;

	return 0;
}

//kill the scenario not in devlist
void sortoutScenario()
{
    int i, j;
    system("rm /mnt/scenario.db.sort");
    usleep(200*1000);
    system("cp /mnt/scenario.db /mnt/scenario.db.sort");
    for(i=0;i<SCENARIOSIZE;i++)
    {
        if(scenariolist[i].src.did == SCENARIO_GATEWAY_DID)
			continue;
        if(isDevRegister(scenariolist[i].src.did) == 0)
        {//not register, reset
            memset(&scenariolist[i], 0, sizeof(scenariolist[i]));
        }
    }
    for(j=0;j<SCENARIOSIZE;j++)
    {
        for(i=0;i<(SCENARIOSIZE-1);i++)
        {
            if(scenariolist[i].src.did == 0)
            {
                memcpy(&scenariolist[i], &scenariolist[i+1], sizeof(scenariolist[i]));
                memset(&scenariolist[i+1], 0, sizeof(scenariolist[i+1]));
            }
        }
    }
    srcCount = SCENARIOSIZE;
    for(i=0;i<SCENARIOSIZE;i++)
    {
        if(scenariolist[i].src.did == 0)
        {
            srcCount = i;
            break;
        }
    }
}

void addsource(unsigned int did)
{
	int x =0;
	for (x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did == did) //shouldnt exist but still,
		{
            SetRunningLog_str_int("Error: addsource() reject by did exist already", did);
			return;
		}
	}
	if(srcCount >= SCENARIOSIZE)
	{
		printf("Scenario buffer overflow!!\n");
		//kill none use scenario
		sortoutScenario();
		if(srcCount >= SCENARIOSIZE)
		{
			srcCount = SCENARIOSIZE-1;
		}
	}
	memset(&scenariolist[srcCount], 0, sizeof(jswscenario) );
	scenariolist[srcCount].src.did = did;
	srcCount ++;

    //for MAG has two scenarios
	jswdev* dev;
	dev = getDevbydid(did);
	if (dev != NULL)
	{
	    if(dev->model == TYPE_MAGNETIC)
	    {//is MAG
            memset(&scenariolist[srcCount], 0, sizeof(jswscenario) );
            scenariolist[srcCount].src.did = did;
            scenariolist[srcCount].src.onoff = 1;
            srcCount ++;
	    }
        else if(dev->model == TYPE_BUTTON)
	    {//is Button
            memset(&scenariolist[srcCount], 0, sizeof(jswscenario) );
            scenariolist[srcCount].src.did = did;
            scenariolist[srcCount].src.onoff = 1;
            scenariolist[srcCount].src.onoff = SCENARIO_ONOFF_BUTTON_LONG_PRESS;
            srcCount ++;
	    }
}

	//printf("save new scenario source, current srcCount %d\n", srcCount);
	saveScenario();
}

void client_setlink(int count, char *buf)
{
	jswlink *link;
	jswlinktarget *target;
	char *p;
	int x=0;
	int y=0;

	if (count <= 0)
	{
		//printf("set link count = 0\n");
		return;
	}

	link = (jswlink*)buf;
	p = buf;
	p += sizeof(jswlink);
	target = (jswlinktarget*)p;

	//printf("client_setlink from app did %d, target count = %d, did %d enable %d\n",link->did, link->target_num, target->did, target->enable);

    jswdev* dev = NULL;
    signed char lowb1 = 0, lowb2 = 0;
	for ( x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did != link->did )
		 continue;
		//printf("found \n");

        //for MAG has two scenarios
        if(link->did == SCENARIO_GATEWAY_DID)
        {//sunrise/sunset
            lowb1 = (scenariolist[x].src.onoff & 0x00FF);
            lowb2 = (link->onoff & 0x00FF);
            if(lowb1 != lowb2)
                continue;
        }else
        {//sensors
            dev = getDevbydid(link->did);
            if(dev != NULL)
            {//found device
                if( (dev->model == TYPE_MAGNETIC) || (dev->model == TYPE_DOOR_LOCK) || (dev->model == TYPE_BUTTON) )
                {//is MAG, Doorlock or Button, not only one scenario
                    lowb1 = (scenariolist[x].src.onoff & 0x00FF);
                    lowb2 = (link->onoff & 0x00FF);
                    if(lowb1 != lowb2)
                        continue;
                }
            }else
            {//no device found
                continue;
            }
        }

		scenariolist[x].src.target_num = link->target_num;
		if ( scenariolist[x].src.target_num > SCENARIO_MAX_TARGET )
			scenariolist[x].src.target_num = SCENARIO_MAX_TARGET;

		scenariolist[x].src.enable = link->enable;
		scenariolist[x].src.onoff = link->onoff;

		if (link->target_num ==0)
			scenariolist[x].src.enable = 0;


		if ( link->target_num > 0)
		{
			for (y=0;y< scenariolist[x].src.target_num;y++)
			{
				target = (jswlinktarget*)p;

				scenariolist[x].target[y].did = target->did;
				scenariolist[x].target[y].enable = target->enable;

				//printf("client_setlink target %d,  did = %d, enable =%d\n",y, target->did, target->enable);
				p += sizeof(jswlinktarget);
			}
		}

		saveScenario();
		break;
	}

}
void set_linkmenu(char *buf)
{
	jswlink *link;
	char *p;
	int x=0;
	int y=0;

	link = (jswlink*)buf;
	p = buf;

    jswdev *dev = NULL;
    signed char lowb1 = 0, lowb2 = 0;
	for ( x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did != link->did )
		 continue;

		//printf("##set link menu found enable = %d , onoff = %d target num  %d\n",link->enable,link->onoff, link->target_num);

        //for MAG has two scenarios
        if(link->did == SCENARIO_GATEWAY_DID)
        {//sunrise/sunset
            lowb1 = (scenariolist[x].src.onoff & 0x00FF);
            lowb2 = (link->onoff & 0x00FF);
            if(lowb1 != lowb2)
                continue;
        }else
        {//sensors
            dev = getDevbydid(link->did);
            if (dev != NULL)
            {//found device
                if( (dev->model == TYPE_MAGNETIC) || (dev->model == TYPE_DOOR_LOCK) || (dev->model == TYPE_BUTTON) )
                {//is MAG, Doorlock or Button, not only one scenario
                    lowb1 = (scenariolist[x].src.onoff & 0x00FF);
                    lowb2 = (link->onoff & 0x00FF);
                    if(lowb1 != lowb2)
                        continue;
                }
            }else
            {//no device found
                continue;
            }
        }

		if (scenariolist[x].src.target_num != link->target_num)
		{
			//printf("##set link menu target num not equal old %d new %d\n",scenariolist[x].src.target_num,link->target_num);
			return;
		}

        //	printf("##set link menu found enable = %d , onoff = %d\n",link->enable,link->onoff);
		scenariolist[x].src.enable = link->enable;
		scenariolist[x].src.onoff = link->onoff;

		saveScenario();
		break;
	}
}


void client_linkmenu(int session)
{
	static char out[1400];
	char *p;
	int x=0;
	p = out;

	if (srcCount == 0)
	{
	//	printf("link source count = 0 \n");
		sendCmdtoClient2(CM_GETLINKMENU, 0, 0,  0,NULL, session);
		return;
	}
	memset(out, 0, sizeof(out));
	for ( x=0;x<srcCount;x++)
	{
		memcpy(p, &scenariolist[x].src, sizeof(jswlink));
		//printf("##get link menu  enable = %d , onoff = %d\n",scenariolist[x].src.enable, scenariolist[x].src.onoff);
		p += sizeof(jswlink);
	}
	//printf("return link menu %d", srcCount);
	sendCmdtoClient2(CM_GETLINKMENU, 0, srcCount,  srcCount * sizeof(jswlink) ,out, session);
}

void client_sourcelink(char* id, int payload_length, int session)
{
	static char out[1400];
	char *p;
	int x=0;
	int len;
	p = out;

	unsigned int cid;
	memcpy(&cid, id, sizeof(int));
	unsigned short onoff = 0;
    if(payload_length == 6)
    {//new, MAG has two scenario
        memcpy(&onoff, &id[4], sizeof(unsigned short));
    }else
    {//old, MAG has one scenario only
        onoff = 1;
    }

//printf("CM_GETSOURCELINK from client did %d \n", cid );
	memset(out, 0, sizeof(out));
	jswdev *dev = NULL;
    signed char lowb1 = 0, lowb2 = 0;
	for ( x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did != cid )
			continue;

        if(cid == SCENARIO_GATEWAY_DID)
        {//sunrise/sunset
            lowb1 = (scenariolist[x].src.onoff & 0x00FF);
            lowb2 = (onoff & 0x00FF);
            if(lowb1 != lowb2)
                continue;
        }else
        {//sensors
            dev = getDevbydid(cid);
            if(dev)
            {//found device
                if( (dev->model == TYPE_MAGNETIC) || (dev->model == TYPE_DOOR_LOCK) || (dev->model == TYPE_BUTTON) )
                {//MAG, Doorlock or Button, not only one scenario
                    lowb1 = (scenariolist[x].src.onoff & 0x00FF);
                    lowb2 = (onoff & 0x00FF);
                    if(lowb1 != lowb2)
                        continue;
                }
            }else
            {// no device found
                continue;
            }
        }

		memcpy(p, &scenariolist[x].src, sizeof(jswlink));
		p += sizeof(jswlink);

		if (scenariolist[x].src.target_num > 0 )
			memcpy(p, &scenariolist[x].target, scenariolist[x].src.target_num * sizeof(jswlinktarget) );

		len = sizeof(jswlink) +  (scenariolist[x].src.target_num * sizeof(jswlinktarget));

		//printf("found Target num %d\n",scenariolist[x].src.target_num );
//printf("CM_GETSOURCELINK target count = %d\n", id, scenariolist[x].src.target_num);
		sendCmdtoClient2(CM_GETSOURCELINK, 0, 1,  len ,out, session);
		return;
	}

	//not found, try to add first
	addsource(cid);
	if( (srcCount > 0) && (srcCount <= SCENARIOSIZE) )
	{
		if(scenariolist[srcCount-1].src.did == cid)
		{
			len = sizeof(scenariolist[srcCount-1]);
			memcpy(out, &scenariolist[srcCount-1], sizeof(scenariolist[srcCount-1]));
            sendCmdtoClient2(CM_GETSOURCELINK, 0, 1,  len ,out, session);
			return;
		}
	}

//	printf("get source link empty\n");
	sendCmdtoClient2(CM_GETSOURCELINK, 0, 0,  0 ,NULL, session);
}



void setarmtable(int count, char* in)
{
	jswarmtable *tbl;
	int x;
	int y;

	tbl = (jswarmtable*)in;
	for (x=0;x<count;x++)
	{
		for (y=0;y<devcount;y++)
		{
			if (devlist[y].did == tbl->did )
			{
				devlist[y].flag = tbl->flag;
				//printf("set armtable did found %d \n", tbl->did);
				savedata();
				break;
			}
		}
		tbl++;
	}

}


//////////////////////////////////////
///scenario !!!!///////////

void startScenario(unsigned int  id, char status)
{
	int x =0;
	int y=0;
	short state;
	jswdev* dev;
	jswdev* tg;
	int onoff;
	int sirenOn = 0;
	int doorchime_track = 0x00;

	// Jeff define reject Scenario check in NOT DISARM
	if (g_armstate != st_disarm )
	{//arm or part-arm
	    if(strstr(gDID, "WGNF-") || strstr(gDID, "WGMT-"))
	    {//allow scenario
	    }else
	    {//not allow scenario
            return;
	    }
	}
	if (srcCount == 0 )
		return;

	for (x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did != id )
			continue;
		if (scenariolist[x].src.enable ==0)//found device, but not enable.
		{
		    if(scenariolist[x].src.did == SCENARIO_GATEWAY_DID) //GW has two scenarios
                continue; //return;
            else
            {//sensor
                dev = getDevbydid(id);
                if (dev == NULL)
                    return;
                else
                {
                    if( (dev->model != TYPE_MAGNETIC) && (dev->model != TYPE_DOOR_LOCK) && (dev->model != TYPE_BUTTON) && (dev->model != TYPE_CAMERA) )
                        return;
                    else //MAG and Doorlock has two or more scenarios
                        continue;
                }
            }
		}
		if (scenariolist[x].src.target_num ==0)//no target setup yet.
			return;
		state = (short)status;

        if(scenariolist[x].src.did == SCENARIO_GATEWAY_DID)
        {//gateway
            signed char highb = 0;
            signed char lowb = 0;
            highb = scenariolist[x].src.onoff >> 8;
            lowb = scenariolist[x].src.onoff & 0x00FF;
            if(lowb != state)
                continue;
        }else
        {//sensor
            dev = getDevbydid(id);
            if (dev ==NULL)
            {
                printf("source dev cannot be found in devlist, scenario scene did %d\n", id);
                SetRunningLog_str_int("Error: startScenario() did not found", id);
                return;
            }
            //printf("Start scenario routine, model= %d, id=%d\n", dev->model, id );
            if (dev->model == TYPE_MAGNETIC)
            {// two state, check onoff.
                //if (state != scenariolist[x].src.onoff )
                if ( scenariolist[x].src.onoff == 0 && state ==RE_MAG_ISON )
                {
                    printf("bypass ,scenario source onoff state is diff with device state\n");
                    continue;
                }
                if ( scenariolist[x].src.onoff == 1 && state ==RE_MAG_ISOFF )
                {
                    printf("bypass ,scenario source onoff state is diff with device state\n");
                    continue;
                }
            }
			else if (dev->model == TYPE_BUTTON)
            {// two state, check press / long press.
                if ( scenariolist[x].src.onoff == SCENARIO_ONOFF_BUTTON_SHORT_PRESS && state ==RE_BUTTON_LONG_PRESS)
                {
                    printf("bypass ,scenario source Button state is diff with device state\n");
                    continue;
                }
                if ( scenariolist[x].src.onoff == SCENARIO_ONOFF_BUTTON_LONG_PRESS && state ==RE_BUTTON_PRESS)
                {
                    printf("bypass ,scenario source Button state is diff with device state\n");
                    continue;
                }
            }
        }

        //target
		for (y=0;y<scenariolist[x].src.target_num;y++)
		{//expect it to be siren or onoff.
		    if(scenariolist[x].target[y].did == SCENARIO_GATEWAY_DID)
		    {
		        if(scenariolist[x].target[y].enable & SCENARIO_GATEWAY_DOORBELL)
		        {//doorbell
                    if(g_armstate == st_disarm)
                        call_buzzer_doorbell();
		        }
                continue;
		    }

			tg =  getDevbydid(scenariolist[x].target[y].did);

			if (tg ==NULL)
			{
				printf("target dev cannot be found in devlist, scenario\n");
                SetRunningLog_str_int("Error: startScenario() did not found", scenariolist[x].target[y].did);
				return;
			}

			if (tg->model == TYPE_CAMERA )
			{ //record_and_preview.
                //for get camera recording or not
                //if(getCameraRecording(scenariolist[x].target[y].did) <= 0)
                {//camera is not recording
                    printf("scenariolist[x].target[y].enable=%d\n", scenariolist[x].target[y].enable);
                    if(scenariolist[x].target[y].enable & SCENARIO_CAMERA_RECORD)
                    {
                        int index = -1;
                        if(scenariolist[x].target[y].enable & SCENARIO_CAMERA_PRESET_1)
                        {//face to preset 1
                            index = 0;
                        }else if(scenariolist[x].target[y].enable & SCENARIO_CAMERA_PRESET_2)
                        {//face to preset 2
                            index = 1;
                        }else if(scenariolist[x].target[y].enable & SCENARIO_CAMERA_PRESET_3)
                        {//face to preset 3
                            index = 2;
                        }
                        if(index >= 0)
                        {
                            cameraGotoPTPoint2(scenariolist[x].target[y].did, index);
                            usleep(100*1000);//3*1000*1000
                        }
                        doRec( scenariolist[x].target[y].did );
                    }
                }
			}else
			{
				//other device
				if ( (scenariolist[x].target[y].enable & 0x01) == 0x01 )//need to be on.
					onoff = 1;
				else
					onoff = 0;
				if (tg->model == TYPE_DOORCHIME)
				{
					if ( (scenariolist[x].target[y].enable & 0xff) == 0x00 )//need to be on.
						 continue;
					else
						doorchime_track = scenariolist[x].target[y].enable ;
				}

				if (tg->model == TYPE_SIREN_INDOOR || tg->model == TYPE_SIREN_OUTDOOR )
				{
					if (g_armstate != st_maintain )
					{
						//printf("siren activate at scenario\n");
						//changeSirenState(onoff);
						// Jeff Mark changeSirenState_inScenario @ 21070906
						//changeSirenState_inScenario(onoff);
						if (onoff == 0)
						{
							writeCommand( tg->did, tg->model, CMD_SIREN_TURNOFF);
						//	newEvent(tg->did, tg->model, CMD_SIREN_TURNOFF, 0);
						}
						else
						{
							writeCommand( tg->did, tg->model, CMD_SIREN_TURNON);
							newEvent(tg->did, tg->model, CMD_SIREN_TURNON, 0);
						}
					}
				}else if (tg->model == TYPE_POWER_ADAPTER)
				{
					//schedule .
					int ret ;
					if (onoff == 0)
					{
						ret = writeCommand( tg->did, tg->model, CMD_ADAPTER_TURNOFF);
						if (ret ==1)
							newEvent(tg->did, tg->model, CMD_ADAPTER_TURNOFF, 0);
					}
					else
					{
						ret = writeCommand( tg->did, tg->model, CMD_ADAPTER_TURNON);
						if (ret ==1)
							newEvent(tg->did, tg->model, CMD_ADAPTER_TURNON, 0);
						paAutoOff(tg->did);
					}
				}
				else if (tg->model == TYPE_DOORCHIME)
				{
					printf("doorchime_track= [%d]\n",doorchime_track);
				    play_doorchime(tg, doorchime_track, 1);
/*
					int ret ;
					unsigned char doorchime_cmd = 0x01;
					if(doorchime_track == 0x00)
						doorchime_cmd = 0x00;
					else if(doorchime_track <= DEF_DOORCHIME_SONG_MAX)
						{
						doorchime_track--;
						doorchime_cmd = ((doorchime_track << 4) & 0xf0) | 0x01;
						}
					ret = writeCommand( tg->did, tg->model, doorchime_cmd);

					printf("doorchime_CMD= [%02X]\n",doorchime_track);
					if (ret ==1)
						newEvent(tg->did, tg->model, 0x01, 0);
*/
				}
			}
		}
	}
}

int check_sensor_with_Scenario(unsigned int  id, char status,char device_type)
{
	printf("check_sensor_with_Scenario(unsigned int  id, char status)\n");
	int x =0;
	jswdev* dev;
	if (srcCount == 0 )
		return 0;
	if(g_armstate != st_disarm)
		return 0;

	for (x=0;x<srcCount;x++)
	{
		if (scenariolist[x].src.did == id )
			{
			if (scenariolist[x].src.enable == 1)//found device, but not enable.
	        {//sensor
	            dev = getDevbydid(id);
	            if (dev == NULL)
	                return 0;
	            else
	            {
	                if(device_type == TYPE_PIR && dev->model == TYPE_PIR && scenariolist[x].src.target_num > 0)
	                	{
	                		printf("TYPE_PIRscenariolist[x].src.target_num = %d\n");
	                    return 1;
	                	}
					if (device_type == TYPE_MAGNETIC && dev->model == TYPE_MAGNETIC && scenariolist[x].src.target_num > 0)
					{// two state, check onoff.
						if ( scenariolist[x].src.onoff == 1 && status ==RE_MAG_ISON )
						{

							printf("TYPE_MAGNETIC RE_MAG_ISON scenariolist[x].src.target_num = %d\n");
							return 1;
						}
						if ( scenariolist[x].src.onoff == 0 && status ==RE_MAG_ISOFF )
						{

							printf("TYPE_MAGNETIC RE_MAG_ISOFF  scenariolist[x].src.target_num = %d\n");
							return 1;
						}
					}

	            }
        }
			}
	}
	return 0;
}


//kill the schedule not in devlist
void sortoutSchedule()
{
    int i, j;
    system("rm /mnt/pa.db.sort;sync");
 //   usleep(200*1000);
    system("cp /mnt/pa.db /mnt/pa.db.sort;sync");
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if(isDevRegister(pa_proplist[i].did) == 0)
        {//not register, reset
            memset(&pa_proplist[i], 0, sizeof(pa_proplist[i]));
        }
    }
    for(j=0;j<MAXDEVICENUM;j++)
    {
        for(i=0;i<(MAXDEVICENUM-1);i++)
        {
            if(pa_proplist[i].did == 0)
            {
                memcpy(&pa_proplist[i], &pa_proplist[i+1], sizeof(pa_proplist[i]));
                memset(&pa_proplist[i+1], 0, sizeof(pa_proplist[i+1]));
            }
        }
    }
    paCount = MAXDEVICENUM;
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if(pa_proplist[i].did == 0)
        {
            paCount = i;
            break;
        }
    }
}

////power adaptor
//struct pa_prop{
//	unsigned int did;
//	timer_t timerid;
//	short elapse;
//	short schedule_count;
//	jswschedule schedlist[16];
//};
void addPAproperties(unsigned int id)
{
	int x=0;

//	if (paCount == MAXDEVICENUM)
//	{
//		printf("max pa in proplist\n");
//		return;
//	}

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did == id )
		{
			printf("adding a pa already exist\n");
            SetRunningLog_str_int("Error: addPAproperties() did exist already", id);
			return;
		}
	}
//all other member is zero.
	if(paCount >= MAXDEVICENUM)
	{
		printf("Schedule buffer overflow!!\n");
		//kill none use schedule
		sortoutSchedule();
		if(paCount >= MAXDEVICENUM)
		{
			paCount = MAXDEVICENUM-1;
		}
	}
	memset(&pa_proplist[paCount], 0, sizeof(pa_proplist[paCount]));
	pa_proplist[paCount].did = id;
	paCount++;

	printf("add property pa in proplist %d\n", paCount);
	savePAprop();

}

void dopaAutooff(timer_t id)
{
	int x;
	jswdev *dev;

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].timerid != id )
			continue;
		dev = getDevbydid(pa_proplist[x].did);
		if (dev == NULL)
			return;
#if 0
// Jeff mark for NoSignal retry
		if (dev->status ==RE_ABUS_AUTOREPORT)
			return;
#endif
		printf("timer do pa autooff timer id %d\n", id);

		int ret = writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNOFF);
		if (ret ==1)
			newEvent(dev->did, dev->model, CMD_ADAPTER_TURNOFF, 0);
		pa_proplist[x].timerid = 0;
		break;
	}

}

void paAutoOff(unsigned int did)
{
	int x;
	jswdev *dev;

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did != did )
			continue;
		dev = getDevbydid(pa_proplist[x].did);
		if (dev == NULL)
			return;
#if 0
		// Jeff mark for NoSignal retry

		if (dev->status ==RE_ABUS_AUTOREPORT)
			return;
#endif
		if (pa_proplist[x].duration > 0)
		{
			if (pa_proplist[x].timerid != 0)
				continue;

			makeTimer(&pa_proplist[x].timerid, pa_proplist[x].duration , 0);

			printf("set pa num %d autooff sec%d, timer id %d\n", did, pa_proplist[x].duration, pa_proplist[x].timerid);
			//pa_proplist[x].timerid = paoff_t;
		}
		break;
	}
}

void setPAofftime( char*buf )
{
	unsigned int id;
	int offtime;
	int x;
	memcpy(&id, buf, sizeof(int));
	memcpy(&offtime, buf+4, sizeof(int));

	printf("set pa offtime , did %u, time %d\n", id, offtime);

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did != id )
			continue;

		pa_proplist[x].duration = offtime;
		savePAprop();
		break;
	}
}

void getPAofftime(char *buf )
{
	unsigned int id, duration;
	int x;
	memcpy(&id, buf, sizeof(int));
	char outbuf[32];
	char *p;

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did != id )
			continue;
		memcpy(outbuf, &id, sizeof(int));
		p = outbuf;
		p += sizeof(int);
		duration = pa_proplist[x].duration;
		memcpy(p, &duration, sizeof(int));
		sendCmdtoClient(CM_PAGETAUTOOFF, 0, 1,  sizeof(int) *2 , outbuf);

		printf("get pa offtime , did %u, time %d\n", pa_proplist[x].did , pa_proplist[x].duration);

		break;
	}
}


void setSiren(char *buf, int mode)
{
	unsigned int id;
	int x;
	jswdev *dev;
	memcpy(&id, buf, sizeof(int));
	//printf("set siren on id %d\n", id);

	dev = getDevbydid(id);
	if (dev == NULL)
	{
		printf("siren id not found\n");
        SetRunningLog_str_int("Error: setSiren() did not found", id);
		return;
	}
	 //TYPE_SIREN_INDOOR ||  TYPE_SIREN_OUTDOOR
	if (dev->model != TYPE_SIREN_INDOOR && dev->model != TYPE_SIREN_OUTDOOR)
	{
		printf("set siren model not correct\n");
        SetRunningLog_str_int("Error: setSiren() invalid model", dev->model);
		return;
	}

	if (mode == 0)
	{
		writeCommand( dev->did, dev->model, CMD_SIREN_TURNOFF);
		dev->status = CMD_SIREN_TURNOFF;
	}
	else
	{
		writeCommand( dev->did, dev->model, CMD_SIREN_TURNON);
		dev->status = CMD_SIREN_TURNON;
	}
//need to check for ack here.
	newEvent(dev->did, dev->model, dev->status, 0);

}

void setPAon(char *buf, int mode)
{
	unsigned int id;
	int x;
	jswdev *dev;
	memcpy(&id, buf, sizeof(int));
	//printf("set pa on id %d\n", id);

	dev = getDevbydid(id);
	
	if (dev == NULL)
	{
		printf("id not found\n");
        SetRunningLog_str_int("Error: setPAon() did not found", id);
		return;
	}

	if (dev->model != TYPE_POWER_ADAPTER )
	{
		printf("pa on model not correct\n");
        SetRunningLog_str_int("Error: setPAon() invalid model", dev->model);
		return;
	}


	if (mode == 0)
	{
		writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNOFF);
		dev->status = CMD_ADAPTER_TURNOFF;
	}
	else
	{
		writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
		dev->status = CMD_ADAPTER_TURNON;
	}
	savedata();

	newEvent(dev->did, dev->model, dev->status, 0);

}

//apple 1226
void testSiren(char *buf, int mode)
{
	unsigned int id;
	int x;
	jswdev *dev;
	memcpy(&id, buf, sizeof(int));
	//printf("set siren on id %d\n", id);

	dev = getDevbydid(id);
	if (dev == NULL)
	{
		printf("siren id not found\n");
        SetRunningLog_str_int("Error: setSiren() did not found", id);
		return;
	}
	 //TYPE_SIREN_INDOOR ||  TYPE_SIREN_OUTDOOR
	if (dev->model != TYPE_SIREN_INDOOR && dev->model != TYPE_SIREN_OUTDOOR)
	{
		printf("set siren model not correct\n");
        SetRunningLog_str_int("Error: setSiren() invalid model", dev->model);
		return;
	}

	if (mode == 0)
	{
		writeCommand( dev->did, dev->model, CMD_SIREN_TURNOFF);
		dev->status = CMD_SIREN_TURNOFF;
	}
	else
	{
		writeCommand( dev->did, dev->model, CMD_SIREN_TURNON);
		dev->status = CMD_SIREN_TURNON;
	}

}

//apple 1226
void testPAon(char *buf, int mode)
{
	unsigned int id;
	int x;
	jswdev *dev;
	memcpy(&id, buf, sizeof(int));
	//printf("set pa on id %d\n", id);

	dev = getDevbydid(id);
	
	if (dev == NULL)
	{
		printf("id not found\n");
        SetRunningLog_str_int("Error: setPAon() did not found", id);
		return;
	}

	if (dev->model != TYPE_POWER_ADAPTER )
	{
		printf("pa on model not correct\n");
        SetRunningLog_str_int("Error: setPAon() invalid model", dev->model);
		return;
	}


	if (mode == 0)
	{
		writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNOFF);
		dev->status = CMD_ADAPTER_TURNOFF;
	}
	else
	{
		writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
		dev->status = CMD_ADAPTER_TURNON;
	}
	//savedata();

}


void delsched(char *buf)
{
	jswschedule* sched;
	int x;
	int y;
	int found = 0;
	sched = (jswschedule*)buf;

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did != sched->did )
			continue;

		for (y=0;y<pa_proplist[x].schedule_count; y++)
		{//update existing
			if (pa_proplist[x].schedlist[y].num != sched->num )
				continue;

			int cnt = pa_proplist[x].schedule_count;

			if (cnt-1 != y) //not lastitem
			{//move the last item here . to keep array continuous.
				memcpy(&pa_proplist[x].schedlist[y], &pa_proplist[x].schedlist[cnt-1], sizeof(jswschedule));
			}
			pa_proplist[x].schedule_count --;
			savePAprop();
			break;
		}
		break;

	}
}

void setsched(char *buf)
{
	jswschedule* sched;
	int x;
	int y;
	int found = 0;
	sched = (jswschedule*)buf;

	printf("set schedule, pacount %d app did %d, index of schedule %d\n", paCount, sched->did , sched->num);

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did != sched->did )
			continue;

		for (y=0;y<pa_proplist[x].schedule_count; y++)
		{//update existing
			if (pa_proplist[x].schedlist[y].num != sched->num )
				continue;
			found =1;
			printf("update existing PA\n");
			pa_proplist[x].schedlist[y].random = sched->random;
			pa_proplist[x].schedlist[y].week = sched->week;
			pa_proplist[x].schedlist[y].starttime=sched->starttime;
			pa_proplist[x].schedlist[y].endtime=sched->endtime;
			pa_proplist[x].schedlist[y].onoff=sched->onoff;
			break;
		}
		if (found ==0)
		{ //add new
			if ( pa_proplist[x].schedule_count >= SCHEDULE_MAX_TARGET)
				return;
			int cnt = pa_proplist[x].schedule_count;

			printf("Set Schedule create new PA prop list, start %d, end %d, week = %d, ison =%d\n",sched->starttime,sched->endtime,sched->week,sched->onoff);
			pa_proplist[x].schedlist[cnt].random = sched->random;
			pa_proplist[x].schedlist[cnt].week = sched->week;
			pa_proplist[x].schedlist[cnt].starttime=sched->starttime;
			pa_proplist[x].schedlist[cnt].endtime=sched->endtime;
			pa_proplist[x].schedlist[cnt].onoff=sched->onoff;
			pa_proplist[x].schedlist[cnt].num =sched->num;

			pa_proplist[x].schedule_count++;
		}
		savePAprop();
		return;
	}
	printf("set schedule, DID not found %d", sched->did);

}

void getsched(char *buf)
{
	int x;
	int y;
	int found = 0;
	unsigned int id;
	memcpy(&id, buf, sizeof(int));

	for (x=0; x<paCount;x++)
	{
		if (pa_proplist[x].did != id )
			continue;

		int cnt = pa_proplist[x].schedule_count;
		printf("get schedule PA prop list cnt = %d\n", cnt);
		if (cnt == 0)
		{
			sendCmdtoClient(CM_GETSCHEDULE, 0, 0,  sizeof(int) , &id);
		}else
		{
			sendCmdtoClient(CM_GETSCHEDULE, 0, cnt,  cnt * sizeof(jswschedule) , &pa_proplist[x].schedlist );
		}
		return;

	}

	printf("Error: get sched DID not found, DID=%d, paCount=%d\n", id, paCount);
}


//int    tm_sec   Seconds [0,60].
//int    tm_min   Minutes [0,59].
//int    tm_hour  Hour [0,23].
//int    tm_mday  Day of month [1,31].
//int    tm_mon   Month of year [0,11].
//int    tm_year  Years since 1900.
//int    tm_wday  Day of week [0,6] (Sunday =0).
//int    tm_yday  Day of year [0,365].
//int    tm_isdst Daylight Savings flag.


int isSameWeekday(short v, int wday)
{
	if (wday == 0)//sunday
	{
		if ( (v & 0x40 )== 0x40 )//need to be on.
			return 1;
	}else if (wday == 1)//monday
	{
		if ( (v & 0x01 )== 0x01 )//need to be on.
			return 1;
	}if (wday == 2)//
	{
		if (( v & 0x02 )== 0x02 )//need to be on.
			return 1;
	}if (wday == 3)//
	{
		if (( v & 0x04) == 0x04 )//need to be on.
			return 1;
	}if (wday == 4)//
	{
		if (( v & 0x08 )== 0x08 )//need to be on.
			return 1;
	}if (wday == 5)//
	{
		if ( (v & 0x10) == 0x10 )//need to be on.
			return 1;
	}if (wday == 6)//sat
	{
		if ( (v & 0x20) == 0x20 )//need to be on.
			return 1;
	}

	return 0;


}

int g_last_schedule[MAXDEVICENUM];//save last 48 schedule processing time
int g_last_schedule_index = 0;//last schedule array index

//reset last schedule
void reset_last_schedule()
{
    memset(g_last_schedule, 0, sizeof(g_last_schedule));
    g_last_schedule_index = 0;
}

//add new schedule processing time to last schedule array
void add_last_schedule(int last_time)
{
    g_last_schedule[g_last_schedule_index] = last_time;
    g_last_schedule_index++;
    if( (g_last_schedule_index < 0) || (g_last_schedule_index >= MAXDEVICENUM) )
        g_last_schedule_index = 0;
    //printf("g_last_schedule_index=%d, last_time=%d\n", g_last_schedule_index, last_time);
}

//check whether can perform schedule or not
//return 0: can't perform schedule, 1:can perform schedule
int check_last_schedule(int last_time)
{
    int i;
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if(g_last_schedule[i] == last_time)
            return 0;
    }
    return 1;
}

//del last_time in last schedule list
//return 0:fail, 1:success
int del_last_schedule(int last_time)
{
    int i;
    if(g_last_schedule_index > 0)
    {
        for(i=0;i<MAXDEVICENUM;i++)
        {
            if(g_last_schedule[i] == last_time)
            {
                g_last_schedule[i] = g_last_schedule[g_last_schedule_index-1];
                g_last_schedule[g_last_schedule_index-1] = 0;
                g_last_schedule_index--;
                if(g_last_schedule_index < 0)
                    g_last_schedule_index = 0;
                return 1;
            }
        }
    }
    return 0;
}

//calculate last time
//is_start: 0->stop, 1->start
int get_last_time(int currenttime, int x, int y, int is_start)
{
    if(is_start == 1)//start
    {
        return (currenttime*10000+x*100+y+99999999);
    }else//stop
    {
        return (currenttime*10000+x*100+y+88888888);
    }
}

//get time level in 30 secs
int get_schedule_level(int hour, int min, int sec)
{
    while(min < 0)
    {
        min += 60;
        hour--;
    }
    while(min >= 60)
    {
        min -= 60;
        hour++;
    }
    while(hour < 0)
    {
        hour += 24;
    }
    while(hour >= 24)
    {
        hour -= 24;
    }
    return (hour*60*60+min*60+sec)/30;
}

void arrangeSchedule()
{
	//adjust time by internet, only 3 minutes while startup
	int update_sensor =0;
	static int adjust_time_count = 0;
	static int last_mday = -1;
    if(adjust_time_count < 6)
    {//6*30 = 180, in 3 mins
        if(getc_popen("cat /sys/class/net/eth0/carrier") == '1')
        {//had rj45 cable pluged, adjust time via internet
            check_date(1);

            if(adjust_time_count >= 3)
                get_sunrise_sunset_data();
        }
        adjust_time_count++;
    }

	//arrange schedule
	int x;
	int y;
	short shh;
	short smm;
	short ehh;
	short emm;
	short currenttime;
	struct tm *utctm;
	jswdev* dev=NULL;
	int last_time = 0;

	time_t now = time(0);
	utctm = gmtime(&now);

	//printf("\n### Time now week=%d hour=%d min=%d current timezone %d\n\n", utctm->tm_wday, utctm->tm_hour, utctm->tm_min, g_setting.timezone);

	//adjust time
	if( (g_setting.timezone != 0) || (g_setting4.extra.DST_enable != 0) )
	{
	    now += (g_setting.timezone*60*60);
		if(g_setting4.extra.DST_enable != 0)
            now += (1*60*60);
        utctm = gmtime(&now);
		//printf("\n### Adject Time now week=%d hour=%d timezone = %d\n\n", utctm->tm_wday, utctm->tm_hour, g_setting.timezone);
	}

    //reset last shedule list
    if(last_mday != utctm->tm_mday)
    {
        reset_last_schedule();
        last_mday = utctm->tm_mday;
    }

	//check list
	for (x=0; x<paCount;x++)
	{
		int cnt = pa_proplist[x].schedule_count;
		if (cnt == 0)
			continue;

		for (y=0;y<cnt;y++)
		{
			if (pa_proplist[x].schedlist[y].onoff == 0 )
			{
//				printf("sched %d is turn off\n", y);
				continue;
			}

			if ( isSameWeekday(pa_proplist[x].schedlist[y].week, utctm->tm_wday) == 0)
			{
//				printf("sched %d weekday not match %d current weekday %d\n",y, pa_proplist[x].schedlist[y].week,  utctm->tm_wday);
				continue;
			}

			//start to check time
			shh = pa_proplist[x].schedlist[y].starttime / 60;
			smm = pa_proplist[x].schedlist[y].starttime % 60;
			ehh = pa_proplist[x].schedlist[y].endtime / 60;
			emm = pa_proplist[x].schedlist[y].endtime % 60;

			//convert current time to min.
			currenttime = (utctm->tm_hour * 60 ) + utctm->tm_min;
			dev = getDevbydid( pa_proplist[x].did );
			if (dev == NULL)
				continue;

//			printf("sched %d, time to seconds starttime %d, endtime %d, currenttime %d\n" ,y,  pa_proplist[x].schedlist[y].starttime, pa_proplist[x].schedlist[y].endtime, currenttime);

#define RANDOM_CHANGE_ON_percentage 10
#define RANDOM_CHANGE_OFF_percentage 5

			if ( currenttime > pa_proplist[x].schedlist[y].starttime && currenttime < pa_proplist[x].schedlist[y].endtime )
			{
				//got it , Interval. check random.
				//
				double sec = difftime(currenttime, pa_proplist[x].elapse);
				if(( sec > 60.0 && pa_proplist[x].schedlist[y].random ==1))
				{
					if(dev->status == CMD_ADAPTER_TURNON)
					{
						if( (rand()%100 ) <= RANDOM_CHANGE_OFF_percentage)
						{

							printf("%d:%d RANDOM OFF @ [%ld] \n",utctm->tm_hour,utctm->tm_min,pa_proplist[x].did);
							writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNOFF);
							dev->status = CMD_ADAPTER_TURNOFF;
							update_sensor = 1;
//							newEvent(dev->did, dev->model, dev->status, 0);

						}
					}
					else
					{
						if( (rand()%100) <= RANDOM_CHANGE_ON_percentage)
						{
							printf("%d:%d RANDOM ON @ [%ld] \n",utctm->tm_hour,utctm->tm_min,pa_proplist[x].did);
							writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
							dev->status = CMD_ADAPTER_TURNON;
							update_sensor = 1;
//							newEvent(dev->did, dev->model, dev->status, 0);

						}
					}

				}
				else
					{
					if(dev->status == RE_ADAPTER_ISOFF)
						{
						printf("%d:%d FORCE ON @ [%ld] \n",utctm->tm_hour,utctm->tm_min,pa_proplist[x].did);
						writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
						dev->status = CMD_ADAPTER_TURNON;
						update_sensor =1;
						}

					}

				if(update_sensor)
					{
					int payload_len =  sizeof(jswdev) * devcount;
					sendCmdtoClient(CM_DEV, 0, devcount,  payload_len,(char*) &devlist) ;
					}

//				printf("sched elapse %d sec\n", (int)sec);


			}else if ( currenttime == pa_proplist[x].schedlist[y].endtime )
			{
                last_time = get_last_time(currenttime, x, y, 0);
                if(check_last_schedule(last_time) == 1)
                {//send once
                    add_last_schedule(last_time);
                    //shut it down;
                    printf("stop PA schedule \n");
                    //if (dev->status != CMD_ADAPTER_TURNOFF)
                    //{
                        //printf("send comm to device , stop PA schedule \n");
                        writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNOFF);
                        dev->status = CMD_ADAPTER_TURNOFF;
                        newEvent(dev->did, dev->model, dev->status, 0);
                    //}
				}

			}else if ( currenttime == pa_proplist[x].schedlist[y].starttime )
			{
                last_time = get_last_time(currenttime, x, y, 1);
                if(check_last_schedule(last_time) == 1)
                {//send once
                    add_last_schedule(last_time);
                    //start it;
                    pa_proplist[x].elapse = time(0);
					if( pa_proplist[x].schedlist[y].random ==1)
					{
                    printf("start PA schedule with random\n");
                    //if ( dev->status != CMD_ADAPTER_TURNON)
                    //{
                        //printf("send comm to device , start PA schedule \n");
                        writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
                        dev->status = CMD_ADAPTER_TURNON;
                        newEvent(dev->did, dev->model, dev->status, 0);
                    //}
					}
					else
					{
                    	printf("start PA schedule \n");
                        writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
                        dev->status = CMD_ADAPTER_TURNON;
                        newEvent(dev->did, dev->model, dev->status, 0);
					}
				}
			}
		}
	}


	//for sunrise /sunset scenario
	int year = utctm->tm_year+1900;
	int mon= utctm->tm_mon+1;
	int day= utctm->tm_mday;

    if( (year == g_sunrisesunset[1].sunrise.tm_year) && (mon == g_sunrisesunset[1].sunrise.tm_mon) &&
        (day == g_sunrisesunset[1].sunrise.tm_mday) )
    {
        //copy tomorrow to today
        memcpy(&g_sunrisesunset[0], &g_sunrisesunset[1], sizeof(jswsunrisesunset));
        memset(&g_sunrisesunset[1], 0, sizeof(jswsunrisesunset));
    }

    if( (year == g_sunrisesunset[0].sunrise.tm_year) && (mon == g_sunrisesunset[0].sunrise.tm_mon) &&
        (day == g_sunrisesunset[0].sunrise.tm_mday) )
    {//the same day
        int hour = utctm->tm_hour;
        int min = utctm->tm_min;
        int sec2 = utctm->tm_sec;

        int now_lvl = 0;
        int sunrise_lvl = 0;
        int sunset_lvl = 0;
        signed char highb = 0;
        signed char lowb = 0;
        now_lvl = get_schedule_level(hour, min, sec2);

        if(srcCount > 0)
        {//with scenario
            for(x=0;x<srcCount;x++)
            {
                if(scenariolist[x].src.did == SCENARIO_GATEWAY_DID)
                {//source is gateway
                    if(scenariolist[x].src.enable != 0)
                    {//enable
                        highb = 0;
                        lowb = 0;
                        highb = scenariolist[x].src.onoff >> 8;
                        lowb = scenariolist[x].src.onoff & 0x00FF;

                        sunrise_lvl = get_schedule_level(g_sunrisesunset[0].sunrise.tm_hour, g_sunrisesunset[0].sunrise.tm_min+highb*1, g_sunrisesunset[0].sunrise.tm_sec);
                        sunset_lvl = get_schedule_level(g_sunrisesunset[0].sunset.tm_hour, g_sunrisesunset[0].sunset.tm_min+highb*1, g_sunrisesunset[0].sunset.tm_sec);

                        if( (lowb == SCENARIO_SOURCE_SUNRISE) && (now_lvl == sunrise_lvl) )
                        {//start sunrise scenario
                            startScenario(SCENARIO_GATEWAY_DID, SCENARIO_SOURCE_SUNRISE);
                        }else if( (lowb == SCENARIO_SOURCE_SUNSET) && (now_lvl == sunset_lvl) )
                        {//start sunset scenario
                            startScenario(SCENARIO_GATEWAY_DID, SCENARIO_SOURCE_SUNSET);
                        }
                    }
                }
            }
        }
    }
}

void startzone(char *buf, int mode)
{
	int x;
	int y;
	int found = 0;
	jswdev *dev;
	unsigned int num;
	memcpy(&num, buf, sizeof(int));
	//printf("start zone %d\n", num);

	for (x=0;x <zoneCount;x++)
	{
		if (zonelist[x].zone.num != num )
			continue;
		found = 1;

		if (zonelist[x].zone.count > 0)
		{
			for (y=0;y <zonelist[x].zone.count;y++)
			{
				//turn on
				dev = getDevbydid(zonelist[x].didlist[y]);
				if (dev == NULL)
					continue;

				if ( dev->model == TYPE_CAMERA )
				{
					if (mode == CMD_ADAPTER_TURNON)
						doRec(dev->did);

				}else if ( dev->model == TYPE_POWER_ADAPTER )
				{

					if (mode == CMD_ADAPTER_TURNOFF)
					{
						writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNOFF); //SHARE WITH SIREN
						dev->status = CMD_ADAPTER_TURNOFF;
					}
					else if (mode == CMD_ADAPTER_TURNON)
					{
						writeCommand( dev->did, dev->model, CMD_ADAPTER_TURNON);
						dev->status = CMD_ADAPTER_TURNON;
					}
					savedata();
				}else if (dev->model == TYPE_SIREN_INDOOR ||  TYPE_SIREN_OUTDOOR == dev->model )
				{
					if (mode == CMD_ADAPTER_TURNOFF)
					{
						writeCommand( dev->did, dev->model, CMD_SIREN_TURNOFF);
						dev->status = CMD_SIREN_TURNOFF;
					}
					else
					{
						writeCommand( dev->did, dev->model, CMD_SIREN_TURNON);
						dev->status = CMD_SIREN_TURNON;
						newEvent(dev->did, dev->model, dev->status, 0);
					}

				}
				//need to check for ack here.
				if (dev->model == TYPE_POWER_ADAPTER )
				newEvent(dev->did, dev->model, dev->status, 0);
			}
		}
	}



}

void setzone(char* buf)
{
	jswzone *z;
	int x=0;
	int found =0;
	char *p;
	z = (jswzone*)buf;
	p = buf + sizeof(jswzone);

	if (z->num > MAXZONE)
	{
		printf("Error: setzone, too large zone %d\n", z->num);
        SetRunningLog_str_int("Error: setzone() invalid num", z->num);
        sendCmdtoClient(CM_SETZONE, 1, 0, 0, NULL);
		return;
	}
	if(strlen(z->name) <= 0)
	{
		printf("Error: setzone, empty name, strlen(z->name)=%d\n", strlen(z->name));
        SetRunningLog_str_int("Error: setzone() invalid string length", strlen(z->name));
        sendCmdtoClient(CM_SETZONE, 1, 0, 0, NULL);
		return;
	}

	printf("setzone, num %d, count %d", z->num, z->count);

	for (x=0;x <zoneCount;x++)
	{
		if (zonelist[x].zone.num != z->num )
			continue;
		found = 1;

		//delete
		if (z->count == -1)
		{
			if (zoneCount == 1)
			{

			}else
			{
				if (zoneCount-1 != x) //not lastitem
				{//move the last item here . to keep array continuous.
					memcpy(&zonelist[x], &zonelist[zoneCount-1], sizeof(struct _zonelist));
				}
			}
			zoneCount --;
			if (zoneCount <0)
				zoneCount = 0;

		}else{
			//update
			strcpy( zonelist[x].zone.name , z->name);
			if (z->count >= 0 && z->count < MAXSENSOR_ZONE)
			{
				zonelist[x].zone.count = z->count;
				if (z->count > 0)
					memcpy(&zonelist[x].didlist, p, sizeof(int) * z->count );
			}
			printf("update exsiting zone\n");
		}
		break;
	}
	if (found ==0 && z->count != -1 )
	{
		if (zoneCount < MAXZONE)
		{
		//add new
			zonelist[zoneCount].zone.num = z->num;
			strcpy( zonelist[x].zone.name , z->name);
			//zonelist[zoneCount].zone.count = z->count;
			if (z->count >= 0 && z->count < MAXSENSOR_ZONE)
			{
				zonelist[zoneCount].zone.count = z->count;
				if (z->count > 0)
					memcpy(&zonelist[zoneCount].didlist, p, sizeof(int) * z->count );
			}
			zoneCount++;
			printf("create zone\n");
		}//else
		//	printf("add zone , but zone count too big %d",z->count ):
        getzone(CM_SETZONE, 1, NULL);
	}

	savezone();
}

//cmd: P2P cmd
//broadcast: 0 -> single session, 1 -> all sessions
//session: single session
void getzone(int cmd, int broadcast, int session)
{
	int x;
	int found = 0;
	unsigned int num;

	char buf[6000];
	char *p = buf;
	int len=0;

	memset(buf, 0, sizeof(buf));

	//printf("client get zone, zonecount %d\n",zoneCount);

	for (x=0;x <zoneCount;x++)
	{
		memcpy(p, &zonelist[x].zone, sizeof(jswzone) );
		p += sizeof(jswzone);
		len += sizeof(jswzone);
		if ( zonelist[x].zone.count > 0)
		{
			memcpy(p, &zonelist[x].didlist, sizeof(int) * zonelist[x].zone.count );
			p += (sizeof(int) * zonelist[x].zone.count) ;
			len +=( sizeof(int) * zonelist[x].zone.count) ;

			//printf("get zone, zone num %d count = %d\n",zonelist[x].zone.num, zonelist[x].zone.count );
		}
	}

	if(broadcast == 0)
	{//single session
        if (len == 0)
        {
            sendCmdtoClient2(cmd, 0, 0, 0 , NULL, session);
        }else
        {
            sendCmdtoClient2(cmd, 0, zoneCount,  len ,buf, session);
        }
	}else
	{//all sessions
        if (len == 0)
        {
            sendCmdtoClient(cmd, 0, 0, 0 , NULL);
        }else
        {
            sendCmdtoClient(cmd, 0, zoneCount,  len ,buf);
        }
	}

}

void setpush(char *buf)
{
	jswpush* push;
	char *p;
	push = (jswpush*)buf;
	p = buf + sizeof(jswpush);

	//printf("setpush push key %s num %s\n", push->key, push->smsnumber):
	//printf("set push, data from app email count %d\n", push->emailcount);

	if(push->emailcount < 0 || pushdata.emailcount > 5)
	{
		printf("set push, email count error  %d\n", push->emailcount);
		return;
	}
	memcpy(&pushdata, buf, sizeof(jswpush));

	if(pushdata.emailcount > 0 )
	{
		memcpy(&emaillist, p,  sizeof(struct em) * pushdata.emailcount );
		//printf("email data from app %s   \n", emaillist[0].email);
	}

	//printf("save pushdata, key %s, smsnum %s, smson %d, route %d, isemail count %d %d, push on %d \n",  pushdata.key, pushdata.smsnumber, pushdata.smson,
	//pushdata.route, pushdata.emailon, pushdata.emailcount, pushdata.pushon );
	savepush();
}

//apple
void getOthers()
{
	int x;
	int found = 0;
	unsigned int num;

	char buf[2046];
	char *p = buf;
	int len=0;

	memset(buf, 0, sizeof(buf));

	memcpy(p, &m_othersData, sizeof(jswOther) );
	p += sizeof(jswOther);
	len += sizeof(jswOther);

	sendCmdtoClient(CM_GET_MAG_ON_BEEP, 0, 1,  len ,buf );

}

void setOthers(char *buf)
{
	jswOther* otherInfo;
	char *p;
	otherInfo = (jswOther*)buf;
	p = buf + sizeof(jswOther);

	//printf("setOthers on:(%d)\n",otherInfo->bBeepInMagOn):

	memcpy(&m_othersData, buf, sizeof(jswOther));

	saveOthers();
}



void getpush()
{
	int x;
	int found = 0;
	unsigned int num;

	char buf[2046];
	char *p = buf;
	int len=0;

	memset(buf, 0, sizeof(buf));
//	printf("get push email count %d\n", pushdata.emailcount);

	//printf("");

	memcpy(p, &pushdata, sizeof(jswpush) );
	p += sizeof(jswpush);
	len += sizeof(jswpush);

	if ( pushdata.emailcount > 0)
	{
		memcpy(p, &emaillist, sizeof(struct em) * pushdata.emailcount );
		//printf("email data 1: %s   \n2:%s\n", emaillist[0], emaillist[1]);
		len += ( sizeof(struct em) * pushdata.emailcount);
	}

	//printf("prepare to send pushdata to client, length  %d  key %s, smsnum %s, smson %d, route %d, isemail count %d %d, push on %d \n", len, pushdata.key, pushdata.smsnumber, pushdata.smson,
	//	pushdata.route, pushdata.emailon, pushdata.emailcount, pushdata.pushon );
	sendCmdtoClient(CM_GETPUSH, 0, 1,  len ,buf );

}

	//char key[64];
	//char smsnumber[32];
	//short smson;
	//short route; //0, 1, 2 basic, gold, direct
	//short pushon;
	//short emailon;
	//short emailcount;

void gettemperature(){

	char out[1000];
	memcpy(out, &g_humid, sizeof(short));
	memcpy(out + sizeof(short), &g_temp, sizeof(short));

	sendCmdtoClient(CM_GETTEMP, 0, 1,  sizeof(short)*2 , out );

}

void factoryReset(int mode)// 0 = clear all, 1 = dont clear jswitem.
{
	remove("/mnt/jswevent.db");
	evtcount = 0;

	if (mode == 0)
	{
		remove("/mnt/jswitem.db");
		remove("/mnt/jswitem2.db");
		devcount = 0;
	}

	// remove NEST account info
	system("rm /mnt/NEST_device.db");
	// remove alarm view
	system("rm /mnt/alarm_info.db");

	// remove Cloud account info , apple
	//system("rm /mnt/user_info.db");
	
	// remove Cloud DA info , apple
	system("rm /mnt/DA_info.db");
 
	remove("/mnt/scenario.db");
	srcCount = 0;

	remove("/mnt/push2.db");
	memset(&pushdata, 0, sizeof(jswpush));

	remove("/mnt/pa.db");
	paCount = 0;

	remove("/mnt/zone.db");
	zoneCount = 0;

	remove("/mnt/setting3.db");
	memset(&g_setting, 0 , sizeof(struct generalinfo));
	strcpy(g_setting.gwprop.name, DEF_SYSTEM_NAME);
	g_setting.gwprop.alarmOn = 1;
	//g_setting.gwprop.ledon =2;
	g_setting.gwprop.ledon =0; // SHC_PRO set to 0
	g_setting.gwprop.entrydelay = 0;
	g_setting.gwprop.exitdelay = 30;
	g_setting.gwprop.duration = 60;
	g_setting.gwprop.timezonechanged = 0;
	g_setting.network.dhcp = 1;
	strcpy( g_setting.acct.adminpwd, "123456");
	strcpy( g_setting.acct.loginpwd, "123456");
	g_setting.gwprop.version = GATEWAY_VERSION;
	g_setting.sirencount = 0;
	is_oldevtexist =0;

	delEvfilelist();

	remove("/mnt/setting4.db");
	memset(&g_setting4, 0 , sizeof(g_setting4));
	g_setting4.extra.push_string_language_id = DEF_PUSH_LANGUAGE;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_ARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_PART_ARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_DISARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_PANIC;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_TAMPER;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_ALARM;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_SENSOR_BATTLOW;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_GATEWAY_BATTLOW;
    g_setting4.extra.push_selection_1 |= DEF_PUSH_SELECTION1_LOST_SIGNAL;

	remove("/mnt/setting5.db");
	memset(&g_setting5, 0 , sizeof(g_setting5));

	system("sync");
}

void factoryReset2()
{
    char *dhcpstr = "auto lo\niface lo inet loopback\n\nauto eth0\niface eth0 inet dhcp\n";

    //system("/usr/sbin/buzzer_beep 100 300 0 1&");
    play_beep_armed();
    usleep(1000);

    strcpy( g_setting.acct.adminpwd, "123456");
    strcpy( g_setting.acct.loginpwd, "123456");
    g_setting.network.dhcp = 1;

    FILE *fp;
    remove("/etc/network/interfaces");
    fp = fopen("/etc/network/interfaces", "wb+");
    if (fp != NULL)
    {
        fwrite(dhcpstr, strlen(dhcpstr), 1, fp);
    }
    fclose(fp);

    savesetting();

    //snx_gpio_close();
	// remove NEST account info
	system("rm /mnt/NEST_device.db");
	system("rm /mnt/alarm_info.db");
	// remove Cloud account info, apple
	//system("rm /mnt/user_info.db");
	
    //system("killall -9 buzzer_beep");
    // clean alarm view
    g_armstate = 0;
    g_ShowAlarmView = 0 ;
	g_AlarmState = 0;
	savesetting4();
    system("sync");
    system("reboot");
    exit(1);
}

int isvalidaddr(char *ip)
{
	struct sockaddr_in sa;
	int res = inet_pton(AF_INET, ip, &(sa.sin_addr) );

	return res;

}


void setnetwork(int len, char * buf)
{
	char *p = buf;
	jswnetwork *nw;
	int dhcpchange =0;
	int staticipchange = 0;
	char *dhcpstr = "auto lo\niface lo inet loopback\n\nauto eth0\niface eth0 inet dhcp\n";
	char staticstr[2046];
	int isvalid;

	if ( len != sizeof(jswnetwork) )
	{
		printf("wrong size of set network payload\n");
		sendCmdtoClient(CM_SETNETWORK, 4, 0, 0 , NULL );
        SetRunningLog_str_int("Error: setnetwork() size not equal", len);
		return;
	}
	nw = (jswnetwork*)buf;

	printf("dhcp%d ip %s netmask %s gateway %s dns %s\n",nw->dhcp,nw->ip,nw->mask,nw->gateway,nw->dns );

	if (nw->dhcp ==1)
		isvalid = 1;//no need to check.
	else
	{//check if ip is valid
		isvalid = isvalidaddr(nw->ip);
		if (isvalid ==0)
		{
			sendCmdtoClient(CM_SETNETWORK, 4, 0, 0 , NULL );
			return;
		}
		isvalid = isvalidaddr(nw->mask);
		if (isvalid ==0)
		{
			sendCmdtoClient(CM_SETNETWORK, 4, 0, 0 , NULL );
			return;
		}
		isvalid = isvalidaddr(nw->gateway);
		if (isvalid ==0)
		{
			sendCmdtoClient(CM_SETNETWORK, 4, 0, 0 , NULL );
			return;
		}
		isvalid = isvalidaddr(nw->dns);
		if (isvalid ==0)
		{
			sendCmdtoClient(CM_SETNETWORK, 4, 0, 0 , NULL );
			return;
		}
	}//check is valid

	//check if any diff with the old one
	if (g_setting.network.dhcp != nw->dhcp)
		dhcpchange = 1;
	if ( strcmp(g_setting.network.ip, nw->ip) != 0 )
		staticipchange = 1;
	if ( strcmp(g_setting.network.mask, nw->mask) != 0 )
		staticipchange = 1;
	if ( strcmp(g_setting.network.gateway, nw->gateway) != 0 )
		staticipchange = 1;
	if ( strcmp(g_setting.network.dns, nw->dns) != 0 )
		staticipchange = 1;

	//time to reboot.
	if (dhcpchange==0 && staticipchange==0)
		return;

	memcpy(&g_setting.network, buf, sizeof(jswnetwork));
	sendCmdtoClient(CM_SETNETWORK, 0, 0, 0 , NULL );


	FILE *fp;
	remove("/etc/network/interfaces");
	fp = fopen("/etc/network/interfaces", "wb+");

	if (g_setting.network.dhcp == 1)
		fwrite(dhcpstr, strlen(dhcpstr), 1, fp);
	else
	{
		memset(staticstr, 0, sizeof(staticstr));
		sprintf(staticstr, "auto lo\niface lo inet loopback\n\nauto eth0\niface eth0 inet static\naddress %s\nnetmask %s\ngateway %s\ndns-nameservers %s\n",
			g_setting.network.ip, g_setting.network.mask, g_setting.network.gateway, g_setting.network.dns);

		//printf(staticstr);
		int t = fwrite(staticstr, strlen(staticstr), 1, fp);
		printf("write count %d", t);
	}

	savesetting();

	fclose(fp);
	system("sync");
	system("reboot");
	exit(1);


}


void getnetwork()
{
	sendCmdtoClient(CM_GETNETWORK, 0, 1, sizeof(jswnetwork) , &g_setting.network );
}

void getgatewayinfo(int sess)
{
//	printf("client get gw prop, version %d\n",g_setting.gwprop.version );
	sendCmdtoClient2(CM_GETGWPROP, 0, 1, sizeof(jswgwprop) , &g_setting.gwprop, sess);
}


	//char name[64];
	//short alarmOn;//default 1.
	//short version;//
	//short ledon;
	//short mcu_version;

void setgatewayinfo(char *in)
{
	jswgwprop * gw;
	gw = (jswgwprop*)in;

//    if(strcmp(gw->name, DEF_RESET_SYSTEM_NAME) == 0)
//        strcpy(g_setting.gwprop.name, DEF_SYSTEM_NAME);
//    else if(strcmp(g_setting.gwprop.name, DEF_SYSTEM_NAME) == 0)
        strcpy(g_setting.gwprop.name, gw->name);
	//g_setting.gwprop.alarmOn = gw->alarmOn;
	play_beep_set_volume(gw->alarmOn);
	g_setting.gwprop.ledon = gw->ledon;

	switch(gw->duration)
	{
		case 0:
			g_setting.gwprop.duration = 0;
		break;
		case 60:
			g_setting.gwprop.duration = 60;
		break;
		case 120:
			g_setting.gwprop.duration = 120;
		break;
		case 180:
			g_setting.gwprop.duration = 180;
		break;
		default:
			g_setting.gwprop.duration = 60;
		break;
	}
	g_setting.gwprop.duration = gw->duration;
	if (g_setting.gwprop.duration < 0)
		g_setting.gwprop.duration = 10;
	if (g_setting.gwprop.duration > 180)
		g_setting.gwprop.duration = 180;


	g_setting.gwprop.entrydelay = gw->entrydelay;
	g_setting.gwprop.exitdelay =gw->exitdelay;

	g_setting.gwprop.mcu_version = gw->mcu_version;

	savesetting();
}


//struct sirenprop{
//	unsigned int did;
//	short ledon; //0 = off, 1= on
//	short soundon;//0=off, 1=low, 2 = high
//	short duration; //seconds
//	short r1; //0 = low , 1= high.
//	short r2;
//};

void setsirenprop(char *in)
{
	struct sirenprop* dev;
	struct sirenprop* old=NULL;
	struct sirenprop last;
	jswdev *siren;
	int x =0;

	dev = (struct sirenprop*)in;
	siren = getDevbydid(dev->did);
	if (siren == NULL)
	{
        printf("Set siren did not found(%d)\n",dev->did);
        SetRunningLog_str_int("Error: setsirenprop() did not found", dev->did);
		return;
	}
#if 0
	// Jeff mark for NoSignal retry

	if (siren->status == RE_ABUS_AUTOREPORT)
	{
		printf("siren %u is offline\n",dev->did );
		return;
	}
#endif
	 //printf("client set siren %u, led =%d, sound=%d, duration=%d\n", dev->did, dev->ledon, dev->soundon, dev->duration);

//	if (  siren == NULL)
//	{
//		printf("Set siren did not found\n");
//		return;
//	}

	for (x=0;x<g_setting.sirencount;x++)
	{
		if (g_setting.sirenlist[x].did == dev->did)
		{
			old = &g_setting.sirenlist[x];
			if (old == NULL)
			{
				last.ledon = -1;
				last.soundon = -1;
				last.duration = -1;
			}
			else
			{
				last.ledon = old->ledon;
				last.soundon = old->soundon;
				last.duration = old->duration;
			}


			old->ledon = dev->ledon;
			old->soundon = dev->soundon;
			old->duration = dev->duration;

			//printf("Edit existing siren prop %u\n", dev->did);
			savesetting();


			break;
		}
	}
	if (old == NULL)
	{
		int cnt = g_setting.sirencount;
		if (cnt < 10)
		{
			  //memcpy(&g_setting.sirenlist[cnt], dev, sizeof(struct sirenprop));

			  g_setting.sirenlist[cnt].did = dev->did;

			  g_setting.sirenlist[cnt].ledon = dev->ledon;
			  g_setting.sirenlist[cnt].soundon = dev->soundon;
			  g_setting.sirenlist[cnt].duration = dev->duration;
			  g_setting.sirencount++;
			 // printf("Add siren prop %u\n", dev->did);
			  savesetting();
		}else
			printf("siren count wrong\n");
	}


    //0, ledonly, 1 sound, 2, both.
    if(  dev->ledon != last.ledon || dev->soundon != last.soundon)
    {
		if ( dev->ledon == 0)
		{
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_VOLUME_OFF);
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_LED_ON);
		}
		else
		{

			if ( dev->soundon == 0)
				writeCommand(siren->did,siren->model, CMD_SIREN_SET_VOLUME_LOW);
			else if ( dev->soundon == 1)
				writeCommand(siren->did,siren->model, CMD_SIREN_SET_VOLUME_HIGH);

			if ( dev->ledon == 1)
				writeCommand(siren->did,siren->model, CMD_SIREN_SET_LED_OFF);
			else if (dev->ledon ==2)
				writeCommand(siren->did,siren->model, CMD_SIREN_SET_LED_ON);

		}
    }
	if(  dev->duration != last.duration)
	{
		if ( dev->duration == 10)
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_PERIOD_10);
		else if ( dev->duration == 30)
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_PERIOD_30);
		else if ( dev->duration == 60)
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_PERIOD_60);
		else if ( dev->duration == 120)
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_PERIOD_120);
		else if ( dev->duration == 180)
			writeCommand(siren->did,siren->model, CMD_SIREN_SET_PERIOD_180);
	}

}

void getsirenprop(char *in)
{
	unsigned int did;
	memcpy(&did, in, sizeof(int));

	int x =0;
	printf("Get siren property[%d] siren count = %d\n",did, g_setting.sirencount);

	for (x=0;x<g_setting.sirencount;x++)
	{
		if (g_setting.sirenlist[x].did == did)
		{
			//old = &g_setting.sirenlist[x];
			sendCmdtoClient(CM_GETSIRENPROP, 0, 1, sizeof(struct sirenprop) , &g_setting.sirenlist[x] );
			return;
		}
	}
	sendCmdtoClient(CM_GETSIRENPROP, 4, 0,0 , NULL );

}

void sirenmaintain(int mode)
{
	int x =0;
	struct sirenprop* dev;
	jswdev *siren;

	if (mode ==0) //going in
	{
		for ( x=0;x<devcount;x++)
		{
			if ( devlist[x].model == TYPE_SIREN_INDOOR || devlist[x].model == TYPE_SIREN_OUTDOOR )
			{
				writeCommand( devlist[x].did,devlist[x].model, CMD_SIREN_SET_VOLUME_OFF);
				usleep(10000);
				//writeCommand( devlist[x].did,devlist[x].model, CMD_SIREN_SET_LED_OFF);
				//usleep(1000);
			}
		}
	}else
	{//leaving maintain mode.


		for (x=0;x<g_setting.sirencount;x++)
		{
		    //0, ledonly, 1 sound, 2, both.
			dev = &g_setting.sirenlist[x];

			siren = getDevbydid(dev->did);
			if (siren == NULL)
				continue;

			if ( dev->ledon == 0)
			{
				writeCommand(siren->did,siren->model, CMD_SIREN_SET_VOLUME_OFF);
				usleep(10000);
				//writeCommand(siren->did,siren->model, CMD_SIREN_SET_LED_ON);
			}
			else
			{
				if ( dev->soundon == 0)
					writeCommand(siren->did,siren->model, CMD_SIREN_SET_VOLUME_LOW);
				else if ( dev->soundon == 1)
					writeCommand(siren->did,siren->model, CMD_SIREN_SET_VOLUME_HIGH);

				usleep(10000);

				if ( dev->ledon == 1)
					writeCommand(siren->did,siren->model, CMD_SIREN_SET_LED_OFF);
				else if (dev->ledon ==2)
					writeCommand(siren->did,siren->model, CMD_SIREN_SET_LED_ON);
			}
			usleep(10000);
		}
	}
}


void updatepwd(char *in, int session)
{
	struct _acct_client *acct;
	int a=0;
	int b=0;
	short ret;
	acct = (struct _acct_client*) in;


	if ( strcmp(acct->oldlogin , g_setting.acct.loginpwd) ==0 )
	{
		if(strlen(acct->loginpwd) > 0)
		{
			strcpy(g_setting.acct.loginpwd, acct->loginpwd);
			a = 1;
		}
	}

	if ( strcmp(acct->oldadmin , g_setting.acct.adminpwd) ==0 )
	{
		if(strlen(acct->adminpwd) > 0)
		{
			strcpy(g_setting.acct.adminpwd, acct->adminpwd);
			b = 1;
		}
	}

	if ( a ==1 && b ==1 )
		ret = 0;
	else if ( a ==1 && b == 0)
		ret = 1;
	else if (a ==0 && b ==1 )
		ret = 2;
	else
		ret = 4;

	if (ret <4)
	{
		printf("save acct info\n");
		savesetting();
	}

	sendCmdtoClient2(CM_SETACCT, ret, 0, 0 ,  NULL, session);

}


//check for No Signal Issue workaround only
int checkNoSignalDevice(int model)
{
    if( (model == TYPE_MAGNETIC) || (model == TYPE_PIR) || (model == TYPE_VIBRATION) || (model == TYPE_DOORCHIME) || (model == TYPE_BUTTON) || \
		(model == TYPE_SIREN_OUTDOOR) || (model == TYPE_SIREN_INDOOR) || (model == TYPE_POWER_ADAPTER) || (model == TYPE_DOOR_LOCK) )
        return 1;
    else
        return 0;
}

//check no signal event
void checkkeepalive()
{
	time_t nw;
	double du;
	int k;
	jswdev *dev=NULL;
	nw = time(0);

	//for check No Signal issue
//	int iNoSignalCount = 0, iNoSignalCount2 = 0;
//	for (k =0;k<devcount;k++)
//	{
//	    iNoSignalCount2 += checkNoSignalDevice(devlist[k].model);
//        du = difftime(nw, keepalivelist[k].lastcheckin);
//        if( (du >= (DEF_SUPERVISION_TIMER_DUR*60*60-10)) && (du < 10800000) ) //4 hours <= x < 3000 hours (10 seconds for buffer)
//        {
//            dev = getDevbydid(keepalivelist[k].did);
//            if (dev != NULL)
//            {
//                iNoSignalCount += checkNoSignalDevice(devlist[k].model);
//            }
//        }
//	}
//printf("iNoSignalCount=%d, iNoSignalCount2=%d\n", iNoSignalCount, iNoSignalCount2);
//    SetRunningLog_str_int("iNoSignalCount=", iNoSignalCount);
//    SetRunningLog_str_int("iNoSignalCount2=", iNoSignalCount2);
//	if( (iNoSignalCount > 0) && (iNoSignalCount == iNoSignalCount2) )
//	{//No Signal Issue happen, reboot
//        log_and_reboot_no_beep("All sensors with no signal, reboot", 1);
//	    return;
//	}

	for (k =0;k<devcount;k++)
	{

			du = difftime(nw, keepalivelist[k].lastcheckin);
			if( (du >= (DEF_SUPERVISION_TIMER_DUR*60*60-10)) && (du < 10800000) ) //4 hours <= x < 3000 hours (10 seconds for buffer)
			{
				dev = getDevbydid(keepalivelist[k].did);
				if (dev != NULL)
				{
					dev->status =RE_ABUS_AUTOREPORT;
					//printf("keepalive disconnect found \n");
//					if( (dev->model != TYPE_REMOTE_CONTROL) && (dev->model != TYPE_CAMERA) )
//                        newEvent(dev->did, dev->model, RE_ABUS_AUTOREPORT, 0);
                    if( (du >= (DEF_SUPERVISION_TIMER_DUR*60*60-10)) && (du < (2*DEF_SUPERVISION_TIMER_DUR*60*60-10)) )
                    {//only send push once
                        char msg[256];
#ifdef DEF_FEATURE_MULTI_PUSH
                        char msg2[256];
                        get_push_string(msg2, STR_ID_LOST_SIGNAL);
                        sprintf( msg, msg2, g_setting.gwprop.name, dev->name, dev->location);
#else
                        sprintf(msg, STRING_TRIGGER_NO_SIGNAL, dev->name, dev->location, gDID);
#endif
//                        if( (dev->model != TYPE_REMOTE_CONTROL) && (dev->model != TYPE_CAMERA) )
//                            pushnotification(NT_NO_SIGNAL, msg, STR_ID_LOST_SIGNAL);
                    }
				}
			}
	}


	//for update sunrise/sunset data
	get_sunrise_sunset_data();
}

//called if device list was full
//remove first dead device, and sort out alive list/device list
//the free list shouldbe the last one (device list)
void freeoneDevice()
{
	int k, j, did;

	system("rm /mnt/jswitem2.db.sort;sync");
	//usleep(200*1000);
	system("cp /mnt/jswitem2.db /mnt/jswitem2.db.sort;sync");

	//get first dead device from alive list
	j = 0;
	for (k=0;k<MAXDEVICENUM;k++)
	{
	    if(keepalivelist[k].lastcheckin < keepalivelist[j].lastcheckin)
	    {
	        j = k;
	    }
	}
	did = keepalivelist[j].did;
    SetRunningLog_str_int("Warning: freeoneDevice() kill device did", did);
	memset(&keepalivelist[j], 0, sizeof(keepalivelist[j]));

	for (k=0;k<(MAXDEVICENUM-1);k++)
	{
	    if(keepalivelist[k].did == 0)
	    {
	        memcpy(&keepalivelist[k], &keepalivelist[k+1], sizeof(keepalivelist[k]));
	        memset(&keepalivelist[k+1], 0, sizeof(keepalivelist[k+1]));
	    }
	}
	memset(&keepalivelist[MAXDEVICENUM-1], 0, sizeof(keepalivelist[MAXDEVICENUM-1]));

	//kill the device from device list
	for (k=0;k<(MAXDEVICENUM-1);k++)
	{
	    if(devlist[k].did == did)
	    {
	        memset(&devlist[k], 0, sizeof(devlist[k]));
	    }
	}
	for (k=0;k<(MAXDEVICENUM-1);k++)
	{
	    if(devlist[k].did == 0)
	    {
	        memcpy(&devlist[k], &devlist[k+1], sizeof(devlist[k]));
	        memset(&devlist[k+1], 0, sizeof(devlist[k+1]));
	    }
	}
	memset(&devlist[MAXDEVICENUM-1], 0, sizeof(devlist[MAXDEVICENUM-1]));

	//recount devcount
	devcount = MAXDEVICENUM;
	for (k=0;k<MAXDEVICENUM;k++)
	{
	    if(devlist[k].did == 0)
	    {
	        devcount = k;
	        break;
	    }
	}
}

int sendevtfromfile(char *filepath, time_t starttime, time_t endtime, int total)
{
	FILE *fp;
	jswevent evttmp[MAXEVENT];
	int count;
	int x;
	int sf=-1; //start time in file
	double sec;

	fp = fopen(filepath, "rb");
	if (fp ==NULL)
	{
		printf("evt file open fail\n");
        SetRunningLog_str("Error: sendevtfromfile() open file failed");
		return 0;
	}

	fread(&count, 1, sizeof(int), fp);
	//printf("count %d on evt data file\n",count);

	if (count <=0 || count > MAXEVENT)
	{
		printf("evt date count error %d\n", count);
		return 0;
	}

	int bread = fread(&evttmp, 1, sizeof(jswevent)* count, fp);
	fclose(fp);

	jswevent evtmp2[MAXEVENT];
	int tmp2 = 0;
	for(x =0;x< count;x++)
	{
	    if( (evttmp[x].time >= starttime) && (evttmp[x].time <= endtime) )
	    {
            memcpy(&evtmp2[tmp2], &evttmp[x], sizeof(jswevent));
            tmp2++;
	    }
	}
	if(tmp2 > 0)
	{
		sendCmdtoClient(CM_GETEVENT, 0,  tmp2,   sizeof(jswevent) * tmp2, (char*) &evtmp2);
	}
	return tmp2;

/*
	//get start st
	for (x =0;x< count;x++)
	{
		time_t tmp =   evttmp[x].time;
		sec = difftime(tmp, starttime);
		if (sec >= 0)
		{// this time value is more recent then searh start time.
			sf = x;
			break;
		}
	}

	if (sf == -1) //nothing valid in this file
	{
		printf("nothing valid in this file\n");
		return 0;
	}

	//loop thro each record and send out
	for (x =sf;x< count;x++)
	{
		time_t tmp =   evttmp[x].time;
		sec = difftime(tmp, endtime);
		if (sec >= 0)
		{// this time value is more recent then searh end time. out of bound
			if (x == sf)
			{
				return 0;
			}else
			{
				//int
				int c = (x -sf);
				int payload_len =  sizeof(jswevent) * c;
				printf("Get Event Loop, send to client %d num of event\n", c);
				sendCmdtoClient(CM_GETEVENT, 0,  c,  payload_len,(char*) &evttmp[sf]);

				return c;

			}
		}else //to the end of the list still within bound, send all, otherwise , continue to check each item
		{
			if (x == (count -1) )
			{

				int c = (count -sf);
				int payload_len =  sizeof(jswevent) * c;
				printf("Get Event Loop reach end, send to client %d num of event\n", c);
				sendCmdtoClient(CM_GETEVENT, 0,  c,  payload_len,(char*) &evttmp[sf]);

				return c;


			}
		}
	}
*/
}

int sendevtfromfile2(char *filepath, time_t starttime, time_t endtime, int total, int did, int model, int status)
{
	FILE *fp;
	jswevent evttmp[MAXEVENT];
	int count;
	int x;
	int sf=-1; //start time in file
	double sec;

	fp = fopen(filepath, "rb");
	if (fp ==NULL)
	{
		printf("evt file open fail\n");
        SetRunningLog_str("Error: sendevtfromfile2() open file failed");
		return 0;
	}

	fread(&count, 1, sizeof(int), fp);

	if (count <=0 || count > MAXEVENT)
	{
		printf("evt date count error %d\n", count);
        SetRunningLog_str_int("Error: sendevtfromfile2() invalid count", count);
		return 0;
	}

	int bread = fread(&evttmp, 1, sizeof(jswevent)* count, fp);
	fclose(fp);

	jswevent evtmp2[MAXEVENT];
	int tmp2 = 0;
	int searchevent = 0;
	for(x =0;x< count;x++)
	{
	    if( (evttmp[x].time >= starttime) && (evttmp[x].time <= endtime) )
	    {
	        searchevent = 0;
	        if(did == 0)
                searchevent++;
            else if(did == evttmp[x].did)
                searchevent++;
	        if(model == 0)
                searchevent++;
            else if(model == evttmp[x].model)
                searchevent++;
	        if(status == 0)
                searchevent++;
            else if(status == evttmp[x].status)
                searchevent++;
            if(searchevent >= 3)
            {
                memcpy(&evtmp2[tmp2], &evttmp[x], sizeof(jswevent));
                tmp2++;
            }
	    }
	}
	if(tmp2 > 0)
	{
		sendCmdtoClient(CM_GETEVENT, 0,  tmp2,   sizeof(jswevent) * tmp2, (char*) &evtmp2);
	}
	return tmp2;
}



void getevent(int cnt, char* buf, int sess)
{
	unsigned int st, et;
	int s;
    int e; //current pos in evfile
	int x;
	int sf=-1;
	int ef= -1; //start file, end file
	int pos;
	int found = 0;
	double sec;
	int evsf = -1;
	int evef = -1;
	int totalsend = 0;
	char path[1024];

	int prv = g_setting.ev.savepos-1;
	if (prv <0)
		prv = MAXEVENTFILES-1; //9

	printf("getevent start currentpos %d,  ev file count %d\n", g_setting.ev.savepos, g_setting.ev.count);


	if (cnt == 0) //get recent 100 event
	{
        pthread_mutex_lock(&getevent_mutex);

		printf("Get event without payload , return first 200");
		if (is_oldevtexist == 0)
		{
			if (evtcount >0)
			{
				int payload_len =  sizeof(jswevent) * evtcount;
				sendCmdtoClient2(CM_GETEVENT, 0,  evtcount,  payload_len,(char*) &evtlist, sess) ;
			}

		}else
		{
			int num = MAXEVENT - evtcount;
			jswevent evtmp[100];

			printf("send evt to client %d from old\n", num);

			if (num > 0)
				memcpy(&evtmp, &o_evtlist[evtcount], num * sizeof(jswevent));
			if (evtcount > 0)
				memcpy(&evtmp[num], &evtlist, evtcount * sizeof(jswevent));

			sendCmdtoClient2(CM_GETEVENT, 0,  MAXEVENT,   sizeof(jswevent) * MAXEVENT, (char*) &evtmp, sess) ;

		}

		//act ast a finish flag to client.
	    sendCmdtoClient2(CM_GETEVENT, 0,  0,  0, NULL, sess) ;

        pthread_mutex_unlock(&getevent_mutex);

		return;
	}

	memcpy(&st, buf, sizeof(int));
	memcpy(&et, buf + sizeof(int), sizeof(int));

	struct tm*	utctm = gmtime(&st);
	printf("\n### Start Time week=%d %d-%d-%d HH %d:%d current timezone %d\n\n", utctm->tm_wday,utctm->tm_year, utctm->tm_mon,utctm->tm_mday,
		utctm->tm_hour, utctm->tm_min, g_setting.timezone);
	utctm = gmtime(&et);
	printf("\n### End Time week=%d %d-%d-%d HH %d:%d current timezone %d\n\n", utctm->tm_wday,utctm->tm_year, utctm->tm_mon,utctm->tm_mday,
		utctm->tm_hour, utctm->tm_min, g_setting.timezone);

    totalsend = 0;
	//Send from ram
	jswevent evtmp2[MAXEVENT];
	int tmp2 = 0;
	for(x =0;x< evtcount;x++)
	{
	    if( (evtlist[x].time >= st) && (evtlist[x].time <= et) )
	    {
            memcpy(&evtmp2[tmp2], &evtlist[x], sizeof(jswevent));
            tmp2++;
	    }
	}
	if(tmp2 > 0)
	{
		sendCmdtoClient2(CM_GETEVENT, 0,  tmp2,   sizeof(jswevent) * tmp2, (char*) &evtmp2, sess);
printf("send event from ram =%d\n", tmp2);
		totalsend += tmp2;
	}

    //send from files
	int i, j;
	char filename2[MAXEVENTFILES][64]; //event filename, MAXEVENTFILES
	char filename3[64];
	for(i=0;i<MAXEVENTFILES;i++)
	{
	    if(strlen(g_setting5.filename[i]) > 0)
            strcpy(filename2[i], g_setting5.filename[i]);
        else
            filename2[i][0] = 0;
	}
	for(i=0;i<(MAXEVENTFILES-1);i++)
	{
        for(j=i+1;j<MAXEVENTFILES;j++)
        {
            if(strcmp(filename2[i], filename2[j]) < 0)
            {//switch
                strcpy(filename3, filename2[i]);
                strcpy(filename2[i], filename2[j]);
                strcpy(filename2[j], filename3);
            }
        }
	}
    int tmpt = 0;
	for (x =0;x<MAXEVENTFILES;x++) //100
	{
		char *nextfile = filename2[x];//g_setting5.filename[x];
		if(strlen(nextfile) != 10)
            continue;
		sprintf(path, "%sEventList/%s", sdpath, nextfile);
		if ( 0 !=  access(path, 0) )
		{
			printf("evt file not exist %s\n", path);
			continue;
		}
		tmpt =sendevtfromfile(path, st, et, totalsend);
printf("path=%s, tmpt=%d\n", path, tmpt);
		totalsend += tmpt;
printf("totalsend=%d\n", totalsend);
		if (totalsend >= MAXRETURNEVENT) //200
			break;
	}

	//close get event
printf("totalsend2=%d\n", totalsend);
	sendCmdtoClient2(CM_GETEVENT, 0,  0,  0, NULL, sess) ; //no event, or notify client that it is ended.
/*
	//find the exact time.
	//get start position
	if (g_setting.ev.count < MAXEVENTFILES) //10
	{
		s = 0;
		e = g_setting.ev.count - 1;
	}
	else
	{
		s = g_setting.ev.savepos;
		e = s - 1;
		if (e <0)
			e = MAXEVENTFILES-1; //9
	}

	//find start time

	for (x=0;x< g_setting.ev.count;x++)
	{
		pos = s + x;
		pos = pos % MAXEVENTFILES;//back to top of list of over boundary //10

		time_t tmp = (time_t)atoll( g_setting5.filename[pos] ); //g_setting.ev.filename[pos]
		utctm = gmtime(&tmp);
		printf("\n### Select Time file  week=%d %d-%d-%d HH %d:%d current filename %u\n", utctm->tm_wday,utctm->tm_year, utctm->tm_mon,utctm->tm_mday,
		utctm->tm_hour, utctm->tm_min, tmp);


		sec = difftime(tmp, st);
		if (sec >= 0)
		{// this time value is more recent then searh start time.
			sf = pos;
			if (g_setting.ev.count < MAXEVENTFILES)//move one file backward in case there are some in between record in the previous file that match the st \ et time. //10
			{
				sf --;
				if (sf <0)
					sf =0;
			}else
			{
				sf --;
				if (sf < 0)
					sf = MAXEVENTFILES-1; //9
			}
			printf("start file found , filename %u \n", tmp);
			break;
		}
	}
	if ( sf == -1)
	{
		//start time not found. beyond current time.
		//search current list.

		for (x =0;x< evtcount;x++)
		{
			time_t tmp =   evtlist[x].time;
			sec = difftime(tmp, st);
			if (sec >= 0)
			{// this time value is more recent then searh start time.
				evsf = x;
				printf("start pos found in evtlist %d.\n", evsf);
				break;
			}
		}
	}

	if ( sf == -1 && evsf == -1)
	{
		printf("No event, cannot find startime at file or evtlist.\n");
		sendCmdtoClient(CM_GETEVENT, 0,  0,  0, NULL) ; //no event, cannot find startime.
		return;

	}

	if ( sf == -1)
	{
		printf("start with what we have at ram.\n");
		for (x =evsf;x< evtcount;x++)
		{
			time_t tmp =   evtlist[x].time;
			sec = difftime(tmp, et);//compare with end time
			if (sec >= 0)
			{// this time value is more recent then searh end time. out of bound
				if (x == evsf)
				{
					sendCmdtoClient(CM_GETEVENT, 0,  0,  0, NULL) ; //no event, cannot find startime.
					return;
				}else
				{
					int c = (x -evsf);
					int payload_len =  sizeof(jswevent) * c;
					printf("evtlist middle of file, send to client total %d event\n", c);
					sendCmdtoClient(CM_GETEVENT, 0,  c,  payload_len,(char*) &evtlist[evsf]);
				}
			}else
			{
				if (evtcount-1 == x)//reach end of list
				{
					int c = (x -evsf);
					int payload_len =  sizeof(jswevent) * c;
					printf("reach end of evt list, send to client total %d of event\n", c);
					sendCmdtoClient(CM_GETEVENT, 0,  c,  payload_len,(char*) &evtlist[evsf]);
				}
			}
		}//end loop in evt list.


	}else
	{

		for (x =0;x<MAXEVENTFILES;x++) //10
		{
			int p = sf + x;
			p = p % MAXEVENTFILES; //10

			if (p > e)
				break; //reach end of list

			char *nextfile = g_setting5.filename[p]; //g_setting.ev.filename[p]

			if ( strlen(nextfile) )
			{
				//break if this file exit endtime boundary
				time_t tmp = (time_t)atoll( g_setting5.filename[p] ); //g_setting.ev.filename[p]
				sec = difftime(tmp, et);
				if (sec >= 0)
				{// this time value is more recent then searh end time.
					break;
				}

				sprintf(path, "%sEventList/%s", sdpath, nextfile);
				if ( 0 !=  access(path, 0) )
				{
					printf("evt file not exist %s\n", path);
					continue;
				}
				//parse and send file
				int tmpt =sendevtfromfile(path, st, et, totalsend);
				totalsend += tmpt;
				if (totalsend >= MAXRETURNEVENT) //200
					break;
			}
		}


		if (totalsend ==0)
		{//somehow this happen.
			printf("no event found\n");
		}else //if (totalsend < 200) get all
		{
			//get the rest on evtlist
			//int remain = 200 - totalsend;
			//printf("less then 200 on file, get the rest on evtlist.\n");
			for (x =0;x< evtcount;x++)
			{
				time_t tmp =   evtlist[x].time;
				sec = difftime(tmp, et);//compare with end time
				if (sec >= 0)
				{// this time value is more recent then searh end time. out of bound
					if (x == 0)
					{
						sendCmdtoClient(CM_GETEVENT, 0,  0,  0, NULL) ; //no more event, send 0 mark end of stream and return.
						return;
					}else
					{
						int c = x+1;
						//if ( c > remain)
						//	c = remain;
						int payload_len =  sizeof(jswevent) * c;
						printf("add from evtlist total %d event\n", c);
						sendCmdtoClient(CM_GETEVENT, 0,  c,  payload_len,(char*) &evtlist);
					}
				}else
				{
					if (evtcount-1 == x)//reach end of list
					{
						int c = evtcount;
						//if ( c > remain)
						//	c = remain;
						int payload_len =  sizeof(jswevent) * c;
						printf("reach end of evt list, send to client total %d of event\n", c);
						sendCmdtoClient(CM_GETEVENT, 0,  c,  payload_len,(char*) &evtlist);
					}
				}
			}//end loop in evt list.
		}
	}
	sendCmdtoClient(CM_GETEVENT, 0,  0,  0, NULL) ; //no event, or notify client that it is ended.
*/
}

void searchevent(int cnt, char* buf, int sess)
{
	unsigned int st, et;
	int s;
    int e; //current pos in evfile
	int x;
	int sf=-1;
	int ef= -1; //start file, end file
	int pos;
	int found = 0;
	double sec;
	int evsf = -1;
	int evef = -1;
	int totalsend = 0;
	char path[1024];

	int prv = g_setting.ev.savepos-1;
	if (prv <0)
		prv = MAXEVENTFILES-1; //9

	printf("getevent start currentpos %d,  ev file count %d\n", g_setting.ev.savepos, g_setting.ev.count);


	if (cnt == 0) //get recent 100 event, not for this command
	{
		//act ast a finish flag to client.
	    sendCmdtoClient2(CM_GETEVENT, 0,  0,  0, NULL, sess) ;

		return;
	}

	memcpy(&st, buf, sizeof(int));
	memcpy(&et, buf + sizeof(int), sizeof(int));
	int did, model, status;
	memcpy(&did, buf + sizeof(int)*2, sizeof(int));
	memcpy(&model, buf + sizeof(int)*3, sizeof(int));
	memcpy(&status, buf + sizeof(int)*4, sizeof(int));
    printf("st=%d, et=%d, did=%u, model=%d, status=%d\n", st, et, did, model, status);

	struct tm*	utctm = gmtime(&st);
	printf("\n### Start Time week=%d %d-%d-%d HH %d:%d current timezone %d\n\n", utctm->tm_wday,utctm->tm_year, utctm->tm_mon,utctm->tm_mday,
		utctm->tm_hour, utctm->tm_min, g_setting.timezone);
	utctm = gmtime(&et);
	printf("\n### End Time week=%d %d-%d-%d HH %d:%d current timezone %d\n\n", utctm->tm_wday,utctm->tm_year, utctm->tm_mon,utctm->tm_mday,
		utctm->tm_hour, utctm->tm_min, g_setting.timezone);

    totalsend = 0;
	//Send from ram
	jswevent evtmp2[MAXEVENT];
	int tmp2 = 0;
	int searchevent = 0;
	for(x =0;x< evtcount;x++)
	{
	    if( (evtlist[x].time >= st) && (evtlist[x].time <= et) )
	    {
	        searchevent = 0;
	        if(did == 0)
                searchevent++;
            else if(did == evtlist[x].did)
                searchevent++;
	        if(model == 0)
                searchevent++;
            else if(model == evtlist[x].model)
                searchevent++;
	        if(status == 0)
                searchevent++;
            else if(status == evtlist[x].status)
                searchevent++;
            if(searchevent >= 3)
            {
                memcpy(&evtmp2[tmp2], &evtlist[x], sizeof(jswevent));
                tmp2++;
            }
	    }
	}
	if(tmp2 > 0)
	{
		sendCmdtoClient2(CM_GETEVENT, 0,  tmp2,   sizeof(jswevent) * tmp2, (char*) &evtmp2, sess);
printf("send event from ram =%d\n", tmp2);
		totalsend += tmp2;
	}

    //send from files
	int i, j;
	char filename2[MAXEVENTFILES][64]; //event filename, MAXEVENTFILES
	char filename3[64];
	for(i=0;i<MAXEVENTFILES;i++)
	{
	    if(strlen(g_setting5.filename[i]) > 0)
            strcpy(filename2[i], g_setting5.filename[i]);
        else
            filename2[i][0] = 0;
	}
	for(i=0;i<(MAXEVENTFILES-1);i++)
	{
        for(j=i+1;j<MAXEVENTFILES;j++)
        {
            if(strcmp(filename2[i], filename2[j]) < 0)
            {//switch
                strcpy(filename3, filename2[i]);
                strcpy(filename2[i], filename2[j]);
                strcpy(filename2[j], filename3);
            }
        }
	}
    int tmpt = 0;
	for (x =0;x<MAXEVENTFILES;x++) //100
	{
		char *nextfile = filename2[x];//g_setting5.filename[x];
		if(strlen(nextfile) != 10)
            continue;
		sprintf(path, "%sEventList/%s", sdpath, nextfile);
		if ( 0 !=  access(path, 0) )
		{
			printf("evt file not exist %s\n", path);
			continue;
		}
		tmpt =sendevtfromfile2(path, st, et, totalsend, did, model, status);
printf("path=%s, tmpt=%d\n", path, tmpt);
		totalsend += tmpt;
printf("totalsend=%d\n", totalsend);
		if (totalsend >= MAXRETURNEVENT) //400
			break;
	}

    //close search event
printf("totalsend2=%d\n", totalsend);
	sendCmdtoClient2(CM_GETEVENT, 0,  0,  0, NULL, sess) ; //no event, or notify client that it is ended.
}

void clearevent()
{
	//clear event setting
	//g_setting.ev.savepos = 0;
	//g_setting.ev.count = 0;

	memset(&g_setting.ev, 0, sizeof(g_setting.ev));
	//memset(g_setting5.filename, 0, sizeof(g_setting5.filename));
	int i;
	for(i=0;i<MAXEVENTFILES;i++)
	{
		memset(g_setting5.filename[i], 0, sizeof(g_setting5.filename[i]));
	}

	evtcount = 0;
	is_oldevtexist = 0;

	memset(evtlist, 0, sizeof(evtlist));
	memset(o_evtlist, 0, sizeof(o_evtlist));

	savesetting();
	savesetting5();

	//clear event files
	delTmpEvdata(1);
	delTmpEvdata(2);
	delAllEvdata();//to o_evtlist
}



void ipc_motionEvent(int did)
{
	//int ist
	jswdev *dev;
	char msg[2000];

	dev = getDevbydid(did);
	if (dev ==NULL)
	{
        SetRunningLog_str_int("Error: ipc_motionEvent() did not found", did);
		return;
	}

	if (g_armstate == st_partarm || st_arm == g_armstate )
	{
		if (checkDevFlag(did) )
		{
			newEvent(did, TYPE_CAMERA, DEV_TRIGGER, 0);

			memset(g_entrydelay_msg, 0, sizeof(g_entrydelay_msg));
			if (g_sirenstate==0)
				entryDelay(did);

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
	}else if  (g_armstate == st_disarm)
	{
		//newEvent(did, TYPE_CAMERA, 0x01, 0);//simply motion event. not trigger.
	}

	startScenario(did,  0x01 );


}

void sdtest(int sess)
{
	char path[1024];
	FILE *fp;
	char buf[128];
	char *str = "testing sd card read write";

	if (0 == access("/var/run/run_in_flash", 0))
	{//file exist
	    printf("====run in flash====\n");
		sendCmdtoClient2(CM_TESTSD, 4, 0, 0, 0, sess);
		return;
	}else
	{//file is not exist
	    printf("====run in SD====\n");
	}

	sprintf(path, "%ssdtest", sdpath);
	printf("sd test path %s\n", path);

	fp = fopen(path, "wb+");
	if (fp ==NULL)
	{
		printf("sd test path open fail\n");

		sendCmdtoClient2(CM_TESTSD, 4, 0, 0, 0, sess);
		return;
	}


	fwrite(str, strlen(str), 1, fp);
	fclose(fp);

	fp = fopen(path, "rb");
	if (fp ==NULL)
	{
		sendCmdtoClient2(CM_TESTSD, 4, 0, 0, 0, sess);
		return;
	}

	memset(buf, 0 ,sizeof(buf));
	int r = fread(buf, 1, 128, fp);
	if (r <= 0)
	{
		sendCmdtoClient2(CM_TESTSD, 4, 0, 0, 0, sess);
		return;
	}

	printf("read data %s, write data %s\n", buf, str);

	if (strcmp(str, buf) != 0 )
	{
		printf("sd card test string not equal\n");
		sendCmdtoClient2(CM_TESTSD, 4, 0, 0, 0, sess);
	}else
	{
		printf("sd card test OK\n");
		sendCmdtoClient2(CM_TESTSD, 0, 0, 0, 0, sess);
	}
}

void writegatewayinfo(char *buf)
{
	int x;
	char str[4096];
	struct gatewayinfodb *db;

	db = (struct gatewayinfodb *) buf;

	if (strlen(db->did) == 0 || strlen(db->license) ==0 || strlen(db->checksum) == 0)
	{
		sendCmdtoClient(CM_WRITEDID, 4, 0, 0, 0);
		return;
	}

	memset(str, 0, sizeof(str));
	sprintf(str, "[\"initstring\"] = \"ECGBFFBJKAIEGHJAEBHLFGEMHLNBHCNIGEFCBNCIBIJALMLFCFAPCHODHOLCJNKIBIMCLDCNOBMOAKDMJGNMIJBJML\";\n[\"password\"] = \"123456\";\n[\"did\"] = \"%s\";\n[\"license\"] = \"%s\";\n[\"apichecksum\"] = \"%s\";\n", db->did, db->license, db->checksum);

	remove("/mnt/gatewayinfoEx.db");

	FILE *fp;
	fp = fopen("/mnt/gatewayinfoEx.db", "wb+");

	if (fp ==NULL)
	{
		sendCmdtoClient(CM_WRITEDID, 4, 0, 0, 0);
		return ;
	}

	fwrite(str, strlen(str), 1, fp);
	fclose(fp);

	remove("/mnt/setting3.db");
	sendCmdtoClient(CM_WRITEDID, 0, 0, 0, 0);

	//for (x=0;x<20;x++)
	//	usleep(1000);

	//system("sync");
	//system("reboot");
	//exit(1);



}


void checkfactoryreset()
{
	int index, num = 0,mode = 0, val = 1;
	static int count = 0;
	gpio_pin_info info;
	char *dhcpstr = "auto lo\niface lo inet loopback\n\nauto eth0\niface eth0 inet dhcp\n";


	if (g_sirenstate == 1 || g_armstate != st_disarm )
		return;

	snx_gpio_open();

    info.pinumber = GPIO_PIN_1;
    info.mode = 0;
    info.value = -1;
    if(snx_gpio_write(info) == GPIO_FAIL)
	{
      printf ("write gpio%d error\n",num);
	 // yellowled(1);
	  return;
	}
    if(mode == 0)
    {
      int r = snx_gpio_read(&info);
	   if (r == GPIO_FAIL)
	   {
			//yellowled(1);
			return;

	   }
     // printf ("%d\n",info.value);

    }

	if (info.value == 1)
		 count = 0; //redled(1);
	else
	if (info.value == 0)
	{
		//set acct pwd
	/*	count ++;
		if (count < 3)
		{
			snx_gpio_close();
			return;
		}
		count = 0;*/
		system("/usr/sbin/buzzer_beep 100 300 0 1&");
		usleep(1000);

//		strcpy( g_setting.acct.adminpwd, "123456");
//		strcpy( g_setting.acct.loginpwd, "123456");
//		g_setting.network.dhcp = 1;
//
//		FILE *fp;
//		remove("/etc/network/interfaces");
//		fp = fopen("/etc/network/interfaces", "wb+");
//		if (fp != NULL)
//		{
//			fwrite(dhcpstr, strlen(dhcpstr), 1, fp);
//		}
//		fclose(fp);
//
//		savesetting();

		snx_gpio_close();

		factoryReset(0);

		system("killall -9 buzzer_beep");

		system("sync");
		system("reboot");
		exit(1);

	}

	snx_gpio_close();

	//info.pinumber = GPIO_PIN_1;
 //   info.mode = 0;


 //    int r =  snx_gpio_read(&info);
 //    // printf ("%d\n",info.value);
 //    // return info.value;
	// if (r == GPIO_FAIL)
	// {
	//	 blueled(0);
	//	 return;
	// }

	//  if (info.value == 1)
	//	  redled(1);
	//  else if (info.value == 0)
	//	  greenled(1);





}

void rftxTest()
{

	//makeTimer( &writeack_t, 0, 0);
	//writeack_t = 0;
	writePairing(0x12345678, 0, 0xFF);
	usleep(5000);

	makeTimer( &rftx_t, 2, 0);

	//writeCommand(0x12345678, TYPE_POWER_ADAPTER, CMD_ADAPTER_TURNON);

	//usleep(1000);

	//makeTimer(&rftest_t, 2, 0);


	//RFModuleWrite( g_DID, 0, 0xFF);
	////if(PLATFORM == "SN98601")		//SN98601's UART is queer that need write 2 times....
	//RFModuleWrite( g_DID, 0, 0xFF);


}

//encrypt P2P package by AES key, and then send it
//DataBuf: fully p2p package, include head and payload
//DataSizeToWrite: length of fully p2p package
int PPPP_Write2(INT32 SessionHandle, UCHAR Channel, CHAR *DataBuf, INT32 DataSizeToWrite, int is_encrypt, char *aeskey)
{

    char buf[MAX_P2P_BUFFER];
    struct _jswproto_hdr hdr, *phdr;
    int outsize;
    int ret = -1;

    if(DataSizeToWrite > (MAX_P2P_BUFFER-sizeof(struct _jswproto_hdr)))
    {
        printf("PPPP_Write2 error: buffer overflow!, size=%d\n", DataSizeToWrite);
        return ret;
    }

    phdr = (struct _jswproto_hdr *)DataBuf;

    memset(&hdr, 0, sizeof(struct _jswproto_hdr));
    hdr.version = phdr->version;
    hdr.cmd = CM_DATA_ENCRYPT;//phdr->cmd;
    hdr.count = phdr->count;
    hdr.errCode = phdr->errCode;

    memset(buf, 0, sizeof(buf));
    if(is_encrypt == 0)
    {
        hdr.payload_length = DataSizeToWrite;
        memcpy(buf, &hdr, sizeof(struct _jswproto_hdr));
        memcpy(buf+sizeof(struct _jswproto_hdr), DataBuf, DataSizeToWrite);
        ret = PPPP_Write(SessionHandle, Channel, buf, sizeof(struct _jswproto_hdr)+DataSizeToWrite);
    }else
    {
        if( (aeskey == NULL) || (strlen(aeskey) <= 0) )
        {
            printf("PPPP_Write2 error: AES key is NULL!\n");
            return ret;
        }
        pthread_mutex_lock(&aes_mutex);
        int outSize;
        char *encryptStr = encryptAES2((unsigned char *) (DataBuf), DataSizeToWrite, &outSize, aeskey);
        pthread_mutex_unlock(&aes_mutex);
        if(encryptStr == NULL)
        {
            printf("PPPP_Write2 error: encryptAES failed!, aeskey=%s\n", aeskey);
            return ret;
        }else
        {
            hdr.payload_length = outSize;
            memcpy(buf, &hdr, sizeof(struct _jswproto_hdr));
            memcpy(buf+sizeof(struct _jswproto_hdr), encryptStr, outSize);
            ret = PPPP_Write(SessionHandle, Channel, buf, sizeof(struct _jswproto_hdr)+outSize);
        }
    }

    return ret;
}

//encrypt P2P package by AES key
//srcBuf: payload encrypted by AES key (include fully p2p package)
//DataSize: size of srcBuf
//destBuf: the buffer to accept decrypted payload (fully p2p package)
int PPPP_Read_Wrap2(char *srcBuf, int DataSize, char *destBuf, int is_encrypt, char *aeskey)
{
    if(is_encrypt == 0)
    {
        memcpy(destBuf, srcBuf, DataSize);
        destBuf[DataSize] = 0;
    }else
    {
        if( (aeskey == NULL) || (strlen(aeskey) <= 0) )
        {
            printf("PPPP_Read_Wrap2 error: no AES key!\n");
            return -1;
        }
        pthread_mutex_lock(&aes_mutex);
        char *decryptStr = decryptAES((unsigned char *) (srcBuf), DataSize, aeskey);
        pthread_mutex_unlock(&aes_mutex);
        if(decryptStr == NULL)
        {
            printf("PPPP_Read_Wrap2 error: decryptAES failed! aeskey=%s!\n", aeskey);
            return -1;
        }
        memcpy(destBuf, decryptStr, DataSize);
        destBuf[DataSize] = 0;
    }

    return 0;
}

//orderless single hash code
int gen_hashcode3(int code, int cc, int index, int max)
{
    switch(index)
    {
    case 0:
        code = (code * (index+cc+13) *6 + 729123 - 67);
        break;
    case 1:
        code = (code + 8146124 - cc*35);
        break;
    case 2:
        code = (code * 153 + cc*739 + 82912362);
        break;
    case 3:
        code = (code * (index + 33 + cc)*13 / 4 + 523283 - 7);
        break;
    case 4:
        code = (code + 18732932 + cc*732);
        break;
    case 5:
        code = (code * 621752+ cc*789 + 30763282);
        break;
    case 6:
        code = (code * 369 + 876*cc + 833298);
        break;
    case 7:
        code = (code * index + cc*73649 + 2716318 - 444);
        break;
    case 8:
        code = (code + 10987653 + cc*3);
        break;
    case 9:
        code = (code * 788349 - (index*5*cc) + 39812442);
        break;
    case 10:
        code = (code * (index+89+cc) * 61 + 821023 - 555);
        break;
    case 11:
        code = (code + 87631592 + cc*2791);
        break;
    case 12:
        code = (code + (index*731984+cc*15243) + 7772991);
        break;
    case 13:
        code = (code * (index+1533+cc*6753) / 35 + 731927);
        break;
    case 14:
        code = (code + 61829443 - (index*77*cc));
        break;
    case 15:
        code = (code * (index+7) * 45*cc + 399);
        break;
    }
    if(code < 0)
    {
        printf("Hash code error: code=%d, index=%d, cc=%d\n", code, index, cc);
        code = 0-code;
    }
    return code % max;
}

//orderless all hash code
void gen_hashcode2(char *text, int cc, int index, int max)
{
    switch(index)
    {
    case 0:
        text[0] = gen_hashcode3(text[0], cc, 0, max);
        text[1] = gen_hashcode3(text[1], cc, 1, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 3, max);
        text[4] = gen_hashcode3(text[4], cc, 4, max);
        text[5] = gen_hashcode3(text[5], cc, 5, max);
        text[6] = gen_hashcode3(text[6], cc, 6, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 9, max);
        text[10] = gen_hashcode3(text[10], cc, 10, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 12, max);
        text[13] = gen_hashcode3(text[13], cc, 13, max);
        text[14] = gen_hashcode3(text[14], cc, 14, max);
        text[15] = gen_hashcode3(text[15], cc, 15, max);
        break;
    case 1:
        text[0] = gen_hashcode3(text[0], cc, 0, max);
        text[1] = gen_hashcode3(text[1], cc, 1, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 4, max);
        text[5] = gen_hashcode3(text[5], cc, 15, max);
        text[6] = gen_hashcode3(text[6], cc, 6, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 9, max);
        text[10] = gen_hashcode3(text[10], cc, 10, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 12, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 14, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 2:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 1, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 4, max);
        text[5] = gen_hashcode3(text[5], cc, 15, max);
        text[6] = gen_hashcode3(text[6], cc, 6, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 9, max);
        text[10] = gen_hashcode3(text[10], cc, 0, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 12, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 14, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 3:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 1, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 12, max);
        text[6] = gen_hashcode3(text[6], cc, 6, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 9, max);
        text[10] = gen_hashcode3(text[10], cc, 0, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 15, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 4, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 4:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 1, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 12, max);
        text[6] = gen_hashcode3(text[6], cc, 9, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 6, max);
        text[10] = gen_hashcode3(text[10], cc, 0, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 15, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 4, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 5:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 1, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 12, max);
        text[6] = gen_hashcode3(text[6], cc, 9, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 6, max);
        text[10] = gen_hashcode3(text[10], cc, 0, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 7, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 4, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 6:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 12, max);
        text[6] = gen_hashcode3(text[6], cc, 9, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 6, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 7, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 4, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 7:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 9, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 6, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 7, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 4, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 8:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 13, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 6, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 7, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 9:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 6, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 7, max);
        text[13] = gen_hashcode3(text[13], cc, 3, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 10:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 3, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 8, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 7, max);
        text[13] = gen_hashcode3(text[13], cc, 6, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 11:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 2, max);
        text[3] = gen_hashcode3(text[3], cc, 3, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 7, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 8, max);
        text[13] = gen_hashcode3(text[13], cc, 6, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 12:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 6, max);
        text[3] = gen_hashcode3(text[3], cc, 3, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 15, max);
        text[8] = gen_hashcode3(text[8], cc, 7, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 8, max);
        text[13] = gen_hashcode3(text[13], cc, 2, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 13:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 6, max);
        text[3] = gen_hashcode3(text[3], cc, 3, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 15, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 11, max);
        text[12] = gen_hashcode3(text[12], cc, 8, max);
        text[13] = gen_hashcode3(text[13], cc, 2, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 14:
        text[0] = gen_hashcode3(text[0], cc, 10, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 6, max);
        text[3] = gen_hashcode3(text[3], cc, 11, max);
        text[4] = gen_hashcode3(text[4], cc, 14, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 12, max);
        text[7] = gen_hashcode3(text[7], cc, 7, max);
        text[8] = gen_hashcode3(text[8], cc, 15, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 3, max);
        text[12] = gen_hashcode3(text[12], cc, 8, max);
        text[13] = gen_hashcode3(text[13], cc, 2, max);
        text[14] = gen_hashcode3(text[14], cc, 9, max);
        text[15] = gen_hashcode3(text[15], cc, 5, max);
        break;
    case 15:
        text[0] = gen_hashcode3(text[0], cc, 9, max);
        text[1] = gen_hashcode3(text[1], cc, 0, max);
        text[2] = gen_hashcode3(text[2], cc, 5, max);
        text[3] = gen_hashcode3(text[3], cc, 11, max);
        text[4] = gen_hashcode3(text[4], cc, 3, max);
        text[5] = gen_hashcode3(text[5], cc, 4, max);
        text[6] = gen_hashcode3(text[6], cc, 7, max);
        text[7] = gen_hashcode3(text[7], cc, 12, max);
        text[8] = gen_hashcode3(text[8], cc, 15, max);
        text[9] = gen_hashcode3(text[9], cc, 13, max);
        text[10] = gen_hashcode3(text[10], cc, 1, max);
        text[11] = gen_hashcode3(text[11], cc, 2, max);
        text[12] = gen_hashcode3(text[12], cc, 6, max);
        text[13] = gen_hashcode3(text[13], cc, 14, max);
        text[14] = gen_hashcode3(text[14], cc, 10, max);
        text[15] = gen_hashcode3(text[15], cc, 8, max);
        break;
    }
}

//generate hash code(16 bytes) by str1(DID) and str2(password), and put the hash code to outBuf
//return 0: successful, else fail
int gen_hashcode(char *str1, char *str2, char *outBuf)
{
    int i;
    char base_text[] = "012m347D89ABCEFGHIJhKLMNO6PQSTzUVwWXYZabcdefgijkl5nopRqrstuvxy";
    int base_text_len = 62;//sizeof(basetext);
    char out_text[16] = "dsW3c5g7P902Ka5X";

    if( (strlen(str1) <= 9) || (strlen(str1) > 64) )
    {
        printf("Str1's length must be bigger than 9!\n");
        return -1;
    }
    if( (strlen(str2) <= 0) || (strlen(str2) > 64) )
    {
        printf("Str2's length must be bigger than 0!\n");
        return -1;
    }

    for(i=0;i<strlen(str1);i++)
    {
        gen_hashcode2(out_text, str1[i], i, base_text_len);
    }
    for(i=0;i<strlen(str2);i++)
    {
        gen_hashcode2(out_text, str2[i], i, base_text_len);
    }

    for(i=0;i<16;i++)
    {
        out_text[i] = base_text[(int)out_text[i]];
        outBuf[i] = out_text[i];
    }

    return 0;
}

void log_and_reboot_no_beep(char *msg, int reboot)
{
    char cmd2[256];
    sprintf(cmd2, "date > %s", MCU_REBOOT_FILE);
    system(cmd2);

    sprintf(cmd2, "echo %s >> %s.log", msg, MCU_REBOOT_FILE);
    system(cmd2);

    sprintf(cmd2, "date >> %s.log", MCU_REBOOT_FILE);
    system(cmd2);
    sprintf(cmd2, "echo >> %s.log", MCU_REBOOT_FILE);
    system(cmd2);

    if(reboot > 0)
    {
        saveTmpEvdata(2);
        sync();
        system("reboot");
    }
}

void log_remote_access(char *msg)
{
    char cmd2[256];
    struct tm *utctm;
    time_t now = time(NULL);

    if( (g_setting.timezone != 0) || (g_setting4.extra.DST_enable != 0) )
    {
        now += (g_setting.timezone*60*60);
        if(g_setting4.extra.DST_enable != 0)
            now += (1*60*60);
    }
    utctm = gmtime(&now);

    sprintf(cmd2, "echo %04d/%02d/%02d %02d:%02d:%02d >> %s",
        utctm->tm_year+1900, utctm->tm_mon+1, utctm->tm_mday, utctm->tm_hour, utctm->tm_min, utctm->tm_sec,
        REMOTE_ACCESS_LOG_FILE);
    system(cmd2);
    sprintf(cmd2, "echo %s >> %s", msg, REMOTE_ACCESS_LOG_FILE);
    system(cmd2);
    sprintf(cmd2, "echo >> %s", REMOTE_ACCESS_LOG_FILE);
    system(cmd2);

    sync();
}


//----for battery low list----
void load_batterylow()
{
    g_battlowcount = 0;

	FILE *fp;
	fp = fopen("/mnt/batterylow.db", "rb");
	if (fp ==NULL)
	{
		DBG_PRINT("battery low data fail to open\n");
        SetRunningLog_str("Error: load_batterylow() open file failed");
		return;
	}
	int bread = fread(&g_battlowcount, 1, sizeof(g_battlowcount), fp);
	bread = fread(&g_battlowlist, 1, sizeof(g_battlowlist), fp);
	fclose(fp);

	if( (g_battlowcount < 0) || (g_battlowcount > MAXDEVICENUM) )
        g_battlowcount = 0;

    if(g_battlowcount > 0)
        check_battlow();
}

void save_batterylow()
{
	FILE *fp;
	int n = 1;
	fp = fopen("/mnt/batterylow.db", "wb");

	if (fp == NULL)
	{
		//DBG_PRINT("save data fail to open\n");
        SetRunningLog_str("Error: save_batterylow() open file failed");
		return;
	}

	fwrite(&g_battlowcount, sizeof(g_battlowcount) , 1, fp);
	fwrite(&g_battlowlist, sizeof(g_battlowlist) , 1, fp);

	fclose(fp);
	sync();
}

//add one list to battlowlist
void add_battlow(unsigned int did, int model)
{
    int i, found;
    found = 0;
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if(did != g_battlowlist[i].did)
            continue;
        found++;
    }

    if(found == 0)
    {//not found
        if(g_battlowcount >= MAXDEVICENUM)
            kill_oldest_battlow();
        if(g_battlowcount < MAXDEVICENUM)
        {
            g_battlowlist[g_battlowcount].did = did;
            g_battlowlist[g_battlowcount].model = model;
            g_battlowlist[g_battlowcount].time = time(NULL);
            g_battlowcount++;
            if(g_battlowcount > MAXDEVICENUM)
                g_battlowcount = MAXDEVICENUM;

            save_batterylow();
        }
    }
}

//remove one list from battlowlist
//return 1: remove successfully, return 0: remove failed
int remove_battlow(unsigned int did)
{
    int i, j;
    j = 0;
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if(did != g_battlowlist[i].did)
            continue;
        memset(&g_battlowlist[i], 0, sizeof(g_battlowlist[i]));
        j++;
        //break;
    }
    if(j > 0)
    {
        sort_out_battlow();
        if(g_battlowcount <= 0)
            if(getMAGOpenCount() <= 0)
                ledoff();
        return 1;
    }else
        return 0;
}

//kill the oldest list from battlowlist
void kill_oldest_battlow(void)
{
    int i;
    int j = -1;
    unsigned int oldest_time = 0xffffffff;
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if( (g_battlowlist[i].time > 0) && (g_battlowlist[i].time < oldest_time) )
        {
            j = i;
            oldest_time = g_battlowlist[i].time;
        }
    }

    if(j >= 0)
    {//found, kill one list
        memset(&g_battlowlist[j], 0, sizeof(g_battlowlist[j]));
        sort_out_battlow();
        //recount_battlow();
    }
}

//sort out battlowlist
void sort_out_battlow()
{
    int i;
    for(i=0;i<(MAXDEVICENUM-1);i++)
    {
        if( (g_battlowlist[i].did == 0) && (g_battlowlist[i+1].did != 0) )
        {
            memcpy(&g_battlowlist[i], &g_battlowlist[i+1], sizeof(g_battlowlist[i]));
            memset(&g_battlowlist[i+1], 0, sizeof(g_battlowlist[i+1]));
        }
    }
    recount_battlow();
}

//recount battlowlist and set to g_battlowcount
void recount_battlow()
{
    int i, j;
    g_battlowcount = 0;
    j = 0;
    for(i=0;i<MAXDEVICENUM;i++)
    {
        if(g_battlowlist[i].did == 0)
        {
            g_battlowcount = i;
            j = 1;
            break;
        }
    }
    if(j == 0)
    {//full
        g_battlowcount = MAXDEVICENUM;
    }

    save_batterylow();
}

//check the battery list is valid or not, and delete the invalid item
void check_battlow()
{
	if( (g_battlowcount >= 0) && (g_battlowcount < MAXDEVICENUM) && (devcount >= 0) && (devcount < MAXDEVICENUM) )
	{
	    if(devcount == 0)
	    {
            memset(g_battlowlist, 0, sizeof(g_battlowlist));
            g_battlowcount = 0;
	    }else
	    {
            int i, j, found;
            for(i=0;i<g_battlowcount;i++)
            {
                found = 0;
                for(j=0;j<devcount;j++)
                {
                    if(devlist[j].did != 0)
                    {
                        if(g_battlowlist[i].did == devlist[j].did)
                        {
                            found = 1;
                            break;
                        }
                    }else
                    {
                        break;
                    }
                }
                if(found == 0)
                {//not found, delete from battery low list
                    g_battlowlist[i].did = 0;
                    sort_out_battlow();
                    i = 0;
                }
            }
        }
	}
}

//show all battlowlist, just for test
//void show_battlow()
//{
//printf("---cclccl--- g_battlowcount=%d \n", g_battlowcount);
//    int i;
//    for(i=0;i<MAXDEVICENUM;i++)
//    {
//        if(g_battlowlist[i].did != 0)
//        {
//printf("---cclccl--- i=%d, did=%u, time=%u \n", i, g_battlowlist[i].did, g_battlowlist[i].time);
//        }else
//            break;
//    }
//}

//return count of mag open
int getMAGOpenCount(void)
{
	int x, ret = 0;
	memset(mag_open_list,0x00,sizeof(mag_open_list));
	for (x=0;x<devcount;x++)
	{
		if( (devlist[x].model == TYPE_MAGNETIC) && (devlist[x].status == RE_MAG_ISON) )
			{
			strcpy(mag_open_list[ret],devlist[x].name);
			ret++;
			}
	}
	return ret;
}

int send_ARM_abnormal_notify()
{
	char msg[1024];
	int MAGOpenCount;
	memset(msg, 0, sizeof(msg));
	MAGOpenCount = getMAGOpenCount();
	if(MAGOpenCount <1 ) return 0;

#ifdef DEF_FEATURE_MULTI_PUSH
		char msg2[256];
		memset(msg2, 0, sizeof(msg2));

		if(MAGOpenCount > 0)
		{
			strcpy(msg2,ARM_ABNORMAL2);
			sprintf( msg, msg2, g_setting.gwprop.name);
			//strcat(msg,ARM_ABNORMAL2);
			int i;
			strcat(msg," [");
			for(i = 0 ; i < MAGOpenCount ; i++)
			{
				if(i > 4) {strcat(msg,"......"); break;}
				if(i > 0) strcat(msg,", ");
				strcat(msg,mag_open_list[i]);
			}
			strcat(msg," ]");
		}


#else
		sprintf( msg, "Your SHC_pro System %s SET ARM but MAG open.", gDID);
#endif

	if (strlen(msg) != 0)
		pushnotification(NT_DISARM, msg, STR_ID_ALARM);

}

//parse string id from whole push string(multi-language)
//return 0 if failed
int get_str_id(char *msg)
{
    char *c1 = NULL;
    c1 = strstr(msg, PUSH_STRING_GAP);
    if(c1)
    {
        char buf2[512];
        memset(buf2, 0, sizeof(buf2));
        memcpy(buf2, msg, c1-msg);
        return atoi(buf2);
    }else
        return 0;
}

//parse push string by language id
void get_str_by_lang_id(char *msg, char *buf2)
{
    int count2 = 0;
    char *c1 = NULL, *c2 = NULL;
    c1 = strstr(msg, PUSH_STRING_GAP);
    c2 = strstr(c1+1, PUSH_STRING_GAP);
    while(c1 && c2)
    {
        count2++;
        char buf3[512];
        memcpy(buf3, c1, c2-c1);
        if(count2 >= g_setting4.extra.push_string_language_id)
        {
            memcpy(buf2, c1+strlen(PUSH_STRING_GAP), c2-c1-strlen(PUSH_STRING_GAP));
            return;
        }
        c1 = c2;
        c2 = strstr(c1+1, PUSH_STRING_GAP);
    }

    buf2[0] = 0;
    return 0;
}

//get push string by language id and string id
void get_push_string(char *msg, int str_id)
{
    if( (g_setting4.extra.push_string_language_id < LANG_ID_GERMAN) || (g_setting4.extra.push_string_language_id >= LANG_ID_LAST) )
        g_setting4.extra.push_string_language_id = DEF_PUSH_LANGUAGE;

    FILE *fp = NULL;
    fp = fopen(PUSH_STRING_CFG_FILE, "r+");
    if(fp == NULL)
    {
        strcpy(msg, DEF_ERROR_PUSH_STRING);
        return;
    }

    char buf[2048];
    char buf2[512];
    int str_id2 = 0;
    int got_str = 0;
    while(!feof(fp))
    {
        fgets(buf, sizeof(buf), fp);
        if(strlen(buf) > 0)
        {
            str_id2 = 0;
            if( (buf[0] != '/') && (buf[1] != '/') )
            {
                str_id2 = get_str_id(buf);
                if( (str_id2 > 0) && (str_id2 < STR_ID_LAST) && (str_id2 == str_id) )
                {
                    memset(buf2, 0, sizeof(buf2));
                    get_str_by_lang_id(buf, buf2);
                    if(strlen(buf2) > 0)
                    {
                        got_str = 1;
                        strcpy(msg, buf2);
                    }else
                        strcpy(msg, DEF_ERROR_PUSH_STRING);

                    break;
                }
            }
        }
    }
    fclose(fp);

    if(got_str <= 0)
    {
        strcpy(msg, DEF_ERROR_PUSH_STRING);
    }

    return;
}

//trigger GW to alarm as doorbell
void call_buzzer_doorbell(void)
{
    system(". ./doorbell.sh&");
}




//sunrise / sunset
//get json value by key
char *get_json_value(char *key, char* json, char *value)
{
	char *p = json;
	char *st, *et;

    //find key
	st = strstr(p, key);
	if (st == NULL)
	{
	    value[0] = 0;
		return NULL;
	}

    p = st + strlen(key);
	st = strstr(p, "\"");
	if (st == NULL)
	{
	    value[0] = 0;
		return NULL;
	}

    p += 1;
	st = strstr(p, "\"");
	if (st == NULL)
	{
	    value[0] = 0;
		return NULL;
	}

    //p += 1;
	et = strstr(st+1, "\"");
	if (et == NULL)
	{
	    value[0] = 0;
		return NULL;
	}

    int len = et - st -1;
    if( (len >= 0) && (len < 20000) )
    {
        memcpy(value, st+1, et - st -1);
        value[len] = 0;
    }else
        SetRunningLog_str_int("Error: get_json_value() buffer overflow", len);

	return st+1;
}

//get json value(int) by key
char *get_json_value_int(char *key, char* json, char *value)
{
	char *p = json;
	char *st, *et;

    //find key
	st = strstr(p, key);
	if (st == NULL)
	{
	    value[0] = 0;
		return NULL;
	}

    p = st + strlen(key);
	st = strstr(p, ":");
	if (st == NULL)
	{
	    value[0] = 0;
		return NULL;
	}
	st += 1;

	et = strstr(st, ",");
	if (et == NULL)
	{
        et = strstr(st, "}");
        if (et == NULL)
        {
            et = strstr(st, "]");
            if (et == NULL)
            {
                value[0] = 0;
                return NULL;
            }
        }
	}

    int len = et - st;
    if( (len >= 0) && (len < 20000) )
    {
        memcpy(value, st, len);
        value[len] = 0;
        char *ch1;
        ch1 = strstr(value, ",");
        if(ch1)
            *ch1 = 0;
        ch1 = strstr(value, "}");
        if(ch1)
            *ch1 = 0;
        ch1 = strstr(value, "]");
        if(ch1)
            *ch1 = 0;
//        if(len > 1)
//            if( (value[len-1] == '}') || (value[len-1] == ']') )
//                value[len-1] = 0;
    }else
        SetRunningLog_str_int("Error: get_json_value_int() buffer overflow", len);

	return st;
}

//adjust date/time
void adjust_date(int *year, int *mon, int *day, int *hour)
{
    //hour
    if((*hour) > 24)
    {
        (*hour) -= 24;
        (*day) ++;
    }else if((*hour) < 0)
    {
        (*hour) += 24;
        (*day) --;
    }

    //day
    int mdays = 31;
    if( ((*mon) == 1) || ((*mon) == 3) || ((*mon) == 5) || ((*mon) == 7) || ((*mon) == 8) || ((*mon) == 10) || ((*mon) == 12) )
        mdays = 31;
    else if( ((*mon) == 4) || ((*mon) == 6) || ((*mon) == 9) || ((*mon) == 11) )
        mdays = 30;
    else if((*mon) == 2)
    {
        if(((*year) % 400) == 0)
            mdays = 29;
        else if(((*year) % 100) == 0)
            mdays = 28;
        else if(((*year) % 4) == 0)
            mdays = 29;
        else
            mdays = 28;
    }
    if((*day) > mdays)
    {
        (*day) -= mdays;
        (*mon) ++;
    }else if((*day) < 0)
    {
        (*day) += mdays;
        (*mon) --;
    }

    //mon
    if((*mon) > 12)
    {
        (*mon) -= 12;
        (*year) ++;
    }else if((*mon) < 0)
    {
        (*mon) += 12;
        (*year) --;
    }

    return;
}

//parse json value to struct tm
//return: parse successfully, else parse failed
int parse_json_value_to_tm(char *value, struct tm *t2)
{
    char value2[256], value3[256];
    char *ch1 = NULL;

    memset(t2, 0, sizeof(struct tm));
    strcpy(value2, value);
    ch1 = strstr(value2, "T");
    if(ch1 && (value2[0] == '2') && (value2[4] == '-') && (value2[7] == '-') )
    {//valid
        //year
        memset(value3, 0, sizeof(value3));
        memcpy(value3, &value2[0], 4);
        t2->tm_year = atoi(value3);
        //mon
        memset(value3, 0, sizeof(value3));
        memcpy(value3, &value2[5], 2);
        t2->tm_mon = atoi(value3);
        //day
        memset(value3, 0, sizeof(value3));
        memcpy(value3, &value2[8], 2);
        t2->tm_mday = atoi(value3);
        //hour
        memset(value3, 0, sizeof(value3));
        memcpy(value3, ch1+1, 2);
        t2->tm_hour = atoi(value3);
        //min
        memset(value3, 0, sizeof(value3));
        memcpy(value3, ch1+4, 2);
        t2->tm_min = atoi(value3);
        //sec
        memset(value3, 0, sizeof(value3));
        memcpy(value3, ch1+7, 2);
        t2->tm_sec = atoi(value3);

        if( (g_setting.timezone != 0) || (g_setting4.extra.DST_enable != 0) )
        {
            int hh = 0;
            hh += g_setting.timezone;
            if(g_setting4.extra.DST_enable != 0)
                hh += 1;
            t2->tm_hour += hh;
            adjust_date(&t2->tm_year, &t2->tm_mon, &t2->tm_mday, &t2->tm_hour);
        }

        return 1;
    }
    return 0;
}

//get sunrise/sunset data one day
void get_single_sunrise_sunset_data(int year, int mon, int day, jswsunrisesunset *data)
{
    char url[256]; //, output[1024];
//    if( (g_setting4.latlng.lat == 0) && (g_setting4.latlng.lng == 0) )
//    {
//        g_setting4.latlng.lat = 36.7201600;
//        g_setting4.latlng.lng = -4.4203400;
//    }
    sprintf(url, "http://api.sunrise-sunset.org/json?lat=%.7f&lng=%.7f&formatted=0&date=%04d-%02d-%02d",
        g_setting4.latlng.lat, g_setting4.latlng.lng, year, mon, day);
printf("url=%s\n", url);
    g_curl_output[0] = 0;
    pthread_mutex_lock(&curl_mutex);
    curl_get(url, "", g_curl_output);
    pthread_mutex_unlock(&curl_mutex);
    if(strlen(g_curl_output) > 0)
    {//valid
        char value[256];

        //parse sunrise
        value[0] = 0;
        get_json_value("sunrise", g_curl_output, value);
        if(strlen(value) > 0)
        {//valid
            parse_json_value_to_tm(value, &data->sunrise);
        }

        //parse sunset
        value[0] = 0;
        get_json_value("sunset", g_curl_output, value);
        if(strlen(value) > 0)
        {//valid
            parse_json_value_to_tm(value, &data->sunset);
        }
    }
}

//get and re-flash sunrise/sunset data
void get_sunrise_sunset_data()
{
    if(previous_net_status != 0)
        return;

    if( (g_setting4.latlng.lat < -360) || (g_setting4.latlng.lat > 360) ||
        (g_setting4.latlng.lng < -360) || (g_setting4.latlng.lng > 360) )
    {
        printf("Invalid latlng, return!!! lat=%f, lng=%f\n", g_setting4.latlng.lat, g_setting4.latlng.lng);
        return;
    }

    time_t now = time(NULL);

    if( (g_setting.timezone != 0) || (g_setting4.extra.DST_enable != 0) )
    {
        now += (g_setting.timezone*60*60);
        if(g_setting4.extra.DST_enable != 0)
            now += (1*60*60);
    }

    struct tm *p = gmtime(&now);
    int year = p->tm_year+1900;
    int mon = p->tm_mon+1;
    int day = p->tm_mday;


    //check data today
printf("today year=%d, mon=%d, day=%d\n", year, mon, day);
    if( (year != g_sunrisesunset[0].sunrise.tm_year) || (mon != g_sunrisesunset[0].sunrise.tm_mon) ||
        (day != g_sunrisesunset[0].sunrise.tm_mday) )
    {
        //copy tomorrow to today
        memcpy(&g_sunrisesunset[0], &g_sunrisesunset[1], sizeof(jswsunrisesunset));
        memset(&g_sunrisesunset[1], 0, sizeof(jswsunrisesunset));
    }else
    {
        return;
    }

    //check data today again
    if( (year != g_sunrisesunset[0].sunrise.tm_year) || (mon != g_sunrisesunset[0].sunrise.tm_mon) ||
        (day != g_sunrisesunset[0].sunrise.tm_mday) )
    {
        //get sunrise/sunset data for today
        memset(&g_sunrisesunset[0], 0, sizeof(jswsunrisesunset));
        get_single_sunrise_sunset_data(year, mon, day, &g_sunrisesunset[0]);
printf("g_sunrisesunset[0] tm_year=%d, tm_mon=%d, tm_mday=%d\n", g_sunrisesunset[0].sunrise.tm_year, g_sunrisesunset[0].sunrise.tm_mon, g_sunrisesunset[0].sunrise.tm_mday);
printf("g_sunrisesunset[0] tm_hour=%d, tm_min=%d, tm_sec=%d\n", g_sunrisesunset[0].sunrise.tm_hour, g_sunrisesunset[0].sunrise.tm_min, g_sunrisesunset[0].sunrise.tm_sec);
    }

    //check data tomorrow
    now += 86400; //(24*60*60);
    p = gmtime(&now);
    year = p->tm_year+1900;
    mon = p->tm_mon+1;
    day = p->tm_mday;
printf("tomorrow year=%d, mon=%d, day=%d\n", year, mon, day);
    if( (year != g_sunrisesunset[1].sunrise.tm_year) || (mon != g_sunrisesunset[1].sunrise.tm_mon) ||
        (day != g_sunrisesunset[1].sunrise.tm_mday) )
    {
        //get sunrise/sunset data for tomorrow
        memset(&g_sunrisesunset[1], 0, sizeof(jswsunrisesunset));
        get_single_sunrise_sunset_data(year, mon, day, &g_sunrisesunset[1]);
printf("g_sunrisesunset[1] tm_year=%d, tm_mon=%d, tm_mday=%d\n", g_sunrisesunset[1].sunrise.tm_year, g_sunrisesunset[1].sunrise.tm_mon, g_sunrisesunset[1].sunrise.tm_mday);
printf("g_sunrisesunset[1] tm_hour=%d, tm_min=%d, tm_sec=%d\n", g_sunrisesunset[1].sunrise.tm_hour, g_sunrisesunset[1].sunrise.tm_min, g_sunrisesunset[1].sunrise.tm_sec);
    }
}

bool getSensorstate(unsigned int nNodeID, jswdev* pDevIfno)
{
	int i = 0;
	bool bRet= false;
	bool bAlive = false;

	//printf("getSensorstate nNodeID=%d \n", nNodeID);
	
	for( i = 0; i < devcount; i++)
	{
		if (devlist[i].did == nNodeID )
		{
			memcpy(pDevIfno, &devlist[i], sizeof(jswdev));
			bAlive = check_sensor_alive(nNodeID);		
			break;
		}

	}

	return bAlive;
}

//apple 1226
int getSensorList(char* pList)
{
	int i = 0;
	bool bRet= false;
	bool bAlive = false;

	if(devcount > 0)
	{		
		memset(pList, 0,  sizeof(jswdev)*devcount);
		memcpy(pList, &devlist,  sizeof(jswdev)*devcount);
	}

	return devcount;
}




