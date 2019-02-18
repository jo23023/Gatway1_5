#include <stdio.h>	 
#include <stdlib.h>   
#include <string.h>    
#include <getopt.h> 			/* getopt_long() */   
#include <fcntl.h>				/* low-level i/o */   
#include <unistd.h>   
#include <pthread.h>
#include <poll.h>
#include "snx_gpio.h"




int snx_check_gpioconf (int pin)
{
  if (pin == GPIO_PIN_0)
  {
    if (CONFIG_GPIO_00_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == GPIO_PIN_1)
  {
    if (CONFIG_GPIO_01_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == GPIO_PIN_2)
  {
    if (CONFIG_GPIO_02_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == GPIO_PIN_3)
  {
    if (CONFIG_GPIO_03_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == GPIO_PIN_4)
  {
    if (CONFIG_GPIO_04_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == GPIO_PIN_5)
  {
    if (CONFIG_GPIO_05_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else
    return GPIO_FAIL;
    
  return GPIO_SUCCESS;      
}
int gpio_number = 4;

#define BUFFER_SIZE		63
char buf[BUFFER_SIZE];
static pthread_mutex_t mutex_mode = PTHREAD_MUTEX_INITIALIZER;

int snx_gpio_open()
{
  int i = 0;
  // if sn98610 pin number 6
#ifdef CONFIG_GPIO_04_MODE
    gpio_number = 6;   
#endif  
  for(i=0;i<gpio_number;i++)
  {
    memset (buf,0,BUFFER_SIZE);
    sprintf(buf, "echo %d > /sys/class/gpio/export", i);
    system (buf);
    
    //printf ("export %s\n",buf);
  }  
	return GPIO_SUCCESS;
}


int snx_gpio_write(gpio_pin_info info)
{

  if (snx_check_gpioconf (info.pinumber) == GPIO_FAIL)
  {
    printf (" gpio conf no support \n");
    return GPIO_FAIL;
  }
  pthread_mutex_lock(&mutex_mode);

	memset (buf,0,BUFFER_SIZE);
  if (info.mode == 1)
    sprintf(buf, "echo out > /sys/class/gpio/gpio%d/direction", info.pinumber);
  else
    sprintf(buf, "echo in > /sys/class/gpio/gpio%d/direction", info.pinumber);   
  if(system (buf) != 0)
  {
    pthread_mutex_unlock(&mutex_mode);
	  return GPIO_FAIL;  
  }
//  printf ("mode %s\n",buf);
  memset (buf,0,BUFFER_SIZE);
  if (info.mode == 1)
  {
    sprintf(buf, "echo %d > /sys/class/gpio/gpio%d/value",info.value,info.pinumber);
    if(system (buf) != 0)
    {
      pthread_mutex_unlock(&mutex_mode);
      return GPIO_FAIL;  
    }
 //   printf ("value %s\n",buf);
  }  
  pthread_mutex_unlock(&mutex_mode);

	return GPIO_SUCCESS;
}
int snx_gpio_read(gpio_pin_info* info)
{
  if (snx_check_gpioconf (info->pinumber) == GPIO_FAIL)
  {
    printf (" gpio conf no support \n");
    return GPIO_FAIL;
  }
  pthread_mutex_lock(&mutex_mode);
  int fd;
  char buf0[10];
	memset (buf,0,BUFFER_SIZE);
  sprintf(buf, "/sys/class/gpio/gpio%d/direction", info->pinumber);
  fd = open (buf,O_RDONLY);
  if (fd < 0)
  {
    
    pthread_mutex_unlock(&mutex_mode);
    return GPIO_FAIL;
  }  
  read(fd, buf0, 10);  
  if (strstr(buf0,"out"))
    info->mode = 1;
  else if(strstr(buf0,"in"))
    info->mode = 0;    
  close (fd);  
  
  memset (buf,0,BUFFER_SIZE);
  sprintf(buf, "/sys/class/gpio/gpio%d/value", info->pinumber);
  fd = open (buf,O_RDONLY);
  if (fd < 0)
  {
    
    return GPIO_FAIL;
  }  
  read(fd, buf0, 10);  
  sscanf(buf0, "%d", &info->value);   
  close (fd);  
  
//  printf ("gpio read %x %x %x\n",info->pinumber,info->mode,info->value);
  pthread_mutex_unlock(&mutex_mode);
	return GPIO_SUCCESS;
}
// 0 none,1 rising ,2 falling ,3 both

int snx_gpio_set_interrupt(int pin, int type)
{        
  char type_buf[15];
  char buf[50];
  gpio_pin_info info;
  memset (type_buf,0,15);
  info.pinumber = pin;
  if (snx_gpio_read(&info) == GPIO_FAIL)
    return GPIO_FAIL;

  if (info.mode == 1)
    return GPIO_FAIL;
                                        

  if (type == INTURREPT_RISING) 
    sprintf(type_buf,"rising");     
  else if (type == INTURREPT_FALLING)  
    sprintf(type_buf,"falling"); 
  else if (type == INTURREPT_BOTH)
    sprintf(type_buf,"both");      
  else
    sprintf(type_buf,"none");   
  
  sprintf (buf," echo %s > /sys/class/gpio/gpio%d/edge",type_buf,pin);
  if(system(buf) != 0)
    return GPIO_FAIL;
     
  return GPIO_SUCCESS;       
}
int snx_gpio_poll (int pin,int timeout)
{
  gpio_pin_info info;
  char buf[50];
  int fd,ret;
  struct pollfd fds;
  info.pinumber = pin;
  
  if (snx_gpio_read(&info) == GPIO_FAIL)
    return GPIO_FAIL;
  if (info.mode == 1)
    return GPIO_FAIL;
    
  sprintf(buf, "/sys/class/gpio/gpio%d/value", info.pinumber);
  fd = open(buf, O_RDWR);
	if(fd < 0){
		fprintf(stderr, "open error:%s\n", buf);
		return GPIO_FAIL;
	}

	if((ret = read(fd, buf, 50)) < 0){
		fprintf(stderr, "read error:%s\n", buf);
		close(fd);
		return GPIO_FAIL;
	}

	fds.fd = fd;
	fds.events = POLLPRI;
  if (timeout <= 0)
    timeout = -1;  
	if((ret = poll(&fds, 1, timeout) <= 0))
	{	
    fprintf(stderr, "poll error or timeout\n");
    close(fd);
    return GPIO_FAIL;
  }
   
  fprintf(stderr, "poll interrupt\n");
	close(fd);   
  return GPIO_SUCCESS;   
}

int snx_gpio_close()
{
	int fd, i;
  for(i=0;i<gpio_number;i++)
  {
    memset (buf,0,BUFFER_SIZE);
    sprintf(buf, "echo %d > /sys/class/gpio/unexport", i);
    if(system (buf) != 0)
      return GPIO_FAIL;
   // printf ("unexport %s\n",buf);
  }  
	return GPIO_SUCCESS;
}
