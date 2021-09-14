#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_webbrowser.sh
#
# Brief:    starting up webenginebrowser
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#


. /etc/profile > /dev/null 2>&1
HOME=/root

#avoid black screen after WUP
#codesys SP16 runtime shows a black window only on first startup which overlays the browser window
function BringToFront
{
  if [ ! -e /home/codesys/eRUNTIME.cfg ]; then
    LINE=$(head -n 1 /etc/specific/rtsversion)
    RTSVER=${LINE:0:1}
    if [ "$RTSVER" == "3" ]; then
       PID_SCRIPT=$(pidof activate_browser_wnd.sh)
       if [ -z "$PID_SCRIPT" ]; then
         /etc/script/activate_browser_wnd.sh > /dev/null 2>&1 &
       fi
    fi
  fi
}
BringToFront

URL=`/etc/config-tools/get_plcselect 0 url`
if [ -z "$URL" ]; then
  URL="http://127.0.0.1/wbm/index.html"
fi

CONF_ZOOM=`getconfvalue /etc/specific/webengine/webengine.conf zoom`
PIDWEBENGINE=`pidof webenginebrowser`

#create QT5 XDG_RUNTIME_DIR
if [ ! -d "/tmp/runtime-root" ]; then
  mkdir /tmp/runtime-root
  chmod 0700 /tmp/runtime-root
fi

if [ -z "$QT_IM_MODULE" ]; then
  export QT_IM_MODULE=vkim
fi

function ShowUrl
{
  local PINCH_GESTURE=`getconfvalue /etc/specific/webengine/webengine.conf zoom`
  if [ -z $PIDWEBENGINE ]; then
    #browser is not started
    if [ "$PINCH_GESTURE" == "0" ]; then
      #echo "disable pinch"
      echo "1" > /tmp/custom_disable_pinch.txt
    fi
    /usr/bin/webenginebrowser $URL > /dev/null 2>&1 &
  fi
}

#Use Plclist to get initial Browser URL

#PLCCOUNT=`/etc/config-tools/get_plcselect plc_count`  
#echo "plccount $PLCCOUNT"  
# several PLCs / URLs configured (list)
PLCSELECTED=`/etc/config-tools/get_plcselect plc_selected`
#echo "plcselected $PLCSELECTED" 
if [ $PLCSELECTED -lt 0 ]; then
  echo "plcselected < 0"
  PLCSELECTED=0
fi

if [ -e "/var/run/framebuffer.done" ]; then
  #start WBM in case of framebuffer / microbrowser mode ends
  PLCSELECTED=0
fi

#show configured website 
URL=`/etc/config-tools/get_plcselect $PLCSELECTED url`
VKB=`/etc/config-tools/get_plcselect $PLCSELECTED vkb`
if [ -z "$URL" ]; then
 echo "URL is empty"
 URL=`/etc/config-tools/get_plcselect 0 url`
 VKB=`/etc/config-tools/get_plcselect 0 vkb`
fi

PIDVKB=`pidof virtualkeyboard`
if [ ! -z $PIDVKB ]; then
  if [ "$VKB" == "disabled" ]; then
    echo "disabled" > /dev/virtualkeyboard
  fi
fi

STATEMONITOR=`getconfvalue /etc/specific/plcselect.conf statemonitor`
PIDTCPMONITOR=`pidof tcp_monitor`
if [ "$STATEMONITOR" == "enabled" ]; then
  if [ -z $PIDTCPMONITOR ]; then
    /usr/bin/tcp_monitor > /dev/null 2>&1 &
  fi
fi

ShowUrl
