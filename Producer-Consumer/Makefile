# -*- Makefile -*-

all: main

main: prodcon.o tands.o
	g++ -O prodcon.o tands.o -o prodcon -pthread -lrt

main.o: prodcon.cpp tands.h main.h
	g++ -c prodcon.cpp tands.h main.h

tands.o: tands.c
	g++ -c tands.c

clean:
	rm -f *.o main
	rm -f *.log
