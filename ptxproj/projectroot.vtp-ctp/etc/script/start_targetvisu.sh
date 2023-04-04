#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_targetvisu.sh
#
# Brief:    starting up codesys targetvisu
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

PIDV3=$(pidof codesys3)
if [ -z "$PIDV3" ]; then
  /etc/init.d/runtime start 3
  sleep 1
  PIDV3=$(pidof codesys3)
fi

if [ ! -z "$PIDV3" ]; then
  wid=$(xdotool search --pid "$PIDV3")
  if [ ! -z "$wid" ]; then
    xdotool windowactivate $wid
  fi
fi

