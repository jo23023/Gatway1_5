echo "resetuser start"

FILE_PATH="/mnt/config.db"
NEW_PW='123456'

echo
echo "check admin_password"
ADMIN_PW=$(grep "admin_password" ${FILE_PATH} | sed 's/^.* = "//g' | sed 's/";$//g')
if [ "$ADMIN_PW" =  "" ]; then
	echo "   Can NOT find admin_password"
else
	sed -i "s,$ADMIN_PW,$NEW_PW,g" ${FILE_PATH}
	echo "   reset admin_password to $NEW_PW"
fi  


echo
echo "check gateway_password"
GATEWAY_PW=$(grep "gateway_password" ${FILE_PATH} | sed 's/^.* = "//g' | sed 's/";$//g')
if [ "$GATEWAY_PW" =  "" ]; then
	echo "   Can NOT find gateway_password"
else
	sed -i "s,$GATEWAY_PW,$NEW_PW,g" ${FILE_PATH}
	echo "   reset gateway_password to $NEW_PW"
fi  

sync

echo
echo "resetuser finish"
