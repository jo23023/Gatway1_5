// Gateway watchdog

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

#define PID_FILE_CERES				"/var/run/ceres.pid"
#define PID_FILE_CERES_UTIL			"/var/run/ceres_util.pid"
#define PID_FILE_IPCHANDLER			"/var/run/ipc_handler.pid"
#define PID_FILE_SCRIPT_START		"/var/run/start.sh.pid"
#define PID_FILE_SCRIPT_CERES_UTIL	"/var/run/ceres_util.sh.pid"
#define PID_FILE_SCRIPT_IPCHANDLER	"/var/run/ipc_handler.sh.pid"

#define WATCH_TEMP_FILENAME "./watchtemp"
#define WATCH_MCU_LIMIT 15000 //15000 secs, 4h10m
#define WATCH_MCU_CHECK_DUR 600 //check every 300 secs
#define REBOOT_LOG_FILENAME "./watchlog"
#define MCU_REBOOT_FILE   "./mcureboot" //tag for reboot by watchdog

#define TRUE			1
#define FALSE			0
#define CLEAN_COUNT		180	// Every 180 times of check kernel message, we do cleanup kernel message

#ifdef DEBUG
	#define DBG_PRINT(_STR_, args...) printf("[ GW WDT     ] "_STR_, ##args);
#else
	#define DBG_PRINT(_STR_, args...) ;
#endif

int g_nPid_Ceres = -1;
//int g_nPid_Ceres_util = -1;
//int g_nPid_Ipc_handler = -1;
int g_nPid_Script_Start = -1;
//int g_nPid_Script_Startup = -1;
//int g_nPid_Script_Ceres_util = -1;
//int g_nPid_Script_Ipc_handler = -1;

const char *g_sKernelErrorMsg[] = \
{
	"oom-killer"
};
const int g_sKernelErrorMsgNumber = sizeof(g_sKernelErrorMsg) / sizeof(const char *);

const char *g_sProcessList[] = \
{
//	"mqttd"
    "ceres"
};
const int g_sProcessListNumber = sizeof(g_sProcessList) / sizeof(const char *);

void WaitAPReady()
{
	struct stat sts;

	// Wait ceres ready
	while(1)
	{
		if(stat(PID_FILE_CERES, &sts) == 0)
		{
			break;
		}
		sleep(1);
	}
}

int LoadFile(char *pFilePath, char *pBuff, int nBuffSize)
{
	int nRet = FALSE;
	FILE *pFile = NULL;

	if((pBuff == NULL) || (nBuffSize < 1))
	{
		DBG_PRINT("Invaild buffer\n");
		return nRet;
	}

	pFile = fopen(pFilePath, "r");
	if(pFile == NULL)
	{
		DBG_PRINT("Open %s fail.\n", pFilePath);
	}
	else
	{
		int c = 0;

		while((c = fgetc(pFile)) != EOF && c != '\n')
		{
			if(strlen(pBuff) >= (nBuffSize - 1))
			{
				DBG_PRINT("Over buffer size!\n");
				break;
			}
			*(pBuff++) = c;
		}
		*(pBuff++) = '\0';

		nRet = TRUE;
		fclose(pFile);
	}

	return nRet;
}

int SystemCommand(const char sCommand[], char *pBuff, int nBuffSize)
{
	int nNeedResult = FALSE;
	char newline[256];
	FILE *pFile = NULL;

	if(strlen(sCommand) < 1)
	{
		DBG_PRINT("Invaild command\n");
	}
	else if(pBuff != NULL)
	{
		nNeedResult = TRUE;
		if(nBuffSize < 1)
		{
			DBG_PRINT("Invaild buffer size\n");
			return FALSE;
		}
	}

	pFile = popen(sCommand, "r");
	if(pFile == NULL)
	{
		DBG_PRINT("Command %s fail.\n", sCommand);
		return FALSE;
	}
	else if(nNeedResult == TRUE)
	{
		int c = 0;

		while((c = fgetc(pFile)) != EOF && c != '\n')
		{
			if(strlen(pBuff) >= (nBuffSize - 1))
			{
				DBG_PRINT("Over buffer size!\n");
				break;
			}
			*(pBuff++) = c;
		}
		*(pBuff++) = '\0';
	}
	pclose(pFile);

	return TRUE;
}

