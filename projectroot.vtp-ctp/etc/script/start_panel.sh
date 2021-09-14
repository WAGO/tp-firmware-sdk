#!/bin/sh
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_panel.sh
#
# Brief:    starting up panel / Xorg
#
# Author:   Ralf Gliese: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#


eeprom_device="/sys/bus/i2c/devices/1-0054/eeprom"
hdmi=0

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
		if [ "$xres" == "1" ] ; then
			devconf="1000"
			addchar_xres="_480_272"
		elif [ "$xres" == "2" ] ; then
			devconf="1001"
			addchar_xres="_640_480"
		elif [ "$xres" == "3" ] ; then
			devconf="1002"
			addchar_xres="_800_480"
		elif [ "$xres" == "5" ] ; then
			devconf="1003"
			addchar_xres="_1280_800"
		elif [ "$xres" == "6" ] ; then
			devconf="1008"
			addchar_xres="_1920_1080"
		elif [ "$xres" == "7" ] ; then
			devconf="1009"
			addchar_xres="_1920_1080"
		elif [ "$xres" == "8" ] ; then
			devconf="1010"
			hdmi="1"
			addchar_xres="_1920_1080"
		else
			echo "screen resolution touch not recognized set default 800x480"
			addchar_xres="_800_480"
			devconf="1002"
		fi
	else
		# no systeminfo use eeprom, read devconf
		devconf=$(hexdump -v -s 0x00001fe -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
		if [ "$devconf" == "1000" ] ; then
			addchar_xres="_480_272"
		elif [ "$devconf" == "1001" ] ; then
			addchar_xres="_640_480"
		elif [ "$devconf" == "1002" ] ; then
			addchar_xres="_800_480"
		elif [ "$devconf" == "1003" ] ; then
			addchar_xres="_1280_800"
		elif [ "$devconf" == "1008" ] ; then
			addchar_xres="_1920_1080"
		elif [ "$devconf" == "1009" ] ; then
			addchar_xres="_1920_1080"
		elif [ "$devconf" == "1010" ] ; then
			hdmi="1"
			addchar_xres="_1920_1080"
		else
			echo "screen resolution touch not recognized set default 800x480"
			addchar_xres="_800_480"
		fi
	fi
	echo 

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

	if [ "$cap" == "1" ] ; then
		echo "cap touch found"
		addchar="_cap"
		xorg_conf_file=$xorg_conf_file$addchar
		#echo 15 > /sys/bus/i2c/devices/2-001b/NTHR_VALUE
	elif [ "$cap" == "0" ] ; then
		echo "res touch found"
		#echo 10 > /sys/bus/i2c/devices/2-001b/NTHR_VALUE
	elif [ "$hdmi" == "1" ] ; then
		echo "hdmi found"
		addchar="_hdmi"
		xorg_conf_file=$xorg_conf_file$addchar
		#echo 10 > /sys/bus/i2c/devices/2-001b/NTHR_VALUE
	else
		echo "cap/res touch not recognized"
	fi

	xorg_conf_file=$xorg_conf_file$addchar_xres

	### get screen orientation
	orientation=$(hexdump -v -s 0x00001f8 -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
	orientation=$(echo ${orientation:3})
	if [ "$orientation" == 0 ] ; then
		addchar=".conf"
		xorg_conf_file=$xorg_conf_file$addchar
	elif [ "$orientation" == 1 ] ; then
		addchar="_CW.conf"
		xorg_conf_file=$xorg_conf_file$addchar
	elif [ "$orientation" == 2 ] ; then
		addchar="_UD.conf"
		xorg_conf_file=$xorg_conf_file$addchar
	elif [ "$orientation" == 3 ] ; then
		addchar="_CCW.conf"
		xorg_conf_file=$xorg_conf_file$addchar
	else
		echo "screen orientation touch not recognized"
		addchar=".conf"
		xorg_conf_file=$xorg_conf_file$addchar
	fi

	if [ -e "$xorg_conf_file" ] ; then
		rm -f /etc/X11/xorg.conf
		ln -s $xorg_conf_file /etc/X11/xorg.conf
		#echo "#### xorg_conf_file=$xorg_conf_file"
	else
		echo "xorg.conf not found ${xorg_conf_file}"
	fi
}

set_xorg_conf

# create java cache directory
mkdir /tmp/.java
mkdir /tmp/.java/deployment
mkdir /tmp/.java/deployment/cache
chmod 755 /tmp/.java/deployment/cache

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
