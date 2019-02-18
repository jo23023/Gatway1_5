#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char sdpath[128];


int isMmcblk0Valid()
{
	return (0 == access("/media/mmcblk0/sn98601/ceres", 0));
}

int isMmcblk0p1Valid()
{
	return (0 == access("/media/mmcblk0p1/sn98601/ceres", 0));
}




int main(int argc, char* argv[])
{
	char old[128];
	char newfile[128];
	char cmd[1024];
	char sh[1024];


	printf("gateway_firmware update run.\n");

	system("killall -9 gateway_wdt");
	usleep(100*1000);
	system("killall -9 ceres");
	usleep(500*1000);

	memset(sdpath, 0, sizeof(sdpath));
	if (isMmcblk0Valid())
		strcpy( sdpath ,"/media/mmcblk0/sn98601/");
	else if(isMmcblk0p1Valid())
		strcpy( sdpath ,"/media/mmcblk0p1/sn98601/");

	sprintf(old,"%sceres", sdpath);
	sprintf(newfile, "%sceresnew", sdpath);
	sprintf(sh, "%sstart.sh", sdpath);

	sprintf(cmd, "cp -p %s %s", newfile, old);
	system(cmd);
	printf("updater start copying file\n");
/*	
	for (int x=0;x<2000;x++)
	{
		usleep(2000);
	}
*/
	system("sync");
	system("reboot");
#if 0
	printf("updater ready to restart\n");
	system(sh);

	printf("updater done\n");

#endif


	
	return 1;
}
