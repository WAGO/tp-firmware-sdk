#!/bin/bash
#-----------------------------------------------------------------------------#
# Script-name: brightnesscontrol
#
# starting brightnesscontrol at boottime
#
# Author: Wolfgang Rückl,  elrest GmbH
#-----------------------------------------------------------------------------#

PID=""

function get_pid {
   PID=`pidof brightness_control`
}

function stop {
   get_pid
   if [ -z $PID ]; then
      echo "brightness_control is not running."
   else
      echo -n "stopping brightness_control .."
      killall brightness_control
      echo "done"
   fi
}


function start {
   get_pid
   if [ -z $PID ]; then
      echo  "starting brightness_control"
      /usr/bin/brightness_control > /dev/null &
      echo "done"
   else
      echo "brightness_control is already running, PID=$PID"
   fi
}

function restart {
   echo  "restarting brightness_control .."
   get_pid
   if [ -z $PID ]; then
      start
   else
      stop
      sleep 1
      start
   fi
}


function status {
   get_pid
   if [ -z $PID ]; then
      echo "brightness_control is not running."
   else
      echo "brightness_control is running, PID=$PID"
   fi
}

case "$1" in
   start)
      start
   ;;
   stop)
      stop
   ;;
   restart)
      restart
   ;;
   status)
      status
   ;;
   *)
      echo "usage: $0 {start|stop|restart|status}"
esac
