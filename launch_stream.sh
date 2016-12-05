#!/bin/bash


#vlc-wrapper --loop high_quality.mp4 --sout '#rtp{dst=127.0.0.1,port=5004,mux=ts}'


#cvlc -vvv v4l2:///dev/video0 --sout '#transcode{vcodec=mp2v,vb=50,acodec=none}:rtp{dst=192.168.2.16,port=50042,mux=ts}'


#cvlc -vvv v4l2:///dev/video0 --sout '#transcode{vcodec=WMV2,vb=800,scale=Auto,acodec=wma2,ab=128,channels=2,samplerate=44100}:rtp{dst=192.168.1.122,port=50042,mux=ts,sap,name=fd}'
#cvlc -vvv v4l2:///dev/video0 --sout '#transcode{vcodec=WMV2,vb=50,acodec=none}:rtp{dst=192.168.1.122,port=50042,mux=ts}'

#:sout-keep'
##transcode{vcodec=h264,scale=1,width=1280,height=720,fps=25,venc=x264{preset=ultrafast,tune=zerolatency,vbv-bufsize=1000,intra-refresh,lookahead=10,keyint=25},acodec=none}:rtp{dst=%s,port=50041,mux=ts}

cvlc --loop high_quality.mp4  --avcodec-hw=vaapi --sout '#transcode{vcodec=h264,scale=1,width=1280,height=720,fps=25,venc=x264{preset=ultrafast,tune=zerolatency},acodec=none}:rtp{dst=192.168.2.50,port=50041,mux=ts}'