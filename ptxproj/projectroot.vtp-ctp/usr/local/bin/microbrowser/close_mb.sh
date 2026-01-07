#!/bin/bash

. /etc/profile

echo "Close MicroBrowser..."
/usr/bin/dialogbox "Starting WBM - please wait ..." Ok -s 3 -platform linuxfb
/etc/script/start_x_wbm.sh
exit
PIDMB=`pidof mb`
if [ ! -z $PIDMB ]; then
  echo "Stop mb ($PIDMB)"
  kill -15 $PIDMB
  sleep 1
fi
# Force close if needed
PIDMB=`pidof mb`
if [ ! -z $PIDMB ]; then
  echo "Force stopping mb!"
  kill -9 $PIDMB
fi
