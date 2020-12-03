#!/bin/bash

while true; do
	clear
	echo "Every 1 second: cat scalers_out.dat 2> /dev/null\n"
	cat scalers_out.dat 2> /dev/null
	sleep 1
done
