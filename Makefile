CC=g++

CFLAGS=-Wall -W -g 



all: main

main: main.c
	$(CC) main.c $(CFLAGS) -o Fuji


clean:
	rm -f Fuji *.o