CONFIG_FILE="/mnt/config.db"

# get SD Card dirctory path
MQTT_FILE=/media/mmcblk0/sn98601/mqttd

if [ -f "${MQTT_FILE}" ]; then
	SD_DIR=/media/mmcblk0
else
	SD_DIR=/media/mmcblk0p1
fi

echo "SD Path is: ${SD_DIR}"

echo "check gateway version"
OLD_VERSION=$(grep "gateway_app_version" ${CONFIG_FILE} | sed 's/^.* = "//g' | sed 's/";$//g')
NEW_VERSION=$(grep "gateway_app_version" ${SD_DIR}/sn98601/config.db | sed 's/^.* = "//g' | sed 's/";$//g')

echo "old version is: ${OLD_VERSION}"
echo "new version is: ${NEW_VERSION}"

if [ "${NEW_VERSION}" == "" ]; then
	echo "Can NOT find new version"
elif [ "${NEW_VERSION}" != "${OLD_VERSION}" ]; then
	sed -i "s,${OLD_VERSION},${NEW_VERSION},g" ${CONFIG_FILE}
	echo "change gateway version to ${NEW_VERSION}"
fi

echo "check temparature freq"
OLD_FREQ=$(grep "frequency" ${CONFIG_FILE} | sed 's/^.* = "//g' | sed 's/";$//g')
NEW_FREQ=$(grep "frequency" ${SD_DIR}/sn98601/config.db | sed 's/^.* = "//g' | sed 's/";$//g')

echo "old version is: ${OLD_FREQ}"
echo "new version is: ${NEW_FREQ}"

if [ "${NEW_FREQ}" == "" ]; then
	echo "Can NOT find new temparature frequency"
elif [ "${NEW_FREQ}" != "${OLD_FREQ}" ]; then
	sed -i "s,${OLD_FREQ},${NEW_FREQ},g" ${CONFIG_FILE}
	echo "change temparature freq to ${NEW_FREQ}"
fi