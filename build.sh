#!/bin/sh
#gcc transcoder.c -o transcoder -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -ggdb
#gcc ts2.c -o ts -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -ggdb
#gcc transcoder-ng.c -o transcoder-ng -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -ggdb
gcc src/vlc_transcode.c -o bin/vlc_tc -lvlc -pthread  -ggdb #-O3
