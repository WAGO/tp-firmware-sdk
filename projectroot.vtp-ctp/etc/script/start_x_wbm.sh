#!/bin/sh
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_x_wbm.sh
#
# Brief:    starting up Xorg and WBM in case of microbrowser closed
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#

# blank framebuffer to black background
dd if=/dev/zero of=/dev/fb0

# start X
/usr/bin/startx &
