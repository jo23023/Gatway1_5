#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <getopt.h>
#include "snx_gpio.h"
//#include "spidev.h"


int snx_check_spiconf (int pin)
{
  if (pin == SPI_GPIO_CLK_PIN)
  {
    if (CONFIG_SPI_GPIO_00_ENABLE == 0)
      return GPIO_FAIL; 
  }
  else if (pin == SPI_GPIO_FS_PIN)
  {
    if (CONFIG_SPI_GPIO_01_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == SPI_GPIO_TX_PIN)
  {
    if (CONFIG_SPI_GPIO_02_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else if (pin == SPI_GPIO_RX_PIN)
  {
    if (CONFIG_SPI_GPIO_03_ENABLE == 0)
      return GPIO_FAIL;  
  }
  else
    return GPIO_FAIL;
  
  return GPIO_SUCCESS;  
    
}
 
char spi_buf[12] = {0};
int fd_spi;
int snx_spi_gpio_open ()
{
  fd_spi = open("/dev/spidev0.0", O_RDWR, 0);
  if (fd_spi < 0) {
		printf ("open the file /dev/spi_gpio failed, spi select=0\n");
		return GPIO_FAIL;
	}
  return GPIO_SUCCESS; 
} 
int snx_spi_gpio_close ()
{
  if (fd_spi)
    close (fd_spi);
 
  return GPIO_SUCCESS;  
}
int snx_spi_gpio_read (gpio_pin_info* info)
{
  int i = 0;
  spi_buf[0] = SPI_GPIO_CLK_PIN;
	spi_buf[3] = SPI_GPIO_FS_PIN;
	spi_buf[6] = SPI_GPIO_TX_PIN;
	spi_buf[9] = SPI_GPIO_RX_PIN;
  
  if (info->pinumber < SPI_GPIO_CLK_PIN || info->pinumber > SPI_GPIO_RX_PIN)
  {
    return GPIO_FAIL;
  }
  if (info->pinumber == SPI_GPIO_CLK_PIN)
    i = 0;
  else if (info->pinumber == SPI_GPIO_FS_PIN)
    i = 1;   
  else if (info->pinumber == SPI_GPIO_TX_PIN)
    i = 2;
  else
    i = 3; 
    
  spi_buf[3*i] = info->pinumber;

  read (fd_spi, spi_buf, 12);
  info->mode = spi_buf[3*i+1];
  info->value = spi_buf[3*i+2];
//  printf ("spi read %x %x %x\n",info->pinumber,info->mode,info->value);
  return GPIO_SUCCESS;
}
int snx_spi_gpio_write (gpio_pin_info info)
{
  int i = 0;
 // printf ("spi %x %x %x\n",info.pinumber,info.mode,info.value);
  spi_buf[0] = SPI_GPIO_CLK_PIN;
	spi_buf[3] = SPI_GPIO_FS_PIN;
	spi_buf[6] = SPI_GPIO_TX_PIN;
	spi_buf[9] = SPI_GPIO_RX_PIN;
  if (info.pinumber < SPI_GPIO_CLK_PIN || info.pinumber > SPI_GPIO_RX_PIN)
  {
    return GPIO_FAIL;
  }

  if (info.pinumber == SPI_GPIO_CLK_PIN)
    i = 0;
  else if (info.pinumber == SPI_GPIO_FS_PIN)
    i = 1;   
  else if (info.pinumber == SPI_GPIO_TX_PIN)
    i = 2;
  else
    i = 3;  
  spi_buf[3*i] = info.pinumber;
  spi_buf[3*i+1] = info.mode;
  spi_buf[3*i+2] = info.value; 
  write (fd_spi, spi_buf, 12); 
  return GPIO_SUCCESS;
}
void printf_spi_buf()
{
  int i = 0;
  for (i=0;i<12;i++)
    printf ("buf[%x] = %x,",i,spi_buf[i]);
  printf ("\n");  
}
