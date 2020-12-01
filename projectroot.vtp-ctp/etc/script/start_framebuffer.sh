#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_framebuffer.sh
#
# Brief:    starting up ininet microbrowser
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

DAEMONIZE="/sbin/start-stop-daemon"

# mark framebuffer mode to detect framebuffer mode in other scripts
touch /var/run/framebuffer.mode

#show configured website 
URL="$1"
ORDER="$(/etc/config-tools/get_typelabel_value ORDER)"

function InitCapTouch
{
if [ "${ORDER:0:3}" == "762" ]; then
  local TOUCH="$(/etc/config-tools/get_typelabel_value TOUCHTYPE)"
  if [ "$TOUCH" == "CAP" ]; then
    export TSLIB_FBDEVICE=/dev/fb0
    export TSLIB_TSDEVICE=/dev/input/captouch
    /usr/bin/ts_calibrate
  fi
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

function StartBrightnessControl
{
local PROGRAM="/usr/bin/brightness_control"
if [ "${ORDER:0:3}" == "762" ]; then
  #start brightness control at the end of this script
  PIDBRIGHT=`pidof brightness_control`
  if [ -z $PIDBRIGHT ]; then
    #echo "start brightness control"
    "$DAEMONIZE" -S -x "$PROGRAM" -o
  fi
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

#Initialize device
InitTouchbeeper
InitMotionsensor
StartBrightnessControl

#Calibrate CAP TOUCH for TSLIB mode
InitCapTouch

#Configure microbrowsers backlight screensaver
SCREENSAVER="$(/etc/config-tools/get_screensaver state)"
if [ "$SCREENSAVER" == "enabled" ]; then  
  TME_SEC="$(/etc/config-tools/get_screensaver time)"
  TME_MIN=$(($TME_SEC/60))
  if [ "$TME_MIN" == "0" ]; then
    TME_MIN=1
  fi
  /etc/config-tools/config_microbrowser "MB_Config.LCD.BLITimeout=$TME_MIN"
else
  /etc/config-tools/config_microbrowser "MB_Config.LCD.BLITimeout=0"
fi

#Start Microbrowser
if [ ! -z "$URL" ]; then
  /etc/config-tools/start_microbrowser $URL > /dev/null 2>&1 &
fi
