#!/bin/bash

#CP / VP / WP demomode
#BOOTAPP="$(/etc/config-tools/get_eruntime bootapp)"
#if [ "$BOOTAPP" == "yes" ]; then
  PIDV3=`pidof codesys3`
  if [ -z $PIDV3 ]; then
  /etc/init.d/runtime start 3
  fi
  echo close > /dev/webenginebrowser
#fi
