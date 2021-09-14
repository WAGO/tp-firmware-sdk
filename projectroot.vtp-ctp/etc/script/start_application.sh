#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_application.sh
#
# Brief:    starting up applications
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

. /etc/profile > /dev/null 2>&1

HOME=/root
eeprom_device="/sys/bus/i2c/devices/1-0054/eeprom"
ORDER="$(/etc/config-tools/get_typelabel_value ORDER)"

#SET FEATURES
if [ "${ORDER:0:3}" == "762" ]; then
  #TP600
  echo "hardware-available=true" >"/etc/specific/features/display"
  echo "hardware-available=true" >"/etc/specific/features/touch"
  echo "hardware-available=false" >"/etc/specific/features/hdmi"
elif [ "${ORDER:0:5}" == "752-8" ]; then
  #EC752-8303
  echo "hardware-available=false" >"/etc/specific/features/display"
  echo "hardware-available=false" >"/etc/specific/features/touch"
  echo "hardware-available=true" >"/etc/specific/features/hdmi"
fi

if [ ! -d "/tmp/font_uploads" ]; then
  mkdir /tmp/font_uploads
  chown www:www /tmp/font_uploads
fi

if [ ! -d "/tmp/png_uploads" ]; then
  mkdir /tmp/png_uploads
  chown www:www /tmp/png_uploads
fi

if [ ! -d "/dev/shm" ]; then
  echo "NOT FOUND /dev/shm"
  mkdir /dev/shm
  chmod 1777 /dev/shm
fi

#create QT5 XDG_RUNTIME_DIR
if [ ! -d "/tmp/runtime-root" ]; then
  mkdir /tmp/runtime-root
  chmod 0700 /tmp/runtime-root
fi

function StartVirtualKeyboard
{
  #START VIRTUALKEYBOARD
  VIRTUALKEYBOARD=`getconfvalue /etc/specific/virtualkeyboard.conf state -l`
  if [ "$VIRTUALKEYBOARD" == "enabled" ]; then
  PIDVIRTUALKEYBOARD=`pidof virtualkeyboard`
  if [ -z $PIDVIRTUALKEYBOARD ]; then
    echo "START VIRTUALKEYBOARD"
    if [ -z "$QT_IM_MODULE" ]; then
      export QT_IM_MODULE=vkim
    fi
    /usr/bin/virtualkeyboard 2>&1 > /dev/null &
    # needs some time to start correctly
    sleep 3
  fi
  fi
}

function StartAudioserver
{
  #START AUDIOSERVER
  PIDAUDIOSERVER=`pidof audioserver`
  if [ -z $PIDAUDIOSERVER ]; then
    echo "START AUDIOSERVER"
    /usr/bin/audioserver 2>&1 > /dev/null
  fi
}

function StartTP
{
  local RUNNING_VERSION="$(/etc/config-tools/get_runtime_config running-version)"
  echo "STARTTP, RUNNING_VERSION: $RUNNING_VERSION"
  if [ -e "/var/run/framebuffer.mode" ]; then
    #start WBM coming from framebuffer mode and microbrowser is closed
    if [ "$RUNNING_VERSION" == "3" ]; then
      mv -f "/var/run/framebuffer.mode" "/var/run/framebuffer.done"
      /etc/init.d/runtime start
      sleep 1
    else
      /etc/script/start_wbm.sh 2>&1 > /dev/null &
    fi
  elif [ "$RUNNING_VERSION" == "0" ]; then
    /etc/script/start_webbrowser.sh 2>&1 > /dev/null &
  fi
}

function InitTouchbeeper
{
if [ "${ORDER:0:3}" == "762" ]; then
  #initialize touchbeeper 
  /etc/config-tools/config_touchbeeper init
  #echo "initialize touchbeeper: $?"
fi
}

function InitMotionsensor
{
if [ "${ORDER:0:3}" == "762" ]; then
  mount -t configfs none /sys/kernel/config
  modprobe si1145
  daemonize "/usr/bin/si1142"

  if [ ! -e /etc/specific/motionsensor.conf ]; then
  /etc/config-tools/config_motionsensor resettofactory
  fi

  # initialize motion sensor
  MOTIONSENSOR=`getconfvalue /etc/specific/motionsensor.conf state -l`
  #echo "$? MOTIONSENSOR=$MOTIONSENSOR"

  if [ "$MOTIONSENSOR" == "enabled" ]; then  
    /etc/config-tools/config_motionsensor init
    #echo "initialize motionsensor: $?"
  fi
fi
}

