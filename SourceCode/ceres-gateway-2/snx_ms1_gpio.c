/*
 * Reset Factory Mode.
 * Copyright (C) 2009 Sonix Technology
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "snx_gpio.h"

#define SFGPIO_GPIO_WRITE 	1
#define SFGPIO_GPIO_READ 	0

#define SF_GPIO_DEVICE			"/dev/sfgpio"

int snx_check_ms1conf (int pin)
{
  if (pin <= 0 || pin == 8 || pin == 9 || pin == 10 || pin >14)
    return GPIO_FAIL;
    
  if (pin == MS1_GPIO_PIN1)
  {
    if (CONFIG_MS1_IO_1_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == MS1_GPIO_PIN2)
  {
    if (CONFIG_MS1_IO_2_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == MS1_GPIO_PIN3)
  {
    if (CONFIG_MS1_IO_3_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == MS1_GPIO_PIN4)
  {
    if (CONFIG_MS1_IO_4_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == MS1_GPIO_PIN5)
  {
    if (CONFIG_MS1_IO_5_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == MS1_GPIO_PIN6)
  {
    if (CONFIG_MS1_IO_6_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == MS1_GPIO_PIN7)
  {
    if (CONFIG_MS1_IO_7_ENABLE == 0)
      return GPIO_FAIL;  
  }
    else if (pin == MS1_GPIO_PIN11)
  {
    if (CONFIG_MS1_IO_11_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == MS1_GPIO_PIN12)
  {
    if (CONFIG_MS1_IO_12_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == MS1_GPIO_PIN13)
  {
    if (CONFIG_MS1_IO_13_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == MS1_GPIO_PIN14)
  {
    if (CONFIG_MS1_IO_14_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else
    return GPIO_FAIL;
    
  return GPIO_SUCCESS;      
}

int fd_sf;
int snx_ms1_gpio_open()
{
	fd_sf = open (SF_GPIO_DEVICE, O_RDWR);
	if (fd_sf < 0)
	{
		printf("Open GPIO Device Failed.\n");
		return GPIO_FAIL;
	}
  return GPIO_SUCCESS; 
}
int snx_ms1_gpio_close ()
{
  if (fd_sf)
    close (fd_sf);
 
  return GPIO_SUCCESS;  
}
int snx_ms1_gpio_read (gpio_pin_info* info)
{
  if (snx_check_ms1conf (info->pinumber) == GPIO_FAIL)
  {
    printf ("ms1 gpio conf no support \n");
    return GPIO_FAIL;
  } 
  if (ioctl (fd_sf, SFGPIO_GPIO_READ,info) == GPIO_SUCCESS)
  {
//    printf ("ms1 read %x %x %x\n",info->pinumber,info->mode,info->value);
    return GPIO_SUCCESS;
  } 	
  else
    return GPIO_FAIL;
     
}
int snx_ms1_gpio_write (gpio_pin_info info)
{
  if (snx_check_ms1conf (info.pinumber) == GPIO_FAIL)
  {
    printf ("ms1 gpio conf no support \n");
    return GPIO_FAIL;
  }
//  printf ("ms1 pin %x %x %x\n",info.pinumber,info.mode,info.value);
  if (ioctl (fd_sf, SFGPIO_GPIO_WRITE,&info) == GPIO_SUCCESS)
    return GPIO_SUCCESS;
  else
    return GPIO_FAIL;
     
}

/*
int main (int argc, char *argv[])
{
  gpio_pin_info info;
  int i,value;
  snx_sf_gpio_open ();
start:  
  for (i=0;i<=14;i++)
  {
    if (i==0 || i==8 || i==9 || i == 10 || i==14)
      continue;
      
    info.pinumber = i;
    info.mode = 1;
    info.value = (rand() % 2);
    snx_sf_gpio_write (info);
    
  
    info.pinumber = i;
    snx_sf_gpio_read (&info);
    printf ("pin %d mode %x value %x\n",i,info.mode,info.value);
  
  }
  sleep (2);
  goto start;
  snx_sf_gpio_close ();
}
*/