#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_plclist.sh
#
# Brief:    starting up browser favorites
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#


. /etc/profile > /dev/null 2>&1
HOME=/root

URL="http://127.0.0.1/plclist/plclist.html"
PIDWEBENGINE=`pidof webenginebrowser`
NOSANDBOX="--no-sandbox"

function ShowUrl
{
  local PINCH_GESTURE=`getconfvalue /etc/specific/webengine/webengine.conf zoom`
  if [ -z $PIDWEBENGINE ]; then
    #browser is not started
    if [ "$PINCH_GESTURE" == "0" ]; then
      #echo "disable pinch"
      echo "1" > /tmp/custom_disable_pinch.txt
    fi
    /usr/bin/webenginebrowser $NOSANDBOX $URL > /dev/null 2>&1 &
  else
    echo "load=$URL" > /dev/webenginebrowser
  fi
}

ShowUrl
/etc/script/show_webbrowser.sh
