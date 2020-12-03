#!/bin/bash

while true; do
	clear
	echo "Every 1 second: cat scalers_in.dat 2> /dev/null\n"
	cat scalers_in.dat 2> /dev/null
	sleep 1
done
