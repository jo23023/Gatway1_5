include Makefile.common

ARCH=sn98601
CC=arm-linux-gcc
LD=arm-linux-gcc
STRIP=arm-linux-strip

#CFLAGS += -DMEMWATCH -DMW_STDIO
LDFLAGS += -lpthread -lm -lrt
DEBUG += -DDEBUG

SRC = \
AES_EnDe.c \
AES_utils.c \
ceres.c \
ceres_util.c \
prodtest.c \
snx_led.c \
snx_gpio.c \
snx_ms1_gpio.c \
camipc.c \
jsw_protocol.c \
rf_module.c \
testRFMain.c \
md5_util.c

# nest realtime
SRC += ../nest_realtime/nest_realtime.c cJSON.c printbuf.c
CFLAGS += -I../nest_realtime



all: ceres

ceres: $(SRC)
	$(CC) -g -lrt $(SRC) -lcrypto -lssl -I./include/curl lib/$(ARCH)/libcurl.a $(CFLAGS) $(LDFLAGS) $(DEBUG) -o $@ -DLINUX


install:
	mkdir out/$(ARCH) -p
	cp start.sh out/$(ARCH)/
	cp ceres out/$(ARCH)

	cp *.lua out/$(ARCH)
	cp lua out/$(ARCH)
	mkdir out/$(ARCH)/EventList -p
	-cp -a $(shell readlink -m www) out/$(ARCH)/
	cp -a sensor out/$(ARCH)/
	mkdir out/$(ARCH)/lib/ -p
	cp lib/$(ARCH)/*so* out/$(ARCH)/lib/
	find out/$(ARCH) -exec $(STRIP) {} \;
	cp bin/$(ARCH)/* out/$(ARCH)/

	cp mailsend out/$(ARCH)/

	cp ceres_util.sh out/$(ARCH)/
	#cp ceres_util out/$(ARCH)/

	cp ipc_handler.sh out/$(ARCH)/
	#cp ipc_handler out/$(ARCH)/

	cp item.db out/$(ARCH)/
	cp config.db out/$(ARCH)/
	cp resetuser.sh out/$(ARCH)/
	cp resetfactory.sh out/$(ARCH)/
	cp switch_ip_config.sh out/$(ARCH)/
	cp default_dhcp.conf out/$(ARCH)/
	cp check_version.sh out/$(ARCH)/
	cp gpio_reset.sh out/$(ARCH)/
	cp gpio_reset out/$(ARCH)/

uninstall:
	rm -rf out/$(ARCH)/*

clean:
	-rm ceres
	-rm -rf out/
