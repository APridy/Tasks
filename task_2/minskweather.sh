#!/bin/bash

set -e

N=$(grep "N=" config.ini | cut -d'=' -f2)

while :
do
	C=$(wget -q -O - https://wttr.in/Minsk?format=j1 | grep temp_C | cut -d':' -f2 | sed 's/[", ]*//g')
	echo Minsk: $C °C
	sleep $N 2>/dev/null || ( echo "Error: invalid N"; exit )
done
