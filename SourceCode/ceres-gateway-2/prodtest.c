#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <crypt.h>
#include <sys/time.h>
#include <time.h>
#include "ceres.h"
#include "jsw_protocol.h"
#include "AES/AES_utils.h"
#include "jsw_rf_api.h"
#include <arpa/inet.h>
#include "ceres_util.h"

#include "snx_gpio.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>


int changeMac(char *input) {
    struct ifreq ifr;
    int s;
   // char mac_char[] = "12:34:56:78:12:34";
	system("ifdown eth0");
	usleep(2000);
 
    sscanf(input, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &ifr.ifr_hwaddr.sa_data[0],
        &ifr.ifr_hwaddr.sa_data[1],
        &ifr.ifr_hwaddr.sa_data[2],
        &ifr.ifr_hwaddr.sa_data[3],
        &ifr.ifr_hwaddr.sa_data[4],
        &ifr.ifr_hwaddr.sa_data[5]
        );
 
    s = socket(AF_INET, SOCK_DGRAM, 0);
   // assert(s != -1);
 
    strcpy(ifr.ifr_name, "eth0");
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    if (ioctl(s, SIOCSIFHWADDR, &ifr) == -1)
	{
		printf("change mac fail\n");
		system("ifup eth0");
		return 0;
	}
	//close(s);
	printf("change mac success\n");
	system("ifup eth0");
    return 1;
}