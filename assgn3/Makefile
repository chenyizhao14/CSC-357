CC=gcc

CFLAGS=-Wall -ansi -Werror -pedantic -g

LDFLAGS=-lm

all: htable hencode hdecode

htable: htable.c huffman.h huffman.c
	$(CC) $(CFLAGS) -o htable htable.c huffman.c $(LDFLAGS)

hencode: hencode.c huffman.h huffman.c
	$(CC) $(CFLAGS) -o hencode hencode.c huffman.c $(LDFLAGS)

hdecode: hdecode.c huffman.h huffman.c
	$(CC) $(CFLAGS) -o hdecode hdecode.c huffman.c $(LDFLAGS)