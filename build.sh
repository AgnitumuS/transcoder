#!/bin/sh
#gcc transcoder.c -o transcoder -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -ggdb
#gcc ts2.c -o ts -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -ggdb
gcc transcoder-ng.c -o transcoder-ng -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -O3
