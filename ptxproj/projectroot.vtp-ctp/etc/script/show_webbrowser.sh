#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   show_browser.sh
#
# Brief:    Bring browser to front
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

LOCKFILE="/tmp/_show_webbrowser_.lock"
if [ -e "$LOCKFILE" ]; then
exit 0
else
touch "$LOCKFILE"
fi

. /etc/profile > /dev/null 2>&1
HOME=/root
PRG="webenginebrowser"
PIDPRG=$(pidof $PRG)

function ActivateWindow
{
  local COUNTER=0
  local WND_ID=""
  
  #WAIT FOR PID
  PIDPRG=$(pidof $PRG)
  while [ -z "$PIDPRG" ]; do
    #not running
    ((COUNTER++))

    #sleep 100 ms
    usleep 100000

    if [ $COUNTER -gt 40 ]; then
      echo "Timeout waiting for pid $PRG"
      break;
    fi
    PIDPRG=$(pidof $PRG)
  done

  #WAIT FOR WND
  COUNTER=0
  PIDPRG=$(pidof $PRG)
  if [ ! -z "$PIDPRG" ]; then
  WND_ID=$(xdotool search --pid "$PIDPRG")
  while [ -z "$WND_ID" ]; do
    #echo "WND: $WND_ID"
    ((COUNTER++))

    sleep 1

    if [ $COUNTER -gt 5 ]; then
      echo "Timeout waiting for window $PRG"
      break;
    fi
    WND_ID=$(xdotool search --pid "$PIDPRG")
  done
  fi
  
  if [ ! -z "$WND_ID" ]; then
     #echo "WND: $WND_ID"
     xdotool windowactivate "$WND_ID"
  fi
}

ActivateWindow
rm -f "$LOCKFILE"
