#!/bin/bash
MBLOG=/var/log/mb.log
# Create links to OpenSSL v.1.0.2 if needed
[ -s /usr/lib/libssl.so.1.0.2 ] || ln -s /usr/lib/libssl.so /usr/lib/libssl.so.1.0.2
[ -s /usr/lib/libcrypto.so.1.0.2 ] || ln -s /usr/lib/libcrypto.so /usr/lib/libcrypto.so.1.0.2

PIDMB=`pidof mb`
if [ ! -z $PIDMB ]; then
 exit 0 
fi

# Clear log and add start info
echo `date` `tty` $0 $* >$MBLOG

# start mb
cd /home/admin/MicroBrowser
/home/admin/MicroBrowser/mb $@ >> $MBLOG 2>&1 &

