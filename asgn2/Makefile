CC = gcc

CFLAGS = -Wall -ansi -Werror -pedantic -g

LDFLAGS=-lm

all: fw

fw: fw.c fw.o
	$(CC) $(CFLAGS) -o fw fw.o hash.o $(LDFLAGS)

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

fw.o: fw.c hash.h
	$(CC) $(CFLAGS) -c fw.c

clean: 
	rm fw test1expected test1.out

test: fw test1.txt test1expected
	diff test1.out test1expected

test1: fw
	./fw < test1.txt > test1.out

test1expected: fw
	/bin/fw < test1.txt > test1expected