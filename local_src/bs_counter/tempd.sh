#!/bin/sh

#
# tempd
#
PATH=/usr/bin:/usr/sbin:/bin:/sbin

PREFIX="tempd: "
TEMPD="/usr/bin/tempd"
#LIGHTTPD_CONF="/etc/lighttpd/lighttpd.conf"

case $1 in

	start)
		echo "${PREFIX}starting"

		if start-stop-daemon --start --quiet --oknodo --exec ${TEMPD}; then
			echo "${PREFIX}done"
		else
			echo "${PREFIX}error, could not start server"
		fi
		;;

	stop)
		echo "${PREFIX}stoppping"

		if start-stop-daemon --stop --quiet --oknodo --exec ${TEMPD}; then
			echo "${PREFIX}done"
		else
			echo "${PREFIX}error, could not stop server"
		fi
		;;

	*)
		echo "${PREFIX}usage: ${0} [start|stop]"
		exit 1
		;;

esac
