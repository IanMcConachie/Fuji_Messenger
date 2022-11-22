CC=g++

CFLAGS=-Wall -W -g 



all: main

main: main.cpp
	$(CC) main.cpp $(CFLAGS) -o Fuji


clean:
	rm -f Fuji *.o