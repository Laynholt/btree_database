
client.o:
	+$(MAKE) -C client

shrm.o:
	+$(MAKE) -C shrm

server.o:
	+$(MAKE) -C server

hash.o:
	+$(MAKE) -C hash

tree.o:
	+$(MAKE) -C tree

cli.out: client.o shrm.o tree.o hash.o
	cc client/client.o shrm/shrm.o tree/tree.o hash/hash.o -o cli.out

ser.out: server.o shrm.o tree.o hash.o
	cc -lpthread server/server.o shrm/shrm.o tree/tree.o hash/hash.o -o ser.out
