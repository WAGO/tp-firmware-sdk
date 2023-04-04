#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_calibration.sh
#
# Brief:    starting touchscreen tslib calibration program
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

eeprom_device="/sys/bus/i2c/devices/1-0054/eeprom"
. /etc/profile > /dev/null 2>&1

devconf=$(hexdump -v -s 0x00001fe -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
if [ "$devconf" = "1010" ] ; then
	if [ -e "/dev/input/touchscreen" ]; then
		/usr/bin/ts_calibrate
	fi
else
	/usr/bin/ts_calibrate
fi