int GetPidByName(const char sName[])
{
	char sCommand[64] = {0};
	char sBuf[512] = "";
	char *pPtr = NULL;
	FILE *pFile = NULL;
	int ret =0;
	sprintf(sCommand,"pidof %s",sName);
	pFile = popen(sCommand, "r");
	if(pFile == NULL)
	{
		DBG_PRINT("Command %s fail.\n", sCommand);
		return FALSE;
	}

    fgets(sBuf, sizeof(sBuf), pFile);
    pclose(pFile);
	ret = atoi(sBuf);
	return ret;
}


/*
int GetPidByName(const char sName[])
{
	char sCommand[64] = "";
	char sBuf[512] = "";
	char *pPtr = NULL;

	snprintf(sCommand, sizeof(sCommand), "ps | grep \"%s\" | grep -v grep", sName);
	if(SystemCommand(sCommand, sBuf, sizeof(sBuf)) == FALSE)
	{
		return FALSE;
	}

	if(strlen(sBuf) < 1)
	{
		return FALSE;
	}

	pPtr = strchr(sBuf, ' ');	// Do twice for skip first space in 'ps'
	pPtr = strchr((pPtr + 2), ' ');
	if(pPtr == NULL)
	{
		return FALSE;
	}

	*pPtr = '\0';
	return atoi(sBuf);
}
*/

int LoadPid()
{
	int nRet = 0;
	char sBuf[64] = "";

	do
	{
		// ceres
		if(LoadFile(PID_FILE_CERES, sBuf, sizeof(sBuf)) == FALSE)
		{
			break;
		}
		g_nPid_Ceres = atoi(sBuf);
		bzero(sBuf, sizeof(sBuf));

		/*
		// ceres_util
		if(LoadFile(PID_FILE_CERES_UTIL, sBuf, sizeof(sBuf)) == FALSE)
		{
			break;
		}
		g_nPid_Ceres_util = atoi(sBuf);
		bzero(sBuf, sizeof(sBuf));

		// ipc_handler
		if(LoadFile(PID_FILE_IPCHANDLER, sBuf, sizeof(sBuf)) == FALSE)
		{
			break;
		}
		g_nPid_Ipc_handler = atoi(sBuf);
		bzero(sBuf, sizeof(sBuf));
		*/

		// start.sh
		if(LoadFile(PID_FILE_SCRIPT_START, sBuf, sizeof(sBuf)) == FALSE)
		{
			break;
		}

		g_nPid_Script_Start = atoi(sBuf);
		bzero(sBuf, sizeof(sBuf));

        /*
		// startup.lua
		g_nPid_Script_Startup = GetPidByName("startup.lua");
		bzero(sBuf, sizeof(sBuf));

		// ceres_util.sh
		if(LoadFile(PID_FILE_SCRIPT_CERES_UTIL, sBuf, sizeof(sBuf)) == FALSE)
		{
			break;
		}

		g_nPid_Script_Ceres_util = atoi(sBuf);
		bzero(sBuf, sizeof(sBuf));

		// ipc_handler.sh
		if(LoadFile(PID_FILE_SCRIPT_IPCHANDLER, sBuf, sizeof(sBuf)) == FALSE)
		{
			break;
		}

		g_nPid_Script_Ipc_handler = atoi(sBuf);
		bzero(sBuf, sizeof(sBuf));
		*/

		nRet = 1;
	} while(0);

	//DBG_PRINT("====== PID List ======\n");
	//DBG_PRINT("  ceres          : %d\n", g_nPid_Ceres);
	//DBG_PRINT("  ceres_util     : %d\n", g_nPid_Ceres_util);
	//DBG_PRINT("  ipc_handler    : %d\n", g_nPid_Ipc_handler);
	//DBG_PRINT("  start.sh       : %d\n", g_nPid_Script_Start);
	//DBG_PRINT("  startup.lua    : %d\n", g_nPid_Script_Startup);
	//DBG_PRINT("  ceres_util.sh  : %d\n", g_nPid_Script_Ceres_util);
	//DBG_PRINT("  ipc_handler.sh : %d\n", g_nPid_Script_Ipc_handler);

	return nRet;
}

