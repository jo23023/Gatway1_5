#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <getopt.h>
#include "snx_pwm_gpio.h"
#include "snx_gpio.h"
   
int snx_check_pwmconf (int pin)
{
  if (pin == PWM_GPIO_PIN0)
  {
    if (CONFIG_PWM_GPIO_00_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == PWM_GPIO_PIN1)
  {
    if (CONFIG_PWM_GPIO_01_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else
    return GPIO_FAIL;
    
  return GPIO_SUCCESS;     
}

int fd_pwm;
int snx_pwm_gpio_open()
{
	fd_pwm = open (PWM_DEVICE, O_RDWR);
	if (fd_pwm < 0)
	{
		printf("Open PWM GPIO Device Failed.\n");
		return GPIO_FAIL;
	}
  return GPIO_SUCCESS; 
}
int snx_pwm_gpio_close ()
{
  if (fd_pwm)
    close (fd_pwm);
 
  return GPIO_SUCCESS;  
}
int snx_pwm_gpio_write (gpio_pin_info info)
{
//  printf ("pwm write %x %x %x\n",info.pinumber,info.mode,info.value);
  if (snx_check_pwmconf (info.pinumber) == GPIO_FAIL)
  {
    printf ("pwm gpio conf no support \n");
    return GPIO_FAIL;
  }  
  
  if(ioctl(fd_pwm, SONIX_PWM_REQUEST, &info.pinumber))
	{
		printf("request failed\n");
		ioctl(fd_pwm, SONIX_PWM_FREE, &info.pinumber);
		return GPIO_FAIL;
	}
  if(ioctl(fd_pwm, PWM_GPIO_WRITE, &info))
  {
    printf ("pwm write error\n");
    return GPIO_FAIL;
  }  
  
  if(ioctl(fd_pwm, SONIX_PWM_FREE, &info.pinumber))
	{
    printf("free failed\n");
  	return PWM_FAIL;
  }    
  return  GPIO_SUCCESS;     
}
int snx_pwm_gpio_read (gpio_pin_info* info)
{
  if (snx_check_pwmconf (info->pinumber) == GPIO_FAIL)
  {
    printf ("pwm gpio conf no support \n");
    return GPIO_FAIL;
  }
  if(ioctl(fd_pwm, SONIX_PWM_REQUEST, &info->pinumber))
	{
		printf("request failed\n");
		ioctl(fd_pwm, SONIX_PWM_FREE, info->pinumber);
		return GPIO_FAIL;
	}
  if(ioctl(fd_pwm, PWM_GPIO_READ, info))
    return GPIO_FAIL;
//  printf ("pwm read %x %x %x \n",info->pinumber,info->mode,info->value);
  if(ioctl(fd_pwm, SONIX_PWM_FREE, &info->pinumber))
	{
    printf("free failed\n");
  	return PWM_FAIL;
  }    
  return  GPIO_SUCCESS;      
}
