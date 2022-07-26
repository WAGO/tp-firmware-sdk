#!/bin/sh
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   setup_hw.sh
#
# Brief:    setup hardware drivers
#
# Author:   Ralf Gliese: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#
eeprom_device="/dev/eeprom"

devconf=$(hexdump -v -s 0x00001fe -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
if [ "$devconf" == "1010" ] ; then
	hdmi="1"
else
	hdmi="0"
fi

if [ "$hdmi" == "0" ] ; then
	### setup qt1070 leds
	echo 100 > /sys/class/leds/pca955x:bright-plus/delay_on
	echo 100 > /sys/class/leds/pca955x:bright-plus/delay_off
	echo 100 > /sys/class/leds/pca955x:bright-minus/delay_on
	echo 100 > /sys/class/leds/pca955x:bright-minus/delay_off
fi
