#!/bin/sh

# Clean and set dmesg level
/bin/dmesg -c > /dev/null
/bin/dmesg -n 1

# Run watchdog
./gateway_wdt &

sleep 3
if [ -e /var/run/gateway_wdt.pid ]; then
	PID="`cat /var/run/gateway_wdt.pid`"
	echo -17 > /proc/$PID/oom_adj
fi