function InitTestability
{
  #START SCREENSHOTQT
  TESTABILITY_SCREENSHOT=`getconfvalue /etc/specific/testability.conf screenshot -l`
  if [ "$TESTABILITY_SCREENSHOT" == "enabled" ]; then
  PIDSCREENSHOTQT=`pidof screenshotqt`
  if [ -z $PIDSCREENSHOTQT ]; then
    #echo "start screenshotqt"
    screenshotqt &
  fi
  fi
}

function StartBrightnessControl
{
if [ "${ORDER:0:3}" == "762" ]; then
  #start brightness control at the end of this script
  PIDBRIGHT=`pidof brightness_control`
  if [ -z $PIDBRIGHT ]; then
    #echo "start brightness control"
    brightness_control &
  fi
fi
}

function StartGestureControl
{
if [ "${ORDER:0:3}" == "762" ]; then
  #START MENUQT and GESTURECONTROL
  GESTURE_STATE=`getconfvalue /etc/specific/gesture.conf state -l`
  GESTURE_MENU=`getconfvalue /etc/specific/gesture.conf menu -l`
  if [ "$GESTURE_STATE" == "enabled" ]; then
  if [ "$GESTURE_MENU" == "yes" ]; then
    PIDMENUQT=`pidof menuqtslide`
    if [ -z $PIDMENUQT ]; then
      #echo "start menuqt"    
      /usr/bin/menuqtslide -nodetection &
      #sleep 1
    fi
    PIDGESTURE=`pidof gesture_control`
    if [ -z $PIDGESTURE ]; then

      ### get xres
      #if systeminfo is provided by cmdline use it, otherwise read eeprom 
      (cat /proc/cmdline | grep "systeminfo=" ) &>/dev/null
      if [ $? -eq 0 ]; then
        xres=$(cat /proc/cmdline)
        xres=$(echo ${xres#*systeminfo=0x00})
        xres=$(echo ${xres:1:2})
        if [ "$xres" == "1" ] ; then
          devconf="1000"
        elif [ "$xres" == "2" ] ; then
          devconf="1001"
        elif [ "$xres" == "3" ] ; then
          devconf="1002"
        elif [ "$xres" == "5" ] ; then
          devconf="1003"
        elif [ "$xres" == "6" ] ; then
          devconf="1008"
        elif [ "$xres" == "7" ] ; then
          devconf="1009"
        elif [ "$xres" == "8" ] ; then
          devconf="1010"
        else
          echo "screen resolution touch not recognized set default 800x480"
          xres="800"
        fi
      else
        # no systeminfo use eeprom, read devconf
        devconf=$(hexdump -v -s 0x00001fe -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
      fi

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
        else
          cap="0"
        fi
      elif [ "$devconf" = "1009" ]; then
        (cat /proc/bus/input/devices | grep "eGalax Inc. eGalaxTouch P80H84") &>/dev/null
        if [ $? -eq 0 ]; then
          cap="1"
        else
          cap="0"
        fi
      elif [ "$devconf" = "1010" ]; then
        cap="0"
      else
        echo "no valid display resolution in eeprom found, use default resolution 800x480"
        cap="0"
      fi

      if [ "$cap" == "1" ] ; then
        echo "start gesture_control_cap"
        /usr/bin/gesture_control_cap &
      elif [ "$cap" == "0" ] ; then
        echo "start gesture_control"
        /usr/bin/gesture_control &
      else
        echo "cap/res touch not recognized"
      fi
    fi
  fi
  fi
fi
}

#Startsequence
StartVirtualKeyboard
StartAudioserver
InitTouchbeeper
InitMotionsensor
InitTestability
StartTP
StartBrightnessControl
StartGestureControl
#Xorg and fluxbox are running so /etc/init.d/runtime is able to start codesys3
touch /var/run/xorg.start.done

/etc/script/setup_hdmi.sh

#not with fluxbox if [ -e /etc/script/wndactivate.sh ]; then
#not with fluxbox /etc/script/wndactivate.sh 2>&1 > /dev/null &
#not with fluxbox fi
