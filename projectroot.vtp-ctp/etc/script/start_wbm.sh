#!/bin/bash
#-----------------------------------------------------------------------------#
# Script-name: start_wbm.sh
#
# starting up webenginebrowser
#
# Author: Wolfgang Rückl,  elrest GmbH
#-----------------------------------------------------------------------------#

. /etc/profile > /dev/null 2>&1
HOME=/root

URL=`/etc/config-tools/get_plcselect 0 url`
VKB=`/etc/config-tools/get_plcselect 0 vkb`
if [ -z "$URL" ]; then
  URL="http://127.0.0.1/wbm-ng/index.html"
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

PIDVKB=`pidof virtualkeyboard`
if [ ! -z $PIDVKB ]; then
  if [ "$VKB" == "enabled" ]; then
    echo "enabled" > /dev/virtualkeyboard
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
