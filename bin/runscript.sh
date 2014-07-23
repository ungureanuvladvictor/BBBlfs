#!/bin/bash

diff(){
  awk 'BEGIN{RS=ORS=" "}
       {NR==FNR?a[$0]++:a[$0]--}
       END{for(k in a)if(a[k])print k}' <(echo -n "${!1}") <(echo -n "${!2}")
}

if [ -z "$1" ];
then
	echo "Please provide an image to flash!"
	exit
fi

echo "We are flashing this all mighty BeagleBone Black with the image from $1!"
echo "Please do not insert any USB Sticks"\
		"or mount external hdd during the procedure."

read -p "When the BeagleBone Black is connected in USB Boot mode press 'y'." -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
	before=($(ls /dev | grep "sd[a-z]$"))

	sudo ./usb_flasher
	sleep 6

	after=($(ls /dev | grep "sd[a-z]$"))
	bbb=($(diff after[@] before[@]))
	
	if [ ${#bbb[@]} != "1" ];
	then
		echo "You inserted an USB stick or mounted an external drive. Please "\
			"rerun the script without doing that."
		exit
	fi

	if [ -z "$bbb" ];
	then
		echo "The BeagleBone Black failed to open g_mass_storage."\
			"Please send the serial log over to vvu@vdev.ro for debugging."
		exit
	fi

	read -p "Are you sure the BeagleBone Black is mounted at /dev/$bbb? " -n 1 -r
	echo

	if [[ $REPLY =~ ^[Yy]$ ]]
	then
		echo "Flashing now, be patient. It will take ~5 minutes!"
		echo
		xzcat $1 | sudo dd of=/dev/$bbb bs=1M
		echo
		echo "Please remove power from your board and plug it again."\
				"You will boot in the new OS!"
	fi
fi