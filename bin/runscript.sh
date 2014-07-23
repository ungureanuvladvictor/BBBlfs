#!/bin/bash

diff(){
  awk 'BEGIN{RS=ORS=" "}
       {NR==FNR?a[$0]++:a[$0]--}
       END{for(k in a)if(a[k])print k}' <(echo -n "${!1}") <(echo -n "${!2}")
}

echo "We are flashing this all mighty BeagleBone Black with the image from $1!"
before=($(ls /dev | grep "sd[a-z]$"))

sudo ./main
sleep 6

after=($(ls /dev | grep "sd[a-z]$"))

bbb=($(diff after[@] before[@]))

read -p "Are you sure the BBB is mounted at /dev/$bbb? " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
	echo "Flashing now, be patient. It will take ~5 minutes!"
	xzcat $1 | sudo dd of=/dev/$bbb bs=1M
	echo "Please remove power from your board and plug it again. You will boot in the new OS!"
fi