#!/bin/sh
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_panel.sh
#
# Brief:    starting up panel / Xorg
#
# Author:   Ralf Gliese: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

HDMI_CONFIG="/etc/specific/hdmi_resolution.conf"
eeprom_device="/sys/bus/i2c/devices/1-0054/eeprom"
hdmi="0"

#pio1 pio2 retain fix
/etc/script/retain_pio2.sh

# set xorg.conf
set_xorg_conf () {
	### get devconf
	#if systeminfo is provided by cmdline use it, otherwise read eeprom 
	(cat /proc/cmdline | grep "systeminfo=" ) &>/dev/null
	if [ $? -eq 0 ]; then
		xres=$(cat /proc/cmdline)
		xres=$(echo ${xres#*systeminfo=0x00})
		xres=$(echo ${xres:1:2})
		if [ "$xres" = "1" ] ; then
			devconf="1000"
			addchar_xres="_480_272"
		elif [ "$xres" = "2" ] ; then
			devconf="1001"
			addchar_xres="_640_480"
		elif [ "$xres" = "3" ] ; then
			devconf="1002"
			addchar_xres="_800_480"
		elif [ "$xres" = "5" ] ; then
			devconf="1003"
			addchar_xres="_1280_800"
		elif [ "$xres" = "6" ] ; then
			devconf="1008"
			addchar_xres="_1920_1080"
		elif [ "$xres" = "7" ] ; then
			devconf="1009"
			addchar_xres="_1920_1080"
		elif [ "$xres" = "8" ] ; then
			devconf="1010"
			hdmi="1"
			if [ -e "$HDMI_CONFIG" ]; then
				#echo "HDMI_CONFIG found"
				hdmi_res=$(cat /etc/specific/hdmi_resolution.conf)
				hdmi_res=${hdmi_res#*=}
				hdmi_res=$(echo $hdmi_res | cut -d '"' -f 2)
				if [ "$hdmi_res" = "1920x1080" ]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1920_1080"
				elif [[ "$hdmi_res" = "1600x1024" ]]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1600_1024"
				elif [[ "$hdmi_res" = "1280x1024" ]]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1280_1024"
				elif [ "$hdmi_res" = "1024x768" ]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1024_768"
				elif [ "$hdmi_res" = "800x480" ]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_800_480"
				else
					echo "$HDMI_CONFIG contains no valid resolution, set default 800x480"
					addchar_xres="_800_480"
				fi
			else
				addchar_xres="_1920_1080"
			fi
		else
			echo "screen resolution touch not recognized set default 800x480"
			addchar_xres="_800_480"
			devconf="1002"
		fi
	else
		# no systeminfo use eeprom, read devconf
		devconf=$(hexdump -v -s 0x00001fe -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
		if [ "$devconf" = "1000" ] ; then
			addchar_xres="_480_272"
		elif [ "$devconf" = "1001" ] ; then
			addchar_xres="_640_480"
		elif [ "$devconf" = "1002" ] ; then
			addchar_xres="_800_480"
		elif [ "$devconf" = "1003" ] ; then
			addchar_xres="_1280_800"
		elif [ "$devconf" = "1008" ] ; then
			addchar_xres="_1920_1080"
		elif [ "$devconf" = "1009" ] ; then
			addchar_xres="_1920_1080"
		elif [ "$devconf" = "1010" ] ; then
			hdmi="1"
			if [ -e "$HDMI_CONFIG" ]; then
				#echo "HDMI_CONFIG found"
				hdmi_res=$(cat /etc/specific/hdmi_resolution.conf)
				hdmi_res=${hdmi_res#*=}
				hdmi_res=$(echo $hdmi_res | cut -d '"' -f 2)
				if [ "$hdmi_res" = "1920x1080" ]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1920_1080"
				elif [[ "$hdmi_res" = "1600x1024" ]]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1600_1024"
				elif [[ "$hdmi_res" = "1280x1024" ]]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1280_1024"
				elif [ "$hdmi_res" = "1024x768" ]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_1024_768"
				elif [ "$hdmi_res" = "800x480" ]; then
					#echo "hdmi_res=$hdmi_res"
					addchar_xres="_800_480"
				else
					echo "$HDMI_CONFIG contains no valid resolution, set default 800x480"
					addchar_xres="_800_480"
				fi
			else
				addchar_xres="_1920_1080"
			fi
		else
			echo "screen resolution not recognized set default 800x480"
			addchar_xres="_800_480"
		fi
	fi
	echo "addchar_xres=$addchar_xres"

	xorg_conf_file=/etc/X11/xorg

	### get touch type
	if [ "$devconf" = "1000" ]; then
		cap="0"
	elif [ "$devconf" = "1001" ]; then
		cap="0"
	elif [ "$devconf" = "1002" ]; then
		(cat /proc/bus/input/devices | grep "PIXCIR HID Touch Panel") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
		else
			cap="0"
		fi
	elif [ "$devconf" = "1003" ]; then
		(cat /proc/bus/input/devices | grep "PIXCIR HID Touch Panel") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
		else
			cap="0"
		fi
	elif [ "$devconf" = "1008" ]; then
		(cat /proc/bus/input/devices | grep "eGalax Inc. eGalaxTouch P80H84") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
			echo 6 > /sys/bus/i2c/devices/2-001b/nthr_value
		else
			cap="0"
		fi
	elif [ "$devconf" = "1009" ]; then
		(cat /proc/bus/input/devices | grep "eGalax Inc. eGalaxTouch P80H84") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
			echo 6 > /sys/bus/i2c/devices/2-001b/nthr_value
		else
			cap="0"
		fi
	elif [ "$devconf" = "1010" ]; then
		hdmi="1"
	else
		echo "no valid display resolution in eeprom found, use default resolution 800x480"
		cap="0"
	fi

	if [ "$hdmi" = "1" ] ; then
		echo "hdmi found"
		if [ -e "/dev/input/touchscreen" ]; then
			#echo "found resistive touchscreen"
			addchar="_hdmi"
		else
			#echo "resistive touchscreen not found" 
			addchar="_hdmi_cap"
		fi		
		xorg_conf_file=$xorg_conf_file$addchar
	else
		if [ "$cap" = "1" ] ; then
			echo "cap touch found"
			addchar="_cap"
			xorg_conf_file=$xorg_conf_file$addchar
		elif [ "$cap" = "0" ] ; then
			echo "res touch found"
		else
			echo "cap/res touch not recognized"
		fi
	fi

	xorg_conf_file=$xorg_conf_file$addchar_xres
	addchar=".conf"
	xorg_conf_file=$xorg_conf_file$addchar

	if [ -e "$xorg_conf_file" ] ; then
		rm -f /etc/X11/xorg.conf
		ln -s $xorg_conf_file /etc/X11/xorg.conf
		#echo "#### xorg_conf_file=$xorg_conf_file"
	else
		echo "xorg.conf not found ${xorg_conf_file}"
	fi
}

set_xorg_conf

# allow user www to read brightness
brightness_file=/sys/class/backlight/backlight/brightness
if [ -e "$brightness_file" ]; then
chmod 666 "$brightness_file"
fi

# allow user www to set / read display orientation
chmod 666 $eeprom_device
echo 23 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio23/direction
chmod 666 /sys/class/gpio/gpio23/value


# decide start mode Xorg or framebuffer
MICROBROWSER="0"
URL=""
PLCSELECTED=`/etc/config-tools/get_plcselect plc_selected`
URL=`/etc/config-tools/get_plcselect $PLCSELECTED url`
MICROBROWSER=`/etc/config-tools/get_plcselect $PLCSELECTED mic`

ORDER="$(/etc/config-tools/get_typelabel_value ORDER)"
if [ "${ORDER:0:5}" == "752-8" ]; then
	#EDGE Controller
	if [ "$MICROBROWSER" == "1" ]; then
		#No MicroBrowser on EDGE Box possible
		MICROBROWSER="0"
		#set startpage to WBM
		/etc/config-tools/config_plcselect plc_selected=0
		#show notification
		/usr/bin/dialogbox "EC752 does not support the MicroBrowser." Ok -s 10 -platform linuxfb
		# blank framebuffer to black background
		dd if=/dev/zero of=/dev/fb0
	fi
fi

if [ "$MICROBROWSER" == "1" ]; then
	# without Xorg except EDGE-BOX
	/etc/script/start_framebuffer.sh "$URL" &
else
	# start X
	export DISPLAY=:0
	echo "STARTING Xorg"
	# look for usb mouse pugged in
	grep -w "EV=17" /proc/bus/input/devices
	if [ "$?" == "0" ]; then
		/usr/bin/startx &
	else
		/usr/bin/startx -- -nocursor &
	fi
fi
