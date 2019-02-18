#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
# include <termios.h>
# include <fcntl.h> 
# include <string.h>
# include <sys/time.h>
# include <sys/types.h>
# include <pthread.h>
# include<sys/ioctl.h>

#include "serialport.h"
#include "ceres.h"
#include "jsw_protocol.h"

#define CHTIME TCSANOW
//pthread_mutex_t  rs232_lock   = PTHREAD_MUTEX_INITIALIZER;

int fdcom =-1;
int open_ports(int comport)
{   
        char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};
        long  vdisable;
        if (comport==1)
        {       fdcom = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY);
                if (-1 == fdcom){
                        DBG_PRINT("Can't Open Serial Port");
                        return(-1);
                }
                else
                        DBG_PRINT("open ttymxc1 .....\n");
        }
        else if(comport==2)
        {       fdcom = open( "/dev/ttyS1", O_RDWR|O_NOCTTY|O_NONBLOCK|O_NDELAY);
                if (-1 == fdcom){
                        DBG_PRINT("Can't Open Serial Port");
                        return(-1);
                }
                else
                        DBG_PRINT("open ttyS1 .....\n");
        }   
        else if (comport==3)
        {
                fdcom = open( "/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY);
                if (-1 == fdcom){   
                        DBG_PRINT("Can't Open Serial Port");   
                        return(-1);   
                }   
                else    
                        DBG_PRINT("open ttyS2 .....\n");   
        }   

        if(fcntl(fdcom, F_SETFL, FNDELAY)<0)   
                printf("fcntl failed!\n");   
        else   
                printf("fcntl=%d\n",fcntl(fdcom, F_SETFL,FNDELAY));   
      
        printf("fd-open=%d\n",fdcom);   

		return fdcom;

}

void set_speed(int fd, int speed)
{
	int i = 0, status = 0;
	struct termios opt;
	int speed_arr[] = {B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
	int name_arr[] = {230400, 115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200,  300};

	tcgetattr(fd, &opt);
	for (i = 0; i < sizeof(speed_arr)/sizeof(int); i++) {
		if (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&opt, speed_arr[i]);
			cfsetospeed(&opt, speed_arr[i]);
			status = tcsetattr(fd, CHTIME, &opt);
			if (status != 0) {
				perror("tcsetattr fd");
				printf("wweeroreeror");
			}
			
			printf("wwwwww=%d,mweeeeeeeeeeeesg=%d\n",status,speed);
			return ;
			
		} 
		tcflush(fd, TCIOFLUSH);
	}	
}

int set_parity(int fd, int databits, int stopbits, char parity)
{
	struct termios opt;
	int c, res;
	struct termios oldtio,newtio;
	char buf[255];
#if  1
	if (tcgetattr(fd, &opt) != 0) {
		perror("set parity");
		return 0;
	}
	
	opt.c_cflag &= ~CSIZE;
	switch (databits) {
	case 5:
		opt.c_cflag |=CS5;
		break;
	case 6:
		opt.c_cflag |=CS6;
		break;
	case 7:
		opt.c_cflag |= CS7;
		break;
	case 8:
		opt.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return 0;
	}
	
	switch (parity) {
	case 'n':
	case 'N':
		opt.c_cflag &= ~PARENB;
		opt.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		opt.c_cflag |= (PARODD | PARENB);
		opt.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		opt.c_cflag |= PARENB;
		opt.c_cflag &= ~PARODD;
		opt.c_iflag |= INPCK;
		break;
	case 's':
	case 'S':
		opt.c_cflag &= ~PARENB;
		opt.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return 0;
	}

	switch (stopbits) {
	case 0:	
	case 1:
		opt.c_cflag &= ~CSTOPB;
		break;
	case 2:
		opt.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return 0;
	}

	if (parity != 'n' || parity != 'N')
		opt.c_iflag |= INPCK;

	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);				/*Input*/
	opt.c_oflag &= ~OPOST;								/*Output*/
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);  //

        opt.c_iflag &= ~ (INLCR | ICRNL | IGNCR);
        opt.c_oflag &= ~(ONLCR | OCRNL);
	
	
	opt.c_cc[VTIME] = 150;//150; //15 seconds 150x100ms = 15s
	opt.c_cc[VMIN] = 13;
		
	tcflush(fd, TCIFLUSH); //update the options and do it now
	 
	
	if (tcsetattr(fd, CHTIME, &opt) != 0) {
		perror("Setup Serial");
		return 0;
	}
