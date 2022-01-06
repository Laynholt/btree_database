all: cli.out ser.out

cli-ser.o:
	+$(MAKE) -C cli-ser

client.o:
	+$(MAKE) -C cli-ser/client

server.o:
	+$(MAKE) -C cli-ser/server

shrm.o:
	+$(MAKE) -C shrm

hash.o:
	+$(MAKE) -C hash

tree.o:
	+$(MAKE) -C tree

cli.out: cli-ser.o client.o shrm.o tree.o hash.o
	cc cli-ser/cli-ser-lib.o cli-ser/client/client.o shrm/shrm.o tree/tree.o hash/hash.o -o cli.out

ser.out: cli-ser.o server.o shrm.o tree.o hash.o
	cc -lpthread cli-ser/cli-ser-lib.o cli-ser/server/server.o shrm/shrm.o tree/tree.o hash/hash.o -o ser.out
