#!/bin/bash

FILES="cbdclib.h
vmeutil.cpp            vmeutil.h
evb_buffer.cpp         evb_buffer.h
evb_caen_atdc.cpp      evb_caen_atdc.h
evb_camac_atdc.cpp     evb_camac_atdc.h
evb_mesytec_atdc.cpp   evb_mesytec_atdc.h
evb_lecroy_1151n.cpp   evb_lecroy_1151n.h
evb_tpu.cpp            evb_tpu.h
evb_tiger_sbs.cpp      evb_tiger_sbs.h
evb_tiger_net.cpp      evb_tiger_net.h
evb_readconfig.cpp     evb_readconfig.h
evb_transfer.h
Makefile
mini-evb.cpp
voinov-evb.cpp
mini-evb.h
vlib.h
bobcat-time.cpp
net-speed-send.cpp
thresholds_e.txt
thresholds_de1.txt 
thresholds_de2.txt 
thresholds_nai.txt 
timerange_nai.txt"

TRANSFER=""

UPASS=`echo Mj81v7aX | tr Xvy78js g2GmGA`

sendfiles() {
    T="$@"
    lftp -e "cd alex ; mput $T ; quit" -u magneg,$UPASS 192.168.0.35
   
}

if test "$#" = 0; then
    for f in $FILES; do
	STAMP=".putit-stamp-$f"
	if test ! -e $STAMP -o $f -nt $STAMP; then
	    TRANSFER="$TRANSFER $f"
	fi
    done
    
    if test -n "$TRANSFER"; then
	echo "Copying $TRANSFER..."
	sendfiles $TRANSFER
	for f in $TRANSFER; do
	    touch ".putit-stamp-$f"
	done
    else
	echo "Nothing to copy."
    fi
    
else
    sendfiles $@
fi
