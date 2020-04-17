# -*- Makefile -*-

all: main

main: main.o
	g++ main.o -o main

main.o: main.cpp main.h
	g++ -c main.cpp

clean:
	rm -f *.o main
