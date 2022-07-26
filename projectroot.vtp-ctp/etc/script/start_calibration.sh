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

. /etc/profile > /dev/null 2>&1

ORDER="$(/etc/config-tools/get_typelabel_value ORDER)"
#do not calibrate EDGE controller
if [ "${ORDER:0:3}" == "762" ]; then
/usr/bin/ts_calibrate
fi
