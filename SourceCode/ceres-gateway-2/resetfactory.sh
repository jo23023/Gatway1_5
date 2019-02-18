echo "resetfactory start"

echo
echo "Setup DHCP"
nvram_set wanConnectionMode DHCP

echo "delete all db files"
DIR="/mnt"

DB_FILE="${DIR}/aeskey.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi

DB_FILE="${DIR}/armlinktable.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi

DB_FILE="${DIR}/config.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi


DB_FILE="${DIR}/item.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi

DB_FILE="${DIR}/item_status.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi

DB_FILE="${DIR}/linktable.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi

DB_FILE="${DIR}/schedule.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi

DB_FILE="${DIR}/virtualdevice.db"
if [ -f "${DB_FILE}" ]; then
	rm -rf ${DB_FILE}
	echo "   delete ${DB_FILE}"
fi


echo
echo "reset config.db"
cp -f "./config.db" "${DIR}"

echo 
echo "reset item.db"
cp -f "./item.db" "${DIR}"

echo
echo "delete all EventList"
rm -rf "./EventList"
mkdir "./EventList"

sync

echo 
echo "resetfactory finish"

chmod -R 777 /mnt

sync

echo "resetuser finish"

echo "reboot"
reboot
