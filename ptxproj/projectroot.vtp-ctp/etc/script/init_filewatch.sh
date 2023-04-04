#!/bin/sh
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   init_filewatch.sh
#
# Brief:    generate browser favorite list in case of config changed
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

while inotifywait -e modify /etc/specific/plcselect.conf; do
	php /etc/script/generate_plclist.php
  if [ -e "/dev/webenginebrowser" ]; then
    echo "reloadplcconfig" > /dev/webenginebrowser
  fi
done
