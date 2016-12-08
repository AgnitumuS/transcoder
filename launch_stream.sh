#!/bin/bash

file_name=$1
destination=$2
quality=$3
port=$4

case "$3" in
    "0")
    sout='#rtp{dst='$destination',port='$port',mux=ts}'
    ;;
    "1")
    sout='#transcode{vcodec=h264,scale=0.5,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15}}:rtp{dst='$destination',port='$port',mux=ts,caching=50000}'
    ;;
    "2")
    sout='#transcode{vcodec=h264,scale=0.25,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15}}:rtp{dst='$destination',port='$port',mux=ts,caching=50000}'
    ;;
    *)
    echo "Quality should be 0 or 1 or 2"
    exit 1
    ;;
esac

echo $sout
cvlc  --loop $1  --sout $sout 