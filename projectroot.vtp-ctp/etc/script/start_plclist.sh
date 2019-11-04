#!/bin/bash

. /etc/profile > /dev/null 2>&1
HOME=/root

URL="http://127.0.0.1/plclist/plclist.html"
PIDWEBENGINE=`pidof webenginebrowser`

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
  else
    echo "load=$URL" > /dev/webenginebrowser
  fi
}

ShowUrl
