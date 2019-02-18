#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
 #include <getopt.h>

#include "jsw_protocol.h"

#define I2C_BUS     "/dev/i2c-0"

int ledfd = 0;

int snx_i2c_open(char *dev)
{
    int fd;
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        printf("open %s failed\n", dev);
        exit(1);
    }
    return fd;
}

int snx_i2c_close(int fd)
{
        return close(fd);
}

int snx_i2c_write(int fd, int chip_addr, void* data, int data_len)
{
    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data ioctl_data;
    int ret;

    msgs[0].addr = chip_addr;
    msgs[0].flags = 0;              //Write Operation
    msgs[0].len = data_len;
    msgs[0].buf = (__u8*)data;

    ioctl_data.nmsgs = 1;
    ioctl_data.msgs = msgs;
    ret = ioctl(fd, I2C_RDWR, &ioctl_data);
    if (ret < 0) {
        printf("%s: ioctl return: %d\n", __func__, ret);
    }

    return ret;
}



int snx_i2c_burst_read(int fd, int chip_addr, int start_addr, int end_addr, void *data)
{
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data ioctl_data;
    int ret;

    msgs[0].addr = chip_addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = data;
    ((__u8 *)data)[0] = start_addr;

    msgs[1].addr = chip_addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = end_addr-start_addr+1;
    msgs[1].buf = data;

    ioctl_data.nmsgs = 2;
    ioctl_data.msgs = msgs;
    ret = ioctl(fd, I2C_RDWR, &ioctl_data);
    if (ret < 0) {
        printf("%s: ioctl return: %d\n", __func__, ret);
    }

    return ret;
}

int snx_i2c_read(int fd, int chip_addr, int addr)
{
    __u8 value = -1;

    snx_i2c_burst_read(fd,chip_addr,addr,addr,&value);

    return value;
}

int set_red_pwm(int fd,char pwm_value)
{
    char value[32];
    int status;

    //Close green PWM
    value[0] = 0x80;

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }

    usleep(10);

    //Close blue PWM
    value[0] = 0x60;

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }

    usleep(10);

    //Set Red PWM
    //printf("I2C Red PWM Write\n\r");
    value[0] = 0x40 | pwm_value;    //Red PWM

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }

    usleep(10);

    return 0;

}

int set_green_pwm(int fd,char pwm_value)
{

    char value[32];
    int status;

    //Close red PWM

    value[0] = 0x40;

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }

    usleep(10);
    //Close blue PWM
    value[0]  = 0x60;

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }

    usleep(10);
    //Set green PWM
    //printf("I2C Green PWM Write\n\r");
    value[0]  = 0x80 | pwm_value;   //green PWM

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }
    usleep(10);
    return 0;

}

int set_blue_pwm(int fd,char pwm_value)
{
    char value[32];
    int status;

    //Close green PWM
    value[0] = 0x80;
    status = snx_i2c_write(fd,0x70,value, 1);
    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }

    usleep(10);
    //Close Red PWM
    value[0]  = 0x40;

    status = snx_i2c_write(fd,0x70,value, 1);

    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }
    usleep(10);
    //Set blue PWM
    //printf("I2C BLUE PWM Write\n\r");
    value[0]  = 0x60 | pwm_value;   //blue PWM

    status = snx_i2c_write(fd,0x70,value, 1);
    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return -1;
    }
    usleep(10);
    return 0;
}

void yellow(char pwm_value)
{
    char value[32];
    int status;

    //Close Red PWM
    value[0]  = 0x40;
    status = snx_i2c_write(ledfd,0x70,value, 1);
    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return;
    }
    usleep(10);

    pwm_value = 19;
    //Set blue PWM
    value[0]  = 0x60 | pwm_value;   //blue PWM
    status = snx_i2c_write(ledfd,0x70,value, 1);
    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return;
    }
    usleep(10);

    pwm_value = 31;
    //Set green PWM
    value[0]  = 0x80 | pwm_value;   //green PWM
    status = snx_i2c_write(ledfd,0x70,value, 1);
    if(status < 0)
    {
        printf("snx_i2c_write Error\n\r");
        return;
    }
    usleep(10);

}

void setled(short color, int mode)
{
    int x;
    int max =1;
    int v = 0x01;
    if (mode ==1 )
        max = 5;

    for ( x=0;x<max;x++)
    {
        if (color == 0)
            set_green_pwm(ledfd,v); //red
        else if (color ==1)
            set_blue_pwm(ledfd,v); //red
        else if (color ==2)
            set_red_pwm(ledfd,v); //red

        usleep(500);
        v <<= 1;
    }

}