#else
     #define BAUDRATE B57600
        tcgetattr(fd,&oldtio); /* save current port settings */
        
        bzero(&newtio, sizeof(newtio));

        memcpy(&newtio, &oldtio, sizeof(struct termios));

        newtio.c_cflag &= ~PARENB;
        newtio.c_cflag &= ~CSTOPB;
        newtio.c_cflag &= ~CSIZE;
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_cflag &= ~CRTSCTS;

        newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        newtio.c_oflag &= ~OPOST;
        
        /* set input mode (non-canonical, no echo,...) */
        newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
         
        newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
 #endif
	return 1;
}

int set_opt(int nSpeed, int nBits, char nEvent, int nStop)   
{   
    set_speed(  fdcom,   nSpeed);
	set_parity(  fdcom,   nBits,   nStop,   nEvent);
}




void microseconds_sleep(unsigned long uSec){
    struct timeval tv;
    tv.tv_sec=uSec/1000000;
    tv.tv_usec=uSec%1000000;
    int err;
   
        err=select(0,NULL,NULL,NULL,&tv);
   
}
#define usleep(x)     microseconds_sleep(x) 
 unsigned int  transmit_data_to_mcu_by_check(int fd,UCHAR *pbuf,int len)
{
      	   int ret=0,i=0,j=0;
	  int timecount0=0;
	    	set_cmdsend_is_finished(0);    
	  	   do
		    {
		           for(i=0;i<len;i++)
		           	{
		              	ret=write(fd,&pbuf[i] ,1);
						 
						    for(j=0;j<200;j++);
		           	}
	  			 
				   usleep(200000);
					   ret=get_cmdsend_is_finished();
					 if(ret==1)
					 	{
					 	 printf("Serail send is ok =%d   !!!!!!!!!!!!!++++++++++++++=========-------------\n",timecount0);
					 	break;
					 	}
			        timecount0++;
					if(timecount0>=3)
						 break;
	  		     
		    		}while(1);
   
}

unsigned int  sendto_mcu_by_rs232(int fd ,int misc,int controlbit, unsigned int value,unsigned int id,
	 unsigned int nodeid, int productype,int timecount,int trycount)
{
	struct timespec ts;
	int ret=0,i;
	int timecount0=0;
	unsigned char COMBUFF[13]={0};

	union RF_DATA_FORMAT rf433data;
	rf433data.FORMAT.Addressid=id;
	rf433data.FORMAT.devicestype=productype;
	rf433data.FORMAT.controlbit=controlbit;
	rf433data.FORMAT.misc=misc;
	rf433data.FORMAT.data2=(value>>16)&0xff;
	rf433data.FORMAT.data1=(value>>8)&0xff;
	rf433data.FORMAT.data0=(value)&0xff;
		
	char tmp=0;
	///pthread_mutex_lock(&rs232_lock);   
	set_send_is_finished(0);
     //  set_cmdsend_is_finished(0);    
			

	ts.tv_sec=timecount;
	ts.tv_nsec=0;
	timecount0=(timecount*100);
		   
	if(timecount0==0)
	{
	   COMBUFF[0]=0x85;
		for(i=1;i<13;i++)
	  	{
	  	  COMBUFF[i]=  rf433data.data[i-1];
	  	}
		transmit_data_to_mcu_by_check(fd,COMBUFF ,sizeof(COMBUFF)); 

		 
	}
	else
 	{
		printf("send to mcu by COM-------------\n");
	/*	printf("ADDRESSID=%02x,%02x,%02x,%02x,,%x\n", rf433data.data[3],rf433data.data[2],rf433data.data[1],rf433data.data[0],rf433data.FORMAT.Addressid);
		printf("NODID=%08x\n", rf433data.FORMAT.nodeid);
		printf("ProductTYPE=%x\n", rf433data.FORMAT.devicestype);
		printf("Controlbit=  %x\n", rf433data.FORMAT.controlbit);*/
do{
	timecount0=(timecount*100);	
	 tmp=0x85;
	 COMBUFF[0]=tmp;
	for(i=1;i<13;i++)
  	{
  	  COMBUFF[i]=  rf433data.data[i-1];
  	}

     transmit_data_to_mcu_by_check(fd,COMBUFF ,sizeof(COMBUFF));
 
	  while(timecount0--)
	   	{
	  	//  sem_timedwait(&send_is_over,&ts); 
	     ret=get_send_is_finished();
		 if(ret==1)
		 	{
		 	 printf("send is over =%d   !!!!!!!!!!!!!++++++++++++++=========-------------\n",ret);
		 	break;
		 	}
	  	   usleep(10000);
	  	}
	     if(ret==1)
	     	{
	     	   break;
	     	}
	}while(trycount--);
         	
	usleep(10000);
           
	///Hand_rflink_status(id,   nodeid,   ret);

 	}
   usleep(10000);


///	pthread_mutex_unlock(&rs232_lock);  
	return ret;
	        
}



