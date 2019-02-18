ARCH=sn98601

GATEWAY_PATH=`pwd`/../..
#CC=/home/victor/work_dir/Sonix/SN98601/SN98601_GatewayModule/toolchain/crosstool-4.5.2/bin/arm-linux-gcc
CC=../../toolchain/crosstool-4.5.2/bin/arm-linux-gcc



#CC=arm-linux-gcc
#LD=arm-linux-gcc
#STRIP=arm-linux-strip

CFLAGS =  -Wall -pipe -std=gnu99 	 		

SRC = updater.c

all: 
	$(CC) -g $(SRC) $(CFLAGS) -o update -DLINUX 

clean:
	rm update

