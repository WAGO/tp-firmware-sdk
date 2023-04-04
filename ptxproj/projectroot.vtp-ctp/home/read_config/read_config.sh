#!/bin/sh

REGISTER_DATA=register_data.txt
OUT_DATA=config_data.txt

if [ -e "$REGISTER_DATA" ]; then
	echo "found $REGISTER_DATA"
	rm $OUT_DATA
	echo "################################### clock summary #######################################"
	echo "################################### clock summary #######################################"  >> $OUT_DATA
	mount -t debugfs debugfs /sys/kernel/debug
	cat /sys/kernel/debug/clk/clk_summary
	cat /sys/kernel/debug/clk/clk_summary  >> $OUT_DATA
	echo "################################### register report #####################################"
	echo "################################### register report #####################################" >> $OUT_DATA
	i=0
	while read line; do
		if [ "${line:0:1}" != "#" ]; then
			#echo "$line"
			REGISTER_ADDRESS="${line%%:*}"
			IOMUXC_SW_PAD_CTL="${line%:*}"
			IOMUXC_SW_PAD_CTL="${IOMUXC_SW_PAD_CTL#*:}"
			#echo "$IOMUXC_SW_PAD_CTL"
			#echo "$REGISTER_ADDRESS"
			PIN_USAGE="${line##*:}"
			#echo "$PIN_USAGE"

			#devmem $REGISTER_ADDRESS 32
			REGISTER_CONTENT="$(devmem $REGISTER_ADDRESS 32)"
			RETVAL=$?
			if [ $RETVAL != 0 ]; then
				echo "ERROR" >> $OUT_DATA
				exit 1
			else
				printf "Nr:%.4d # IOMUXC_SW_PAD_CTL=%40s # USAGE=%20s #  VALUE=0x%.8x\n" $i $IOMUXC_SW_PAD_CTL $PIN_USAGE $REGISTER_CONTENT
				printf "Nr:%.4d # IOMUXC_SW_PAD_CTL=%40s # USAGE=%20s #  VALUE=0x%.8x\n" $i $IOMUXC_SW_PAD_CTL $PIN_USAGE $REGISTER_CONTENT >> $OUT_DATA
			fi
			#echo "$i IOMUXC_SW_PAD_CTL=$IOMUXC_SW_PAD_CTL" 
		#	printf "Nr:%.4d # IOMUXC_SW_PAD_CTL=%40s # USAGE=%20s #  VALUE=0x%.8x\n" $i $IOMUXC_SW_PAD_CTL $PIN_USAGE $REGISTER_CONTENT
			i=$(( $i + 1 ))
		fi
	done < "$REGISTER_DATA"
else
	echo "file $REGISTER_DATA not found"
	exit 1
fi