void KillProcessByPid(int nPid)
{
	char sCommand[16];

	if(nPid < 1)
	{
		DBG_PRINT("Invalid pid %d\n", nPid);
		return;
	}

	bzero(sCommand, sizeof(sCommand));
	snprintf(sCommand, sizeof(sCommand), "kill %d", nPid);

	SystemCommand(sCommand, NULL, 0);
}

void KillProcess()
{
	DBG_PRINT("KillProcess\n");

	//LoadPid();
	//KillProcessByPid(g_nPid_Script_Start);
	//KillProcessByPid(g_nPid_Script_Startup);
	//KillProcessByPid(g_nPid_Script_Ceres_util);
	//KillProcessByPid(g_nPid_Script_Ipc_handler);
	KillProcessByPid(g_nPid_Ceres);
	//KillProcessByPid(g_nPid_Ceres_util);
	//KillProcessByPid(g_nPid_Ipc_handler);
	SystemCommand("killall -9 ceres", NULL, 0);
	//SystemCommand("killall -9 ceres_util", NULL, 0);
	//SystemCommand("killall -9 ipc_handler", NULL, 0);
}

void CleanKernelMsg()
{
	SystemCommand("/bin/dmesg -c", NULL, 0);
}

int RunProgram(const char sName[])
{
	DBG_PRINT("Run [%s]\n", sName);

	struct stat sts;
	char sCommand[256] = {0};

	// Run each program
	if(strcmp(sName, "mqttd") == 0)
	{
		char sMqttCommand[] = "cd /media/%s/sn98601/ && export LD_LIBRARY_PATH=lib && ./mqttd -c mqttd.conf&";

		if(stat("/media/mmcblk0/sn98601/mqttd", &sts) == 0)
		{
			snprintf(sCommand, sizeof(sCommand), sMqttCommand, "mmcblk0");
		}
		else if(stat("/media/mmcblk0p1/sn98601/mqttd", &sts) == 0)
		{
			snprintf(sCommand, sizeof(sCommand), sMqttCommand, "mmcblk0p1");
		}
		else
		{
			DBG_PRINT("Program [%s] not found\n", sName);
		}
	}
	if(strcmp(sName, "ceres") == 0)
	{
		printf("============restart ceres====================\n");
		if(access("/media/mmcblk0/sn98601/ceres", F_OK)==0)
		{
    		save_watchlog("restart ceres");
			system("./ceres &");
			return 1;
		}
		else if(access("/media/mmcblk0p1/sn98601/ceres", F_OK)==0)		
		{			
    		save_watchlog("restart ceres");
			system("./ceres &");
			return 1;
		}

	}
	

	if(strlen(sCommand) != 0)
	{
		return SystemCommand(sCommand, NULL, 0);
	}
	else
	{
		return FALSE;
	}
}

int CheckProcessAlive()
{
	//#if 0
	int i, j;
	int nPid = 0;
	static int counter =0;
	for(i=0; i<g_sProcessListNumber; i++)
	{
		//DBG_PRINT("Check program [%s]\n", g_sProcessList[i]);
		nPid = GetPidByName(g_sProcessList[i]);
		if(nPid > 0)
		{
			counter = 0;
			continue;
		}
		if(counter++ > 2) return FALSE;
		// Retry 3 times, or return false
		DBG_PRINT("Missing program [%s] !!!\n", g_sProcessList[i]);
		for(j=0; j<3; j++)
		{
			if(RunProgram(g_sProcessList[i]) == TRUE)
			{
				sleep(3); // Wait 3 sec for program
				break;
			}

			DBG_PRINT("Program [%s] execute fail(%d)\n", g_sProcessList[i], j);
			if(j >= 2)
			{
				return FALSE;
			}
		}
	}
    //#endif

	return TRUE;
}

int CheckMCUAlive()
{
    static int err_cnt = 0;
    struct stat buf;
    if(stat(WATCH_TEMP_FILENAME, &buf) == 0)
    {//has file
        err_cnt = 0;
        int time_now = time((time_t*)NULL);
        if( (time_now > 1400000000) && (time_now < 1800000000) )
        {//legal time duration, 2015-2026
            if(difftime(time_now, buf.st_mtime) >= WATCH_MCU_LIMIT)
            {//maybe MCU crash, reboot
                return FALSE;
            }
        }
    }else
    {//no file
        err_cnt++;
        if(err_cnt >= (WATCH_MCU_LIMIT/WATCH_MCU_CHECK_DUR+2) )
        {
            err_cnt = 0;
            return FALSE;
        }
    }

	return TRUE;
}

