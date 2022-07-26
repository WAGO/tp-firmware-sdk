#!/bin/sh 
rm -f /lib/udev/rules.d/65-weston-res-touchscreen.rules

echo 'SUBSYSTEM=="input", KERNEL=="event[0-9]*",
ENV{ID_INPUT_TOUCHSCREEN}=="0",
ENV{LIBINPUT_CALIBRATION_MATRIX}="'$2 $3 $4 $5 $6 $7'"' >> /lib/udev/rules.d/65-weston-res-touchscreen.rules
