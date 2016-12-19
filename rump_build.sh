#!/bin/sh

x86_64-rumprun-netbsd-gcc src/rump_transcode.c -o bin/tc_rump  -pthread  -L /usr/lib/ -lvlc