int DetectKernelError()
{
	int i;
	int nRet = FALSE;
	char sKernelMsg[32767];

	memset(sKernelMsg, 0, sizeof(sKernelMsg));
	nRet = SystemCommand("/bin/dmesg", sKernelMsg, sizeof(sKernelMsg));

	if((nRet == TRUE) || (strlen(sKernelMsg) > 0))
	{
		//DBG_PRINT("sKernelMsg size : %d\n", strlen(sKernelMsg));
		for(i=0; i<g_sKernelErrorMsgNumber; i++)
		{
			if(strstr(sKernelMsg, g_sKernelErrorMsg[i]) != NULL)
			{
				DBG_PRINT("Detect [%s] !!!\n", g_sKernelErrorMsg[i]);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void save_watchlog(char* msg)
{
    char cmd2[256];
    sprintf(cmd2, "date >> %s", REBOOT_LOG_FILENAME);
    system(cmd2);
    sprintf(cmd2, "echo \"%s\" >> %s", msg, REBOOT_LOG_FILENAME);
    system(cmd2);
    sprintf(cmd2, "echo >> %s", REBOOT_LOG_FILENAME);
    system(cmd2);
    sprintf(cmd2, "date > %s", MCU_REBOOT_FILE);
    system(cmd2);
    sync();
}

void MonitorSystem()
{
	int nClearCnut = 0;

	while(1)
	{
		nClearCnut++;
		//DBG_PRINT("[%d][%d]\n", nClearCnut, (nClearCnut % 30));

		// Detect kernel error
		if( ((nClearCnut % 30) == 0) && (DetectKernelError() == TRUE) )
		{
		    save_watchlog("Reboot by DetectKernelError()");
			break;	// Exit this function will reboot gateway
		}

		// Check process alive (every 30 sec) Jeff change to 10
		if(((nClearCnut % 10) == 0) && (CheckProcessAlive() == FALSE))
		{
		    save_watchlog("Reboot by CheckProcessAlive()");
			break;
		}

		// Check MCU alive (every 600 secs)
		if(((nClearCnut % WATCH_MCU_CHECK_DUR) == 0) && (CheckMCUAlive() == FALSE))
		{
		    save_watchlog("Reboot by CheckMCUAlive()");
            char cmd2[256];
            sprintf(cmd2, "rm %s", WATCH_TEMP_FILENAME);
            system(cmd2);
			break;
		}

		// Check count
		if((nClearCnut % CLEAN_COUNT) == 0)
		{
			//DBG_PRINT("nClearCnut : %d, clean!\n", nClearCnut);
			CleanKernelMsg();
			//nClearCnut = 0;
		}

		sleep(1);
	}
}

void ParseSignal(int nSignal)
{
	DBG_PRINT("Parse signal %d\n", nSignal);

	switch(nSignal)
	{
		case SIGINT:
		case SIGKILL:
		case SIGTERM:
		{
			DBG_PRINT("Do reboot\n");
			SystemCommand("/sbin/reboot" ,NULL ,0);
			break;
		}
		default:
			break;
	}
}

int main(int argc, char* argv[])
{
	DBG_PRINT("gateway_wdt run.\n");

	// Save pid itself
	{
		char sCmd[128] = "";

		snprintf(sCmd, sizeof(sCmd), "echo %d > /var/run/gateway_wdt.pid", getpid());
		system(sCmd);
	}

	// Setup signal capture
	//signal(SIGINT, ParseSignal);	// ctrl + c
	signal(SIGKILL, ParseSignal);
	signal(SIGTERM, ParseSignal);

	sleep(1);
	WaitAPReady();

	if(LoadPid() == TRUE)
	{
		DBG_PRINT("LoadPid success, start monitor kernel.\n");
		MonitorSystem();
	}

	DBG_PRINT("Reboot.\n");
	KillProcess();
	SystemCommand("/sbin/reboot" ,NULL ,0);

	return 0;
}
