CC = gcc
CFLAGS = -Wall -g -std=c11

all: A1

A1: main.o
	$(CC) $(CFLAGS) -o A1 main.o

main.o: main.c a1.h
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm -f *.o 
