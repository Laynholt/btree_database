all: client.o server.o cli-ser.o

client.o:
	+$(MAKE) -C client

server.o:
	+$(MAKE) -C server

cli-ser.o: cli-ser-lib.c cli-ser-lib.h
	cc -c cli-ser-lib.c
