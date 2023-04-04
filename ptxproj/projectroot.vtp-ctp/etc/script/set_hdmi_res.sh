#!/bin/sh

hdmi_res=$(cat /boot/sd/loader/entries/VTPCTP-762-4xxx-1010.conf)
echo $hdmi_res

#remove everything before video=
hdmi_res=$(echo ${hdmi_res#*video=})

#cut after first blank
hdmi_res=$(echo $hdmi_res | cut -d ' ' -f 1)
echo "current hdmi_res=$hdmi_res"

hdmi_res_conf=$(cat /etc/specific/hdmi_resolution.conf)
#remove everything before hdmi_res=
hdmi_res_conf=$(echo ${hdmi_res_conf#*hdmi_res=})

hdmi_res_conf=$(echo $hdmi_res_conf | cut -d '"' -f 2)

if [ "$hdmi_res_conf" = "1920x1080" ]; then
	hdmi_res_new=HDMI-A-1:1920x1080M@50
elif [ "$hdmi_res_conf" = "1600x1024" ]; then
	hdmi_res_new=HDMI-A-1:1600x1024M@60
elif [ "$hdmi_res_conf" = "1280x1024" ]; then
	hdmi_res_new=HDMI-A-1:1280x1024M@60
elif [ "$hdmi_res_conf" = "1024x768" ]; then
	hdmi_res_new=HDMI-A-1:1024x768M@60
elif [ "$hdmi_res_conf" = "800x480" ]; then
	hdmi_res_new=HDMI-A-1:800x480M@60
else
	echo "error /etc/specific/hdmi_resolution.conf configured resolution not ok"
	exit 1
fi

#hdmi_res_new=HDMI-A-1:1280x1024M@60
#echo "hdmi_res_new=$hdmi_res_new"

sed -i "s|$hdmi_res|$hdmi_res_new|g" /boot/sd/loader/entries/VTPCTP-762-4xxx-1010.conf
sed -i "s|$hdmi_res|$hdmi_res_new|g" /boot/emmc0/loader/entries/VTPCTP-762-4xxx-1010.conf
sed -i "s|$hdmi_res|$hdmi_res_new|g" /boot/emmc1/loader/entries/VTPCTP-762-4xxx-1010.conf


echo "############################################"
cat /boot/sd/loader/entries/VTPCTP-762-4xxx-1010.conf
cat /boot/emmc0/loader/entries/VTPCTP-762-4xxx-1010.conf
cat /boot/emmc1/loader/entries/VTPCTP-762-4xxx-1010.conf

sync

exit 0

