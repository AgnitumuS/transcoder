#!/bin/bash


cvlc -vvv video.mp4 --sout '#rtp{dst=127.0.0.1,port=50040,mux=ts}'


#cvlc -vvv v4l2:///dev/video0 --sout '#transcode{vcodec=mp2v,vb=50,acodec=none}:rtp{dst=192.168.2.16,port=50042,mux=ts}'


#cvlc -vvv v4l2:///dev/video0 --sout '#transcode{vcodec=WMV2,vb=800,scale=Auto,acodec=wma2,ab=128,channels=2,samplerate=44100}:rtp{dst=192.168.1.122,port=50042,mux=ts,sap,name=fd}'
#cvlc -vvv v4l2:///dev/video0 --sout '#transcode{vcodec=WMV2,vb=50,acodec=none}:rtp{dst=192.168.1.122,port=50042,mux=ts}'

#:sout-keep'