void yellowled(int mode)
{
//  yellow(1);

//  char value[32];
//  int x;
//  int max =1;
//  int status;
//  int v = 0x01;
//  if (mode ==1 )
//      max = 5;
//
//  if(mode == 0)
//  {
//      value[0]  = 0x40;
//      status = snx_i2c_write(ledfd,0x70,value, 1);
//      usleep(10);
//
//      value[0]  = 0x80 | 0x08;    //green PWM
//      status = snx_i2c_write(ledfd,0x70,value, 1);
//      usleep(10);
//
//      value[0]  = 0x60 | 0x01; //0x0A;//18;//19;  //blue PWM
//      status = snx_i2c_write(ledfd,0x70,value, 1);
//      usleep(500);
//  }else
//  {
//      value[0]  = 0x40;
//      status = snx_i2c_write(ledfd,0x70,value, 1);
//      usleep(10);
//
//      value[0]  = 0x80 | 0x1F;    //green PWM
//      status = snx_i2c_write(ledfd,0x70,value, 1);
//      usleep(10);
//
//      value[0]  = 0x60 | 0x05; //0x0A;//18;//19;  //blue PWM
//      status = snx_i2c_write(ledfd,0x70,value, 1);
//      usleep(500);
//  }

    //for ( x=0;x<max;x++)
    //{
    //      value[0]  = 0x40;
    //      status = snx_i2c_write(ledfd,0x70,value, 1);
    //      usleep(10);

    //      value[0]  = 0x80 | 0x1F;    //green PWM
    //      status = snx_i2c_write(ledfd,0x70,value, 1);
    //      usleep(10);

    //      value[0]  = 0x60 | 0x05; //0x0A;//18;//19;  //blue PWM
    //      status = snx_i2c_write(ledfd,0x70,value, 1);
    //      usleep(500);

    //  v <<= 1;
    //}

}

void redled(int mode)
{
//  setled(0, mode);
    //green
}

void greenled(int mode)
{
//  setled(1, mode);
    //set_blue_pwm(ledfd,32);
}

void blueled(int mode)
{
//  setled(2, mode);
    //set_red_pwm(ledfd,32);
}

void ledoff()
{
//  char value[32];
//  int status;
//
//  //Close red PWM
//  value[0] = 0x40;
//  status = snx_i2c_write(ledfd,0x70,value, 1);
//  if(status < 0)
//  {
//      printf("snx_i2c_write Error\n\r");
//      return -1;
//  }
//  usleep(10);
//  //Close blue PWM
//  value[0]  = 0x60;
//  status = snx_i2c_write(ledfd,0x70,value, 1);
//
//  if(status < 0)
//  {
//      printf("snx_i2c_write Error\n\r");
//      return -1;
//  }
//
//  usleep(10);
//
//  //Close green PWM
//  value[0] = 0x80;
//
//  status = snx_i2c_write(ledfd,0x70,value, 1);
//
//  if(status < 0)
//  {
//      printf("snx_i2c_write Error\n\r");
//      return -1;
//  }

}

int initled()
{
    char write_data[16];
    int i;
    int status;

    char pwm_value;

    ledfd = snx_i2c_open(I2C_BUS);

    if(ledfd > 0)
    {
        //printf("I2C OPEN OK\n\r");

        //RGB Driver I2C Write Test
        //RGB Driver I2C Salve address = 0x70
        //printf("I2C LED Current Step Write\n\r");
        write_data[0] = 0x20 | 0x1F ;   //LED Current Step/


        status = snx_i2c_write(ledfd,0x70,write_data, 1);

        if(status < 0)
        {
            printf("snx_i2c_write Error\n\r");
            return;
        }

        //printf("I2C LED Current Step Write doen\n\r");
        return 1;
    }
    else
    {
        printf("led init error \n");

    return 0;
    }

}


int ledtest()
{
    system(". /etc/script/jsw_control_led.sh 14 -b 1&");
    usleep(400000);
    system(". /etc/script/jsw_control_led.sh 12 -b 1&");
    usleep(400000);

    redled(0);
    usleep(400000);
    greenled(0);
    usleep(400000);
    blueled(0);
    usleep(400000);
    yellowled(0);
    usleep(400000);

    //usleep(200000);

    redled(1);
    usleep(300000);
    greenled(1);
    usleep(300000);
    blueled(1);
    usleep(300000);
    yellowled(1);
    usleep(300000);

    ledoff();

    system(". /etc/script/jsw_control_led.sh 14 -b 0");
    usleep(400000);
    system(". /etc/script/jsw_control_led.sh 12 -b 0");
    usleep(400000);
    system(". /etc/script/jsw_control_led.sh 14 -v 0");
    usleep(400000);
    system(". /etc/script/jsw_control_led.sh 12 -v 1");
    usleep(400000);

    //int fd;
    //char write_data[16];
    //int i;
    //int status;
    //char pwm_value;

    ////fd = snx_i2c_open(I2C_BUS);
 //
 //
    //if(ledfd > 0)
    //{
    //  //printf("I2C OPEN OK\n\r");

    //  //RGB Driver I2C Write Test
    //  //RGB Driver I2C Salve address = 0x70
    //
    //  //printf("I2C LED Current Step Write\n\r");
    //  //write_data[0] = 0x20 | 0x1F ; //LED Current Step
    //

    //  //status = snx_i2c_write(fd,0x70,write_data, 1);

    ///*    if(status < 0)
    //  {
    //      printf("snx_i2c_write Error\n\r");
    //      return;
    //  }*/

    //  printf("I2C LED test\n\r");

    //  pwm_value = 0x01;
    //  int cnt = 0;

    //  while(1)
    //  {
    //      cnt++;
    //      if (cnt == 10000)
    //          break;

    //      for(i=0;i<5;i++)
    //      {
    //
    //          status = set_red_pwm(fd,pwm_value);
    //          usleep(500000);

    //          status = set_blue_pwm(fd,pwm_value);
    //          usleep(500000);

    //          status = set_green_pwm(fd,pwm_value);
    //          usleep(500000);

    //          pwm_value <<= 1;

    //      }
    //      usleep(500000);

    //      pwm_value = 0x01;   //reset PWM VALUE
    //
    //  }
 //
 //
 //
    //  //snx_i2c_close(fd);
 //     }
    //return 0;
}


