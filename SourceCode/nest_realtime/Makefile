include Makefile.common

ARCH=sn98601
CC=arm-linux-gcc
LD=arm-linux-gcc
STRIP=arm-linux-strip

CFLAGS += -I../ceres-gateway-2/include -DDEBUG -DLINUX
LDFLAGS += -L../ceres-gateway-2/lib/$(ARCH) -lcrypto -lssl -lcurl -lpthread -lrt -lm -lPPPP_API -ldl

SRC= \
main.c \
nest_realtime.c \
../ceres-gateway-2/cJSON.c \
../ceres-gateway-2/printbuf.c \


all: nest_realtime

nest_realtime: $(SRC)
	$(CC) -g $(SRC) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	-rm -f nest_realtime
