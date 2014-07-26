#!/bin/bash

diff(){
  awk 'BEGIN{RS=ORS=" "}
       {NR==FNR?a[$0]++:a[$0]--}
       END{for(k in a)if(a[k])print k}' <(echo -n "${!1}") <(echo -n "${!2}")
}

is_file_exists(){
	local f="$1"
	[[ -f "$f" ]] && return 0 || return 1
}

usage(){
	echo "Usage: $0 image_file"
	echo "Supported images are just in .img.xz format."
	exit 1
}

echo

[[ $# -eq 0 ]] && usage

if ( ! is_file_exists "$1")
then
	echo "Please provide an existing flash file."
	usage
	exit 1
fi

echo "We are flashing this all mighty BeagleBone Black with the image from $1!"
echo "Please do not insert any USB Sticks"\
	    "or mount external hdd during the procedure."
echo 

read -p "When the BeagleBone Black is connected in USB Boot mode press [yY]." -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
	before=($(ls /dev | grep "sd[a-z]$"))

	./usb_flasher

    rc=$?
    if [[ $rc != 0 ]];
    then
        echo "The BeagleBone Black cannot be put in USB Flasing mode. Send "\
                "logs to vvu@vdev.ro together with the Serial output from the"\
                "BeagleBone Black."
        exit $rc
    fi

    echo -n "Waiting for the BeagleBone Black to be mounted"
    for i in {1..12}
    do
        echo -n "."
        sleep 1
    done
    echo 

	after=($(ls /dev | grep "sd[a-z]$"))
	bbb=($(diff after[@] before[@]))
	
	if [ -z "$bbb" ];
	then
		echo "The BeagleBone Black cannot be detected. Either it has not been"\
                " mounted or the g_mass_storage module failed loading. "\
			    "Please send the serial log over to vvu@vdev.ro for debugging."
		exit 1
	fi
	
    if [ ${#bbb[@]} != "1" ];
	then
		echo "You inserted an USB stick or mounted an external drive. Please "\
			"rerun the script without doing that."
		exit 1
	fi

	read -p "Are you sure the BeagleBone Black is mounted at /dev/$bbb?[yY]" -n 1 -r
	echo

	if [[ $REPLY =~ ^[Yy]$ ]];
	then
		echo "Flashing now, be patient. It will take ~5 minutes!"
		echo
		xzcat $1 | dd of=/dev/$bbb bs=1M
		echo
		echo "Resizing partitons now, just as a saefty measure if you flash 2GB image on 4GB board!"
		echo -e "d\n2\nn\np\n2\n\n\nw" | fdisk /dev/$bbb > /dev/null
		echo "Please remove power from your board and plug it again."\
				"You will boot in the new OS!"
	fi
fi
