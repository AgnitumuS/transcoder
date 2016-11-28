#!/bin/sh
gcc transcoder.c -o transcoder -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -ggdb
