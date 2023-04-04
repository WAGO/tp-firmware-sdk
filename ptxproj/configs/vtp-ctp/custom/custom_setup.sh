#!/bin/sh

gpio_sd_cd=116

gpio_direction_input $gpio_sd_cd
gpio_get_value $gpio_sd_cd
sd_is_not_present=$?

if [ $sd_is_not_present = 0 ]; then
    # card is present, mount sd card
    mkdir /sd
    mount /dev/disk0.0 fat /sd
    if [ -e /sd/uImage ]; then
	echo -e "\t\e[00;32mPrepare Bootargs\e[00m"
	global linux.bootargs.base="rw console=ttyO0,115200 bootversion=01.01.xx_00"
	global linux.bootargs.dyn.macaddr="macaddr=00:30:DE:FF:00:C0"
	global linux.bootargs.devconf="devconf=0xf"
	global linux.bootargs.bootmode="pac_boot_id=0x82"
	global linux.bootargs.dyn.root="root=/dev/mmcblk0p2 rootfstype=ext3 rootwait"
	echo -e "\t\e[00;32mBoot Linux\e[00m."
	bootm sd/uImage
    else
        echo -e "\t\e[00;31mERROR '/sd/uImage' not found!\e[00m."
	umount /sd
	rmdir /sd
	exit 1
    fi
else
    echo -e "\t\e[00;32mERROR sd card not present\e[00m."
    exit 1
fi
