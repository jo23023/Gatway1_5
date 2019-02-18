
NET=/etc/network
#NET=.

#Reset network setting to default (DHCP enable)
cp -f default_dhcp.conf $NET/interfaces
	

#check dhcp_enable seeting
dhcp_enable=`cat /mnt/config.db |grep dhcp_enable |cut -d '"' -f4`
if [ "$dhcp_enable" = '0' ]; then 
	#overwrite dhcp setting by static mode

	#read user setting
	ip=`cat /mnt/config.db| grep wan_ip |cut -d '"' -f4 `
	mask=`cat /mnt/config.db|grep wan_netmask|cut -d '"' -f4 `
	gateway=`cat /mnt/config.db|grep wan_gateway|cut -d '"' -f4 `
	dns=`cat /mnt/config.db|grep wan_dns|cut -d '"' -f4 `
	
	#check the address value
	check=' grep ^[0-9]\{1,3\}\.[0-9]\{1,3\}.[0-9]\{1,3\}.[0-9]\{1,3\}$'
	echo ---------- Network Settings: -----------
	echo $ip    | $check 	;ip_ok=$?
	echo $mask  | $check	;mask_ok=$?
	echo $gateway | $check	;gateway_ok=$?
	echo $dns   | $check	;dns_ok=$?

	#Make sure the referance is correct 
	if [ $ip_ok == 0 ] && [ $gateway_ok == 0 ] ; then 	
		sed -i 's/iface eth0 inet dhcp/iface eth0 inet static/g' $NET/interfaces

		#write setting to /etc/network/interfaces	
		sed -i "\$a address $ip" 		$NET/interfaces	
		sed -i "\$a gateway $gateway"   	$NET/interfaces	

		#Auto set mask
		if [ $mask_ok == 0 ] ; then
			sed -i "\$a netmask $mask"  		$NET/interfaces	
		else
			sed -i "\$a netmask 255.255.255.0"	$NET/interfaces	
		fi

		#Auto set dns
		if [ $dns_ok == 0 ] ; then
			sed -i "\$a dns-nameservers $dns"   	$NET/interfaces	
		else
			sed -i '\$a dns_nameservers 8.8.8.8 168.95.162.1'
		fi

		echo "Switch to static IP"

	else
		echo "Statiac ip setting is incorrect! Switch to dynamic ip"
	fi
else 
	#Default:do nothing
	echo "Switch to dynamic IP"

fi

#Restart network service
/etc/init.d/S40network restart

