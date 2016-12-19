CC=gcc
CFLAGS=-O3 -lvlc -pthread
BIN=bin/tc

x86 :  src/vlc_transcode.c  
	${CC} src/vlc_transcode.c  ${CFLAGS} -o ${BIN} 
clean :
	rm -rf bin/*
	rm -rf src/*.o
