CC=gcc
CFLAGS=-Wall

PROGS =	client server

all: ${PROGS}

client: tcpselectcli.o IOHelper.o
	CC -o ./bin/client tcpselectcli.o IOHelper.o
server: tcpselectsrv.o IOHelper.o
	CC -o ./bin/server tcpselectsrv.o IOHelper.o
clean:
	rm -f server client tcpselectcli.o tcpselectsrv.o IOHelper.oz