ssize_t tread(int fd, void *buf, size_t nbytes, unsigned int timout)
{

   int nfds;
   fd_set readfds;
   struct timeval  tv;
   tv.tv_sec = timout;
   tv.tv_usec = 0;
   FD_ZERO(&readfds);
   FD_SET(fd, &readfds);
   nfds = select(fd+1, &readfds, NULL, NULL, &tv);
   if (nfds <= 0) {
          // if (nfds == 0)
             //     errno = ETIME;
          return(-1);
   }
   return(read(fd, buf, nbytes));
}

ssize_t treadn(int fd, void *buf, size_t nbytes, unsigned int timout)
{
	size_t nleft;
	ssize_t nread;
	nleft = nbytes;
	while (nleft > 0){
		if ((nread = tread(fd, buf, nleft, timout)) < 0){
			if (nleft==nbytes)
				return(-1);/* error, return -1 */
			else
				break;/* error, return amount read so far */
		} else if (nread == 0){
			break;/* EOF */
		}
		//READ > 0
		nleft -= nread;
		buf += nread;
	}
	return(nbytes - nleft);/* return >= 0 */
}

void serial_thread(void* arg)
{

	sendto_mcu_by_rs232(comfd,T_ID_WR,0,0,rt5350ID,0,0,2,0);	 
	  	 printf("sendto_mcu_by_rs232\n"); 
	while (1)
 	{ 
 	   keycount++;
 	      sleep(1);
		  timecount++;
		  if(timecount>=30)
		  {
		  	timecount=0;
			//sendto_mcu_by_rs232(comfd,MISC_GW_TMPHUM,TMPHUM_CHECK,0,rt5350ID,0,
				//		GATEWAY,0,0);
 		    sendto_mcu_by_rs232(comfd,T_ID_WR,keycount&0xff,0,rt5350ID,0,0,0,0);	
				//  printf("sendto_mcu_by_rs232----------------------------------\n"); 
		  }
		 /* if(!gpio_read(1))
 		 	{
 		 	   keycount++;
			   if(keycount>=7)
			   	{
			   	      printf("reset ket is pressed===================!!!!!\n");  
					    call_lua_excut_cmd(lua_state,"default_machine",NULL);
					keycount=0;
			   	}
 		 	}  */
	
	}

}

void read_port(void* arg)
{    

	ssize_t retval;
	unsigned char msg[40];

    while(1)
    {
  
		retval=treadn(fdcom,msg,13,30);

		switch (retval)
		{
			case 0:
				break;

			case -1:
			//perror("select");
				break;

			default:
			{

				switch(msg[0]&0xf0)
				{
					case 0xA4://ARC433-read rollcode cmd
   					   break;
					case 0x80://ARC433-rfpacket
					  //  handle_868_data_from_com(nodeinfo,msg);
		    			break;
					 
					case 0xA0: 
			 			// handle_433_data_from_com(nodeinfo,msg);
			 			break;
						
					case 0xB5://ARC433-rfpacket
		    			break;
				}
			}
			break;
		}
    }//WHILE
}