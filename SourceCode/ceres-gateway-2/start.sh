#!/bin/sh -x

# Save pid itself
echo $$ > /var/run/start.sh.pid

export LD_LIBRARY_PATH=lib

# ensure databases are writable
chmod -R 777 /mnt

# check firmware version
if [ -f "/media/mmcblk0/sn98601/check_version.sh" ]; then
	sh /media/mmcblk0/sn98601/check_version.sh
else
	sh /media/mmcblk0p1/sn98601/check_version.sh
fi
# Run gateway watchdog
./gateway_wdt.sh&

./gpio_reset.sh&

./mqttd -c mqttd.conf&

sleep 1

./ceres_util.sh&

./ipc_handler.sh&

while [ 1 ] 
do
	./lua startup.lua
done
