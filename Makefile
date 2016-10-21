CC=gcc
CFLAGS= -ansi -Wall -pedantic

CLIENTCFILES=client.c message.c
SERVERCFILES=server.c message.c queue.c

all: clean client server

client:
	$(CC) $(CFLAGS) $(CLIENTCFILES) -o client -lpthread

server:
	$(CC) $(CFLAGS) $(SERVERCFILES) -o server

clean:
	rm -f *~ client.o client server.o server

realclean:
	rm -f *~ client server *.o
