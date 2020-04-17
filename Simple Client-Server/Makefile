# -*- Makefile -*-

all: server client

server: server.o
	g++ server.o -o server

client: client.o
	g++ client.o -o client

server.o: server.cpp
	g++ -c server.cpp

client.o: client.cpp
	g++ -c client.cpp

clean:
	rm -f *.o main
