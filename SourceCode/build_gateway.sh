#!/bin/bash

SD_upgrade_target="target_FW"
usage()
{
	STR=${PLATEFORM_TABLE// /|}

	ECHO "usage : $0 [plateform]{$STR} [optopn]{all|build|buildapp|install|clean|makefw}" COLOR_RED
}

check_env()
{
	which arm-linux-gcc > /dev/null
	if [ $? -gt 0 ]; then
		source_env
	fi
}

source_env()
{
	ECHO "Source env file, plateform : $PLATEFORM" COLOR_YELLOW

	cd ../env
	source ./source_$PLATEFORM.env
	cd - > /dev/null
}

check_gwapp()
{
	ECHO "Check gateway app exist" COLOR_YELLOW

	if [ -e ../../SN98601_GatewayApp/SourceCode/Makefile ]; then
		GATEWAYAPP_EXIST=1
	else
		ECHO "GatewayApp folder need put with GatewayModule in the same folder."
	fi
}

check_plateform()
{
	for NAME in $PLATEFORM_TABLE
	do
		if [ "$1" == "$NAME" ]; then
			PLATEFORM=$NAME
			break
		fi
	done

	if [ -z "$PLATEFORM" ]; then
		usage
		exit 1
	fi
}

build_target()
{
	for TARGET in $TARGET_LIST
	do
		ECHO "Build $TARGET" COLOR_YELLOW

		cd $TARGET
		make -f $TARGET_MAKEFILE.$PLATEFORM
		[ $? -gt 1 ] && ECHO "STOP" COLOR_RED && exit 1
		cd - > /dev/null
	done

	ECHO "Done" COLOR_YELLOW
}

build_gwapp()
{
	if [ $GATEWAYAPP_EXIST -eq 0 ]; then
		return;
	fi

	ECHO "Build Gateway App" COLOR_YELLOW

	cd $GATEWAYAPP_PATH/SourceCode
	make clean && make
	[ $? -gt 1 ] && ECHO "STOP" COLOR_RED && exit 1
	cd - > /dev/null

	ECHO "Install Gateway App" COLOR_YELLOW

	case $PLATEFORM in
		rt5350)
			mkdir $MODULE_PATH/out/$PLATEFORM/out -p
			cp $GATEWAYAPP_PATH/SourceCode/out868/* $MODULE_PATH/out/$PLATEFORM/out/
			;;
		sn98601)
			mkdir $MODULE_PATH/out/$PLATEFORM/out -p
			cp $GATEWAYAPP_PATH/SourceCode/out868/* $MODULE_PATH/out/$PLATEFORM/out/
			;;
		*)
			ECHO "Unknow plateform:"$PLATEFORM", exit!" COLOR_RED
			exit 1
			;;
	esac
}

clean_target()
{
	for TARGET in $TARGET_LIST
	do
		ECHO "Clean $TARGET" COLOR_YELLOW

		cd $TARGET
		make -f $TARGET_MAKEFILE.$PLATEFORM clean
		cd - > /dev/null
	done

	ECHO "Done" COLOR_YELLOW
}

install_target()
{
	ECHO "Install ceres-gateway-2" COLOR_YELLOW

	cd $MODULE_PATH
	make -f $TARGET_MAKEFILE.$PLATEFORM install
	cd - > /dev/null

	#mkdir $MODULE_PATH/out/$PLATEFORM -p

	for TARGET in $TARGET_FILE_LIST
	do
		ECHO "Install $TARGET" COLOR_YELLOW
		[ ! -e $TARGET ] && ECHO "File $TARGET no exist!" COLOR_RED && exit 1
		cp $TARGET $MODULE_PATH/out/$PLATEFORM/
	done

	# Another file
	cp Gateway_wdt/gateway_wdt.sh $MODULE_PATH/out/$PLATEFORM/

	ECHO "Done" COLOR_YELLOW
}

make_firmware()
{
	ECHO "Build gwteway FW, plateform:"$PLATEFORM

	build_gwapp
}

#script start
source ../env/script.lib

MODULE_PATH=./ceres-gateway-2
# ceres-gateway-2 must be the last one
TARGET_LIST=" \
				ceres-util \
				IPCam_Handler_2 \
				Gateway_wdt \
				ceres-gateway-2"
# not include ceres-gateway-2
TARGET_FILE_LIST=" \
					ceres-util/ceres_util \
					IPCam_Handler_2/ipc_handler \
					Gateway_wdt/gateway_wdt"
TARGET_MAKEFILE="Makefile"

PLATEFORM_TABLE="sn98601"
PLATEFORM=""

GATEWAYAPP_EXIST=0
GATEWAYAPP_PATH="../../SN98601_GatewayApp"

if [ "$2" == "" ] ; then
	COMMAND="all"
else
	COMMAND=$2
fi
if [ "$1" == "" ] ; then 
	check_plateform "sn98601"	
else
	check_plateform $1
fi 
check_env
check_gwapp

case $COMMAND in
	all)
		clean_target
		build_target
		install_target
		make_firmware
		cp -f ceres-gateway-2/out/sn98601/ceres $SD_upgrade_target/ceres2
		cp -f Gateway_wdt/gateway_wdt $SD_upgrade_target/
		cp -f ceres-gateway-2/push_string.cfg $SD_upgrade_target/
		cp -f updateUtil/update $SD_upgrade_target/
#		cp -f ceres-gateway-2/out/sn98601/ceres /var/www/html/
		;;
	build)
		build_target
		;;
	buildapp)
		build_gwapp
		;;
	install)
		install_target
		;;
	clean)
		clean_target
		;;
	makefw)
		make_firmware
		;;
	*)
		usage
		exit 1
		;;
esac
