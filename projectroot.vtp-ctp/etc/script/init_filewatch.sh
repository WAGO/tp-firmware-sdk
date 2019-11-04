#!/bin/sh
while inotifywait -e modify /etc/specific/plcselect.conf; do
	php5 /etc/script/generate_plclist.php
  if [ -e "/dev/webenginebrowser" ]; then
    echo "reloadplcconfig" > /dev/webenginebrowser
  fi
done
