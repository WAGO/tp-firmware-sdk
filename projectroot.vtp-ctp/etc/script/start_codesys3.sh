#!/bin/bash

PIDV3=`pidof codesys3`
if [ -z $PIDV3 ]; then
/etc/init.d/runtime start 3
fi
