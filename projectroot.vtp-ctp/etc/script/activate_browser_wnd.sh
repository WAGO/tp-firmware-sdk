#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   activate_browser_wnd.sh
#
# Brief:    bring webenginebrowser to front
#
# Author:   elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

# run only once
# use rc.once.d

# First startup of eruntime 3.5.16.3 creates an empty black screen which overlays the browser window only at first start

. /etc/profile > /dev/null 2>&1
HOME=/root

function ActivateWindow
{
  PID_BROWSER=$(pidof webenginebrowser)
    if [ ! -z "$PID_BROWSER" ]; then
      WND_ID=$(xdotool search --pid "$PID_BROWSER")
      if [ ! -z "$WND_ID" ]; then
         xdotool windowactivate "$WND_ID"
      fi
  fi
}

sleep 10

#ensure browser focus
for i in {1..10}; do 
sleep 8
ActivateWindow
done

exit 0
