CC = g++
flags = -Wall

all: client server

client: client.o
	$(CC) $(flags) -o client client.o

client.o: client.cpp util.h
	$(CC) $(flags) -c client.cpp

server: server.o
	$(CC) $(flags) -o server server.o

server.o: server.cpp util.h
	$(CC) $(flags) -c server.cpp

clean:
	rm -f *.o
	rm -f client
	rm -f server

runc: client
	./client

runs: server
	./server

run: all
	./server & ./client
	