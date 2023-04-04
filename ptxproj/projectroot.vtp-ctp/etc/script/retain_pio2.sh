#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2021-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   retain_pio2.sh
#
# Brief:    fix wretain_direct.conf content
#
# Author:   elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

FNAME="wretain_direct.conf"
NNAME="pio3_wretain_direct.conf"
LNAME="pio2_wretain_direct.conf"

PIO=$(/etc/config-tools/get_typelabel_value PANELTYPE)
if [[ "$PIO" == "PIO2" || "$PIO" == "PIO1" ]]; then
  LIST=$(find /etc -type l -name "$FNAME")
  if [ -z "$LIST" ]; then
    #echo "no link found"
    if [ -e "/etc/$FNAME" ]; then
      mv -f "/etc/$FNAME" "/etc/$NNAME"
    fi
    ln -s "/etc/$LNAME" "/etc/$FNAME"
    sync
  fi
fi

if [ "$PIO" == "PIO3" ]; then
  if [ -e "/etc/$NNAME" ]; then
    rm -f "/etc/$FNAME"
    mv -f "/etc/$NNAME" "/etc/$FNAME"
    sync
  fi
fi
